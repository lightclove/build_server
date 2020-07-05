/** @file
 * @brief Build/compilation client-server application
 *
 * Build/compilation server implemetation
 *
 * Copyright (C) 2020 OKTET Labs, St.-Petersburg, Russia
 *
 * @author Dmitry Iliyushko <Dmitry.Iliyushko@oktetlabs.ru>
 *
 * $Id: $
 */
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#define BUFFERT 512
#define BACKLOG 1   /* Size of the client's queue */
#define MAX_FILE_NAME_SIZE 256

/* Functions declarations */
static int
create_server_socket(int port, in_addr_t ip );
static int
build_file(char *filename, char *dirname, char *ccoutput);
static int
template_file_gen(char *filename, char *result_filename, char *result_dirname);

/**
* @brief Function allowing the creation of a socket
 * and its attachment to the system
* Returns a file descriptor in the process descriptor table
* bind allows its definition with the system
* @param port port of the socket
*
* @return soket's file descriptor value.
*/
static int
create_server_socket(int port, in_addr_t ip )
{
    int l;
    int sfd;
    int yes = 1;

    sfd = socket(PF_INET, SOCK_STREAM, 0);

    if (sfd == -1)
    {
        perror("Socket failure");
        close(sfd);
        return EXIT_SUCCESS;
    }
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("Setsockopt error");
        close(sfd);
        exit(5);
    }
    /* Preparing the address of the destination socket */
    l = sizeof(struct sockaddr_in);
    struct sockaddr_in sock_serv, sock_clt;
    memset(&sock_serv, 0, l);
    sock_serv.sin_family      = AF_INET;
    sock_serv.sin_port        = htons(port);
    sock_serv.sin_addr.s_addr = ip;

    /* Assign an identity to the socket */
    if (bind(sfd, (struct sockaddr *) &sock_serv, l) == -1)
    {
        perror("Fail the binding");
        close(sfd);
        return EXIT_FAILURE;
    }
    struct in_addr addr;
    memset(&addr, 0,  sizeof (struct in_addr));
    addr.s_addr = ip;
    printf("Server started and listening on %s:%d\n", inet_ntoa (addr), port);
    return sfd;
}

/**
 * @brief This function generates a tempory file
 * @param filename The name of file
 * @param result_filename The output file name
 * @param result_dirname The output tempory directory name
 * @return 0 - generation succeed, 1 - otherwise
 */
static int
template_file_gen(char *filename, char *result_filename, char *result_dirname)
{
    char template_dirname[] = "/tmp/tmpdir.XXXXXXXX";
    char *dirname = mkdtemp(template_dirname);
    if (!dirname)
    {
        perror("Unable to create temp directory\n");
        return 1;
    }
    sprintf(result_filename,"%s/%s",dirname,filename);
    strcpy(result_dirname,dirname);
    return 0;
}

/**
 * @brief This function build file using default C compiler
 * @param filename The name of file
 * @param dirname Directory path
 * @param ccoutput The output log file of C compiler
 * @return 0 - building successed, 1 - otherwise
 */
static int
build_file(char *filename, char *dirname, char *ccoutput)
{
    char cmd[256];
    sprintf(cmd,"gcc %s 2> %s/output.log",filename,dirname);
    int ret = 0;

    if (ret = system(cmd))
    {
        sprintf(ccoutput,"%s/output.log",dirname);
        printf("Compilation: FAILED, log: %s\n",ccoutput);
        return 1;
    }
    printf("Compilation: OK (%d)\n",ret);
    return 0;
}

int
main(int argc, char **argv)
{
    /* Socket variables and file descriptors */
    int sfd;
    int fd;
    struct sockaddr_in sock_clt;
    /* Connection variables */
    ushort clt_port;
    in_addr_t ip_p; //= 0x7f000001; /* Default ip address is 127.0.0.1 */
    int port_p = 1234; /* Default port is 1234*/
    char dst[INET_ADDRSTRLEN];
    char buffer[BUFFERT];//char buffer[100] = { 0, };
    unsigned int length = sizeof(struct sockaddr_in);
    char ch; /* char for the parameter to read from argv */
    /* Variables for the saving the file in FS when get it from client */
    char filename[MAX_FILE_NAME_SIZE];
    char result_filename[MAX_FILE_NAME_SIZE];
    char result_dirname[MAX_FILE_NAME_SIZE];
    char result_outname[MAX_FILE_NAME_SIZE];
    int i = 0;
    long n, m, count = 0;
    unsigned int nsid;

    /* Parsing the command line parameters and arguments */
    while ((ch = getopt(argc, argv, "d:p:")) != EOF)
        switch (ch)
        {
            case 'd':
                ip_p = inet_addr(optarg);
                break;
            case 'p':
                port_p = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Unknown option: '%s'\n", optarg);
                return 1;
        }
    /* Creating the socket file descriptor */
    sfd = create_server_socket( port_p, ip_p);
    /* Listen and accepting the connections from the clients */
    while(1)
    {
        listen(sfd, BACKLOG);
        /* Function that waits for the connected client */
        nsid = accept(sfd, (struct sockaddr *) &sock_clt, &length);
        if (nsid == -1)
        {
            perror("Connection acceptance failure");
            close(sfd);
            return EXIT_FAILURE;
        }
        else
        {
            if (inet_ntop(AF_INET, &sock_clt.sin_addr, dst, INET_ADDRSTRLEN) == NULL)
            {
                perror("Socket error");
                close(sfd);
                exit(4);
            }
            /* Accepting the connections from the clients */
            clt_port = ntohs(sock_clt.sin_port);
            printf("Connection accepted from the : %s : %d\n", dst, clt_port);
            /* Prepare to getting the file */
            memset(filename,0,sizeof(filename));

            for (i=0;i<MAX_FILE_NAME_SIZE;i++)
            {
                recv(nsid, filename+i, 1, 0);
                if (*(filename+i) == '\n')
                {
                    *(filename+i) = 0;
                    break;
                }
            }
            if (strncmp(filename,"File",strlen("File")))
            {
                perror("Filename error");
                exit(5);
            }

            char *pfilename = filename + sizeof("File");
            if (template_file_gen(pfilename, result_filename, result_dirname))
            {
                exit(6);
            }
            if ((fd = open(result_filename, O_CREAT | O_WRONLY, 0600)) == -1)
            {
                perror("Open fail");
                close(fd);
                exit(3);
            }
            printf("%s\n",result_filename);
            char ssize[256];
            unsigned long size = 0;

            for (i=0;i<MAX_FILE_NAME_SIZE;i++)
            {
                recv(nsid, ssize+i, 1, 0);
                if (*(ssize+i) == '\n')
                {
                    *(ssize+i) = 0;
                    break;
                }
            }
            sscanf(ssize,"%ld",&size);
            count = 0;
            //bzero(buffer, BUFFERT);
            n = recv(nsid, buffer, BUFFERT, 0);

            while (n && count < size)
            {
                if (n == -1)
                {
                    perror("Recieving failure");
                    close(sfd);
                    exit(5);
                }
                if ((m = write(fd, buffer, n)) == -1)
                {
                    perror("Files writing failure");
                    exit(6);
                }
                count = count + m;
                if (count >= size)
                    break;
                //bzero(buffer, BUFFERT);
                n = recv(nsid, buffer, BUFFERT, 0);
            }
            close(fd);
            /* Building and compiling the files */
            if (build_file(result_filename,result_dirname,result_outname))
            {
                int l = sprintf(buffer,"Result: Fail\n");
                if (fd = open(result_outname, O_RDONLY))
                {
                    n = read(fd, buffer, BUFFERT);
                    while (n)
                    {
                        if (n == -1)
                        {
                            perror("Reading fails");
                            close(fd);
                            return EXIT_FAILURE;
                        }
                        m = send(nsid, buffer, n,0);
                        if (m == -1)
                        {
                            perror("File sending error");
                            close(fd);
                            return EXIT_FAILURE;
                        }
                        count += m;
                        //bzero(buffer, BUFFERT);
                        n = read(fd, buffer, BUFFERT);
                    }
                    m = send(nsid, buffer, n,0);
                }
            }
            else
            {
                int l = sprintf(buffer,"Result: OK\n");
                m = send(nsid, buffer,l,0);
            }
            rmdir(result_dirname);
        }
        close(nsid);
        printf("End of the transmission with  %s.%d\n", dst, clt_port);
        printf("Number of bytes received : %ld \n", count);
        printf("_________________________________________________________\n");
    }
    close(sfd);
    return EXIT_SUCCESS;
}
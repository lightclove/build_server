/** @file
 * @brief Build/compilation client-server application
 *
 * Build/compilation client implemetation
 *
 * Copyright (C) 2020 OKTET Labs, St.-Petersburg, Russia
 *
 * @author Dmitry Iliyushko <Dmitry.Iliyushko@oktetlabs.ru>
 *
 * $Id: $
 */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
/* Size of the buffer used to send the file in several blocks */
#define BUFFERT 512
#define LENGTH 512
/* Functions definition*/
static int create_client_socket(int port, in_addr_t ip );
static int send_file(char* fs_name );

static struct sockaddr_in sock_serv;

/* file information used to send as an array to the one TCP-socket */
typedef struct fmetadata {
    FILE *fp;
    char *fname;
    char *fcontent;
} fmetadata;
/**
* Function allowing the creation of a socket returns a file descriptor
*
* @param port port of the socket
* @param ipaddr  ip address of the socket
* @return soket's file descriptor value.
*
*
*/
static int
create_client_socket(int port, in_addr_t ip)
{
    int l;
    int sfd;

    sfd = socket(PF_INET, SOCK_STREAM, 0);

    if (sfd == -1)
    {
        perror("Socket failure");
        return EXIT_FAILURE;
    }
    // preparing the address of the destination socket
    l = sizeof(struct sockaddr_in);
    memset(&sock_serv, 0, l);
    sock_serv.sin_family = AF_INET;
    sock_serv.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &sock_serv.sin_addr) == 0)
    {
        printf("Invalid IP address\n");
        return EXIT_FAILURE;
    }
    return sfd;
}
/**
* Function allowing the sending file to the socket returns succ stasus
*
* @param port port of the socket
* @param ipaddr  ip address of the socket
* @return soket's file descriptor value.
*
*
*/
static int
sendfile(char* fs_name)
{
    int sockfd;
    //char* fs_name = "client.c";
    char sdbuf[LENGTH];
    printf("[Client] Sending %s to the Server... ", fs_name);
    FILE *fs = fopen(fs_name, "r");

    if(fs == NULL)
    {
        printf("ERROR: File %s not found.\n", fs_name);
        close(sockfd);
        exit(1);

    }

    memset(sdbuf, 0,  LENGTH);
    int fs_block_sz;
    while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs)) > 0)
    {
        if(send(sockfd, sdbuf, fs_block_sz, 0) < 0)
        {
            fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
            close(sockfd);
            break;
        }
        memset(sdbuf, 0,  LENGTH);
    }
    printf("Ok file %s from the client sent!\n", fs_name);
    close(sockfd);
    return (0);
}

int
main(int argc, char **argv)
{
    /* Connection variables */
    int sfd, fd;
    char buf[BUFFERT];
    int l = sizeof(struct sockaddr_in);
    off_t count = 0, m, sz;
    long int n;
    struct stat buffer;
    char ch; /* char for the parameter to read from argv */
    int i;

    fmetadata file_array[] = {
            {"/foo/bar/a.txt", NULL, NULL},
            {"/foo/bar/b.txt", NULL, NULL},
            {"/foo/bar/c.txt", NULL, NULL}
    };
    in_addr_t ip_p; /* Default ip address is 127.0.0.1 = 0x7f000001 */
    int port_p = 1234; /* Default port is 1234*/

    while ((ch = getopt(argc, argv, "d:p:m:u:")) != -1)
    {
        switch (ch) {
            case 'd':
                ip_p = inet_addr(optarg);
                break;
            case 'p':
                port_p = atoi(optarg);
                break;
            case 'm':
                // @ToDo make some noise...
                break;
            case 'u':
                // @ToDo breaking the law...
                break;
            case '?':
                if (optopt == 'd')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr,"Unknown option character `\\x%x'.\n", optopt);
            default:
                abort();
        }
    }

    int index;
    for (index = optind; index < argc; index++)
    {
        printf ("Files to send to the server: %s\n", argv[index]);
        send_file(argv[index]);
        // @ToDo
    }
    sfd = create_client_socket(port_p, ip_p);

    if ((fd = open(argv[3], O_RDONLY)) == -1)
    {
        perror("Socket opening failure");
        return EXIT_FAILURE;
    }
    /* file size */
    if (stat(argv[3], &buffer) == -1)
    {
        perror("Stat fail");
        return EXIT_FAILURE;
    }
    else
        sz = buffer.st_size;
    // preparation of the transmission
    memset(&buf, 0, BUFFERT);

    if (connect(sfd, (struct sockaddr *) &sock_serv, l) == -1)
    {
        perror("Connection error\n");
        exit(3);
    }

    n = sprintf(buf,"File %s\n",argv[3]); // @ToDo What should i do?
    m = sendto(sfd, buf, n, 0, (struct sockaddr *) &sock_serv, l);
    // if(send(sockfd, sdbuf, fs_block_sz, 0) < 0) // @ToDo So what ?
    if (m == -1)
    {
        perror("File sending error");
        return EXIT_FAILURE;
    }

    n = sprintf(buf,"%ld\n",sz);
    m = sendto(sfd, buf, n, 0, (struct sockaddr *) &sock_serv, l);

    if (m == -1)
    {
        perror("File sending error");
        close(sfd);
        return EXIT_FAILURE;
    }

    n = read(fd, buf, BUFFERT);

    while (n)
    {
        if (n == -1)
        {
            perror("Reading fails");
            close(sfd);
            return EXIT_FAILURE;
        }

        m = sendto(sfd, buf, n, 0, (struct sockaddr *) &sock_serv, l);

        if (m == -1)
        {
            perror("File sending error");
            close(sfd);
            return EXIT_FAILURE;
        }
        count += m;
        memset(buf, 0, BUFFERT);
        n = read(fd, buf, BUFFERT);
    }
//    /* Sending data to the server */
//    m = sendto(sfd, buf, 0, 0, (struct sockaddr *) &sock_serv, l);

    while (n = recv(sfd, buf, BUFFERT, 0))
    {
        printf("%s",buf);
    }

    printf("Bytes transferred:% ld \n", count);
    printf("Total size:% ld \n", sz);

    close(sfd);
    return EXIT_SUCCESS;
}
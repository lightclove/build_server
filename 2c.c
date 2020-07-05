#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#define PORT 8080
#define BUFSIZE 20

int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char status[15]={0};
    char buffer[BUFSIZE]={0};
    int read_size=0,i=1,sent_size=0;

    FILE *f1,*f2;
    f1=fopen("M1.txtt","r");
    f2=fopen("M2.txt","r");

    if(f1==NULL)
    {
        printf("Unable open file\n");
        exit(0);
    }
    if(f2==NULL)
    {
        printf("Unable open file\n");
        exit(0);
    }
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    while((read_size=fread(buffer,1,BUFSIZE,f1))>0)
    {
        sent_size=send(sock,buffer,read_size , 0 );
        fprintf(stderr,"%d th sent_size  %d\n",i,sent_size); //Just printing how many bytes have been sent in every iteration.
        if(read_size!=BUFSIZE)
        {
            fprintf(stderr,"%dth read... read_size is not 20 and it is %d\n",i,read_size ); //printing the last few remaining bytes when the last read from the file might not have exact 20 bytes
        }
        i++;
    }

    strcpy(status,"done");                      //Flag to be sent to the server to indicate that the file transfer has been completed
    send(sock,status,strlen(status)+1, 0 );
    printf("First file sent\n");


    for(i=0;i<BUFSIZE;i++)
        buffer[i]='\0';
    i=1;
    while((read_size=fread(buffer,1,BUFSIZE,f2))>0)
    {
        sent_size=send(sock,buffer,read_size , 0 );
        fprintf(stderr,"%d th sent_size  %d\n",i,sent_size); //Just printing how many bytes been sent in every iteration.
        if(read_size!=20)
        {
            fprintf(stderr,"%d th read...read_size is not 20 and it is %d\n",i,read_size );//printing the last few remaining bytes when the last read from the file might not have exact 20bytes
        }
        i++;
    }
    send(sock,status,strlen(status)+1, 0 );
    printf("Second file sent\n");
    fclose(f1);
    fclose(f2);
    close(sock);
    return 0;
}
//
// Created by tester on 26.06.2020.
//

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080
#define BUFSIZE 20

int main(int argc, char const *argv[])
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char status[15]={0},buffer[BUFSIZE]={0};
    int read_size=0,i=1,j;

    FILE *f1,*f2;
    f1=fopen("R1.txt","w+");
    f2=fopen("R2.txt","w+");

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

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Server Waiting\n");
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    while((read_size=recv(new_socket,buffer,BUFSIZE,0))>0)
    {
        printf("%d th Read size %d \n",i,read_size );
        if(read_size!=BUFSIZE)
        {
            printf("%d th read... read size is:%d, Data read : ",i,read_size );
            for(j=0;j<read_size;j++) //Printing the contents of the buffer when read size is less than 20 ()
                printf("%c",buffer[j]);
            printf("\n");

            if(strcmp(buffer,"done")==0)
            {
                printf("Flag received : done\n");
                break;
            }
        }
        fwrite( buffer,sizeof(char),read_size,f1);
        i++;
    }

    printf("\nFirst File received\n");

    for(i=0;i<BUFSIZE;i++)
        buffer[i]='\0';

    i=1;

    while((read_size=recv(new_socket,buffer,BUFSIZE,0))>0)
    {
        printf("%d th Read size %d \n",i,read_size );
        if(read_size!=BUFSIZE)
        {
            printf("%d th read... read size is:%d, Data read : ",i,read_size );
            for(j=0;j<read_size;j++) //Printing the contents of the buffer when read size is less than 20
                printf("%c",buffer[j]);
            printf("\n");

            if(strcmp(buffer,"done")==0)
            {
                printf("Flag received : done");
                break;
            }
        }
        fwrite( buffer,sizeof(char),read_size,f2);
        i++;
    }

    printf("\nSecond File received\n");

    fclose(f1);
    fclose(f2);
    close(new_socket);
    return 0;
}
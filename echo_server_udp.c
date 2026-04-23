#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct sockaddr_in serv_addr, cli_addr;

int sockfd, r, w, cli_addr_len;
unsigned short serv_port = 25020;
char serv_ip[] = "127.0.0.1";
char buff[128];

int main()
{
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, (&serv_addr.sin_addr));

    printf("\nUDP ECHO SERVER.\n");

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("\nSERVER ERROR: Cannot create socket.\n");
        exit(1);
    }

    if((bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("\nSERVER ERROR: Cannot bind.\n");
        close(sockfd);
        exit(1);
    }

    cli_addr_len = sizeof(cli_addr);
    for( ; ; )
    {
        printf("\nSERVER: Waiting for messages...Press Ctrl + c to stop:\n");
        
        bzero(buff, 128);
        if((r = recvfrom(sockfd, buff, 128, 0, (struct sockaddr*)&cli_addr, &cli_addr_len)) < 0)
        {
            printf("\nSERVER ERROR: Cannot receive message from client.\n");
        }
        else
        {
            buff[r] = '\0';
            printf("\nSERVER: Received %s from %s.\n", buff, inet_ntoa(cli_addr.sin_addr));

            if((w = sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr*)&cli_addr, cli_addr_len)) < 0)
                printf("\nSERVER ERROR: Cannot send message to the client.\n");
            else
                printf("\nSERVER: Echoed back %s to %s.\n", buff, inet_ntoa(cli_addr.sin_addr));
        }
    }
}

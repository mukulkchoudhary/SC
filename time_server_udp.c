#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct sockaddr_in serv_addr, cli_addr;

int sockfd, r, w, cli_addr_len;
unsigned short serv_port = 25020;
char serv_ip[] = "127.0.0.1";
char buff[128], reply[256];

int main()
{
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, (&serv_addr.sin_addr));

    printf("\nUDP TIME SERVER.\n");

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
    printf("\nSERVER: Waiting for time requests...\n");

    while(1)
    {
        bzero(buff, 128);
        if((r = recvfrom(sockfd, buff, 128, 0, (struct sockaddr*)&cli_addr, &cli_addr_len)) < 0)
        {
            printf("\nSERVER ERROR: Cannot receive.\n");
            continue;
        }
        buff[r] = '\0';

        if(strncmp(buff, "exit", 4) == 0)
        {
            printf("\nSERVER: Client disconnected.\n");
            continue;
        }

        if(strncmp(buff, "time", 4) == 0 || strncmp(buff, "TIME", 4) == 0)
        {
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            strftime(reply, 256, "Current Time: %H:%M:%S\nCurrent Date: %d-%m-%Y\n", tm_info);
        }
        else
        {
            strcpy(reply, "Unknown command. Type 'time' for current time.\n");
        }

        if((w = sendto(sockfd, reply, strlen(reply), 0, (struct sockaddr*)&cli_addr, cli_addr_len)) < 0)
            printf("\nSERVER ERROR: Cannot send.\n");
    }
    close(sockfd);
    return 0;
}

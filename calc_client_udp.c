#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct sockaddr_in serv_addr;

int sockfd, r, w;
unsigned short serv_port = 25020;
char serv_ip[] = "127.0.0.1";
char buff[128];

int main()
{
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, (&serv_addr.sin_addr));

    printf("\nUDP CALCULATOR CLIENT.\n");

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("\nCLIENT ERROR: Cannot create socket.\n");
        exit(1);
    }

    printf("\nCLIENT: Connected to calculator server.\n");
    printf("Format: number operator number (e.g., 5 + 3)\n");
    int len = sizeof(serv_addr);

    while(1)
    {
        printf("\nEnter expression or 'exit': ");
        fgets(buff, 128, stdin);
        buff[strcspn(buff, "\n")] = '\0';

        if((w = sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr*)&serv_addr, len)) < 0)
        {
            printf("\nCLIENT ERROR: Cannot send.\n");
            continue;
        }

        if(strncmp(buff, "exit", 4) == 0)
            break;

        bzero(buff, 128);
        if((r = recvfrom(sockfd, buff, 128, 0, (struct sockaddr*)&serv_addr, &len)) < 0)
            printf("\nCLIENT ERROR: Cannot receive.\n");
        else
        {
            buff[r] = '\0';
            printf("\nResult: %s", buff);
        }
    }
    close(sockfd);
    return 0;
}

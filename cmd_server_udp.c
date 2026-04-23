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
char buff[256], reply[4096];

int main()
{
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, (&serv_addr.sin_addr));

    printf("\nUDP COMMAND EXECUTOR SERVER.\n");

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
    printf("\nSERVER: Waiting for command requests...\n");

    while(1)
    {
        bzero(buff, 256);
        if((r = recvfrom(sockfd, buff, 256, 0, (struct sockaddr*)&cli_addr, &cli_addr_len)) < 0)
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

        printf("\nSERVER: Executing command: %s\n", buff);

        FILE *fp = popen(buff, "r");
        if(fp == NULL)
        {
            strcpy(reply, "Error: Cannot execute command\n");
        }
        else
        {
            bzero(reply, 4096);
            char line[256];
            while(fgets(line, sizeof(line), fp))
            {
                strcat(reply, line);
            }
            pclose(fp);
            if(strlen(reply) == 0)
                strcpy(reply, "Command executed successfully (no output)\n");
        }

        if((w = sendto(sockfd, reply, strlen(reply), 0, (struct sockaddr*)&cli_addr, cli_addr_len)) < 0)
            printf("\nSERVER ERROR: Cannot send.\n");
        else
            printf("\nSERVER: Response sent.\n");
    }
    close(sockfd);
    return 0;
}

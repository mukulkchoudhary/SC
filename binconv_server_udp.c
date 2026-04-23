#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct sockaddr_in serv_addr, cli_addr;

int sockfd, r, w, cli_addr_len;
unsigned short serv_port = 25020;
char serv_ip[] = "127.0.0.1";
char buff[128], reply[256];

void decToBin(int num, char *bin)
{
    char temp[64];
    int i = 0;
    if(num == 0)
        strcpy(bin, "0");
    else
    {
        while(num > 0)
        {
            temp[i++] = (num % 2) + '0';
            num /= 2;
        }
        for(int j = 0; j < i; j++)
            bin[j] = temp[i - 1 - j];
        bin[i] = '\0';
    }
}

int binToDec(char *bin)
{
    int dec = 0, base = 1;
    int len = strlen(bin);
    for(int i = len - 1; i >= 0; i--)
    {
        if(bin[i] == '1')
            dec += base;
        base *= 2;
    }
    return dec;
}

int main()
{
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_aton(serv_ip, (&serv_addr.sin_addr));

    printf("\nUDP BINARY CONVERTER SERVER.\n");

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
    printf("\nSERVER: Waiting for conversion requests...\n");

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

        char op = buff[0];
        char *data = buff + 2;

        if(op == 'B' || op == 'b')
        {
            int num = atoi(data);
            char binStr[64];
            decToBin(num, binStr);
            sprintf(reply, "Decimal %d = Binary %s\n", num, binStr);
        }
        else if(op == 'D' || op == 'd')
        {
            int dec = binToDec(data);
            sprintf(reply, "Binary %s = Decimal %d\n", data, dec);
        }
        else
        {
            sprintf(reply, "Error: Use B for decimal to binary, D for binary to decimal\n");
        }

        if((w = sendto(sockfd, reply, strlen(reply), 0, (struct sockaddr*)&cli_addr, cli_addr_len)) < 0)
            printf("\nSERVER ERROR: Cannot send.\n");
    }
    close(sockfd);
    return 0;
}

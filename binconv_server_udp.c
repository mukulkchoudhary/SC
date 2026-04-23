#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

struct sockaddr_in serv_addr, cli_addr;
int sockfd, r, w, cli_addr_len;
unsigned short serv_port = 25020;
char serv_ip[] = "127.0.0.1";
char buff[256], reply[512];

int precedence(char op)
{
    if(op == '+' || op == '-') return 1;
    if(op == '*' || op == '/') return 2;
    return 0;
}

double applyOp(double a, double b, char op)
{
    if(op == '+') return a + b;
    if(op == '-') return a - b;
    if(op == '*') return a * b;
    if(op == '/') return (b != 0) ? a / b : 0;
    return 0;
}

double solveMath(char *exp)
{
    double values[100];
    char ops[100];
    int vTop = -1, oTop = -1;
    int len = strlen(exp);
    
    for(int i = 0; i < len; i++)
    {
        if(isspace(exp[i])) continue;
        
        if(isdigit(exp[i]) || exp[i] == '.')
        {
            double val = 0;
            int decimal = 0;
            double div = 1;
            while(i < len && (isdigit(exp[i]) || exp[i] == '.'))
            {
                if(exp[i] == '.')
                {
                    decimal = 1;
                    i++;
                    continue;
                }
                if(decimal == 0)
                    val = (val * 10) + (exp[i] - '0');
                else
                {
                    div *= 10;
                    val = val + (exp[i] - '0') / div;
                }
                i++;
            }
            values[++vTop] = val;
            i--;
        }
        else if(exp[i] == '(')
        {
            ops[++oTop] = exp[i];
        }
        else if(exp[i] == ')')
        {
            while(oTop != -1 && ops[oTop] != '(')
            {
                double val2 = values[vTop--];
                double val1 = values[vTop--];
                values[++vTop] = applyOp(val1, val2, ops[oTop--]);
            }
            if(oTop != -1) oTop--;
        }
        else if(exp[i] == '+' || exp[i] == '-' || exp[i] == '*' || exp[i] == '/')
        {
            while(oTop != -1 && precedence(ops[oTop]) >= precedence(exp[i]))
            {
                double val2 = values[vTop--];
                double val1 = values[vTop--];
                values[++vTop] = applyOp(val1, val2, ops[oTop--]);
            }
            ops[++oTop] = exp[i];
        }
    }
    
    while(oTop != -1)
    {
        double val2 = values[vTop--];
        double val1 = values[vTop--];
        values[++vTop] = applyOp(val1, val2, ops[oTop--]);
    }
    
    return values[vTop];
}

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

    printf("\nUNIFIED UDP SERVER (Binary + Decimal + BODMAS Calculator).\n");

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
    printf("\nSERVER: Waiting for requests...\n");

    while(1)
    {
        bzero(buff, 256);
        bzero(reply, 512);
        
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

        char op = buff[0];
        char *data = buff + 2;

        printf("\nSERVER: Received request: %s\n", buff);

        if(op == 'B' || op == 'b')
        {
            int num = atoi(data);
            char binStr[64];
            decToBin(num, binStr);
            sprintf(reply, "Decimal %d = Binary %s\n", num, binStr);
            printf("SERVER: Converted Decimal to Binary\n");
        }
        else if(op == 'D' || op == 'd')
        {
            int dec = binToDec(data);
            sprintf(reply, "Binary %s = Decimal %d\n", data, dec);
            printf("SERVER: Converted Binary to Decimal\n");
        }
        else if(op == 'M' || op == 'm')
        {
            double result = solveMath(data);
            sprintf(reply, "Expression: %s\nResult: %.2f\n", data, result);
            printf("SERVER: Solved math expression\n");
        }
        else
        {
            strcpy(reply, "Error: Use B <num> for Dec to Bin, D <binary> for Bin to Dec, M <expression> for Math\n");
        }

        if((w = sendto(sockfd, reply, strlen(reply), 0, (struct sockaddr*)&cli_addr, cli_addr_len)) < 0)
            printf("\nSERVER ERROR: Cannot send.\n");
        else
            printf("SERVER: Response sent.\n");
    }
    close(sockfd);
    return 0;
}

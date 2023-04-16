#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define MAX_BUF_SIZE   1024

int main()
{
    struct sockaddr_in server_addr;
    int comm_sock = 0;
    int server_addr_len = 0;;
    unsigned char recvBuf[MAX_BUF_SIZE]={0,};
    unsigned char sendBuf[MAX_BUF_SIZE]={0,};

    comm_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (comm_sock == -1)
    {
        printf("error :\n");
        return 1;
    }

    memset(&server_addr, 0x00, sizeof(server_addr));
    
    server_addr.sin_family = AF_INET;
    //server_addr.sin_addr.s_addr = inet_addr("192.168.122.154");
    //server_addr.sin_addr.s_addr = inet_addr("192.168.122.124");
    server_addr.sin_addr.s_addr = inet_addr("192.168.122.107");    
    //server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(9999);

    server_addr_len = sizeof(server_addr);

    if (connect(comm_sock, (struct sockaddr *)&server_addr, server_addr_len) == -1)
    {
        printf("connect error :\n");
        return 1;
    }
    
    memset(sendBuf, 0x00, MAX_BUF_SIZE);
    memset(recvBuf, 0x00, MAX_BUF_SIZE);
    
    while(1){
        int c;
        printf("input message : ");
        scanf("%[^\n]s", sendBuf);
        
        if (write(comm_sock, sendBuf, MAX_BUF_SIZE) <= 0)
        {
            printf("write error\n");
            return 1;
        }

        if (read(comm_sock, recvBuf, MAX_BUF_SIZE) <= 0)
        {
            printf("read error\n");
            return 1;
        }

        printf("read : %s\n", recvBuf);
        if ((strcmp(sendBuf, "q") == 0) || (strcmp(sendBuf, "bye") == 0))   break;

        while(c = getchar() != '\n' && c != EOF);
        memset(sendBuf, 0x00, MAX_BUF_SIZE);
        memset(recvBuf, 0x00, MAX_BUF_SIZE);
    }
    close(comm_sock);

    return 0;
}

/*
#define DORSALSTREAM_GIT_VERSION     "54f76e,DESKTOP-QI1S4RO"
#define DORSALSTREAM_BUILD_TIMESTAMP "20210802:100955"
    printf("AT+CIPSTA? = %s\n", buf);
    esp8266_wifi_exec("AT+CIPDINFO=1", 0, buf, sizeof(buf));
    printf("AT+CIPDINFO=1 = %s\n", buf);
*/





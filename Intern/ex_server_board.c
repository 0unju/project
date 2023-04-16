#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define MAX_BUF_SIZE     1024

int main()
{
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    
    int option = 1;
    int connect_sock = 0;
    int comm_sock = 0;
    int client_addr_len = 0;
    int read_len = 0;
    int write_len = 0;
    int ret = 0;
    unsigned char recvBuf[MAX_BUF_SIZE] = {0, };

    client_addr_len = sizeof(client_addr);
    connect_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(connect_sock == -1)
    {
        printf("SOCKET CREATE ERROR!!\n");
        return 1;
    }

    memset(&server_addr, 0x00, sizeof(server_addr)); 
    setsockopt(connect_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    server_addr.sin_family = AF_INET;
    //server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_addr.s_addr = inet_addr("192.168.122.107");
    server_addr.sin_port = htons(1111);
    printf("Server : %s\n", inet_ntoa(server_addr.sin_addr));

    ret = bind(connect_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("wait...\n");
    listen(connect_sock, 5);

    memset(&client_addr, 0X00, sizeof(client_addr));
    comm_sock = accept(connect_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    printf("New Client : %s\n", inet_ntoa(client_addr.sin_addr));

    memset(recvBuf, 0x00, MAX_BUF_SIZE);
    if(read(comm_sock, recvBuf, MAX_BUF_SIZE) <= 0)
    {
        printf("read error : \n");
        close(comm_sock);
        return 0;
    }
    printf("receive message : %s\n", recvBuf);
    if(write(comm_sock, recvBuf, MAX_BUF_SIZE) <= 0)
    {
        printf("write error : \n");
        close(comm_sock);
    }
    sleep(100);

    /*
    while(1)
    {
        if(read(comm_sock, recvBuf, MAX_BUF_SIZE) <= 0)
        {
            printf("read error : \n");
            close(comm_sock);
            continue;
        }

        printf("receive message : %s\n", recvBuf);

        if(write(comm_sock, recvBuf, MAX_BUF_SIZE) <= 0)
        {
            printf("write error : \n");
            close(comm_sock);
        }
        if ((strcmp(recvBuf, "q") == 0) || (strcmp(recvBuf, "bye") == 0))   break;
        memset(recvBuf, 0x00, MAX_BUF_SIZE);
    }
    */

    close(comm_sock);
    close(connect_sock);

    return 0;
}
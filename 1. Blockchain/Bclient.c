#include <stdio.h> //STanDard Input Output
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> //"socket ?�수 ?�용" , "inet_addr ?�수 ?�용"
#include <netinet/in.h> // "inet_addr ?�수 ?�용", 
#include <sys/stat.h>
#include <arpa/inet.h> // "inet_addr ?�수 ?�용" 
#include <sys/types.h> // "socket ?�수 ?�용"
//#include <Windows.h>

#define MAX_BUF_SIZE   1024

int main()
{
   struct sockaddr_in server_addr;
   int comm_sock = 0;
   int server_addr_len = 0;
   unsigned char recvBuf[MAX_BUF_SIZE]={0,};
   unsigned char sendBuf[MAX_BUF_SIZE]={0,};
	int i = 0;
   comm_sock = socket(AF_INET, SOCK_STREAM, 0);

   if (comm_sock == -1)
   {
      printf("error :\n");
      return 1;
   }

   memset(&server_addr, 0x00, sizeof(server_addr));

   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = inet_addr("211.62.225.211");
   server_addr.sin_port = htons(12222);

   server_addr_len = sizeof(server_addr);

   if (connect(comm_sock, (struct sockaddr *)&server_addr, server_addr_len) == -1)
   {
      printf("connect error :\n");
      return 1;
   }

   memset(sendBuf, 0x00, MAX_BUF_SIZE);
   memset(recvBuf, 0x00, MAX_BUF_SIZE);

   while(1)
   {
      int c;
      printf("input : ");
      scanf("%[^\n]s", sendBuf);
      
      sendBuf[0] -= 0x30;
      if (write(comm_sock, sendBuf, MAX_BUF_SIZE) <= 0)
      {
         printf("write error\n");
         return 1;
      }

      if (read(comm_sock, recvBuf, MAX_BUF_SIZE) <= 0)
      {
         printf("read error 1\n");
         return 1;
      }

      printf("read1 : %s\n", recvBuf);
      printf("%02X ",recvBuf[0]);
      printf("\n");
      
      memset(recvBuf, 0x00, strlen(recvBuf));
      if (read(comm_sock, recvBuf, MAX_BUF_SIZE) <= 0)
      {
         printf("read error 2\n");
         return 1;
      }

      printf("read2 : %s\n", recvBuf);
      for(i=0; i < 100; i++)
         printf("%02X ",recvBuf[i]);
      printf("\n");

      while(c = getchar() != '\n' && c != EOF);
      memset(sendBuf, 0x00, MAX_BUF_SIZE);
      memset(recvBuf, 0x00, MAX_BUF_SIZE);
   }
   
   return 0;
}

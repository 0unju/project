#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <termios.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_BUF_SIZE    10000
#define DEVPORT     "/dev/ttyAMA0"
#define SERVER_IP   "210.123.39.58"
#define MY_IP       "192.168.64.79"
#define MY_PORT     19000
#define SZ      3

int server_addr_len = 0;
int len = 0, len2 = 0;
int read_len = 0;
unsigned char check[4] = {0x00, };
int num = 0; // check 확인하기 위한 용도
int file_len;
unsigned char Buf[MAX_BUF_SIZE] = {0x00, };
unsigned char dust[10]; // 받은 먼지 내 밑의 클라이언트에게 전송하기 위해 전역변수 사용
int Time[6];
int serial_fd = 0;
int rasp_server(char* ip);
unsigned char SendDust[15]; // 받은 먼지 내 밑의 클라이언트에게 전송하기 위해 전역변수 사용
int flag1 = 0; // 다른 라즈베리로부터 받은 먼지가 있으면 1 없으면 0
int flag2 = 0; // 다른 라즈베리로부터 받은 먼지 검증하기 전 1, 검증한 후 0으로 바꾸기
int flag3 = 0; // ip table 요청
int flag4 = 0; // ID 등록

struct IP_Address {
   int NUM;
    int PORT;
   char IP[15];
}; 

struct IP_Address IA[SZ];

typedef struct { 
     int tm_sec; /* 초 - [0～61] （閏秒を考慮） */ 
     int tm_min; /* 분 - [0～59] */ 
     int tm_hour; /* 시 - [0～23] */ 
     int tm_mday; /* 일 - [1～31] */ 
     int tm_mon; /* 월 - [0～11] */ 
     int tm_year; /* 1900부터의 년 */ 
 } tm; 

/* < multithread >
raspberrypi <--> 1. mcu 
                 2. raspberrypi
                 3. server 
*/

void* thread_1(void* data); // <-> get dust data from mcu by 10seconds
void* thread_2(void* data); // <-> raspberry pi (client)
void* thread_3(void* data); // <-> server
void* thread_4(void* data); // <-> raspberry pi (server)

/* thread 1 */        
int set_uart(char* device_name, int baudrate);
int data_read(int serial_fd, unsigned char* dust, int* Time); // get dust data from MCU

/* thread 2 */
int RtoR_client(unsigned char* dust, int* Time); // raspberrypi <-> raspberrypi

/* thread 3 */
int DtoS(unsigned char* dust, int* Time); // raspberrypi <-> MCU

/* thread 4 */
int rasp_server(char* ip);

int main()
{
    struct sockaddr_in server_addr;
    int thr_id;
    pthread_t p_thread[4];
    int status1, status2, status3, status4;
    int a = 1, b = 2, c = 3, d = 4;
    int comm_sock = 0;

    /* comm sock */
    comm_sock = socket(PF_INET, SOCK_STREAM, 0);

    if (comm_sock == -1)
    {
        printf("error :\n");
        return -2;
    }

    /* server information */
    memset(&server_addr, 0x00, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP); 
    server_addr.sin_port = htons(MY_PORT); 

    server_addr_len = sizeof(server_addr);

    /* connect */
    int connect1 = connect(comm_sock, (struct sockaddr *)&server_addr, server_addr_len);
    if (connect1 == -1)
    {
        printf("connect error\n\n");
    }

    while(1)
    {
        if(connect1 == -1)
        {
            while(connect1 != -1)
            {
                connect1 = connect(comm_sock, (struct sockaddr *)&server_addr, server_addr_len);
            }
        }

        sleep(2);
        thr_id = pthread_create(&p_thread[2], NULL, thread_3, (void*)&c);
        if (thr_id < 0)
        {
            perror("thread create error : \n");
            exit(0);
        }
    }

#if 0
    /* create thread */
    // thread 1
    thr_id = pthread_create(&p_thread[0], NULL, thread_1, (void*)&a);
    if (thr_id < 0)
    {
        perror("thread create error : \n");
        exit(0);
    }
    // thread 2
    sleep(2);
    thr_id = pthread_create(&p_thread[1], NULL, thread_2, (void*)&b);
    if (thr_id < 0)
    {
        perror("thread create error : \n");
        exit(0);
    }

    // thread 3
    sleep(2);
    thr_id = pthread_create(&p_thread[2], NULL, thread_3, (void*)&c);
    if (thr_id < 0)
    {
        perror("thread create error : \n");
        exit(0);
    }

    sleep(2);
    thr_id = pthread_create(&p_thread[3], NULL, thread_4, (void*)&d);
    if (thr_id < 0)
    {
        perror("thread create error : \n");
        exit(0);
    }

    /* close thread   */
    pthread_join(p_thread[0], (void**)&status1);
    pthread_join(p_thread[1], (void**)&status2);
    pthread_join(p_thread[2], (void**)&status3);
    pthread_join(p_thread[3], (void**)&status4);
#endif

    pthread_join(p_thread[2], (void**)&status3);
    printf("programing is end\n");
    return 0;
}

/* <-> mcu */
void* thread_1(void* data) 
{    
    printf("\n");
    int id;
    int i = 1;
    id = *((int*)data);
    serial_fd = set_uart(DEVPORT, B115200);

    while (1)
    {
        printf("\n/***** thread 1 *****/\n"); 
        printf("\n%d : #%d-dust\n", i, id);
        data_read(serial_fd, dust, Time);
        i++;
        usleep(2000000);
    }
    printf("\n");
}

/* <-> raspberrypi (client) */
void* thread_2(void* data)
{   
    printf("\n");
    int id;
    int i = 1;
    id = *((int*)data);

    while(1)
    {
        printf("\n/***** thread 2 *****/\n"); 
        printf("\n%d : #%d-rasp\n", i, id);
        RtoR_client(dust, Time);
        usleep(2000000);
        i++;
    }
    printf("\n");
}

/* <-> Server */
void* thread_3(void* data)
{   
    printf("\n");
    int id;
    int i = 1;
    id = *((int*)data);
    while(1)
    {
        printf("\n/***** thread 3 *****/\n"); 
        printf("\n%d : #%d-rasp\n", i, id);
        DtoS(dust, Time);
        usleep(2000000);
        i++;
    }
    printf("\n");
}

/* raspberry pi (server) */
void* thread_4(void* data)
{   
    printf("\n");
    int id;
    int i = 1;
    id = *((int*)data);
    while(1)
    {
        // printf("\n/***** thread 4 *****/\n"); 
        // printf("%d : #%d\n", i, id);
        rasp_server(MY_IP);
        // i++;
    }
    printf("\n");
}

int set_uart(char* device_name, int baudrate)
{
   struct termios newtio;
   int serial_fd;

   memset(&newtio, 0, sizeof(newtio));
   serial_fd = open((char*)device_name, O_RDWR | O_NOCTTY);

   printf("[serial_fd : %d]\n", serial_fd);

   if (serial_fd < 0)
   {
      printf("serial fd open fail !!!\n");
      return -1;
   }

   newtio.c_cflag = baudrate;
   newtio.c_cflag |= CS8; //data:8/ stop:1bit
   newtio.c_cflag |= CLOCAL;
   newtio.c_cflag |= CREAD;
   newtio.c_iflag = IGNPAR;
   newtio.c_oflag = 0;
   newtio.c_lflag = 0;
   newtio.c_cc[VTIME] = 0;
   newtio.c_cc[VMIN] = 1;

   tcflush(serial_fd, TCIFLUSH);
   tcsetattr(serial_fd, TCSANOW, &newtio);

   return serial_fd;
}

int data_read(int serial_fd, unsigned char* dust, int* Time)
{
    unsigned char dustBuf[10] = {0x00, };
   unsigned char buf[MAX_BUF_SIZE] = { 0x00, };

    int len = 0;
    int j = 0;
    int i = 0;
    int TIME[6] = {0x00, };

    memset(dustBuf, 0x00, 10);
 
    while(1)
   {
      len = read(serial_fd, &buf[j], 1);
      if (len == 1)
      {
            dustBuf[j]=buf[j];
            time_t current_time = time(NULL);
            struct tm * t = localtime(&current_time);
            TIME[0] = t->tm_year + 1900;
            TIME[1] = t->tm_mon + 1;
            TIME[2] = t->tm_mday;
            TIME[3] = t->tm_hour;
            TIME[4] = t->tm_min;
            TIME[5] = t->tm_sec;

            for(int i=0;i<6;i++)
            {
                Time[i]=TIME[i];
            }

         if (dustBuf[j] == 0xab)
         {
            printf("\n");
                printf("- time : ");
                for(int i = 0; i < 3; i++)
                {
                    printf("%d", Time[i]);
                    if(i == 2)
                    {
                        printf(" ");
                        break;
                    }
                    printf("-");
                }
                for(int i = 3; i < 6; i++)
                {
                    printf("%d", Time[i]);
                    if(i == 5)
                    {
                        break;
                    }
                    printf(":");
                }
                printf("\n");
                break;
         }
         j++;
      }
    }

   /* save dust data to 'dust' buf */
   printf("- dust : ");
    for(i = 0; i < 10; i++)
    {
        dust[i] = dustBuf[i];
        printf("%02x ", dust[i]);
    }
    printf("\n");
    
    return 0;
}


int RtoR_client(unsigned char* dust, int* Time)
{
    struct sockaddr_in server_addr;
    struct IP_Address IA[SZ];
    int comm_sock = 0;
    int server_addr_len = 0;;
    int i , j, result = 0;
    unsigned char recvBuf[MAX_BUF_SIZE] = {0x00, };
    unsigned char sendBuf[MAX_BUF_SIZE] = {0x00, };
    int recvBuf_Time[MAX_BUF_SIZE] = {0, };
    int sendBuf_Time[MAX_BUF_SIZE] = {0, };
    char buf[SZ][50];
    char *my_ip = MY_IP;

    /* id */
    unsigned char data_se_id = 0;
    unsigned char data_re_id = 0;

    unsigned char data_sign[64] = {0x00, };

    /* get ip from 'ip1.txt' to send dust data to next raspberrypi */

    if(flag4 == 1)
    {
        FILE *fp = fopen("ip1.txt", "r");

        for (i = 0; i < SZ; i++)
        fgets(buf[i], sizeof(buf[0]), fp); 

        for (i = 0; i < SZ; i++)
        {
            IA[i].NUM = atoi(strtok(buf[i], " "));
            IA[i].PORT = atoi(strtok(NULL, " "));
            strcpy(IA[i].IP, strtok(NULL, "\n\r"));
        }

        for (i = 0; i < SZ; i++)
        {
            if (strcmp(my_ip, IA[i].IP) == 0 && (MY_PORT == IA[i].PORT))
            {
                if (i < SZ - 1)
                    result = i + 1;
                else
                    result = 0;
                break;
            }
            else 
            {
                if (i == SZ - 1)
                {
                    printf("file registered or ID registered is not running\n");
                    return -1;
                }
            }
        }

        /* comm sock */
        comm_sock = socket(PF_INET, SOCK_STREAM, 0);

        if (comm_sock == -1)
        {
            printf("error :\n");
            return -2;
        }

        /* server information */
        memset(&server_addr, 0x00, sizeof(server_addr));

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(IA[result].IP); 
        server_addr.sin_port = htons(IA[result].PORT); 
        server_addr_len = sizeof(server_addr);

        /* connect */
        if (connect(comm_sock, (struct sockaddr *)&server_addr, server_addr_len) == -1)
        {
            fprintf(stderr, "connect error[%d-%s]\n", errno, strerror(errno));
            return -3;
        }

        /* id */
        //   data_se_id[0] = ;
        //   data_re_id[0] = (char)IA[result].NUM;
        
        /* sendBuf */
        memset(sendBuf, 0x00, MAX_BUF_SIZE);
        memset(sendBuf_Time, 0, MAX_BUF_SIZE);

        sendBuf[0] = 0x06;
        sendBuf[1] = 0x00;
        sendBuf[2] = 0x0C;

        // put dust data(15) to sendBuf
        for(i = 5; i < 15; i++)
        {
            sendBuf[i] = dust[i];
        }
        printf("\n");

        // id(2) == sendBuf[3]||sendBuf[4]
        if(IA[result].NUM > 255)
        {
            printf("count > 255\n");
            sendBuf[3] = IA[result].NUM / 256;
            sendBuf[4] = IA[result].NUM % 256;
        }
        else
        {
            sendBuf[3] = 0x00;
            sendBuf[4] = IA[result].NUM;
        }
        // sendBuf[3] = data_se_id;
        // sendBuf[4] = data_re_id;

        // put dust data sign(64) to sendBuf
        for (i = 0; i < 64; i++)
        {
            sendBuf[i + 15] = data_sign[i];
        }

        for(i = 0; i < 6; i++)
        {
            sendBuf_Time[i] = Time[i];
        }

        // print sendBuf
        printf("- dust : ");
        for(i = 0; i < 15; i++)
        {
            printf("%02X ", sendBuf[i]);
        }
        printf("\n");
        printf("- time : ");
        for(i = 0; i < 3; i++)
        {
            printf("%d",sendBuf_Time[i]);
            if(i == 2)
            {
                break;
            }
            printf("-");
        }
        printf(" ");
        for(i = 3; i < 6; i++)
        {
            printf("%d",sendBuf_Time[i]);
            if(i == 5)
            {
                break;
            }
            printf(":");
        }
        printf("\n");

        // write 
        if (write(comm_sock, sendBuf, MAX_BUF_SIZE) <= 0)
        {
            printf("write error\n");
            return -4;
        }
        else
            printf("write success\n");

        if (write(comm_sock, sendBuf_Time, MAX_BUF_SIZE) <= 0)
        {
            printf("write error\n");
            return -4;
        }
        else
            printf("write time success\n");

        if (flag1 == 1)
        {
            write(comm_sock, SendDust, sizeof(SendDust));
            printf("send dust success\n");
            flag1 = 0;
            flag2 = 1;
        }

        // read 
        memset(recvBuf, 0x00, MAX_BUF_SIZE);
        if (read(comm_sock, recvBuf, MAX_BUF_SIZE) <= 0)
        {
            printf("read error\n");
            return -5;
        }

        memset(recvBuf_Time, 0, MAX_BUF_SIZE);
        if(read(comm_sock, recvBuf_Time, MAX_BUF_SIZE)<=0)
        {
            printf("read error\n");
            return -5;
        }

        if(recvBuf[0] == 0x07)
        {
            /* block response 찍어 보기 */

            puts("");
            printf("recv protocol : %02x\n", recvBuf[0]);
            printf("recv length : %02x ", recvBuf[1]);
            printf("%02x\n", recvBuf[2]);
            printf("S/F: %02x\n", recvBuf[3]);

            if(recvBuf[3] == 0x00)
            {
                printf("fail!!!\n");
            }
            
            else if(recvBuf[3] == 0x01)
            {
                printf("success!!!\n");
            }
        }
        close(comm_sock);
        return 0;
    }
}

int rasp_server(char* ip)
{
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    struct IP_Address IA[SZ];
    int connect_sock = 0;
    int comm_sock = 0;
    int client_addr_len = 0;
    int n = 0, ret = 0, i = 0, j, result, size;
    unsigned char sendBuf[MAX_BUF_SIZE] = {0, };
    unsigned char recvBuf[MAX_BUF_SIZE] = {0, };
    unsigned char dataBuf[MAX_BUF_SIZE] = {0, };
    char buf[SZ][40];
    char *my_ip = MY_IP;
    int option = 1;
    unsigned char datatype = 0x03;
    unsigned char datamain[10] = {0xAA, 0xC0, 0x70, 0x00, 0xD6, 0x00, 0x13, 0x1F, 0x78, 0xAB};
    unsigned char datasign[64] = {0x00};
    unsigned char server_id[1];

    if(flag4 == 1)
    {
        printf("\n/***** thread 4 *****/\n"); 
        
        FILE *fp = fopen("ip1.txt", "r");

        printf("server program start !\n");   

        for (i = 0; i < SZ; i++)
            fgets(buf[i],sizeof(buf[0]),fp);  

        for (i = 0; i < SZ; i++)
        {
            IA[i].NUM = atoi(strtok(buf[i], " "));
            IA[i].PORT = atoi(strtok(NULL, " "));
            strcpy(IA[i].IP, strtok(NULL, "\n\r"));
        }

        for (i = 0; i < SZ; i++)
        {
            if (strcmp(my_ip, IA[i].IP) == 0)
            {
                if (i < SZ - 1)
                    result = i + 1;
                else
                    result = 0;
                break;
            }
            else
            {
                if(i == SZ - 1)
                {   
                    printf("file registered or ID registered is not running\n", i);
                    return 1;
                }
            }
        }

        client_addr_len = sizeof(client_addr);

        connect_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (connect_sock == -1)
        {
            printf("SOCKET CREATE ERROR!1!\n");
            return 1;
        }
        else
        {
            printf("\n");
            printf("Socket create success.\n");
        }

        memset(&server_addr, 0x00, sizeof(server_addr));

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
        server_addr.sin_port = htons(MY_PORT); 

        setsockopt(connect_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        ret = bind(connect_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

        listen(connect_sock, 5);

        memset(&client_addr, 0x00, sizeof(client_addr));
        comm_sock = accept(connect_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        printf("\nNew Client : %s\n",inet_ntoa(client_addr.sin_addr)); 
        printf("Sever Address : %s\n", IA[i].IP);
        printf("\n");

        memset(recvBuf, 0x00, MAX_BUF_SIZE);

        if ((n = read(comm_sock, recvBuf, MAX_BUF_SIZE)) <= 0)
        {
            printf("read error : \n");
            close(comm_sock);
            return 0;
        }

        if (recvBuf[0] == 0x06)
        {
            puts("");
            printf("recv protocol : %02x\n", recvBuf[0]);
            printf("recv length : %02x ", recvBuf[1]);
            printf("%02x\ndata : ", recvBuf[2]);

            for(i = 3; i < 15; i++)
            {
                printf("%02x ", recvBuf[i]);
            }

            /* to raspi response */
            // data type(1) || length(2) || 성공 여부(1)
            sendBuf[0] = 0x07;
            sendBuf[1] = 0x00;
            sendBuf[2] = 0x01;
            
            if(recvBuf[1] == 0x00 && recvBuf[2] == 0x0C)
            {
                sendBuf[3] = 0x01; // success
                write(comm_sock, sendBuf, 4);
                printf("success!!!\n");
            }
            
            else
            {
                sendBuf[3] = 0x00; // fail
                write(comm_sock, sendBuf, 4);
                printf("fail!!!\n");
            }
        }

        for(i = 0; i < 15; i++)
        {
            SendDust[i] = recvBuf[i];
        }

        flag1 = 1;

        close(comm_sock);

        /*
        if ((int)recvBuf[0] == 3)
        {
            printf("This data is mcu's data\n");
            size = (int)recvBuf[1] * 256 + (int)recvBuf[2];
        }

        printf("\n");
        printf("receive message :  ");
        for (j = 1 + 2; j < size; j++)
        {
            if (j == 3)
            printf("\n(data)\n");
            else if (j == 13)
            printf("\n(sign)\n");   
            
            printf("%02X ", (int)recvBuf[j]);
            
            if (j>13 && j % 8 == 4)
            printf("\n");
        }

        printf("\n");
        memset(dataBuf, 0x00, MAX_BUF_SIZE);
        */

        close(connect_sock);
        return 0;
    }
}

int DtoS(unsigned char* dust, int* Time)
{
    struct sockaddr_in server_addr;
    struct IP_Address IA[SZ];
    int comm_sock = 0;
    int server_addr_len = 0;;
    int i, j, k;
    int count = 0, len = 0, result = 0;
    unsigned char recvBuf[MAX_BUF_SIZE]={0x00, };
    unsigned char sendBuf[MAX_BUF_SIZE]={0x00, };
    unsigned char data[MAX_BUF_SIZE] = {0, };
    unsigned char ptr[MAX_BUF_SIZE] = {0, };
    int recvBuf_Time[MAX_BUF_SIZE]={0, };
    int sendBuf_Time[MAX_BUF_SIZE]={0, };
    char buf[SZ][50];
    char *my_ip = MY_IP;

    /* id */
    unsigned char data_se_id = 0;
    unsigned char data_re_id = 0;

    unsigned char data_sign[64] = {0x00, };
#if 0
    /* comm sock */
    comm_sock = socket(PF_INET, SOCK_STREAM, 0);

    if (comm_sock == -1)
    {
        printf("error :\n");
        return -2;
    }

    /* server information */
    memset(&server_addr, 0x00, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP); 
    server_addr.sin_port = htons(MY_PORT); 

    server_addr_len = sizeof(server_addr);

    /* connect */
    if (connect(comm_sock, (struct sockaddr *)&server_addr, server_addr_len) == -1)
    {
        fprintf(stderr, "connect error[%d-%s]\n", errno, strerror(errno));
        return -3;
    }
#endif
    /* ip table 요청 */
    sendBuf[0] = 0x02;
    sendBuf[1] = 0x00;
    sendBuf[2] = 0x00;

    len = write(comm_sock, sendBuf, 3);

    if(len < 0)
    {
        printf("write error\n");
    }

    len = read(comm_sock, recvBuf, sizeof(recvBuf));
    if(len < 0)
    {
        printf("read error\n");
    }

    if(recvBuf[0] == 0x02)
    {
        /* ip table 생성 */
        puts("");
        printf("recv protocol : %02x\n", recvBuf[0]);
        printf("recv length : %02x ", recvBuf[1]);
        printf("%02x\n", recvBuf[2]);
        printf("read : %s\n\n", &recvBuf[3]);

        FILE * fp; // 받은 데이터 ip1.txt 파일로 만들기
        fp = fopen("ip1.txt", "w");

        if(fp == NULL)
        {
            printf("FILE open error\n");
            return 1;
        }

        for(j = 0; j < len - 3; j++)
        {
            ptr[j] = recvBuf[j + 3]; // 데이터만 저장
        }

        fwrite(ptr, 1, len - 3, fp); // 받은 길이만큼 txt 파일에 쓰기
        fclose(fp);

        FILE * fp1; // ip1.txt 파일에 써 있는 데이터 잘 써졌는지 확인해 보기
        fp1 = fopen("ip1.txt", "r");

        fread(data, 1, MAX_BUF_SIZE, fp1);
        printf("data : ");
        printf("%s\n", data);

        fclose(fp1);

        flag3 = 1;
    }

    else
    {
        printf("file receive fail\n");
        return 0;
    }

    /* ip 등록 요청 */
    FILE *fp4;
    fp4 = fopen("ip1.txt","r");

    if (fp4 == NULL)
    {
        printf("No ip1.txt\n");
        return 0;
    }  
    else 
    {
        file_len = fread(Buf, 1, MAX_BUF_SIZE, fp4);
        // printf("%d\n", file_len);

        if(file_len <= 0)
        {
            printf("ip1.txt is empty\n");
            return 0;
        }
        else
        {
            for(int i = 0; i < file_len; i++)
            {
                if(Buf[i] == '\n') // newline
                    count++;
            }
            count += 1;
        }
        // printf("%d\n", count);
        fclose(fp4);
    }

    sendBuf[0] = 0x01;
    sendBuf[1] = 0x00; // 길이 
    sendBuf[2] = 0x02;

    if(count > 255)
    {
        printf("count > 255\n");
        sendBuf[3] = count / 256;
        sendBuf[4] = count % 256;
    }
    else
    {
        sendBuf[3] = 0x00;
        sendBuf[4] = count;
    }

    printf("MY ID: %02x %02x\n", sendBuf[3], sendBuf[4]);
    len = write(comm_sock, sendBuf, 5);
    if(len < 0)
    {
        printf("write error\n");
        return 0;
    }

    len = read(comm_sock, recvBuf, sizeof(recvBuf));
    if(len < 0)
    {
        printf("read error\n");
        return 0;
    }

    if(recvBuf[0] == 0x01)
    {
        puts("");
        printf("recv protocol : %02x\n", recvBuf[0]);
        printf("recv length : %02x ", recvBuf[1]);
        printf("%02x\n", recvBuf[2]);
        printf("S/F : %02x\n", recvBuf[3]);
        printf("result : ");
        if(recvBuf[3] == 0x00)
        {
            printf("fail!!!\n");
        }
        else if(recvBuf[3] == 0x01)
        {
            printf("success!!!\n");
            if(flag3 == 1)
            {
                flag4 = 1;
                flag3 = 0;
            }
        }
        else if(recvBuf[3] == 0x02)
        {
            printf("update!!!\n");
        }
        else if(recvBuf[3] == 0x03)
        {
            printf("duplicate!!!\n");
        }
    }

    pid_t pid;
    fork(); // 하나는 블록 계속 받기, 하나는 먼지 전송, 검증

    if(pid == 0)
    {
        len = read(comm_sock, recvBuf, sizeof(recvBuf));
        if(len < 0)
        {
            printf("read error\n");
        }

        if(recvBuf[0] == 0x05)
        {
            puts("");
            printf("recv protocol : %02x\n", recvBuf[0]);
            printf("recv length : %02x ", recvBuf[1]);
            printf("%02x\n", recvBuf[2]);
            printf("read : %s\n\n", &recvBuf[3]);

            for(i = 1; i < MAX_BUF_SIZE; i++)
            {
                char filename[MAX_BUF_SIZE];
                sprintf(filename, "block%d.txt", i); // block1, block2, ... 를 filename에 저장
                FILE * fp2;
                fp2 = fopen(filename, "rb"); // rb는 파일이 없으면 NULL

                if(fp2 == NULL)
                    {
                        fp2 = fopen(filename, "w");
                        fwrite(ptr, 1, read_len - 3, fp2);
                        fclose(fp2);

                        FILE * fp3;
                        fp3 = fopen(filename, "r");

                        unsigned char data1[MAX_BUF_SIZE] = {0, };

                        fread(data1, 1, MAX_BUF_SIZE, fp3);
                        printf("data : ");
                        printf("%s\n", data1);

                        /* block response */
                        
                        for(k = 0; k < read_len - 3; k++)
                        {
                            if(ptr[k] == data1[k])
                                num ++;
                        }

                        printf("num: %d\n", num);
                        printf("len: %d\n", read_len - 3);

                        if(num == (read_len - 3)) // 프로토콜 번호, 길이 제외한 데이터
                        {
                            check[0] = 0x05; // protocol
                            check[1] = 0x00; // length
                            check[2] = 0x01; // length
                            check[3] = 0x01; // success
                            write(comm_sock, check, 4);
                        }
                        else
                        {
                            check[0] = 0x05; // protocol
                            check[1] = 0x00; // length
                            check[2] = 0x01; // length
                            check[3] = 0x00; // fail
                            write(comm_sock, check, 4);
                        }

                        fclose(fp3);
                        break;
                    }
                else
                    fclose(fp2);
            }
        }
    }

    else
    {
        /********** 먼지 데이터 전송 **********/
        /* id */
        data_se_id = (char)IA[i].NUM;
        data_re_id = (char)IA[result].NUM;

        /* sendBuf */
        memset(sendBuf, 0x00, MAX_BUF_SIZE);
        memset(sendBuf_Time, 0, MAX_BUF_SIZE);

        // put dust data(15) to sendBuf
        sendBuf[0] = 0x03;
        sendBuf[1] = 0x00;
        sendBuf[2] = 0x0C;
        
        // id(2)
        sendBuf[3] = data_se_id;
        sendBuf[4] = data_re_id;

        for(i = 5; i < 15 ; i++)
        {
            sendBuf[i] = dust[i];
        }
        printf("\n");

        // put dust data sign(64) to sendBuf
        for (i = 0; i < 64; i++)
        {
            sendBuf[i + 15] = data_sign[i];
        }

        for(i = 0; i < 6; i++)
        {
            sendBuf_Time[i] = Time[i];
        }

        // print sendBuf
        printf("<input message>\n");
        printf("- dust : ");
        for(i = 0; i < 79; i++)
        {
            printf("%02X ", sendBuf[i]);
        }
        printf("\n");

        printf("- time : ");
        for(i = 0; i < 3; i++)
        {
            printf("%d", sendBuf_Time[i]);
            if(i == 2)
            {
                break;
            }
            printf("-");
        }
        printf(" ");

        for(i = 3; i < 6; i++)
        {
            printf("%d", sendBuf_Time[i]);
            if(i == 5)
            {
                break;
            }
            printf(":");
        }
        printf("\n");

        // write 
        if (write(comm_sock, sendBuf, MAX_BUF_SIZE) <= 0)
        {
            printf("write error\n");
            return -4;
        }

        if (write(comm_sock, sendBuf_Time, MAX_BUF_SIZE) <= 0)
        {
            printf("write error\n");
            return -4;
        }

        // read 
        memset(recvBuf, 0x00, MAX_BUF_SIZE);
        if (read(comm_sock, recvBuf, MAX_BUF_SIZE) <= 0)
        {
            printf("read error\n");
            return -5;
        }

        memset(recvBuf_Time, 0, MAX_BUF_SIZE);
        if(read(comm_sock, recvBuf_Time, MAX_BUF_SIZE)<=0)
        {
            printf("read error\n");
            return -5;
        }

        if(recvBuf[0] == 0x03)
        {
            /* server로 부터 답변 */
            memset(sendBuf, 0x00, MAX_BUF_SIZE);

            puts("");
            printf("recv protocol : %02x\n", recvBuf[0]);
            printf("recv length : %02x ", recvBuf[1]);
            printf("%02x\n", recvBuf[2]);
            printf("S/F: %02x\n", recvBuf[3]);

            if(recvBuf[3] == 0x00)
            {
                printf("fail!!!\n");
            }

            else if(recvBuf[3] == 0x01)
            {
                printf("success!!!\n");
            }
        }

        /***** 먼지 데이터 검증 *****/
        if (flag2 == 1)
        {
            sendBuf[0] = 0x04;
            sendBuf[1] = 0x00;
            sendBuf[2] = 0x0F;
            sendBuf[3] = data_se_id; // 전파한 Raspi ID
            sendBuf[4] = 0x00;
            sendBuf[5] = data_re_id; // 검증한 Raspi ID
            sendBuf[6] = 0x00;

            for(i = 0; i < 10; i ++)
            {
                sendBuf[i + 7] = SendDust[i]; // 먼지 데이터 저장
            }

            if(SendDust[0] == 0xaa && SendDust[8] == SendDust[2] + SendDust[3] + SendDust[4] + SendDust[5] + SendDust[6] + SendDust[7] && SendDust[9] == 0xab)
            {
                sendBuf[17] = 0x01;
            }

            else
            {
                sendBuf[17] = 0x00;
            }

            len = write(comm_sock, sendBuf, 18);
            if(len < 0)
            {
                printf("write error\n");
                return 0;
            }

            flag2 = 0;
        }
    }

    close(comm_sock);
    return 0;
}
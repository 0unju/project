#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/socket.h>  
#include <sys/types.h>
#include <pthread.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include "BlockCreate_H.c"

// sudo reboot

#define _CRT_SECURE_NO_WARNINGS
//#define MAX_RASP 10
#define MAX_BUF_SIZE 1024

int MAX_RASP = 0;
int Block_Num;
int Block_Create;
int Client_Pthread_Id[20] = {0, };
int clnt_cnt = 0;
int clnt_socks[10];

enum Name{
    ID = 1,
    IP,
    DATA,
    DATA_VERI,
    BLOCK_CHECK,
    BLOCK
};

/*
    성공 1 실패 0
    ID 등록 : 0x01
    - 새로 등록 / 갱신 / 변동 없음 (등록 실패 : 같은 IP인데 ID가 다른 경우)
    - 01 || length(2byte) || 
    IP table 요청 : 0x02
    - 02 || length(2byte) || IP table
    데이터 전송 : 0x03
    - 03 || length(2byte) || ID || 먼지 센서 데이터
    데이터 검증 결과 요청 : 0x04
    - 04 || length(2byte) || 검증한 ID || 데이터를 보낸 ID || 먼지 센서 데이터 || 검증 결과
    Block 전파 : 0x05
    - 05 || length(2byte) || 블록 정보
*/

typedef struct {
    char id[3];
    char ip[20];
} RP;

int strlength(char* s,char* len)
{
    char buf[3] ={0,};
    int i = 0; 
    int count = 0;
    int len_s = 0;
    for ( i = 0; s[i] != '\0'; i++)
    //while(*s != '\0')
    {
        count++;   
        //printf("strlength : %d\n",count);
    }
    strncpy(buf, len, sizeof(char)*2);
    len_s = atoi(buf);
    if (len_s == count)
    {
        return 1;
    }
    else
    {
        return 0;
    }   
}

void print(char* Id, char* buf, int a)
{
    FILE* fp = NULL;
	int i = 0;

    if(a == 0)
    {
        fp = fopen(Id,"a+");
        assert(fp != NULL);
    }
	
    for (i = (5 + a); i < (15 + a); i++)
	{
		printf("%x", buf[i]);
        if(a == 0)
        {
            fprintf(fp, "%x", buf[i]);
        }
	}

    printf(" %s 00\n", &buf[15]);
    fprintf(fp, " %s 00 \n", &buf[15]);

    fclose(fp);
}


float veri( char* buf, int i,char* Id )
{
    FILE* fp = NULL;

    char veri_num = 0x00;
    int veri_sum = 0;  // veri_sum / rasp_num >0.5
    
    float verification = 0;
    int veri = 0;

    char ras_sum = 0x00;  //파일에 저장되어있는 (데이터를 검증한) ras의 개수
    int ras = 0;
    char buf1[128] = { 0x00, };

    fp = fopen(Id,"r+");
    assert(fp != NULL);

    printf("i = %d\n\n", i);
    fseek(fp,14*(i-1)+11, SEEK_SET);
    printf("fseek pass\n");
    veri_num = fgetc(fp);
    veri_sum = veri_num -'0';
    veri_sum = veri_sum + (buf[17]-'0');
    printf("veri_sum = %d\n\n", veri_sum);
    fseek(fp,14*(i-1) + 11,SEEK_SET);
    printf("fseek pass\n");
    fprintf(fp,"%d",veri_sum);
    printf("fprintf pass\n");

    fseek(fp,14*(i-1)+12,SEEK_SET);
    printf("fseek pass\n");
    ras_sum = fgetc(fp);
    ras = ras_sum -'0';
    ras = ras + 1;
    fseek(fp,14*(i-1) + 12,SEEK_SET);
    printf("fseek pass\n");
    fprintf(fp,"%d",ras);
    printf("fprintf pass\n");

    fseek(fp,14*(i-1) + 12,SEEK_SET);
    printf("fseek pass\n");
    ras_sum = fgetc(fp);            
    if ((ras_sum -'0') == (MAX_RASP - 1))
    {
        printf("if pass\n");

        fseek(fp,14*(i-1) + 11, SEEK_SET);
        printf("fseek pass1\n");
        veri_num = fgetc(fp);
        veri = veri_num -'0';
        verification = veri / (MAX_RASP - 1);
    }
    fclose(fp);
    return verification;
}

int ip_length(int sockfd, int iplen)
{
    FILE* fp = NULL;

    fp = fopen("ip.txt","w+");
    if(iplen == 15)
    {
        fclose(fp);
        return 1;
    }
    else if(iplen < 15)
    {
        for(int i = 0; i < 15 - iplen; i++) fprintf(fp, " ");
        fclose(fp);
        return 1;
    }
    else
    {
        write(sockfd, "Wrong IP\n\n", sizeof(char) * 10);
        fclose(fp);
        return 2;    
    }
}

void send_block(int sockfd,char* node_buf)
{
    char block[128];
    FILE* fp = NULL;
    char buf[MAX_BUF_SIZE];
    memset(buf, 0x00, MAX_BUF_SIZE);

    sprintf(block, "block%s.txt", &node_buf[3]);
    fp = fopen(block, "r");
    assert(fp != NULL);
    fread(buf, sizeof(buf), 1, fp);
    fclose(fp);
    char buf0[2] = {0x05,0x00};
    buf0[1] = strlen(buf);
    write(sockfd, buf0, sizeof(char) * 2); // 서버에서도 보낸 내용을 확인하기 위해
}


void* thread_func(void* data)
{
    /*
    주소를 입력값으로 가지는 이유 :
    스레드는 프로세스의 메모리 공간을 공유함 > 소켓 주소 값으로 모든 스레드가 소켓 메모리 공간을 공유할 수 있음
    
    지금 코드에서는 여러 스레드가 공유하는 메모리 공간에 대한 보호가 고려되어 있지 않다고 함 > 뮤텍스 사용(p.380)
    */ 
    int sockfd = *((int*)data);
    int read_len;
    int write_len = 0;
    socklen_t addrlen;
    char buf[MAX_BUF_SIZE];
    char buff[MAX_BUF_SIZE];
    char block[128];
    struct sockaddr_in client_addr;
    FILE* fp = NULL;
    int i;
   
    memset(buf, 0x00, MAX_BUF_SIZE);
    addrlen = sizeof(client_addr);
    getpeername(sockfd, (struct sockaddr*)&client_addr, &addrlen);

    /*
        if문 -> case문
    */
    
    while((read_len = read(sockfd, buf, MAX_BUF_SIZE)) > 0)
    {
        
        printf("Read Data %s(%d) - recvBuff[0] : %02x\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buf[0]);
        pthread_t pthread;
                
        if (strlength(&buf[3],&buf[1]) == 1)  // client가 보낸 데이터 length와 실제 data의 길이가 같으면
        {
            #if 1 // switch 문으로 작성
            switch (buf[0])
            {
            case ID:
            {
                pthread = pthread_self();
                printf("pthread id = %ud\n", (int)pthread);
               
                // 10201 : 1(프로토콜) 02(길이) 01(ID)
                char id[3];
                RP data[20];
                char ip_buf[128] = { 0x00, };
                char buf0[2] = {0x01,0x01};
                int i;

                int iplen = 0;
                strncpy(id, &buf[3], sizeof(char)*2);
                iplen = strlen(inet_ntoa(client_addr.sin_addr));
                printf("iplen = %d", iplen);
                fp = fopen("ip.txt", "r+");
                //fseek(fp, 0, SEEK_SET);
                fseek(fp, 25*MAX_RASP, SEEK_SET); // ID는 그대로 놔두면 되니까 ID 뒷부분에 커서 이동
                fprintf(fp, "%s %d %s", id, ntohs(client_addr.sin_port), inet_ntoa(client_addr.sin_addr));
                if(iplen <= 15)
                {
                    for(int i = 0; i < 15 - iplen; i++) fprintf(fp, " ");
                }
                else
                {
                    write(sockfd, "Wrong IP\n\n", sizeof(char) * 10);
                }                                
                //ip_length(sockfd, iplen);
                fprintf(fp,"\n");

                MAX_RASP++; // 새로운 라즈베리가 연결됐다는 뜻이니까 전체 라즈베리파이의 개수를 증가
                printf("MAX_RASP = %d\n", MAX_RASP);

                memset(buf,0x00,MAX_BUF_SIZE);
                buf[0] = 0x01; //새로운 ID와 IP를 입력했음
                fclose(fp);

                write_len = write(sockfd, buf0, sizeof(char) * 1);
                printf("write len : %d\n",write_len);
                break;        
            }

            case IP:
            {
                // 2(프로토콜)
                char buf0[2] = {0x02,0x00};
                fp = fopen("ip.txt", "r");               
                assert(fp != NULL);
                fread(buf, sizeof(buf), 1, fp);
                fclose(fp);
                buf0[1] = strlen(buf);
                write(sockfd, buf0, sizeof(char) * 2); // 서버에서도 보낸 내용을 확인하기 위해
                break;
            }

            case DATA:
            {
                // 3|길이|아이디|데이터|타임스탬프
                // 3 12 01 1234567890
                // 0x01 0x02 0304050607080900
                // 데이터 검증 결과를 라즈베리 기기
                // 서버에서 검증하려면 > 서명값까지 받고
                // 안하려면 검증결과만
                // DATA || 길이 || 아이디 || 먼지 데이터 || 타임스탬프 -> 아이디 분류  
                // if 아이디 01 -> 01을 제외하고 검증결과 수집 -> .txt로 저장
                /*char buf[MAX_BUF_SIZE] = { 0x03, 0x00, 0x0c, 0x00, 0x01, 0x01, 0x02,
                                                                0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00,
                                                                    };
                */
                char id4[3]= { 0x00, };

                strncpy(id4, &buf[3], sizeof(char)*2);
                printf("ID : %s\n\n", id4);

                char Id[128] = {0,};
                sprintf(Id,"Id%s.txt",id4);
                fp = fopen(Id, "a"); 
                printf("ID : %s\n\n", Id);

                printf("data : ");

                for (int i = 5; i < 15; i++)
                {
                    printf("%c", buf[i]);
                    fprintf(fp, "%c", buf[i]);
                }

                // 먼지 센서랑 연결했을 때 써야하는 코드
                // printf("%d-%02d-%02d %02d:%02d:%02d\n", sendbufTIME[0], sendbufTIME[1], sendbufTIME[2], sendbufTIME[3], sendbufTIME[4], sendbufTIME[5]);

                // 임시로 buf를 입력되어 있는 걸 쓸 때 쓰는 코드
                printf(" %s 00\n", &buf[15]);
                fprintf(fp, " %s 00 \n", &buf[15]);
                
                fclose(fp);

                char buf0[2] = {0x03,0x01};
                write(sockfd, &buf0, sizeof(char) * 2);
                memset(buf, 0x00, MAX_BUF_SIZE);
                buf[0] = 0x01;
                break;
            }

            case DATA_VERI:
            {
                // 4|길이|전파아이디|검증아이디|데이터|검증결과
                // 4 15 01 02 1234567890 1 
                // 데이터 검증 결과

                FILE* dust_fp = NULL;

                char Id[128];
                char dustdata[11] ={0x00,};   
                char verified_data[11] ={0x00,};     
                char data_id[3];  //데이터를 전파한 아이디
                char veri_id[3];  //검증한 아이디
                
                float verification = 0;
                char buf1[128] = { 0x00, };
                int i = 0;

                strncpy(data_id, &buf[3], sizeof(char)*2);
                strncpy(veri_id, &buf[5], sizeof(char)*2);
                strncpy(verified_data, &buf[7], sizeof(char)*10);
                printf("%s\n\n", data_id);

                sprintf(Id,"Id%s.txt",data_id);
                fp=fopen(Id,"r+");
                
                while (!feof(fp))
                {
                    fgets(buf1, sizeof(buf1), fp);
                    i++;
                    strncpy(dustdata, &buf1[0], sizeof(char)*10);
                    if (strcmp(dustdata, verified_data) == 0)
                    {
                        if (data_id != veri_id)
                        {
                            verification = veri(buf, i, Id);
                            
                            memset(buf,0x00,MAX_BUF_SIZE);
                            if (verification > 0.5)
                            {
                                sprintf(Id,"Dustdata%d.txt",Block_Num);
                                dust_fp=fopen(Id,"a");
                                assert(dust_fp != NULL);

                                fprintf(dust_fp,"%s %s\n",data_id,verified_data); 
                                buf[0] = 0x01;  
                                fclose(dust_fp);
                            }
                            
                            else
                            {
                                buf[0] = 0x03;
                            }
                            
                            printf("if pass2\n");
                            
                            
                            //검증 성공 여부에서 검증했을때 0.5이하로 나와 파일에 올라가지 않은 경우는??

                            break;
                            printf("break pass\n");
                        }

                        else
                        {
                            memset(buf,0x00,MAX_BUF_SIZE);
                            buf[0] = 0x02;
                            break;
                        }                 
                    }
                    else
                    {
                        continue;
                    }
                }

                printf("while break\n");
                fclose(fp);
        

                char buf0[2] = {0x04,0x01};
                write(sockfd, &buf0, sizeof(char) * 2);
                memset(buf,0x00,MAX_BUF_SIZE);
                buf[0] = 0x01;
                break;
            }

            case BLOCK_CHECK:
            {
                // BLOCK_CHECK : Client가 요청하면 현재 BLOCK_NUM-1을 전송

                // 현재 코드는 제일 최근에 만들어진 블록만 전송
                send_block(sockfd,buf);
                char buf0[2] = {0x05,0x01};
                write(sockfd, &buf0, sizeof(char) * 2);
                memset(buf,0x00,MAX_BUF_SIZE);
                buf[0] = 0x01;    
                break;
            }

            // 그래서 Client는 받은 숫자와 자신이 현재 갖고 있는 블록 파일명을 확인해서 없는 번호를 찾고
            // 프로토콜 BLOCK을 이용하여 받기

            /*
            case BLOCK
            {
                // 6|길이|받을 블록의 Num
            }
            */

            default:
             break;
            }
            #endif
        }
        
        else
        {
            write(sockfd,"Data length is Wrong!",sizeof(char)*20);
        }
        
        printf("%s\n", buf);
        printf("buf length : %d\n", strlen(buf));
        write_len = write(sockfd, buf, strlen(buf)); // 서버에서도 보낸 내용을 확인하기 위해
        printf("Write len : %d\n",write_len);
        memset(buf, 0x00, MAX_BUF_SIZE);
    }
    fp = fopen("ip.txt","r+");
    FILE* fp_new = NULL;
    fp_new = fopen("ip_del.txt","w");
    assert(fp_new != NULL);
    
    while(1)
    {
        char line[25];
        int file_port =0;
        fgets(line, 25, fp);
        if(feof(fp))    break;

        for (i = 3  ; i < 8 ; i++)
            file_port = file_port * 10 + (line[i]-'0');

        if (file_port != ntohs(client_addr.sin_port))
            fprintf(fp_new, "%s", line);
    }
    
    fclose(fp_new);
    fclose(fp);

    system("rm -rf ip.txt");

    if ( rename("ip_del.txt", "ip.txt") != 0 ) {
        printf("file name error\n");
    }

    // client 삭제
    for(i=0; i<clnt_cnt; i++)
    {
        if(sockfd == clnt_socks[i])
        {
            while(i < clnt_cnt)
            {
                clnt_socks[i] = clnt_socks[i+1];
                i++;
            }
            break;
        }
    }
    clnt_cnt--;

    close(sockfd);
    MAX_RASP -= 1;
    printf("Worker Thread end\n");

    return 0;
}

void* thread_func_block(void* data)
{
    int sockfd = *((int*)data);
    Block* A = NULL;
    char block[128];
    char buff[MAX_BUF_SIZE];
    int i;
    FILE* fp = NULL;

    while(1)
    {
        memset(buff, 0x00, MAX_BUF_SIZE);
        
        printf("sleep before\n\n");
        sleep(13);
        printf("sleep after\n\n");

        Block_Create = MAX_RASP;
        printf("In Block Create : %d, Max ras %d\n", Block_Create, MAX_RASP);
        
        init(&A);
        printf("init pass\n");
        
        createblock(&A, Block_Num, MAX_RASP);
        printf("createblock pass\n");            
        
        sprintf(block, "block%d.txt", Block_Num);
        fp = fopen(block, "r");
        printf("fopen pass\n");
        
        fread(buff, sizeof(buff), 1, fp);
        printf("fread pass\n");
        
        for(i=0; i<clnt_cnt; i++)
            write(clnt_socks[i], buff, sizeof(buff));
        printf("write pass\n");
        
        B_free(&A);
        Block_Num++;
        printf("%d\n",Block_Num);
    }
}

int main(int argc, char** argv)
{
    printf("MAX_RASP : %d\nBlock_Num : %d\nBlock_Create : %d\n\n", MAX_RASP, Block_Num, Block_Create);

    int listen_fd, client_fd;
    socklen_t addrlen;
    int readn;
    char buf[MAX_BUF_SIZE];
    pthread_t thread_id;
    struct sockaddr_in server_addr, client_addr;
    time_t start,end;
    
    FILE* fp = NULL;
    int i = 0; int n;
    
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return 1;
    memset((void*)&server_addr, 0x00, sizeof(server_addr));
    printf("listen_fd = %d\n", listen_fd);

    int option = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(12222);

    if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind error");
        return 1;
    }

    if(listen(listen_fd, 5) == -1)
    {
        perror("listen error");
        return 1;
    }

    start = time(NULL);
    end = time(NULL);

    // 블록 만드는 thread
    pthread_create(&thread_id, NULL, thread_func_block, (void*)&listen_fd);
    pthread_detach(thread_id);

    while(1)
    {        
        addrlen = sizeof(client_addr);
        client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
        clnt_socks[clnt_cnt++] = client_fd;

    #if 1
        // 블록 넣기
        if(client_fd == -1)
        {
            printf("accept error\n");
        }

        else
        {
            for(i=0;i < clnt_cnt; i++)
                printf("clnt_socks[%d] = %d\n", i, clnt_socks[i]);
        
            /* p.376 새로운 (워커)스레드를 생성하는 함수
                int pthread_create(스레드 식별 번호, 스레드 특성(기본은 NULL),
                                만들고자 하는 스레드 함수 포인터, 스레드 함수 실행될 때 넘어갈 매개변수); */
            // 워커 스레드는 클라이언트와 통신해야하기 때문에 매개변수에 client_fd 사용
            pthread_create(&thread_id, NULL, thread_func, (void*)&client_fd);
            printf("pthread_id : %d\n\n", thread_id);
            /* p.377, p.381 워커 스레드를 메인 스레드로부터 분리시키는 함수
            : pthread_join 함수를 쓰면 워커 스레드가 종료 신호를 보낼 때까지 메인 스레드는 봉쇄되어 다음 accept 함수 사용 불가
            > detach 함수를 사용하여 워커 스레드의 종료를 기다리지 않음
                int pthread_detach(스레드 식별 번호); */
            pthread_detach(thread_id);

            // ip.txt에 있는 걸 삭제
            printf("end else\n\n");
        }
    #endif
    }

    getchar();

    return 0;
    
}

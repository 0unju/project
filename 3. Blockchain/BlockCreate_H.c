#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <openssl/sha.h>

//#define MAX_RASP 3
#define MAX_BUF_SIZE 1024

#if 0
typedef struct { 
     int tm_sec; /* 초 - [0～61] （閏秒を考慮） */ 
     int tm_min; /* 분 - [0～59] */ 
     int tm_hour; /* 시 - [0～23] */ 
     int tm_mday; /* 일 - [1～31] */ 
     int tm_mon; /* 월 - [0～11] */ 
     int tm_year; /* 1900부터의 년 */ 
 } tm; 
#endif

typedef struct Header {
   struct tm timestamp; // 타임스탬프
   int num_of_rasp; // (검증에 참여한) 전체 RaspberryPi의 개수
   unsigned char pre_hash[32]; // 이전 블록 해시
}Header;

typedef struct Block {   
   struct Header *Head; // 블록 헤더
   int size_block; // 블록 크기
   int num_of_data; // 데이터 개수
   //unsigned char** data; // 데이터
}Block;

//export MALLOC_CHECK_=0

void init(Block** B)
{
	(*B) = (Block*)calloc(1,sizeof(Block));
	(*B)->size_block = 0;
	(*B)->num_of_data = 0;	

	(*B)->Head = (Header*)calloc(1,sizeof(Header));
	(*B)->Head->num_of_rasp = 0;

	for (int i = 0; i < 32; i++)
	{
		(*B)->Head->pre_hash[i] = 0;
	}

	(*B)->Head->timestamp.tm_year = 0;
	(*B)->Head->timestamp.tm_mon = 0;
	(*B)->Head->timestamp.tm_mday = 0;
	(*B)->Head->timestamp.tm_hour = 0;
	(*B)->Head->timestamp.tm_min = 0;
	(*B)->Head->timestamp.tm_sec = 0;
}

#if 0
void time_1(struct tm* t,Block** x)
{
	(*x)->Head->timestamp->tm_year = t->tm_year + 1900;
	(*x)->Head->timestamp->tm_mon = t->tm_mon + 1;
	(*x)->Head->timestamp->tm_mday = t->tm_mday;
	(*x)->Head->timestamp->tm_hour = t->tm_hour;
	(*x)->Head->timestamp->tm_min = t->tm_min;
	(*x)->Head->timestamp->tm_sec = t->tm_sec;
    printf("TIME_1) Timestamp : \n%d-%02d-%02d %02d:%02d:%02d\n",(*x)->Head->timestamp->tm_year, (*x)->Head->timestamp->tm_mon,
         (*x)->Head->timestamp->tm_mday, (*x)->Head->timestamp->tm_hour, (*x)->Head->timestamp->tm_min, (*x)->Head->timestamp->tm_sec );
}
#endif

void Pre_Hash(Block** A,int num)
{
   FILE* fp = NULL;
   char block[100]={0,};
   char buf[1024]={0,};
   char buf1[1024]={0,};
   char hash[32]={0,};
   int i;
   int j = 0;

   if (num == 0)
   {
      (*A)->Head->pre_hash[0] = 0;
   }

   else
   {
      sprintf(block,"block%d.txt",num-1);

      fp=fopen(block,"r");

      for (i = 0; i < 6; i++)
      {
         memset(buf,0x00,1024);
         fgets(buf,sizeof(buf),fp);
         if (i % 2 ==1)
         {
            strncpy(&buf1[j],buf,strlen(buf));
            j = strlen(buf);

         }   
      }

    	SHA256(buf1,strlen(buf1),hash);
    	//strncpy((*A)->Head->pre_hash,hash,strlen(hash));
		strncpy((*A)->Head->pre_hash,hash, 32);
		//printf("copy hash len : %d\n", strlen(hash));
   }
}

void createHeader(Block** x,int Block_Num,int MAX_RASP)
{
	FILE* fp = NULL;
	char block[100];
	sprintf(block, "block%d.txt", Block_Num);
	fp = fopen(block, "w");
	assert(fp != NULL);
	time_t current_time = time(NULL);
	struct tm* t = localtime(&current_time);
	Pre_Hash(x,Block_Num);

	//time_1(t, x);
	(*x)->Head->num_of_rasp = MAX_RASP;
	(*x)->Head->timestamp.tm_year = t->tm_year + 1900;
	(*x)->Head->timestamp.tm_mon = t->tm_mon + 1;
	(*x)->Head->timestamp.tm_mday = t->tm_mday;
	(*x)->Head->timestamp.tm_hour = t->tm_hour;
	(*x)->Head->timestamp.tm_min = t->tm_min;
	(*x)->Head->timestamp.tm_sec = t->tm_sec;

	printf("Timestamp : \n%d-%02d-%02d %02d:%02d:%02d\n",(*x)->Head->timestamp.tm_year, (*x)->Head->timestamp.tm_mon,
			(*x)->Head->timestamp.tm_mday, (*x)->Head->timestamp.tm_hour, (*x)->Head->timestamp.tm_min, (*x)->Head->timestamp.tm_sec);
	fprintf(fp, "Timestamp : \n%d-%02d-%02d %02d:%02d:%02d\n",(*x)->Head->timestamp.tm_year, (*x)->Head->timestamp.tm_mon,
			(*x)->Head->timestamp.tm_mday, (*x)->Head->timestamp.tm_hour, (*x)->Head->timestamp.tm_min, (*x)->Head->timestamp.tm_sec);
//	printf("TIME_2) Timestamp : \n%d-%02d-%02d %02d:%02d:%02d\n",(*x)->Head->timestamp->tm_year, (*x)->Head->timestamp->tm_mon,
//			(*x)->Head->timestamp->tm_mday, (*x)->Head->timestamp->tm_hour, (*x)->Head->timestamp->tm_min, (*x)->Head->timestamp->tm_sec );
//	fprintf(fp, "Timestamp : \n%d-%02d-%02d %02d:%02d:%02d\n",(*x)->Head->timestamp->tm_year, (*x)->Head->timestamp->tm_mon,
//			(*x)->Head->timestamp->tm_mday, (*x)->Head->timestamp->tm_hour, (*x)->Head->timestamp->tm_min, (*x)->Head->timestamp->tm_sec );

	fprintf(fp, "Number of Node : \n%d\n", (*x)->Head->num_of_rasp);
	fprintf(fp, "Pre Hash : \n");

	for (int i = 0; i < 32; i++)
	{
		fprintf(fp, "%02x ", (*x)->Head->pre_hash[i]);
	}
	fprintf(fp, "\n");

	printf("sizeof(num_of_rasp) = %d\n", sizeof((*x)->Head->num_of_rasp)); //4	
	printf("sizeof(pre_hash) = %d\n", sizeof((*x)->Head->pre_hash));	// 32
	printf("sizeof(timestamp) = %d\n", sizeof((*x)->Head->timestamp));	//44
	printf("sizeof(Header) = %d\n", sizeof((*x)->Head)); //4
	printf("sizeof(Header) = %d\n", sizeof(Header));// 80
	fclose(fp);
}

int size_count(int cnt)
{
   int ret = 0;
   ret = cnt / 100;
   switch (ret)
   {
   case 0:
      if(cnt / 10 == 0) return 0;
      else return 1;
      break;
   default:
      return 2;
      break;
   }
}

void createblock(Block** x,int Block_Num,int MAX_RASP)//,unsigned char data[32][13]) 
{
	FILE* fp = NULL;
	FILE* fp_D = NULL;    
	char data[100];
	char block[100];
	char buf[MAX_BUF_SIZE];
	char buf_D[MAX_BUF_SIZE];
	char buff[MAX_BUF_SIZE];

	memset(buf, 0x00, MAX_BUF_SIZE);
	memset(buf_D, 0x00, MAX_BUF_SIZE);
	memset(buff, 0x00, MAX_BUF_SIZE);

	sprintf(data, "Dustdata%d.txt", Block_Num);
	fp_D = fopen(data, "r");
	assert(fp_D != NULL);
	printf("fopen pass\n");
	sprintf(block, "block%d.txt", Block_Num);
	fp = fopen(block, "a");
	assert(fp != NULL);
	printf("fopen pass22\n");

	int count = 0;
	createHeader(x, Block_Num, MAX_RASP);
	printf("createHeader pass\n");
	fread(buf, sizeof(buf), 1, fp);
	printf("fread pass11\n");
	fread(buf_D, sizeof(buf_D), 1, fp_D);
	printf("fread pass\n");
    fseek(fp_D, 0, SEEK_END);    
    count = ftell(fp_D) / 13;          

	fprintf(fp, "Size of block :      \n");
	(*x)->num_of_data = count;
	fprintf(fp, "Number of data : %d\n", (*x)->num_of_data);
	fprintf(fp, "Data :\n");
	fprintf(fp, "%s", buf_D);
	fclose(fp);
	fclose(fp_D);

	fp = fopen(block, "r+");
    assert(fp != NULL);

	int size, A, B;
    fseek(fp, 0, SEEK_END);    
    size = ftell(fp);          
    printf("%d\n", size); //97

	printf("MAX_RASP = %d, size = %d\n", size_count(MAX_RASP), size_count(size));

	size = size + size_count(MAX_RASP);//98 +1  > 10
	A = size_count(size); // 두자리면 A = 1

	(*x)->size_block = size;
	fseek(fp, 178 + size_count(MAX_RASP), SEEK_SET);
	fprintf(fp, "%d", (*x)->size_block);
    fclose(fp);
}

void B_free(Block** B)
{
   /*
   free((**B)->Head->timestamp);
   (**B)->Head->timestamp = NULL;
   free((**B)->Head);
   (**B)->Head = NULL;
   free(**B);
   **B = NULL;
   free(*B);
   *B = NULL;
   */
	free((*B)->Head);
	(*B)->Head = NULL;
	free(*B);
	*B = NULL;     
}

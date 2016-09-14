#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<stdlib.h>
#include"fat32.h"
#include<string.h>

#define INVALID 0x20;

void boot_info(boot_BPB * , FILE *);
int dir_info(DIR_ENTRY *,FILE *,unsigned int ,unsigned int );
unsigned int get_int(unsigned short , unsigned short );
char long_entry(unsigned char );
void print(unsigned char *,int);



int main(int argc, char* argv[])
{
	FILE *fp;
	boot_BPB *ptr;
	unsigned int Firstdata_sec;
	int ret;
	DIR_ENTRY *dir_buff;

	if(argc!=2){
		printf("Usage:- < %s file_name >",argv[0]);
		return -1;
	}
	if((fp =fopen (argv[1],"rb"))==NULL){
		printf("ERROR\n");
		return -1;
	}

	ptr =(boot_BPB *) malloc(sizeof(boot_BPB));
	dir_buff =(DIR_ENTRY *) malloc(sizeof(DIR_ENTRY));	
	boot_info(ptr,fp);

	Firstdata_sec =( (ptr->BPB_RsvdSecCnt) + (ptr->BPB_NumFATs) * (ptr-> BPB_FATSz32) ) * (ptr->BPB_BytsPerSec);// first Data sector

	rewind(fp);
	ret =dir_info(dir_buff,fp,Firstdata_sec,2);

	fclose(fp);
	return 0;
}
int dir_info(DIR_ENTRY *dir_buff,FILE *fp,unsigned int Firstdata_sec,unsigned int N)
{


	int i,j,no;
	unsigned char *buff,ret,ch;
	unsigned int Nextdata=0;

	unsigned long offset;

//	offset=ftell(fp);
	rewind(fp);

	Nextdata=Firstdata_sec + (N-2)*0x200; 
	fseek(fp,Nextdata,SEEK_SET);



	while(1){
		fread(&ch,1,1,fp);
		fseek(fp,-1,SEEK_CUR);

		if(ch==0xe5 || ch==0x2e || ch==0x20){
			fseek(fp,+32,SEEK_CUR);
			continue;
		}


		if(ch==0x00){
			return 0;
		}


		ret=long_entry(ch);
		no =32*ret + 32;

		buff = (char*)malloc(no);
		fread(buff,no,1,fp);
		
		
		memcpy(dir_buff,&buff[no-32],32);


		if(dir_buff->DIR_Attr & 0x10)
		{

			
			printf("\t Directory\t");
			
			print(buff,ret); // directory name;
	

			N= get_int(dir_buff->DIR_FstClusHI,dir_buff->DIR_FstClusLO);	       
			offset=ftell(fp);

			dir_info(dir_buff,fp,Firstdata_sec,N);

			fseek(fp,offset,SEEK_SET);
		}
		
		else
		{
			
			printf("\t file    \t");
			print(buff,ret); // print file name;
		
		}

//		free(buff);
	
}
}

void boot_info(boot_BPB * ptr, FILE *fp)
{

	fread(ptr,512,1,fp);

	printf("BS_jmpBoot=%x %x %x\n",ptr->BS_jmpBoot[2],ptr->BS_jmpBoot[1],ptr->BS_jmpBoot[0]);

	printf("BPB_BytePerSec=%x\n",ptr->BPB_BytsPerSec);
	printf("BPB_SecPerClus=%d\n",ptr->BPB_SecPerClus);
	printf("BPB_NumFATs=%d\n",ptr->BPB_NumFATs);
	printf("RsvdSecCnt=%X \n",ptr->BPB_RsvdSecCnt);
	printf("BPB_FATSz32=%x\n",ptr->BPB_FATSz32);

}
unsigned int get_int(unsigned short num1, unsigned short num2)
{
	unsigned int n1,n2,result;
	n1=n2=result=0;

	n1=num1;
	num1=num1 << 16;

	n2=num2;

	result=(n1 | n2);
	return result;
}

char long_entry(unsigned char ch)
{
	unsigned int a; 

	if(ch == 0x20 || ch== 0x2E || ch == 0x00 )
	{
		return ch;
	}

	if((ch & 0xF0) == 0x40)
	{
		ch = ch & 0x0F;
		return ch;
	}
	else
		return INVALID;
}

void print(unsigned char *buff,int ret)
{
	int i,j;

	for(i=ret ;i > 0;i--){
	
		for(j=32*(i-1);j < 32*i;j++){
			if(j==0)  // skip first byte;
			continue;

			if (isprint(buff[j])!=0){
				printf("%c",buff[j]);
			}

		}
		printf("\n");
	}
}

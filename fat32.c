#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<stdlib.h>
#include"fat32.h"
#include<string.h>

void boot_info(boot_BPB * ptr, FILE *fp);
int dir_info(DIR_ENTRY *dir_buff,FILE *fp,unsigned int Firstdata_sec,unsigned int N);
unsigned int get_int(unsigned short num1, unsigned short num2);


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

	Firstdata_sec =( (ptr->BPB_RsvdSecCnt) + (ptr->BPB_NumFATs) * (ptr-> BPB_FATSz32) ) * (ptr->BPB_BytsPerSec);


rewind(fp);
ret =dir_info(dir_buff,fp,Firstdata_sec,2);

fclose(fp);
return 0;
}
int dir_info(DIR_ENTRY *dir_buff,FILE *fp,unsigned int Firstdata_sec,unsigned int N)
{


	int i,j;
	unsigned char buff[64];
	unsigned int Nextdata=0;

	unsigned long offset;
	
	offset=ftell(fp);
	rewind(fp);

	Nextdata=Firstdata_sec + (N-2)*0x200;
	fseek(fp,Nextdata,SEEK_SET);

	while(1){
		fread(buff,1,1,fp);

		if(buff[0]=='A'){			
			fread(&buff[1],63,1,fp);
			memcpy(dir_buff,&buff[32],sizeof(DIR_ENTRY));
		}

		else{
			fread(&buff[1],31,1,fp);
			memcpy(dir_buff,&buff[1],sizeof(DIR_ENTRY));

		}


		if(buff[0]==0xe5 || buff[0]==0x2e || buff[0]==0x20)
			continue;

		if(dir_buff->DIR_Name[0]==0x00){
			return 0;
		}

		if(dir_buff->DIR_Attr & 0x10 ){
			unsigned char name[11];
			int i,j;

			for(j=0,i=0;i<11;i++){
				if(dir_buff->DIR_Name[i]==0x20)
					continue;
				name[j++]=dir_buff->DIR_Name[i];
			}

			name[j]='\0';
			printf("DIR_Name=%s\n",name);

			N= get_int(dir_buff->DIR_FstClusHI,dir_buff->DIR_FstClusLO);	       
			offset=ftell(fp);

			dir_info(dir_buff,fp,Firstdata_sec,N);
			fseek(fp,offset,SEEK_SET);
		}

		else {			
			printf("File_Name=%s\n",dir_buff->DIR_Name);
		}
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

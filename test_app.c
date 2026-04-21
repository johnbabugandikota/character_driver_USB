#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

int8_t write_buf[1024];
int8_t read_buf[1024];


void menu(){
	printf("options are :\n");

	printf("1. read\n");
	printf("2.write\n");
	printf("3.close\n");
}

int main(){
	int fd;
	char option;
	fd=open("/dev/john_usb_device",O_RDWR);
	if(fd<0){
		printf("cannot open device\n");
		return 0;
	}
	while(1){
		menu();
		printf("enter the option\n");
		scanf(" %c",&option);
		switch(option){
		case '1': printf("Reading from driver \n");
			  read(fd,read_buf,1024);
			  printf("done\n");
			  printf("data being read: %s\n",read_buf);
			  break;
		case '2': printf("enter string to write to driver\n");
			scanf(" %[^\t\n]s",write_buf);
			printf("data writing\n");
			write(fd,write_buf,1024);
			break;
		case '3' : close(fd);
			   exit(1);
			  break;
		default: printf("enter valid option\n");
			 break;
		}
	}
	close(fd);
}

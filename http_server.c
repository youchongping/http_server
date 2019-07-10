#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "user_method.h"
#define MAX_RCV_SIZE 3000
const char* header = "HTTP/1.1 200 OK\r\nContent-type: text\r\n\r\n";
int GET_cb(int client_fd,char* full_buf,int full_len)
{
	fprintf(stdout,".......%s \n",__FUNCTION__);
 	char buf[1024] = {0};
	send(client_fd,header,strlen(header),0);

	memset(buf,0,sizeof(buf));
	snprintf(buf,sizeof(buf),"every thing is ok\r\n");
	if(send(client_fd, buf, strlen(buf), 0)<0)
	{
		perror("send");
		return 1;
	}

	
	return 0;
		
}

int POST_cb(int client_fd,char* full_buf,int full_len)
{
 	fprintf(stdout,".......%s \n",__FUNCTION__);
	send(client_fd,header,strlen(header),0);
	return 0;
}

int HEAD_cb(int client_fd,char* full_buf,int full_len)
{
	fprintf(stdout,".......%s \n",__FUNCTION__);
	send(client_fd,header,strlen(header),0);
	return 0;
}
struct method methods[]=
{
	{"GET",GET_cb},
	{"POST",POST_cb},
	{"HEAD",HEAD_cb}
};

int request_process(int client_fd,char* full_buf,int full_len)
{
	int i = 0;
	fprintf(stdout,"rcv %d : \n%s \n",full_len,full_buf);
	for(i=0;i<ARRAY_SIZE(methods);i++)
	{
		if(strstr(full_buf,methods[i].str)!=NULL)
		{
				methods[i].cb(client_fd,full_buf,full_len);
		}
		
	}
	if(i>=ARRAY_SIZE(methods))
	{
		fprintf(stdout,"method not support!");
	}
	
}
/*************************************************************************/
int main(int argc,char** argv)
{
	int server_fd = -1;
	int client_fd = -1;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t client_addrlen;
	unsigned short port;
	/*socket*/
	server_fd = socket(AF_INET,SOCK_STREAM,0);
	if(server_fd<0)
	{
		perror("socket()");
		exit(EXIT_FAILURE);
	}
	/*bind*/
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr("172.16.0.12");
	if(bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
	{
		perror("bind()");
		close(server_fd);
		exit(EXIT_FAILURE);
	}
	
	/*if port==0 ,allocating a port*/
	if (port == 0)  /* if dynamically allocating a port */
    	{
        	socklen_t namelen = sizeof(server_addr);
        	if (getsockname(server_fd, (struct sockaddr *)&server_addr, &namelen) == -1)
            	{
			perror("getsockname()");
			close(server_fd);
			exit(EXIT_FAILURE);
		}
        	port = ntohs(server_addr.sin_port);
    	}

	/*listen*/
	if(listen(server_fd,5)<0)
	{
		perror("listen()");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	printf("httpd running on address:http://%s:%d\n", inet_ntoa(server_addr.sin_addr),port);
	
	int r = 0;
	char buf[128]={0};
	char full_buf[MAX_RCV_SIZE]={0};
	int  full_len = 0;
	client_addrlen = sizeof(client_addr);
	while(1)
	{
		/*accept*/
		client_fd = accept(server_fd,(struct sockaddr*)&client_addr,&client_addrlen);
		fprintf(stdout,".......%s \n",__FUNCTION__);
		if(client_fd < 0)
		{
			perror("accept()");
			//close(client_fd);
			continue;
		}
		else
		{
			printf("client address:%s:\n", inet_ntoa(client_addr.sin_addr));
			do
			{
				memset(buf,0,sizeof(buf));
				/*recv*/
				r = recv(client_fd,buf,sizeof(buf),0);
				full_len += r;
				strncat(full_buf,buf,r);
				printf("r:%d \n",r);
				if(r<sizeof(buf))
					break;

			}while(r>0);			
			/*request_process*/
			request_process(client_fd,full_buf,full_len);
			close(client_fd);
		}
	}
	close(server_fd);
	close(client_fd);
	exit(0);
	
}

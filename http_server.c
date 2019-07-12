#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "user_method.h"
#include <time.h>
#define MAX_RCV_SIZE 3000
#define CACHE_SIZE 512
#define SERVER_PORT "1989"

char* header = "HTTP/1.1 200 OK\r\nServer: http_server_demo\r\nContent-type: text\r\n";
char* header_no_method = "HTTP/1.1 405 Method Not Allowed\r\n";
void get_len(const char* buf,int* header_len,int* content_len);

char* now_time(void)
{
	time_t t;
	time(&t);
	return asctime(gmtime(&t));
}

int send_header(int client_fd,int body_len,int flag)
{
	char* buf = (char*)malloc(CACHE_SIZE);
	bzero(buf,CACHE_SIZE);
	if(flag)
		snprintf(buf,CACHE_SIZE,"%s""Content-Length: %d\r\n""Date: %s GMT\r\n\r\n",header,body_len,now_time());
	else
		snprintf(buf,CACHE_SIZE,"%s""Content-Length: %d\r\n""Date: %s GMT\r\n\r\n",header_no_method,body_len,now_time());
	
	if(send(client_fd, buf, strlen(buf), 0)<0)
	{
		perror("send");
		return 1;
	}
	free(buf);
	return 0;

}

int send_body(int client_fd,char* content_body,int content_len)
{
	int len = 0;
	int s;

	while(content_len)
	{
		len = (content_len>=CACHE_SIZE)?CACHE_SIZE:content_len;
		s = send(client_fd,content_body,len,0);
		if(s<0)
		{
			perror("send");
			return 1;
		}
		content_body += s;
		content_len -= s;	
	}
	return 0;
}
int GET_cb(int client_fd,char* full_buf,int full_len)
{
	fprintf(stdout,".......%s \n",__FUNCTION__);
	char buf[] = "take easy,everthing is ok";
	
 	send_header(client_fd,strlen(buf),1);
	send_body(client_fd,buf,strlen(buf));	
	return 0;
		
}

int POST_cb(int client_fd,char* full_buf,int full_len)
{
 	fprintf(stdout,".......%s \n",__FUNCTION__);
	int  content_len = 0;
	int  header_len = 0;

	get_len(full_buf,&header_len,&content_len);
 	send_header(client_fd,content_len,1);
	send_body(client_fd,full_buf + header_len,content_len);	

	return 0;
}

int HEAD_cb(int client_fd,char* full_buf,int full_len)
{
	fprintf(stdout,".......%s \n",__FUNCTION__);
	send_header(client_fd,0,1);

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
	int method_exist = 0;
	fprintf(stdout,"rcv %d : \n%s \n",full_len,full_buf);
	for(i=0;i<ARRAY_SIZE(methods);i++)
	{
		if(strstr(full_buf,methods[i].str)!=NULL)
		{
				methods[i].cb(client_fd,full_buf,full_len);
				method_exist = 1;
		}
		
	}
	if(!method_exist)
	{
		fprintf(stdout,"method not support!\n");
		send_header(client_fd,0,0);
	}
	return 0;
	
}
void get_len(const char* buf,int* header_len,int* content_len)
{
	int  rcv_len = 0;
	char len_buf[32]={0};
	char* ptr_start = NULL;
	char* ptr_end = NULL;
	
	ptr_start = strstr(buf,"Content-Length: ");
	if(ptr_start ==  NULL)
	{
		*content_len = 0;
	}
	else
	{
		ptr_start += strlen("Content-Length: ");
		ptr_end = strstr(ptr_start,"\r\n");
		if(((ptr_end - ptr_start)>=0 ) && ((ptr_end - ptr_start) < sizeof(buf)))
		{
			memset(len_buf,0,sizeof(buf));
			memcpy(len_buf,ptr_start,ptr_end - ptr_start);
			*content_len = atoi(len_buf);
		}
	}
		
	ptr_end = strstr(buf,"\r\n\r\n");
	if(ptr_end)
	{
		*header_len = (ptr_end - buf) + strlen("\r\n\r\n");//header lenth should not longer than CACHE_SIZE(512)
	}
	
	fprintf(stdout,"header_len:%d content_len:%d \n",*header_len,*content_len);
	return;
}
/*************************************************************************/
int main(int argc,char** argv)
{
	int server_fd = -1;
	int client_fd = -1;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t client_addrlen;
	/*socket*/
	server_fd = socket(AF_INET,SOCK_STREAM,0);
	if(server_fd<0)
	{
		perror("socket()");
		exit(EXIT_FAILURE);
	}
	/*bind*/
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(SERVER_PORT));
	server_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr("0.0.0.0");
	if(bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
	{
		perror("bind()");
		close(server_fd);
		exit(EXIT_FAILURE);
	}
	
	/*if port==0 ,allocating a port*/
	if (ntohs(server_addr.sin_port) == 0) 
    	{
        	socklen_t namelen = sizeof(server_addr);
        	if (getsockname(server_fd, (struct sockaddr *)&server_addr, &namelen) == -1)
            	{
			perror("getsockname()");
			close(server_fd);
			exit(EXIT_FAILURE);
		}
    	}

	/*listen*/
	if(listen(server_fd,5)<0)
	{
		perror("listen()");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	printf("httpd running on address:http://%s:%d\n", inet_ntoa(server_addr.sin_addr),ntohs(server_addr.sin_port));
	
	int r = 0;
	char* buf = (char*)malloc(CACHE_SIZE);
	char* full_buf = (char*)malloc(MAX_RCV_SIZE);
	int  full_len = 0;
	int  content_len = 0;
	int  header_len = 0;
	int header_parsed = 0;
	char* content_body;
	client_addrlen = sizeof(client_addr);
	while(1)
	{
		/*accept*/
		client_fd = accept(server_fd,(struct sockaddr*)&client_addr,&client_addrlen);
		if(client_fd < 0)
		{
			perror("accept()");
			continue;
		}
		else
		{
			printf("client address:%s %s\n", inet_ntoa(client_addr.sin_addr),now_time());
			do
			{
				memset(buf,0,sizeof(buf));
				/*recv*/
				r = recv(client_fd,buf,CACHE_SIZE,0);
				full_len += r;
				if(full_len<=MAX_RCV_SIZE)
					strncat(full_buf,buf,r);
				//printf("r:%d \n",r);
				if(!header_parsed)
				{
					get_len(full_buf,&header_len,&content_len);
					if(header_len)
						header_parsed = 1;
				}
				if(full_len >= (header_len + content_len) || full_len>MAX_RCV_SIZE)//recv complete
				{
					header_len = 0;
					content_len = 0;
					header_parsed = 0;
					break;
				}

			}while(r>0);			
			/*request_process*/
			if(full_len>MAX_RCV_SIZE)full_len -= r ;
			request_process(client_fd,full_buf,full_len);
			memset(full_buf,0,MAX_RCV_SIZE);
			full_len = 0;
			close(client_fd);
		}
	}
	free(full_buf);
	free(buf);
	close(server_fd);
	exit(0);
	
}

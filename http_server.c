#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX_RCV_SIZE 3000

int main(int argc,char** argv)
{
	int sock_fd = -1;
	int accepted_sock_fd = -1;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t addrlen;
	unsigned short port;
	/*socket*/
	sock_fd = socket(AF_INET,SOCK_STREAM,0);
	if(sock_fd<0)
	{
		perror("socket()");
		exit(EXIT_FAILURE);
	}
	/*bind*/
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sock_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
	{
		perror("bind()");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}
	

	if(listen(sock_fd,5)<0)
	{
		perror("listen()");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}
	
	int r = 0;
	char buf[128]={0};
	char full_buf[MAX_RCV_SIZE]={0};
	int  full_len = 0;
	addrlen = sizeof(client_addr);
	while(1)
	{
		accepted_sock_fd = accept(sock_fd,(struct sockaddr*)&client_addr,&addrlen);
		if(accepted_sock_fd < 0)
		{
			perror("accept()");
			continue;
		}
		else
		{
			do
			{
				memset(buf,0,sizeof(buf));
				r = recv(accepted_sock_fd,buf,sizeof(buf),0);
				if(r<=0)
				{
					perror("listen()");
					close(sock_fd);
					break;
				}
				full_len += r;
				strncat(full_buf,buf,r);

			}while(r>0);
			fprintf(stdout,"rcv %d : \n%s \n",full_len,full_buf);
		}
	}
	
}

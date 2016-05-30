#include <unistd.h>
#include <sys/types.h>       /* basic system data types */
#include <sys/epoll.h>
#include <sys/socket.h>      /* basic socket definitions */
#include <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>       /* inet(3) functions */
#include <netdb.h> 	      /*gethostbyname function */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <queue>
#include <iostream>
#include <pthread.h>
#include <string>
#include <semaphore.h>
#include <fcntl.h>
#include "dht11.h"
#include <sys/procfs.h>
#include <sys/ioctl.h>
using namespace std;

#define MAXLINE							1016
#define MAXBUFSIZE						1024
#define SIZE_OFFSET						2
#define ASD_GENERAL_MESSAGE_HAND_LEN	8
#define MAX_EVENTS						500

///////////////////////////////////////////////////////
typedef struct{
	unsigned char identity;					//指定是请求报文还是反馈报文
	unsigned char size;						//长度
	unsigned char type;						//指定具体报文类型
	unsigned char authgroup;				//指明是否是主监控PC
	unsigned char optional[4];				//网关ID，用于定位到某个网关
	unsigned char s[MAXLINE];				//该字段表示上面提到的具体报文内容
}ASD_GENERAL_MESSAGE;

typedef union{
	unsigned char ucid[4];
	int iid;
}UCID_IID;
///////////////////////////////////////////////////////

int count = 0;
struct Info {
	int equip_num;
	int len;
	char data[1024];
};

//相关全局变量：EPOLL描述符，服务器端监听SOCKET，数据缓冲区以及事件数组
int 				epollfd;
int 				connfd;
struct epoll_event 	eventList[MAX_EVENTS];
queue<Info>			recv_que;
sem_t 				sem;						//semaphore
pthread_t 			a_thread;
pthread_mutex_t 	mutex;
int 				g_maxLen = 1024;
int 				g_remainLen = 0;
char 				recvline[MAXLINE];
char				monitorOptional[4];



/**************************************************
  函数名：setnonblocking
  功能：将通信SOCKET设置为非阻塞
***************************************************/
int setnonblocking(int sockfd)
{
	if ( fcntl( sockfd, F_SETFL, fcntl( sockfd, F_GETFD, 0 ) | O_NONBLOCK ) == -1 )
	{
		return -1;
	}
	return 0;
}

 void recvET(int sockfd)
 {
	//memset(recvline, 0, MAXLINE);
 	while(true)  
 	{
 		bool exit_flag = false;
 		int len = 0;
 		ioctl(sockfd, FIONREAD, &len);
 		if(len>=2){
 			int real_size = 0;
 			int r = recv(sockfd, recvline, SIZE_OFFSET, MSG_PEEK);  
 			if(r == 2) {
 				real_size = recvline[1];
 			} else {
 				printf("errno : %d\n", errno);
 				return;
 			}
 			memset(recvline, 0, MAXLINE);

  			while(true) {
  				int r = recv(sockfd, recvline + g_remainLen, real_size - g_remainLen, 0);  
  				if(r < 0){
										// 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读  
										// 在这里就当作是该次事件已处理处.  
  					if(errno == EAGAIN) {
  						printf("EAGAIN\n");
  						exit_flag = true;
  						break;  
  					} else {
  						return;  
  					}
  				} else if(r == 0) {
										// 这里表示对端的socket已正常关闭. 
  					printf("echoclient: server terminated prematurely\n");
  					break;
  				}

  				g_remainLen += r;
  				if (g_remainLen == real_size) {
  					ASD_GENERAL_MESSAGE package;
  					memcpy(&package, recvline, real_size);
  					printf("Recv from srv, content : %d-%d-%d-%d-%s-%d %d\n", 
  						package.identity, package.size, package.type, 
  						package.authgroup, package.optional, package.s[0], package.s[1]);

  					g_remainLen = 0;
					memcpy(monitorOptional,package.optional,4);
  					memset(recvline, 0, MAXLINE);
  				}

  			}

  			if (exit_flag)
  				break;		

  		}

  	}
  }


/////////////////////////////////////////////////////////////////////////////////////////////////
  void handle(int sockfd)
  {
  	recvET(sockfd);
  }

  void handleEvent(struct epoll_event* pEvent)
  {
  	printf("handleEvent function, HANDLE: %d, EVENT is %d\n",
  		pEvent->data.fd,
  		pEvent->events);

  	if ( (pEvent->events & EPOLLERR) ||
  		(pEvent->events & EPOLLHUP) ||
  		!(pEvent->events & EPOLLIN)
  		)
  	{
  		printf ( "epoll error\n");
  		close (pEvent->data.fd);
  		return;
  	}


		if (pEvent->data.fd == connfd) {//如果发生事件的fd是监听的srvfd，那么接受请求。
			handle(connfd);
		}
		//printf("exit handleEvent\n");
	}


	int EpollHandle()
	{
		/* 创建 epoll 句柄，把监听 socket 加入到 epoll 集合里 */
		epollfd = epoll_create(MAX_EVENTS);
		struct epoll_event event;
		/*
		   EPOLLIN：      表示对应的文件描述符可以读；
		   EPOLLOUT：     表示对应的文件描述符可以写；
		   EPOLLPRI：     表示对应的文件描述符有紧急的数据可读；
		   EPOLLERR：     表示对应的文件描述符发生错误；
		   EPOLLHUP：     表示对应的文件描述符被挂断；
		   EPOLLET：      表示对应的文件描述符有事件发生；
		 */
		   event.events = EPOLLIN | EPOLLET;
		   event.data.fd = connfd;
		   if ( epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event) < 0 )
		   {
		   	printf("epoll Add Failed: fd=%d\n", connfd);
		   	return -1;
		   }

		   printf( "epollEngine startup\n");

		   while (1)
		   {
				/*等待事件发生*/
		   	int nfds = epoll_wait(epollfd, eventList, MAX_EVENTS, -1);
		   	if ( nfds == -1 )
		   	{
		   		printf( "epoll_wait" );
						//continue;
		   	}
				//printf("nfds : %d\n", nfds);
				/* 处理所有事件 */
		   	int n = 0;
		   	for (; n < nfds; n++)
		   		handleEvent(eventList + n);
		   	printf("sth enddown\n");
		   }

		   close(epollfd);
		   close(connfd);
		};

		int ConnectToSrv(char* servInetAddr, int servPort)
		{
			struct sockaddr_in servaddr;
			connfd = socket(AF_INET, SOCK_STREAM, 0);

			bzero(&servaddr, sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			servaddr.sin_port = htons(servPort);
			inet_pton(AF_INET, servInetAddr, &servaddr.sin_addr);

			if (setnonblocking( connfd ) < 0 )
			{
				printf( "setnonblock error" );
				return -1;
			}

			if (connect(connfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1 &&
				errno != EINPROGRESS) {
				printf("Error in connect: %d,%s\n", errno,strerror(errno));
			close(connfd);
			return -1;
			}
	}

	void* SendToSrv(void *arg)
	{
		ASD_GENERAL_MESSAGE conn;
		memset(&conn,0,sizeof(conn));
		conn.identity=121;
		UCID_IID uti;
		uti.iid=123;
		memcpy(conn.optional,uti.ucid,4);
		char addr[]="ZJU YuQuan";
		memcpy(conn.s,addr,sizeof(addr));
		char sendline[MAXBUFSIZE]="";
		int size = ASD_GENERAL_MESSAGE_HAND_LEN + sizeof(addr);
		conn.size=size;
		memcpy(sendline,(unsigned char*)(&conn),conn.size);
		int len = -1;
		while(len <= 0)
			len = write(connfd,sendline,conn.size);
		DHT11 sensor;
		while(true)
		{
			char sendline[MAXBUFSIZE] = "";
			ASD_GENERAL_MESSAGE dht11_data; 
			memset(&dht11_data,0,sizeof(dht11_data));
			memcpy(dht11_data.optional,monitorOptional,4);
			dht11_data.identity = 126;
			
			data get_data=sensor.dht11_read();
			memcpy(dht11_data.s, (unsigned char*)(&get_data), sizeof(get_data));
			int size = ASD_GENERAL_MESSAGE_HAND_LEN + sizeof(get_data);
			dht11_data.size = size;
				//cout<<size<<endl;
			memcpy(sendline, (unsigned char*)(&dht11_data), size);
				//cout<<(int)sendline[8]<<endl;
				// for (int i = 0; i < 5; i++) {
				// 	sendline[0] = i + '0';
			int len = write(connfd, sendline, size);
			//cout<<"Send:"<<sendline<<endl;
			if (len > 0) {
				cout<<"dht11:"<<(int)dht11_data.s[0]<<"  "<<(int)dht11_data.s[1]<<endl;
				//printf("SendData: %d - %s\n", len, sendline);
			} else if (len <= 0) {
				close(connfd);
				printf("SendData:[fd=%d] error[%d]\n", (int)connfd);

			}

		}
	}

	int main(int argc, char **argv)
	{
		char * servInetAddr = "192.168.1.100";
		int servPort = 1117;

				//可以在执行的时候带参数，第一个参数为srv的IP，第二个为srv的端口
		if (argc == 2) {
			servInetAddr = argv[1];
		}
		if (argc == 3) {
			servInetAddr = argv[1];
			servPort = atoi(argv[2]);
		}
		if (argc > 3) {
			printf("usage: echoclient <IPaddress> <Port>\n");
			return -1;
		}

		ConnectToSrv(servInetAddr, servPort);
		int res = pthread_create(&a_thread, NULL, SendToSrv, NULL);  
		if (res != 0)  
		{  
			perror("Thread creation failed!");  
			exit(EXIT_FAILURE);  
		}  
		EpollHandle();
		close(connfd);
		sem_destroy(&sem);
				//pthread_mutex_destroy(&mutex);
		printf("exit\n");
		exit(0);
	}

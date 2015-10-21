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

using namespace std;




// #include <fcntl.h>
// #include <time.h>

#define MAXLINE		1024
//int MAXLINE= 1024;
#define MAX_EVENTS	500

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
		int r = recv(sockfd, recvline + g_remainLen, sizeof(recvline) - g_remainLen, 0);  
		if(r < 0){
		  
			// 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读  
			// 在这里就当作是该次事件已处理处.  
			if(errno == EAGAIN) {
				printf("EAGAIN\n");
				break;  
			}
			else  
				return;  
		}  
		else if(r == 0)  
		{
			// 这里表示对端的socket已正常关闭. 
			printf("echoclient: server terminated prematurely\n");
			break;
		}

		g_remainLen += r;
		if(g_remainLen == g_maxLen) {
			printf("Recv from srv, content : %s\n", recvline);
			g_remainLen = 0;
			memset(recvline, 0, MAXLINE);
		}

		cout << "@@@@" << r << "@@@@" << endl;
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
	event.events = EPOLLIN;// | EPOLLET;
	event.data.fd = connfd;
	if ( epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event) < 0 )
	{
		printf("epoll Add Failed: fd=%d\n", connfd);
		return -1;
	}

	printf( "epollEngine startup\n");
	//char sendline[MAXLINE];// = {'d', 'f', 'v', 'b'};
	// printf("Input : ");
	// scanf("%s", sendline);

	// for (int i = 0; i < 5; i++) {
	// 	sendline[0] = i + '0';
	// 	int len = write(connfd, sendline, MAXLINE);
	// 	if (len > 0) {
	// 		printf("SendData: %d - %s\n", len, sendline);
	// 	} else if (len <= 0) {
	// 		close(connfd);
	// 		printf("SendData:[fd=%d] error[%d]\n", (int)connfd);
	// 	}
	// }
	

	while (1)
	{
		/*等待事件发生*/
		int nfds = epoll_wait(epollfd, eventList, MAX_EVENTS, -1);
		printf("sth happen\n");
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
	char sendline[MAXLINE] = "sfasfsf";
	for (int i = 0; i < 5; i++) {
		sendline[0] = i + '0';
		int len = write(connfd, sendline, MAXLINE);
		if (len > 0) {
			printf("SendData: %d - %s\n", len, sendline);
		} else if (len <= 0) {
			close(connfd);
			printf("SendData:[fd=%d] error[%d]\n", (int)connfd);
		}
	}
}

int main(int argc, char **argv)
{
	char * servInetAddr = "127.0.0.1";
	int servPort = 6888;
	//char buf[MAXLINE];
	//int connfd;
	//struct sockaddr_in servaddr;

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

	printf("welcome to echoclient\n");

	//handle(connfd);     /* do it all */
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

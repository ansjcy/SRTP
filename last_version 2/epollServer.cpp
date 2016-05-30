#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <sys/procfs.h>
#include <sys/ioctl.h>

using namespace std;

#define MAX_EVENTS  500
#define BUFFER_LEN  1024
#define SRV_IP      "172.18.136.218"
#define SRV_PORT    9100
#define LISTEN_QUEUE 100	/*监听SOCKET的等待队列*/
#define MAXLINE                         1016
#define MAXBUFSIZE                      1024
#define SIZE_OFFSET                     2
#define ASD_GENERAL_MESSAGE_HAND_LEN    8


typedef struct{
    unsigned char identity;                 //指定是请求报文还是反馈报文
    unsigned char size;                     //长度
    unsigned char type;                     //指定具体报文类型
    unsigned char authgroup;                //指明是否是主监控PC
    unsigned char optional[4];              //网关ID，用于定位到某个网关
    unsigned char s[MAXLINE];               //该字段表示上面提到的具体报文内容
}ASD_GENERAL_MESSAGE;

//相关全局变量：EPOLL描述符，服务器端监听SOCKET，数据缓冲区以及事件数组
int         epollfd;
int         srvfd;
int			gatewayFd;
int 		monitorFd;
bool		flag = false;
int			count;
char        buffer[BUFFER_LEN];
struct	    epoll_event eventList[MAX_EVENTS];
int 		g_maxLen = 1024;
int 		g_remainLen = 0;
char 		recvline[BUFFER_LEN];



//处理函数
void AcceptConn(int fd);
void RecvData(int fd);
void SendData(int fd);



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


/******************************************************
函数名：initSrvSocket
功能：初始化服务器端监听SOCKET
参数：port 监听端口
*******************************************************/
int initSrvSocket(int port)
{
   struct sockaddr_in srvAddr;
   bzero( &srvAddr, sizeof( srvAddr ) );
   srvAddr.sin_family = AF_INET;
   srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   srvAddr.sin_port = htons (port);

   srvfd = socket( AF_INET, SOCK_STREAM, 0 );
   if ( srvfd == -1 )
   {
       printf( "can't create socket file" );
       return -1;
   }

   int opt = 1;
   setsockopt(srvfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

   if (setnonblocking( srvfd ) < 0 )
   {
       printf( "setnonblock error" );
       return -1;
   }

   if (bind( srvfd, ( struct sockaddr * ) &srvAddr, sizeof( struct sockaddr ) ) == -1 )
   {
       printf( "bind error" );
       return -1;
   }

   if (listen(srvfd, LISTEN_QUEUE) == -1)
   {
       printf( "listen error");
       return -1;
   }
}




/**************************************************
函数名：AcceptConn
功能：接受客户端的链接
参数：srvfd：监听SOCKET
***************************************************/
void AcceptConn(int srvfd)
{
	
    struct sockaddr_in sin;
    socklen_t len = sizeof(struct sockaddr_in);
    bzero(&sin, len);

    //接受来自客户端的连接
    int confd;
   	if(!flag){
		gatewayFd = accept(srvfd, (struct sockaddr*)&sin, &len);
		confd = gatewayFd;
		flag = true;
	}
	else{
		monitorFd = accept(srvfd, (struct sockaddr*)&sin, &len);
		confd = monitorFd;	
		//cout<<"flag is true"<<endl;
	}

    if (confd < 0) {
     printf("%s: bad accept\n");
     return;
 } else {
    printf("Accept Connection: %d\n", confd);
}

setnonblocking(confd);

    //将新建立的连接添加到EPOLL的监听中
struct epoll_event event;
event.data.fd = confd;
event.events =  EPOLLIN | EPOLLET;
epoll_ctl(epollfd, EPOLL_CTL_ADD, confd, &event);
}

void RecvData(int sockfd)
{
	sockfd = gatewayFd;

    while(true)  
    {
        bool exit_flag = false;
        int len = 0;
        ioctl(sockfd, FIONREAD, &len);
		//cout<<len<<endl;
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
                        //printf("EAGAIN\n");
                        exit_flag = true;
                        break;  
                    } else {
                        return;  
                    }
                } else if(r == 0) {
                // 这里表示对端的socket已正常关闭. 
                    printf("echoclient: client terminated prematurely\n");
                    exit_flag = true;
                    break;
                }
				
                g_remainLen += r;
                if (g_remainLen == real_size) {
                    ASD_GENERAL_MESSAGE package;
					memset(buffer,0,MAXLINE);
                    memcpy(&package, recvline, real_size);
					//cout<<(int)recvline[1]<<endl;
					memcpy(buffer,recvline,real_size);
                    //printf("Recv from client, content : %d-%d-%d-%d-%s-%d %d\n", 
                      //  package.identity, package.size, package.type, 
                       // package.authgroup, package.optional, package.s[0], package.s[1]);
					printf("Recv from client, T: %d; H: %d.\n",package.s[0],package.s[1]);
                    g_remainLen = 0;
                    memset(recvline, 0, MAXLINE);
                }

            }
        } else {
			break;
		}
        if (exit_flag)
        	break;      
        
    }
}


//发送数据
void SendData(int fd)
{
	fd = monitorFd;
    //printf("SendData function\n");
    int len;
    len = send(fd, buffer, BUFFER_LEN, 0);
   // len = send(fd, dataTmp, 20, 0);

    if (len > 0) {
        printf("SendData: %d\n", buffer[1]);
    } else if (len <= 0) {
        close(fd);
        printf("SendData:[fd=%d] error[%d]\n", (int)fd);
    }
    //}

}


void handleEvent(struct epoll_event* pEvent)
{
    //printf("handleEvent function, HANDLE: %d, EVENT is %d\n",
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


    if (pEvent->data.fd == srvfd) {//如果发生事件的fd是监听的srvfd，那么接受请求。
        AcceptConn(srvfd);
        printf ( "Accept one connection\n");
    } else {
        RecvData(pEvent->data.fd);
		if(flag){
        	SendData(pEvent->data.fd);
			//cout<<flag<<endl;
		}
     //   epoll_ctl(epollfd, EPOLL_CTL_DEL, pEvent->data.fd, pEvent);
    }

}


int EPollServer()
{

    int  srvPort = 6888;
    initSrvSocket(srvPort);

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
    event.data.fd = srvfd;
    if ( epoll_ctl(epollfd, EPOLL_CTL_ADD, srvfd, &event) < 0 )
    {
        printf("epoll Add Failed: fd=%d\n", srvfd);
        return -1;
    }

    printf( "epollEngine startup：port %d\n", srvPort);

    while(1)
    {
        /*等待事件发生*/
        int nfds = epoll_wait(epollfd, eventList, MAX_EVENTS, -1);
        if ( nfds == -1 )
        {
            printf( "epoll_wait" );
            continue;
        }

        /* 处理所有事件 */
        int n = 0;
        for (; n < nfds; n++)
         handleEvent(eventList + n);
 }


	close(gatewayFd);
	close(monitorFd);
	close(epollfd);
	close(srvfd);
}


int main(int argc, char **argv)
{
    EPollServer();
    return 0;
}

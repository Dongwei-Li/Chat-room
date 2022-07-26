#include <stdio.h>
#include <sys/types.h>     
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

struct sockaddr_in seraddr;//存放服务器属性的结构体
struct Msg{
	char type;
	char name[20];
	char account[20];
    char passwd[20];
	char send_msg[255];
    char send_name[20];	
}msg,msg1;

int Menu();
void Sign_In();
void Sign_Out();
void Group_Chat();
void Private_Chat();
int cfd;
int main()
{
	cfd=socket(AF_INET,SOCK_STREAM,0);
    seraddr.sin_family=AF_INET;
	seraddr.sin_port=htons(9999);
	seraddr.sin_addr.s_addr=inet_addr("192.168.18.138");
    int tmp1=connect(cfd,(struct sockaddr *)&seraddr,sizeof(seraddr));//连接服务器
    if(tmp1 != 0)
    {
        printf("服务器连接失败！\n");
        return -1;
    }
    while(1)
    {
        int a = Menu();
	    switch(a)
	    {
		    case 1:Sign_In();break;	
		    case 2:Sign_Out();break;	
		    case 3:Group_Chat();break;	
		    case 4:Private_Chat();break;	
		    case 0:return 0;	
		    default:printf("输入错误，请重新输入！\n");
	    }
    }	
	return 0;
}
 

void Sign_In()//注册函数
{
    char buf[100];//存放服务器发来的信息，是否注册成功
    msg.type = 1;	
    printf("请设置你的名字：");
    scanf("%s",msg.name);
	printf("请设置你的账号：");
    scanf("%s",msg.account);
    printf("请设置你的密码：");
    scanf("%s",msg.passwd);
    send(cfd,&msg,sizeof(msg),0);//将注册信息发送给服务器
    recv(cfd,buf,100,0);
    printf("%s\n",buf);
    return;
}

void Sign_Out()//销户函数
{
    char buf[100];//存放服务器发来的信息，是否销户成功
    msg.type = 2;
    printf("请输入要销户的名字：");
    scanf("%s",msg.name);
	printf("请输入要销户的账号：");
    scanf("%s",msg.account);
    printf("请输入要销户的密码：");
    scanf("%s",msg.passwd);
    send(cfd,&msg,sizeof(msg),0);//将销户信息发送给服务器
    recv(cfd,buf,50,0);
    printf("%s\n",buf);
    return;
}

void *myrecv(void *arg)//接收服务器发来的群聊消息
{
	while(1)
    {
        char buf1[255];
	    memset(buf1,0,sizeof(buf1));
		read(cfd,buf1,255);	
		printf("%s\n",buf1);
    }
}

void Group_Chat()//群聊函数
{
	memset(&msg,0,sizeof(msg));
    msg.type = 3;
	printf("请输入登录名字：");
    scanf("%s",msg.name);
    printf("请输入登录账号：");
    scanf("%s",msg.account);
    printf("请输入登录密码：");
    scanf("%s",msg.passwd);
    send(cfd,&msg,sizeof(msg),0);//向服务器发送登录群聊的登录信息
	printf("**欢迎来到群聊室**\n");	
	printf("退出请输入：>>\n");
	while(1)
	{
		pthread_t pd;
		pthread_create(&pd,NULL,myrecv,0);
		sleep(1);
		scanf("%s",msg.send_msg);
		if(strcmp(msg.send_msg,">>") == 0)
		{
			pthread_cancel(pd);
			return;
		}
		write(cfd,&msg,sizeof(msg));		
	}
    return;
}


void *my_recv(void *arg)//接收服务器发来的私聊消息
{
	while(1)
    {
	    memset(&msg1,0,sizeof(msg1));
		read(cfd,&msg1,sizeof(msg1));	
		if(strcmp(msg.name,msg1.send_name)==0)
		{
			printf("%s说%s\n",msg1.name,msg1.send_msg);
		}
		else
		{
			continue;
		}
		
    }
}

void Private_Chat()//私聊函数
{
	memset(&msg,0,sizeof(msg));
	msg.type=4;
	printf("请输入登录姓名：");
	scanf("%s",msg.name);
	printf("请输入登录账号：");
    scanf("%s",msg.account);
	printf("请输入登录密码：");
	scanf("%s",msg.passwd);
	send(cfd,&msg,sizeof(msg),0);
    printf("请输入对方姓名：");
    scanf("%s",msg.send_name);
	printf("**欢迎来到私聊室**\n");	
	printf("退出请输入：>>\n");
	while(1)
	{
		pthread_t pd;
		pthread_create(&pd,NULL,my_recv,0);
		sleep(1);
		
		scanf("%s",msg.send_msg);
		if(strcmp(msg.send_msg,">>") == 0)
		{
			pthread_cancel(pd);
			return;
		}
		write(cfd,&msg,sizeof(msg));		
	} 
}

int Menu()//菜单函数
{
    printf("****>>>QQ聊天室<<<****\n");
    printf("*      1、注册       *\n");
    printf("*      2、注销       *\n");
    printf("*      3、群聊       *\n");
    printf("*      4、私聊       *\n");
    printf("*  ~~~~0、退出~~~~   *\n");  
    printf("请输入要执行操作的序号：\n");
    int m;
    scanf("%d",&m);
    return m;
}











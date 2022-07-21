#include<stdio.h>
#include <sys/types.h>     
#include <sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>

struct sockaddr_in seraddr;
struct Msg{
	char type;
	char name[20];
    char passwd[20];
//	char message[255];
}msg;

int Menu();
void Add();
void Destroy();
int cfd;
int main()
{
	cfd=socket(AF_INET,SOCK_STREAM,0);
    seraddr.sin_family=AF_INET;
	seraddr.sin_port=htons(9999);
	seraddr.sin_addr.s_addr=inet_addr("192.168.18.135");
    int tmp1=connect(cfd,(struct sockaddr *)&seraddr,sizeof(seraddr));//链接服务器
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
		    case 1:Add();break;	
		    case 2:Destroy();break;	
		    case 3:;break;	
		    case 4:;break;	
		    case 0:return 0;	
		    default:printf("操作有误！\n请重新输入!\n");
	    }
    }	
	return 0;
}
 
//注册函数
void Add()
{
    char buf[100];
    msg.type = 1;	
    printf("请设置你的名字：\n");
    scanf("%s",msg.name);
    printf("请设置你的密码：\n");
    scanf("%s",msg.passwd);
    send(cfd,&msg,sizeof(msg),0);
    recv(cfd,buf,50,0);
    printf("%s\n",buf);
    return;
}

void Destroy()
{
    char buf[100];
    msg.type = 2;
    printf("请输入要销户的名字：\n");
    scanf("%s",msg.name);
    printf("请输入要销户的密码：\n");
    scanf("%s",msg.passwd);
    send(cfd,&msg,sizeof(msg),0);
    recv(cfd,buf,50,0);
    printf("%s\n",buf);
    return;
}

//菜单函数
int Menu()
{
    printf("**聊天室**\n");
    printf("1、注册\n");
    printf("2、注销\n");
    printf("3、群聊\n");
    printf("4、私聊\n");
    printf("0、退出\n");
    printf("请输入要执行操作的序号：\n");
    int m;
    scanf("%d",&m);
    return m;
}











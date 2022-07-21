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
}msg;

int Menu();
void Sign_In();
void Sign_Out();
int cfd;
int main()
{
	cfd=socket(AF_INET,SOCK_STREAM,0);
    seraddr.sin_family=AF_INET;
	seraddr.sin_port=htons(9999);
	seraddr.sin_addr.s_addr=inet_addr("192.168.18.136");
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
		    case 3:;break;	
		    case 4:;break;	
		    case 0:return 0;	
		    default:printf("输入错误，请重新输入！\n");
	    }
    }	
	return 0;
}
 

void Sign_In()//注册函数
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

void Sign_Out()//销户函数
{
    char buf[100];
    msg.type = 2;
    printf("请输入要销户的名字：");
    scanf("%s",msg.name);
    printf("请输入要销户的密码：");
    scanf("%s",msg.passwd);
    send(cfd,&msg,sizeof(msg),0);
    recv(cfd,buf,50,0);
    printf("%s\n",buf);
    return;
}

int Menu()//菜单函数
{
    printf("****QQ聊天室****\n");
    printf("*   1、注册    *\n");
    printf("*   2、注销    *\n");
    printf("*   3、群聊    *\n");
    printf("*   4、私聊    *\n");
    printf("****0、退出*****\n");  
    printf("请输入要执行操作的序号：\n");
    int m;
    scanf("%d",&m);
    return m;
}











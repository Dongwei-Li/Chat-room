/***************************************************************************
日期：2022-7-26
作者：7号驿栈
功能：基于Linux操作系统，实现一个简易的网络聊天室
	1、可实现账户注册、账户注销、群聊、私聊功能
	2、用到网络编程、多路IO复用、数据库、线程等知识
	3、客户端需在编译的时候添加 -lmysqlclient -L /usr/lib/mysql/plugin
	4、服务端需在编译的时候添加 -lpthread
CSDN博客地址：https://blog.csdn.net/qq_46513803
***************************************************************************/
#include <stdio.h>
#include <sys/types.h>     
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <mysql/mysql.h>

struct sockaddr_in seraddr;
MYSQL my_database;
MYSQL *ret;
struct Msg{//存放客户端输入的内容
	char type;
    char name[20];
	char account[20];
    char passwd[20];
	char send_msg[255];
    char send_name[20];
}msg;

int sfd;
int main()
{
	//创建套接字
	sfd=socket(AF_INET,SOCK_STREAM,0);
    seraddr.sin_family=AF_INET;
	seraddr.sin_port=htons(9999);
	seraddr.sin_addr.s_addr=inet_addr("192.168.18.138");
    //绑定服务端信息
    int tmp=bind(sfd,(struct sockaddr *)&seraddr,sizeof(seraddr));
	if(tmp < 0)
    {
        perror("bind");
        return -1;
    }
	//监听网络
    int tmp1=listen(sfd,127);//这里监听127个客户端
    if(tmp1 < 0)
    {
        printf("监听网络失败！\n");
        return -1;
    }
	//使用select表格  多路IO复用
	int MAX=0;
    fd_set list,lock;
    //清空表格
    FD_ZERO(&lock);
    //将服务端本身的套接字放到表格里
    FD_SET(sfd,&lock);
    MAX=sfd;
    ret = mysql_init(&my_database);   //初始化结构体
	while(1)
	{
		list = lock;
        int tmp2 = select(MAX+1,&list,NULL,NULL,NULL);//监测文件异动，如果产生异动，返回事件的个数
        if(tmp2 < 0)
        {
            perror("select");
            return -1;
        }
		int i;
        for(i=3;i< MAX+1;i++)//遍历表格中的每一个套接字
		{
			if(FD_ISSET(i,&list)) //判断文件描述符是否在表格中
			{
				if(i==sfd)//当读取到服务端本身的套接字时
				{
					struct sockaddr cliaddr; //存放客户端的IP类型，端口号
					socklen_t len;
					int cid = accept(sfd,&cliaddr,&len);//和客户端建立链接 //参数：服务端套接字，客户端核心结构体，结构体大小。返回值：通讯套接字
					FD_SET(cid,&lock);//添加通讯套接字到表格中
					if(MAX < cid)
					{
						MAX=cid;//保证MAX始终为最大套接字
					}
					
					printf("客户端%d上线了!\n",cid);
				}
				else//读取到客户端套接字时
				{					
                    mysql_real_connect(ret,"localhost","root","1","mysql_203",0,NULL,0);   //连接数据库
					memset(&msg,0,sizeof(msg));//初始化客户端存放输入信息的空间
					int lens = read(i,&msg,sizeof(msg));//通过客户端套接字，读取客户端输入内容的长度
					if(lens == 0)//说明没有读取到客户端内容
					{
						close(i);
						printf("客户端%d已下线！\n",i);
						FD_CLR(i,&lock);//删除下线的客户端套接字
					}
					else//当读取到客户端输入的内容时
					{
						int npc = mysql_query(ret,"select * from chat;");//执行sql语句,查找表格中所有数据
						if(npc != 0)
						{
							printf("数据查询失败！\n");
							return -1;
						}
						//获取查询的数据信息
						MYSQL_RES * str = NULL;//初始化数据库结构体
						str = mysql_store_result(ret);//获取执行sql语句后结果的集合
						if(str == NULL)
						{
							printf("获取数据信息异常！\n");
							return -1;
						}
						//获取行的数目
						unsigned int row;
						row=mysql_num_rows(str);//获取查询到的表格的行数
						//获取每一行的信息
						if(msg.type==1)//设置注册为一类信息
						{
							//获取每一行的信息
							if(row == 0)//当数据库表格行数为0时，说明表格中没有任何信息，此时可以插入客户端信息
							{								
								char buf1[100];
								sprintf(buf1,"insert into chat values(\"%s\",\"%s\",\"%s\");",msg.name,msg.account,msg.passwd);
								mysql_query(ret,buf1);  //执行sql语句，插入姓名，账号，密码
								send(i,"账号注册成功！",50,0);//账户注册成功后，向客户端发送注册成功的信息							
								printf("账号注册成功！\n");
							}	
							else//当数据库表格行数不为0时，表示查找这个数据库  找到了对应的记录，不能再进行重复注册
							{																					
								MYSQL_ROW myrow;//存放获取数据库表格行信息的结构体
								int flag;	
								for(int j=0;j<row;j++)//遍历每一行
								{
									flag=0;
									myrow=mysql_fetch_row(str);//获取行的内容
									if((strcmp(myrow[0],msg.name) == 0) && (strcmp(myrow[1],msg.account) == 0))//将行信息中的第一行的第一个数据与输入的名字比较...看是否相同
									{
										flag = 1;
										break;//当找到相同内容时，停止遍历
									}
								}
								if(flag == 1)//当flag==1时，说明找到了相同的内容
								{
									send(i,"信息已存在，账号注册失败！",50,0); 
								}
								else//flag依然为0，说明遍历没有找到相同的内容
								{          							
									char buf[100];
									sprintf(buf,"insert into chat values(\"%s\",\"%s\",\"%s\");",msg.name,msg.account,msg.passwd);
									int npc1 = mysql_query(ret,buf);    //执行sql语句，插入数据
									if(npc1!= 0)
									{
										printf("账户注册失败！\n");
										return -1;
									}
									send(i,"账号注册成功！",50,0);
									printf("账号注册成功！\n");
								}//mysql_free_result(str);   //释放查询信息的结构体
                            }	 //mysql_close(ret);   //断开数据库连接
						}
						else if(msg.type==2)//设置销户为二类信息
						{
							//获取每一行的信息
                            MYSQL_ROW myrow;
                            int flag;
                            for(int j=0;j<row;j++)//遍历每一行
                            {
                                flag=0;
                                myrow=mysql_fetch_row(str);//获取行的内容
                                if((strcmp(myrow[0],msg.name) == 0) && (strcmp(myrow[1],msg.account) == 0) && (strcmp(myrow[2],msg.passwd) == 0))//如果之前注册的信息和现在输入的信息相同，说明找到了要销户的账户
                                {
                                    flag = 1;
                                    break;//当找到了要销户的账户信息，停止遍历
                                }
                            }
                            if(flag == 1)//当找到了要销户的账户信息时
                            {
                                char buf[100];
                                sprintf(buf,"delete from chat  where name = (\"%s\") and account=(\"%s\");",msg.name,msg.account);
                                int npc2 = mysql_query(ret,buf);    //执行sql语句，删除这一行的账户信息
                                send(i,"账号注销成功！",50,0);
								printf("账号注销成功！\n");
                            }
                            else
                            {
                                send(i,"账号或密码错误，账号注销失败！",50,0);
								printf("账号注销失败！\n");
                            }
						}
						else if(msg.type==3)//设置群聊为三类信息
						{
							MYSQL_ROW myrow;
							int flag;
							for(int j=0;j<row;j++)//遍历所有账户信息，判断是否已经注册过账户
							{
								flag=0;
								myrow=mysql_fetch_row(str);
								if((strcmp(myrow[0],msg.name) == 0) && (strcmp(myrow[1],msg.account) == 0) && (strcmp(myrow[2],msg.passwd) == 0))
								{
									flag = 1;
									break;
								}
							}
							if(flag == 1)
							{                          
								printf("%s群聊登录成功!\n",msg.name);
								if(strlen(msg.send_msg) != 0)
								{
									printf("客户端%d发送消息:%s\n",i,msg.send_msg);
									printf("%s说：%s\n",msg.name,msg.send_msg);
									char mesg[255];
									sprintf(mesg,"%s说：%s",msg.name,msg.send_msg);
									for(int k=sfd+1;k<MAX+1;k++)
									{
										if(k != i)      //转发给其他客户端
										{										
											write(k,mesg,sizeof(mesg));											
										}
										else
										{
											continue;
										}
									}
								}
							}
							else
							{              			
								send(i,"群聊登录失败！",255,0);
								printf("群聊登录失败！\n");
							}
                        }
						else if(msg.type == 4)//设置私聊为四类信息
						{														
							MYSQL_ROW myrow;//获取每一行的信息
							int flag;
							for(int j=0;j<row;j++)
							{
								flag=0;
								myrow=mysql_fetch_row(str);
								if((strcmp(myrow[0],msg.name) == 0) && (strcmp(myrow[1],msg.account) == 0) && (strcmp(myrow[2],msg.passwd) == 0))
								{
									flag = 1;
									break;
								}
							}  
							if(flag == 1)
							{                          
								printf("%s私聊登录成功!\n",msg.name);
								if(strlen(msg.send_msg) != 0)
								{
									printf("客户端%d发送消息:%s\n",i,msg.send_msg);
									printf("%s说%s\n",msg.name,msg.send_msg);
									for(int k=sfd+1;k<MAX+1;k++)
									{
										if(k != i)      //转发给其他客户端
										{
											write(k,&msg,sizeof(msg));
										}
										else
										{
											continue;
										}
									}
								}
								
							}
							else
							{              
								printf("客户端输入信息有误，登录失败！\n");
								send(i,"信息有误，登录失败！",255,0);
							}
						}	
						mysql_free_result(str);   //释放查询信息的结构体						
					}
					
				}
			}
		}
	}
	//快速释放底层IP
    int flg=1;
    setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&flg,sizeof(int));
	return 0;
}




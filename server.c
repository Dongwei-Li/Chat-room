#include<stdio.h>
#include <sys/types.h>     
#include <sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include<mysql/mysql.h>

struct sockaddr_in seraddr;
MYSQL my_database;
MYSQL *ret;
struct Msg{//存放客户输入的内容
	char type;
    char name[20];
    char passwd[20];
//	char message[255];
}msg;
int sfd;
int main()
{
	//创建套接字
	sfd=socket(AF_INET,SOCK_STREAM,0);
    seraddr.sin_family=AF_INET;
	seraddr.sin_port=htons(9999);
	seraddr.sin_addr.s_addr=inet_addr("192.168.18.135");
    //绑定服务端信息
    int tmp=bind(sfd,(struct sockaddr *)&seraddr,sizeof(seraddr));
	if(tmp < 0)
    {
        perror("bind");
        return -1;
    }
	//监听网络
    int tmp1=listen(sfd,27);//这里监听27个客户端
    if(tmp1 < 0)
    {
        printf("监听网络失败！\n");
        return -1;
    }
	int MAX=0;
    fd_set list,lock;
    //清空表格
    FD_ZERO(&lock);
    //将服务端本身的套接字放到表格里面
    FD_SET(sfd,&lock);
    //MAX值需要做转换
    MAX=sfd;
    ret = mysql_init(&my_database);   //初始化结构体
    mysql_real_connect(ret,"localhost","root","1","mysql_203",0,NULL,0);   //连接数据库
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
        for(i=3;i< MAX+1;i++)
		{
			if(FD_ISSET(i,&list)) //判断文件描述符是否在表格中
			{
				if(i==sfd)
				{
					struct sockaddr cliaddr;
					socklen_t len;
					int cid = accept(sfd,&cliaddr,&len);
					FD_SET(cid,&lock);
					if(MAX < cid)
					{
						MAX=cid;
					}
					printf("客户端%d上线了!\n",cid);
				}
				else
				{
					memset(&msg,0,sizeof(msg));
					int lens = read(i,&msg,sizeof(msg));//读取到内容长度
					if(lens == 0)
					{
						close(i);
						printf("客户端%d已下线!\n",i);
						FD_CLR(i,&lock);
					}
					else
					{
						int npc = mysql_query(ret,"select * from chat;");    //执行sql语句,查找表格中所有数据
						if(npc != 0)
						{
							printf("数据查询失败！\n");
							return -1;
						}
						//获取查询的数据信息
						MYSQL_RES * str = NULL;//初始化数据库结构体
						str = mysql_store_result(ret);//获取结果集合的结构体
						if(str == NULL)
						{
							printf("获取数据信息异常！\n");
							return -1;
						}
						//获取行的数目和列的数目
						unsigned int lin,row;
						lin=mysql_num_fields(str);
						row=mysql_num_rows(str);
						//获取每一行的信息
						if(msg.type==1)//注册
						{
							//获取每一行的信息
							if(row == 0)
							{
								send(i,"账号注册成功！",50,0);
								char buf1[100];
								sprintf(buf1,"insert into chat values(\"%s\",\"%s\");",msg.name,msg.passwd);
								mysql_query(ret,buf1);    //执行sql语句
								printf("账号注册成功！\n");
							}	
							else
							{																					
								MYSQL_ROW myrow;
								int flag;	
								for(int j=0;j<row;j++)
								{
									flag=0;
									myrow=mysql_fetch_row(str);
									if((strcmp(myrow[0],msg.name) == 0) && (strcmp(myrow[1],msg.passwd) == 0))
									{
										flag = 1;
										break;
									}
								}
								if(flag == 1)
								{
									send(i,"信息已存在，账号注册失败！",50,0); 
								}
								else
								{          
                                
									send(i,"账号注册成功！",50,0);
									char buf[100];
									sprintf(buf,"insert into chat values(\"%s\",\"%s\");",msg.name,msg.passwd);
									int npc1 = mysql_query(ret,buf);    //执行sql语句，插入数据
									if(npc1!= 0)
									{
										printf("数据存储失败！\n");
										return -1;
									}
									printf("账号注册成功！\n");
								}//mysql_free_result(str);   //释放查询信息的结构体
                            }	 //mysql_close(ret);   //断开数据库连接
						}
						else if(msg.type==2)//销户
						{
							//获取每一行的信息
                            MYSQL_ROW myrow;
                            int flag;
                            for(int j=0;j<row;j++)
                            {
                                flag=0;
                                myrow=mysql_fetch_row(str);
                                if((strcmp(myrow[0],msg.name) == 0) && (strcmp(myrow[1],msg.passwd) == 0))
                                {
                                    flag = 1;
                                    break;
                                }
                            }
                            if(flag == 1)
                            {
                                char buf[100];
                                sprintf(buf,"delete from chat  where name = (\"%s\");",msg.name);
                                int npc2 = mysql_query(ret,buf);    //执行sql语句
                                send(i,"账号注销成功！",50,0);
                            }
                            else
                            {
                                send(i,"输入账号信息有误，账号注销失败！",50,0);
                            }
						}
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















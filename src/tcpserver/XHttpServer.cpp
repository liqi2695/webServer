#include "XHttpServer.h"
#include "XHttpClient.h"
#include<thread>
#include<iostream>
#include<sys/epoll.h>
using namespace std;
#define EPOLL_SIZE 50
void XHttpServer::Stop()
{
	isexit = true;
}
void XHttpServer::Main()
{
	//创建epoll
	int epfd = epoll_create(256);
	cout<<"epfd:"<<epfd<<endl;
    //注册事件
	struct epoll_event ev;
	ev.data.fd = server.sock;
	ev.events = EPOLLIN|EPOLLET;
	epoll_ctl(epfd,EPOLL_CTL_ADD,server.sock,&ev);//新增epoll在事件中
	struct epoll_event *event;//最多等待二十个事件
	event = static_cast<epoll_event*>(malloc(sizeof(struct epoll_event)*EPOLL_SIZE));
	while (!isexit)
	{
		int count = epoll_wait(epfd,event,EPOLL_SIZE,500);
		//cout<<"epoll_wait"<<count<<endl;
		if(count < 0) {
			WriteLog("epoll_wait() error XHttpServer.cpp 27");
			cout<<"epoll_wait() error"<<endl;
			continue;
		}
		for(int i = 0;i<count;i++)
		{
			if(event[i].data.fd == server.sock)
			{
				cout<<"info if"<<endl;
				XTcp client = server.Accept();
				if(client.sock <= 0) continue;
				ev.data.fd = client.sock;
				ev.events = EPOLLIN|EPOLLET;
				epoll_ctl(epfd,EPOLL_CTL_ADD,client.sock,&ev);
			}
			else
			{
				cout<<"info else"<<endl;
				XTcp client;
				client.sock = event[i].data.fd;
				XHttpClient *th = new XHttpClient();
				th->Start(client);
				epoll_ctl(epfd,EPOLL_CTL_DEL,server.sock,&ev);
				client.Close();
			}
		}
		// XTcp client = server.Accept();
		// if (client.sock <= 0) continue;
		// XHttpClient *th = new XHttpClient();
		// int i=0;
		// cout<<"中文测试"<<i++<<endl;
		// th->Start(client);
	}
}
bool XHttpServer::Start(unsigned short port)
{


	isexit = false;
	server.CreateSocket();
	if (!server.Bind(port))
	{ 	
		WriteLog("bind faild XHttpServer.cpp 30");
		return false;
	}	
	WriteLog("bind success XHttpServer.cpp 30");
	thread sth(&XHttpServer::Main, this);
	sth.detach();


	return true;
}

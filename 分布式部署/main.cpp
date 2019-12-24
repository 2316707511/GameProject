#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdio>
#include <hiredis/hiredis.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>

#include <hiredis/async.h>//�첽API
#include <event.h>
#include <hiredis/adapters/libevent.h>

using namespace std;

#if 0

int mycallback(redisAsyncContext* c, void* preply, void* preivdata);

int main()
{
	// 1 ����һ��libevent
	auto base = event_base_new();
	//2 �첽����redis-server
	auto pc = redisAsyncConnect("122.51.68.107", 6379);
	//3 �������ĺ�libevent
	redisLibeventAttach(pc, base);
	//4 ִ�ж�������� �ص�����
	redisAsyncCommand(pc, (redisCallbackFn*)mycallback, NULL, "subscribe create_room");
	// 5 ִ��libeventѭ��
	event_base_dispatch(base);

	return 0;
}

int mycallback(redisAsyncContext* c, void* preply, void* preivdata)
{
	redisReply* reply = (redisReply*)preply;

	if ("message" != std::string(reply->element[0]->str))
	{
		return 0;
	}

	//1 IP  �� ���Լ��Ƿ�һ��
	std::string room_info = reply->element[2]->str;
	int pos = room_info.find(":");

	std::string netIP = room_info.substr(0, pos);
	std::string myIP(getenv("myip"));

	if (netIP != myIP)
		return 0;

	//2  ִ�д�������Ľű�
	std::string room_no = room_info.substr(pos + 1, room_info.size() - pos - 1);
	std::string cmd = "./create_room.sh " + room_no;
	std::string port;
	FILE* pf = popen(cmd.c_str(), "r");
	if (pf != NULL)
	{
		char buff[32] = { 0 };
		fread(buff, 1, sizeof(buff), pf);
		port.append(buff);
		port.pop_back();
		pclose(pf);
	}
	//3  ����������Ķ˿ں�

	auto redispc = redisConnect("122.51.68.107", 6379);
	if (redispc != NULL)
	{
		freeReplyObject(redisCommand(redispc, "publish server_port %s", port.c_str()));
		redisFree(redispc);
	}

	return 0;
}

#else

//�ػ����� + ���ӽ���
void Daemon()
{
	//�򿪻򴴽�һ���ļ�
	int fd = open("./loginServer.log", O_RDWR | O_CREAT | O_APPEND, 0644);
	if (-1 == fd)
	{
		perror("open");
		return;
	}

	//����д�����ض���fd
	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	close(fd);

	//�����ӽ��� �������˳�
	if (fork() > 0)
	{
		//������
		exit(0);
	}

	//�ӽ��̴����Ự
	int ret = setsid();
	if (-1 == ret)
	{
		perror("setsid");
		return;
	}

}


int main()
{
	Daemon();

	//����redis���ݿ�
	auto pc = redisConnect("122.51.68.107", 6379);
	if (pc == NULL)
	{
		return 0;
	}

	freeReplyObject(redisCommand(pc, "subscribe create_room"));

	while (1)
	{
		redisReply * reply = NULL;
		redisGetReply(pc, (void **)&reply);

		if (string(reply->element[0]->str) != "message")
		{
			return 0;
		}

		//1 IP  �� ���Լ��Ƿ�һ��
		string room_info = reply->element[2]->str;
		freeReplyObject(reply);
		int pos = room_info.find(":");

		string netIP = room_info.substr(0, pos);
		string myIP(getenv("myip"));

		if (netIP != myIP)
		{
			continue;
		}

		//2  ִ�д�������Ľű�
		string room_no = room_info.substr(pos + 1, room_info.size() - pos - 1);
		string cmd = "./create_room.sh " + room_no;
		string port;

		FILE* pf = popen(cmd.c_str(), "r");
		if (pf != NULL)
		{
			char buff[32] = { 0 };
			fread(buff, 1, sizeof(buff), pf);
			port.append(buff);
			port.pop_back();
			pclose(pf);
		}

		//3  ����������Ķ˿ں�
		auto redispc = redisConnect("122.51.68.107", 6379);
		if (redispc != NULL)
		{
			freeReplyObject(redisCommand(redispc, "publish server_port %s", port.c_str()));
			redisFree(redispc);
		}
	}
	return 0;
}
#endif
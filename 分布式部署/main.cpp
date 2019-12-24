#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdio>
#include <hiredis/hiredis.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>

#include <hiredis/async.h>//异步API
#include <event.h>
#include <hiredis/adapters/libevent.h>

using namespace std;

#if 0

int mycallback(redisAsyncContext* c, void* preply, void* preivdata);

int main()
{
	// 1 定义一个libevent
	auto base = event_base_new();
	//2 异步链接redis-server
	auto pc = redisAsyncConnect("122.51.68.107", 6379);
	//3 绑定上下文和libevent
	redisLibeventAttach(pc, base);
	//4 执行订阅命令，绑定 回掉函数
	redisAsyncCommand(pc, (redisCallbackFn*)mycallback, NULL, "subscribe create_room");
	// 5 执行libevent循环
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

	//1 IP  和 我自己是否一样
	std::string room_info = reply->element[2]->str;
	int pos = room_info.find(":");

	std::string netIP = room_info.substr(0, pos);
	std::string myIP(getenv("myip"));

	if (netIP != myIP)
		return 0;

	//2  执行创建房间的脚本
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
	//3  发布创建后的端口号

	auto redispc = redisConnect("122.51.68.107", 6379);
	if (redispc != NULL)
	{
		freeReplyObject(redisCommand(redispc, "publish server_port %s", port.c_str()));
		redisFree(redispc);
	}

	return 0;
}

#else

//守护进程 + 监视进程
void Daemon()
{
	//打开或创建一个文件
	int fd = open("./loginServer.log", O_RDWR | O_CREAT | O_APPEND, 0644);
	if (-1 == fd)
	{
		perror("open");
		return;
	}

	//将读写错误重定向到fd
	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	close(fd);

	//创建子进程 父进程退出
	if (fork() > 0)
	{
		//父进程
		exit(0);
	}

	//子进程创建会话
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

	//连接redis数据库
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

		//1 IP  和 我自己是否一样
		string room_info = reply->element[2]->str;
		freeReplyObject(reply);
		int pos = room_info.find(":");

		string netIP = room_info.substr(0, pos);
		string myIP(getenv("myip"));

		if (netIP != myIP)
		{
			continue;
		}

		//2  执行创建房间的脚本
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

		//3  发布创建后的端口号
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
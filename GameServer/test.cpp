#include<iostream>
#include "zinx.h"
#include "ZinxTCP.h"
#include "ZinxTimer.h"
#include "ZinxTimerDeliver.h"
#include "GameChannel.h"
#include "GameRobot.h"

#include <unistd.h>

using namespace std;

//守护进程 + 监视进程
void Daemon()
{
	//打开或创建一个文件
	int fd = open("./game.log", O_RDWR | O_CREAT | O_APPEND, 0644);
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

	//子进程 监视 子子进程 执行任务
	while (1)
	{
		int pid = fork();
		if (pid == -1)
		{
			perror("fork");
			break;
		}
		else if (pid > 0)
		{
			//子进程 监视进程
			int status = 0;
			wait(&status);
			if (WIFEXITED(status))
			{
				//子子正常退出 则监视进程退出
				if (WEXITSTATUS(status) == 0)
				{
					exit(0);
				}
			}
			else
			{
				cout << "子子进程非正常退出 ， 创建新的子子进程" << endl;
				continue;
			}
		}
		else
		{
			//子子进程 执行任务
			break;
		}
	}

}

int main(int argc , char ** argv)
{

	if ((argc == 2) && (string(argv[1]) == "daemon"))
	{
		Daemon();
	}

	ZinxTCPListen * serv = new ZinxTCPListen(6666, new GameChannelFactory);
	ZinxKernel::ZinxKernelInit();

	//ZinxKernel::Zinx_Add_Channel(*serv);
	ZinxKernel::Zinx_Add_Channel(*serv);

	ZinxTimer * timer = new ZinxTimer;
	ZinxKernel::Zinx_Add_Channel(*timer);

	GameRobot robot;
	ZinxKernel::Zinx_Add_Role(robot);

	ZinxKernel::Zinx_Run();

	ZinxKernel::ZinxKernelFini();

	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}

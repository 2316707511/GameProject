#include<iostream>
#include "zinx.h"
#include "ZinxTCP.h"
#include "ZinxTimer.h"
#include "ZinxTimerDeliver.h"
#include "GameChannel.h"
#include "GameRobot.h"

#include <unistd.h>

using namespace std;

//�ػ����� + ���ӽ���
void Daemon()
{
	//�򿪻򴴽�һ���ļ�
	int fd = open("./game.log", O_RDWR | O_CREAT | O_APPEND, 0644);
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

	//�ӽ��� ���� ���ӽ��� ִ������
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
			//�ӽ��� ���ӽ���
			int status = 0;
			wait(&status);
			if (WIFEXITED(status))
			{
				//���������˳� ����ӽ����˳�
				if (WEXITSTATUS(status) == 0)
				{
					exit(0);
				}
			}
			else
			{
				cout << "���ӽ��̷������˳� �� �����µ����ӽ���" << endl;
				continue;
			}
		}
		else
		{
			//���ӽ��� ִ������
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

#include "GameChannel.h"
#include "GameProtocol.h"
#include "GameRole.h"

#include <iostream>

using namespace std;


GameChannel::GameChannel(int fd):ZinxTcpData(fd)
{
}

GameChannel::~GameChannel()
{
	if (nullptr != mProtocol)
	{
		ZinxKernel::Zinx_Del_Proto(*mProtocol);
		delete mProtocol;
	}

	if (nullptr != mRole)
	{
		ZinxKernel::Zinx_Del_Role(*mRole);
		delete mRole;
	}
}

AZinxHandler * GameChannel::GetInputNextStage(BytesMsg & _oInput)
{
	//ͨ������һ����������Э��
	return mProtocol;
}



ZinxTcpData * GameChannelFactory::CreateTcpDataChannel(int _fd)
{
	//����һ��GameChannel���� Ȼ�󷵻�
	GameChannel *channel = new GameChannel(_fd);

	//����Э������
	GameProtocol *protocol = new GameProtocol;

	//������ɫ�����
	GameRole *role = new GameRole;

	channel->mProtocol = protocol;
	channel->mRole = role;

	protocol->mChannel = channel;
	protocol->mRole = role;

	role->mChannel = channel;
	role->mProtocol = protocol;

	//���Э�鵽kernel����
	ZinxKernel::Zinx_Add_Proto(*protocol);

	//��ӽ�ɫ��kernel�й���
	ZinxKernel::Zinx_Add_Role(*role);

	return channel;
}

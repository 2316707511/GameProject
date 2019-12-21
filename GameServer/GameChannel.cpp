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
	//通道层下一个处理者是协议
	return mProtocol;
}



ZinxTcpData * GameChannelFactory::CreateTcpDataChannel(int _fd)
{
	//创建一个GameChannel对象， 然后返回
	GameChannel *channel = new GameChannel(_fd);

	//创建协议层对象
	GameProtocol *protocol = new GameProtocol;

	//创建角色层对象
	GameRole *role = new GameRole;

	channel->mProtocol = protocol;
	channel->mRole = role;

	protocol->mChannel = channel;
	protocol->mRole = role;

	role->mChannel = channel;
	role->mProtocol = protocol;

	//添加协议到kernel管理
	ZinxKernel::Zinx_Add_Proto(*protocol);

	//添加角色到kernel中管理
	ZinxKernel::Zinx_Add_Role(*role);

	return channel;
}

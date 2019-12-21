#pragma once
#include "ZinxTCP.h"


class GameProtocol;
class GameRole;

class GameChannel :
	public ZinxTcpData
{
public:
	GameChannel(int fd);
	virtual ~GameChannel();

	// ͨ�� ZinxTcpData �̳�
	virtual AZinxHandler * GetInputNextStage(BytesMsg & _oInput) override;

public:
	GameProtocol *mProtocol;
	GameRole *mRole;
};


class GameChannelFactory:public IZinxTcpConnFact
{
	// ͨ�� IZinxTcpConnFact �̳�
	virtual ZinxTcpData * CreateTcpDataChannel(int _fd) override;
};

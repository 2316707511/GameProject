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

	// 通过 ZinxTcpData 继承
	virtual AZinxHandler * GetInputNextStage(BytesMsg & _oInput) override;

public:
	GameProtocol *mProtocol;
	GameRole *mRole;
};


class GameChannelFactory:public IZinxTcpConnFact
{
	// 通过 IZinxTcpConnFact 继承
	virtual ZinxTcpData * CreateTcpDataChannel(int _fd) override;
};

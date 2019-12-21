#pragma once
#include "zinx.h"
#include <iostream>

using namespace std;

class GameChannel;
class GameRole;

class GameProtocol :
	public Iprotocol
{
public:
	GameProtocol();
	virtual ~GameProtocol();

	// Í¨¹ý Iprotocol ¼Ì³Ð 
	virtual UserData * raw2request(std::string _szInput) override;
	virtual std::string * response2raw(UserData & _oUserData) override;
	virtual Irole * GetMsgProcessor(UserDataMsg & _oUserDataMsg) override;
	virtual Ichannel * GetMsgSender(BytesMsg & _oBytes) override;

public:
	GameChannel *mChannel;
	GameRole *mRole;

private:
	string mLastBuf;

};


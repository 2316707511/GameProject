#include "GameProtocol.h"
#include "GameChannel.h"
#include "GameRole.h"
#include "GameMsg.h"
#include <iostream>

using namespace std;

GameProtocol::GameProtocol()
{
}


GameProtocol::~GameProtocol()
{
}

UserData * GameProtocol::raw2request(std::string _szInput)
{
	mLastBuf.append(_szInput);

	int MsgID = 0;
	int MsgLen = 0;
	string strContent;

	GameMsg * GMsg = nullptr;

	//һ���� ������8���ֽ�
	//��Ϣ��ʽ: ��Ϣ����(4) + ��ϢID(4) + ��Ϣ����
	while (mLastBuf.size() >= 8)
	{
		MsgLen = mLastBuf[0] |
			mLastBuf[1] << 8 |
			mLastBuf[2] << 16 |
			mLastBuf[3] << 24;

		MsgID = mLastBuf[4] |
			mLastBuf[5] << 8 |
			mLastBuf[6] << 16 |
			mLastBuf[7] << 24;

		//���ٻ�ȡ��һ�������İ�
		if (mLastBuf.size() - 8 >= MsgLen)
		{
			strContent = mLastBuf.substr(8, MsgLen);
			mLastBuf.erase(0, MsgLen + 8);
			
			//���յ������ݷ����л�
			GameSingleTLV *pSingle = new GameSingleTLV((GameSingleTLV::GameMsgType)MsgID, strContent);

			if (GMsg == nullptr)
			{
				GMsg = new GameMsg;
			}
			//�����л�����ָ�뽻����ɫ��
			GMsg->m_MsgList.push_back(pSingle);
		}
		//ȱ��
		else
		{
			break;
		}
	}
	return GMsg;
}

std::string * GameProtocol::response2raw(UserData & _oUserData)
{
	int MsgID;
	int MsgLen;

	string * retBuf = nullptr;

	GameMsg * GMsg = dynamic_cast<GameMsg *>(&_oUserData);

	for (auto it : GMsg->m_MsgList)
	{
		if (retBuf == nullptr)
		{
			retBuf = new string;
		}

		string buf;

		buf = it->Serialize();

		//retBuf = MsgLen + MsgID + buf
		MsgID = it->m_MsgType;

		MsgLen = buf.size();

		retBuf->push_back(MsgLen & 0xff);
		retBuf->push_back(MsgLen >> 8 & 0xff);
		retBuf->push_back(MsgLen >> 16 & 0xff);
		retBuf->push_back(MsgLen >> 24 & 0xff);

		retBuf->push_back(MsgID & 0xff);
		retBuf->push_back(MsgID >> 8 & 0xff);
		retBuf->push_back(MsgID >> 16 & 0xff);
		retBuf->push_back(MsgID >> 24 & 0xff);

		retBuf->append(buf);
	}
	return retBuf;
}

Irole * GameProtocol::GetMsgProcessor(UserDataMsg & _oUserDataMsg)
{
	return mRole;
}

Ichannel * GameProtocol::GetMsgSender(BytesMsg & _oBytes)
{
	return mChannel;
}

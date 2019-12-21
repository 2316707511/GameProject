#include "GameMsg.h"
#include "msg.pb.h"

using namespace std;
using namespace pb;

GameMsg::GameMsg()
{
}


GameMsg::~GameMsg()
{
	for (auto it : m_MsgList)
	{
		delete it;
	}
}

//序列化
GameSingleTLV::GameSingleTLV(GameMsgType _Type, google::protobuf::Message * _poGameMsg)
{
	m_MsgType = _Type;
	m_poGameMsg = _poGameMsg;
}

//反序列化
GameSingleTLV::GameSingleTLV(GameMsgType _Type, std::string _szInputData)
{
	m_MsgType = _Type;

	switch (_Type)
	{
		//同步玩家
	case GAME_MSG_LOGON_SYNCPID:
		m_poGameMsg = new SyncPid;
		break;

		//聊天
	case GAME_MSG_TALK_CONTENT:
		m_poGameMsg = new Talk;
		break;

		//新位置
	case GAME_MSG_NEW_POSTION:
		m_poGameMsg = new Position;
		break;

		//技能触发
	case GAME_MSG_SKILL_TRIGGER:
		m_poGameMsg = new SkillTrigger;
		break;

		//技能命中
	case GAME_MSG_SKILL_CONTACT:
		m_poGameMsg = new SkillContact;
		break;

		//切换场景
	case GAME_MSG_CHANGE_WORLD:
		m_poGameMsg = new ChangeWorldRequest;
		break;

		//普通广播
	case GAME_MSG_BROADCAST:
		m_poGameMsg = new BroadCast;
		break;

		//玩家下线
	case GAME_MSG_LOGOFF_SYNCPID:
		m_poGameMsg = new SyncPid;
		break;

		//获取周边玩家
	case GAME_MSG_SUR_PLAYER:
		m_poGameMsg = new SyncPlayers;
		break;

	case GAME_MSG_SKILL_BROAD:
		m_poGameMsg = new SkillTrigger;
		break;

		//技能命中
	case GAME_MSG_SKILL_CONTACT_BROAD:
		m_poGameMsg = new SkillContact;
		break;

		//切换场景响应
	case GAME_MSG_CHANGE_WORLD_RESPONSE:
		m_poGameMsg = new ChangeWorldResponse;
		break;
	}

	if (m_poGameMsg != nullptr)
	{
		m_poGameMsg->ParseFromString(_szInputData);
	}
}

//析构
GameSingleTLV::~GameSingleTLV()
{
	if (m_poGameMsg != nullptr)
	{
		delete m_poGameMsg;
		m_poGameMsg = nullptr;
	}
}

//序列化
std::string GameSingleTLV::Serialize()
{
	string buf;
	m_poGameMsg->SerializeToString(&buf);

	return buf;
}

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

//���л�
GameSingleTLV::GameSingleTLV(GameMsgType _Type, google::protobuf::Message * _poGameMsg)
{
	m_MsgType = _Type;
	m_poGameMsg = _poGameMsg;
}

//�����л�
GameSingleTLV::GameSingleTLV(GameMsgType _Type, std::string _szInputData)
{
	m_MsgType = _Type;

	switch (_Type)
	{
		//ͬ�����
	case GAME_MSG_LOGON_SYNCPID:
		m_poGameMsg = new SyncPid;
		break;

		//����
	case GAME_MSG_TALK_CONTENT:
		m_poGameMsg = new Talk;
		break;

		//��λ��
	case GAME_MSG_NEW_POSTION:
		m_poGameMsg = new Position;
		break;

		//���ܴ���
	case GAME_MSG_SKILL_TRIGGER:
		m_poGameMsg = new SkillTrigger;
		break;

		//��������
	case GAME_MSG_SKILL_CONTACT:
		m_poGameMsg = new SkillContact;
		break;

		//�л�����
	case GAME_MSG_CHANGE_WORLD:
		m_poGameMsg = new ChangeWorldRequest;
		break;

		//��ͨ�㲥
	case GAME_MSG_BROADCAST:
		m_poGameMsg = new BroadCast;
		break;

		//�������
	case GAME_MSG_LOGOFF_SYNCPID:
		m_poGameMsg = new SyncPid;
		break;

		//��ȡ�ܱ����
	case GAME_MSG_SUR_PLAYER:
		m_poGameMsg = new SyncPlayers;
		break;

	case GAME_MSG_SKILL_BROAD:
		m_poGameMsg = new SkillTrigger;
		break;

		//��������
	case GAME_MSG_SKILL_CONTACT_BROAD:
		m_poGameMsg = new SkillContact;
		break;

		//�л�������Ӧ
	case GAME_MSG_CHANGE_WORLD_RESPONSE:
		m_poGameMsg = new ChangeWorldResponse;
		break;
	}

	if (m_poGameMsg != nullptr)
	{
		m_poGameMsg->ParseFromString(_szInputData);
	}
}

//����
GameSingleTLV::~GameSingleTLV()
{
	if (m_poGameMsg != nullptr)
	{
		delete m_poGameMsg;
		m_poGameMsg = nullptr;
	}
}

//���л�
std::string GameSingleTLV::Serialize()
{
	string buf;
	m_poGameMsg->SerializeToString(&buf);

	return buf;
}

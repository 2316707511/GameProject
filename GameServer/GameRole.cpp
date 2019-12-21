#include "GameRole.h"
#include "GameChannel.h"
#include "GameProtocol.h"
#include "GameMsg.h"
#include "msg.pb.h"
#include "RandomName.h"
#include "ZinxTimer.h"
#include "ZinxTimerDeliver.h"

#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <random>
#include <hiredis/hiredis.h>

using namespace std;
using namespace pb;

class TimeOut :public TimerOutProc
{

	// ͨ�� TimerOutProc �̳�
	virtual void Proc() override
	{
		auto players = ZinxKernel::Zinx_GetAllRole();

		if (players.size() == 1)
		{
			//�˳����
			ZinxKernel::Zinx_Exit();
		}

	}
	virtual int GetTimerSec() override
	{
		return 20;
	}
} g_out;


int GameRole::sm_PlayerID = 0;

//���õ�ǰʱ��Ϊ��������
default_random_engine g_random_engine(time(nullptr));

//���캯��
GameRole::GameRole()
{

	sm_PlayerID++;
	//���ID
	m_PlayerID = sm_PlayerID;
	//�������
	m_PlayerName = RandomName::GetInstance().GetName();
	//m_PlayerName = "Player";

	//�������
	m_x = 100 + g_random_engine() % 20;
	m_z = 100 + g_random_engine() % 20;
	m_y = 0;

	//��ҳ���
	m_v = 0;

	//���Ѫ��
	m_hp = 1000;
}

//��������
GameRole::~GameRole()
{
}

//��ɫ��ʼ��
bool GameRole::Init()
{
	//ע���Զ��˳�����
	ZinxTimerDeliver::GetInstance().UnRegisterProcObject(g_out);

	//��ʼ����ɫID �� ����
	GameMsg * pMsg = MakeLogonSyncPid();
	SendOut(pMsg, mProtocol);

	//��ȡ����
	mCurrentWorld = WorldManager::GetInstance().GetWorld(1);
	
	//�ڵ�ǰ��������Լ�
	mCurrentWorld->AddPlayer(this);

	//��ʼ����½λ��
	pMsg = MakeInitPosBroadcast();
	SendOut(pMsg, mProtocol);
	
	//��ʼ��������ҵ�½λ��
	pMsg = MakeSurPlays();
	SendOut(pMsg, mProtocol);

	//���͸��������������Ϣ
	auto players = mCurrentWorld->GetSurPlayers(this);

	//auto players = ZinxKernel::Zinx_GetAllRole();

	//���Լ���λ�÷��͸��������	
	for (auto it : players)
	{
		if (this == it)
		{
			continue;
		}

		GameRole * role = dynamic_cast<GameRole *>(it);

		//��������ҹ㲥λ��
		pMsg = MakeInitPosBroadcast();
		SendOut(pMsg, role->mProtocol);
		
		//��ȡ�������λ��
		//pMsg = role->MakeInitPosBroadcast();
		//SendOut(*pMsg, *mProtocol);
	}

	//��������ּ�¼��redis���ݿ���
	redisContext * pc = redisConnect("127.0.0.1", 6379);
	if (pc != nullptr)
	{
		freeReplyObject(redisCommand(pc, "lpush game_name %s", m_PlayerName.c_str()));
		redisFree(pc);
	}

	return true;
}

//����Э��㷢����������
UserData * GameRole::ProcMsg(UserData & _poUserData)
{
	GameMsg * pMsg = dynamic_cast<GameMsg *>(&_poUserData);
	
	for (auto single : pMsg->m_MsgList)
	{
		switch (single->m_MsgType)
		{
		//ͬ�����λ�� 
		case GameSingleTLV::GAME_MSG_NEW_POSTION:
		{
			auto pbMsg = dynamic_cast<pb::Position*>(single->m_poGameMsg);
			
			cout << "GAME_MSG_NEW_POSTION x: " << pbMsg->x()
				<< " y: " << pbMsg->y()
				<< " z: " << pbMsg->z() << endl;
			
			this->ProcNewPosition(pbMsg->x(), pbMsg->y(), pbMsg->z(), pbMsg->v());
			break;
		}
		
		//��ȡ�㲥��Ϣ
		case GameSingleTLV::GAME_MSG_TALK_CONTENT:
		{
			auto pbMsg = dynamic_cast<pb::Talk*>(single->m_poGameMsg);
			this->ProcTalkContent(pbMsg->content());
			break;
		}

		//��ȡ�л����糡������
		case GameSingleTLV::GAME_MSG_CHANGE_WORLD:
		{
			auto pbMsg = dynamic_cast<pb::ChangeWorldRequest *>(single->m_poGameMsg);
			
			this->ProcChangeWorld(pbMsg->srcid(), pbMsg->targetid());

			break;
		}

		//��ȡ���ܴ�������
		case GameSingleTLV::GAME_MSG_SKILL_TRIGGER:
		{
			auto pbMsg = dynamic_cast<pb::SkillTrigger *>(single->m_poGameMsg);

			this->ProcSkillTrigger(pbMsg);

			break;
		}

		//��ȡ������������
		case GameSingleTLV::GAME_MSG_SKILL_CONTACT:
		{
			auto pbMsg = dynamic_cast<pb::SkillContact *>(single->m_poGameMsg);

			this->ProcSkillContact(pbMsg);

			break;
		}
		}
	}
	return nullptr;
}

//��ͨ��������ʱ ������������
void GameRole::Fini()
{
	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto r : players)
	{
		if (r == this)
		{
			continue;
		}

		auto role = dynamic_cast<GameRole*>(r);

		//������ǰ������ߵ���Ϣ
		auto pMsg = MakeLogoffSyncPid();

		//������Ϣ����ǰ���е����
		SendOut(pMsg, role->mProtocol);
	}

	//�ӵ�ǰ��ͼ��ɾ������
	mCurrentWorld->DelPlayer(this);

	if (this->mProtocol != nullptr)
	{
		RandomName::GetInstance().ReleaseName(m_PlayerName);
	}

	//��������ִ�redis���ݿ���ɾ��
	redisContext * pc = redisConnect("127.0.0.1", 6379);
	if (pc != nullptr)
	{
		freeReplyObject(redisCommand(pc, "lrem game_name 1 %s", m_PlayerName.c_str()));
		redisFree(pc);
	}

	//�����Ϊ���һ�����ʱ ע���Զ��˳�����
	if (ZinxKernel::Zinx_GetAllRole().size() == 1)
	{
		ZinxTimerDeliver::GetInstance().RegisterProcObject(g_out);
	}
}

//��װ�������� �� ID
GameMsg * GameRole::MakeLogonSyncPid()
{
	SyncPid *sync = new SyncPid;
	sync->set_username(m_PlayerName.c_str());
	sync->set_pid(m_PlayerID);

	GameSingleTLV * single = new GameSingleTLV( GameSingleTLV::GAME_MSG_LOGON_SYNCPID, sync);

	GameMsg *retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);

	return retMsg;
}

//��װ����ͨ�Ź㲥
GameMsg * GameRole::MakeTalkBroadcast(std::string _talkContent)
{
	auto pbMsg = new BroadCast;
	pbMsg->set_pid(m_PlayerID);
	pbMsg->set_username(m_PlayerName);
	pbMsg->set_tp(1);
	pbMsg->set_content(_talkContent);

	GameSingleTLV * single = new GameSingleTLV(GameSingleTLV::GAME_MSG_BROADCAST, pbMsg);

	GameMsg *retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);

	return retMsg;
}

//��װ����λ��
GameMsg * GameRole::MakeInitPosBroadcast()
{
	auto pbMsg = new BroadCast;
	pbMsg->set_pid(m_PlayerID);
	pbMsg->set_username(m_PlayerName);
	pbMsg->set_tp(2);
	auto pos = pbMsg->mutable_p();
	pos->set_bloodvalue(m_hp);
	pos->set_v(m_v);
	pos->set_x(m_x);
	pos->set_y(m_y);
	pos->set_z(m_z);

	GameSingleTLV * single = new GameSingleTLV(GameSingleTLV::GAME_MSG_BROADCAST, pbMsg);

	GameMsg *retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);

	return retMsg;
}

//��װ�ƶ���Ϣ
GameMsg * GameRole::MakeNewPosBroadcast()
{
	BroadCast * pbMsg = new BroadCast;
	pbMsg->set_pid(this->m_PlayerID);
	pbMsg->set_username(this->m_PlayerName);
	pbMsg->set_tp(4);

	auto pos = pbMsg->mutable_p();
	pos->set_x(m_x);
	pos->set_y(m_y);
	pos->set_z(m_z);
	pos->set_v(m_v);
	pos->set_bloodvalue(m_hp);

	GameSingleTLV * single = new GameSingleTLV(GameSingleTLV::GAME_MSG_BROADCAST, pbMsg);

	GameMsg * retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);
	return retMsg;
}

//��װ������Ϣ
GameMsg * GameRole::MakeLogoffSyncPid()
{
	SyncPid *sync = new SyncPid;
	sync->set_username(m_PlayerName.c_str());
	sync->set_pid(m_PlayerID);

	GameSingleTLV * single = new GameSingleTLV(GameSingleTLV::GAME_MSG_LOGOFF_SYNCPID, sync);

	GameMsg *retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);

	return retMsg;
}

//��װ��Χ�����Ϣ
GameMsg * GameRole::MakeSurPlays()
{
	auto pbMsg = new SyncPlayers;

	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto it : players)
	{
		//�����Լ�
		if (this == it)
		{
			continue;
		}

		GameRole * r = dynamic_cast<GameRole *>(it);
		
		auto ps = pbMsg->add_ps();
		ps->set_pid(r->m_PlayerID);
		ps->set_username(r->m_PlayerName);

		auto pos = ps->mutable_p();
		pos->set_x(r->m_x);
		pos->set_z(r->m_z);
		pos->set_y(r->m_y);
		pos->set_v(r->m_v);
		pos->set_bloodvalue(r->m_hp);
	}

	GameSingleTLV * single = new GameSingleTLV(GameSingleTLV::GAME_MSG_SUR_PLAYER, pbMsg);

	GameMsg * retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);

	return retMsg;
}

//��װ�л�������Ϣ
GameMsg * GameRole::MakeChangeWorldResponse(int srcId, int targetId)
{
	/*
	int32 Pid=1;
    int32 ChangeRes=2;
    int32 SrcId=3;
    int32 TargetId=4;
    //�л�������ĳ�����
    Position P=5;
	*/
	ChangeWorldResponse * pbMsg = new ChangeWorldResponse();

	pbMsg->set_pid(m_PlayerID);
	pbMsg->set_changeres(1);
	pbMsg->set_srcid(srcId);
	pbMsg->set_targetid(targetId);

	auto pos = pbMsg->mutable_p();
	pos->set_x(m_x);
	pos->set_y(m_y);
	pos->set_z(m_z);
	pos->set_v(m_v);
	pos->set_bloodvalue(m_hp);

	GameSingleTLV *single = new GameSingleTLV(GameSingleTLV::GAME_MSG_CHANGE_WORLD_RESPONSE, pbMsg);

	GameMsg *retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);
	return retMsg;
}

//��װ���ܴ�����Ϣ
GameMsg * GameRole::MakeSkillTrigger(pb::SkillTrigger * trigger)
{
	SkillTrigger * pbMsg = new SkillTrigger(*trigger);

	GameSingleTLV *single = new GameSingleTLV(GameSingleTLV::GAME_MSG_SKILL_BROAD, pbMsg);

	GameMsg * retMsg = new GameMsg;

	retMsg->m_MsgList.push_back(single);

	return retMsg;
}

//��װ����������Ϣ
GameMsg * GameRole::MakeSkillContact(pb::SkillContact * contact)
{
	SkillContact * pbMsg = new SkillContact(*contact);

	GameSingleTLV *single = new GameSingleTLV(GameSingleTLV::GAME_MSG_SKILL_CONTACT_BROAD, pbMsg);

	GameMsg * retMsg = new GameMsg;

	retMsg->m_MsgList.push_back(single);

	return retMsg;

}

//�㲥�µ�λ��
void GameRole::ProcNewPosition(float _x, float _y, float _z, float _v)
{
	//�ж�λ���Ƿ�ı�
	if (mCurrentWorld->GridChanged(this, _x, _z))
	{
		//λ�÷����˸ı�

		//��ȡ֮ǰ����Χ���
		auto oldList = mCurrentWorld->GetSurPlayers(this);

		//��֮ǰ�ĸ�����ɾ�����
		mCurrentWorld->DelPlayer(this);

		m_x = _x;
		m_y = _y;
		m_z = _z;
		m_v = _v;

		//��ȡ�µ���Χ���
		auto newList = mCurrentWorld->GetSurPlayers(this);

		//��ʧ��Ұ��Χ������
		ViewDisappear(oldList, newList);

		//��ʾ��Ұ��Χ�ڵ����
		ViewAppear(oldList, newList);

		//���µĸ����м������
		mCurrentWorld->AddPlayer(this);

	}
	else
	{
		//λ��û�з����ı�
		m_x = _x;
		m_y = _y;
		m_z = _z;
		m_v = _v;
	}

	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto it : players)
	{
		GameRole *pRole = dynamic_cast<GameRole *>(it);

		//������Լ�������
		if (it == this)
		{
			continue;
		}

		GameMsg * pMsg = MakeNewPosBroadcast();

		//����ǰλ�÷����������
		SendOut(pMsg, pRole->mProtocol);
	}
}

//�㲥������Ϣ
void GameRole::ProcTalkContent(std::string content)
{

	auto players = ZinxKernel::Zinx_GetAllRole();

	for (auto it : players)
	{
		GameRole *pRole = dynamic_cast<GameRole *>(it);

		GameMsg * pMsg = MakeTalkBroadcast(content);

		SendOut(pMsg, pRole->mProtocol);
	}

}

//�л����糡��
void GameRole::ProcChangeWorld(int srcId, int targetWorldId)
{
	//������Χ��� ���Ѿ�����
	auto players = mCurrentWorld->GetSurPlayers(this);
	
	for (auto r : players)
	{
		if (r == this)
		{
			continue;
		}

		auto pbMsg = MakeLogoffSyncPid();

		GameRole * role = dynamic_cast<GameRole *>(r);

		SendOut(pbMsg, role->mProtocol);
	}

	//�ӵ�ǰ�������Ƴ������
	mCurrentWorld->DelPlayer(this);

	//�л�����
	if (1 == targetWorldId)
	{
		//�������
		m_x = 100 + g_random_engine() % 20;
		m_z = 100 + g_random_engine() % 20;
		m_y = 0;

		//��ҳ���
		m_v = 0;

		//���Ѫ��
		m_hp = 1000;
	}
	else if (2 == targetWorldId)
	{
		//�������
		m_x = 50 + g_random_engine() % 40;
		m_z = 50 + g_random_engine() % 40;
		m_y = 0;

		//��ҳ���
		m_v = 0;

		//���Ѫ��
		m_hp = 1000;
	}

	//���µ���������Ӹ����
	mCurrentWorld = WorldManager::GetInstance().GetWorld(targetWorldId);
	mCurrentWorld->AddPlayer(this);

	//���߿ͻ����л�����
	auto pbMsg = MakeChangeWorldResponse(srcId, targetWorldId);
	SendOut(pbMsg, mProtocol);

	//������������Լ���λ��
	players = mCurrentWorld->GetSurPlayers(this);
	for (auto r : players)
	{
		GameRole * role = dynamic_cast<GameRole *>(r);
		pbMsg = MakeInitPosBroadcast();
		SendOut(pbMsg, role->mProtocol);
	}

	//��ȡ�������λ��
	pbMsg = MakeSurPlays();
	SendOut(pbMsg, mProtocol);
	
}

//���ܴ�������
void GameRole::ProcSkillTrigger(pb::SkillTrigger * trigger)
{
	//�㲥���������
	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto r : players)
	{
		if (r == this)
		{
			continue;
		}

		GameRole * role = dynamic_cast<GameRole *>(r);
		auto pbMsg = MakeSkillTrigger(trigger);

		SendOut(pbMsg , role->mProtocol);
	}

}

//����������
void GameRole::ProcSkillContact(pb::SkillContact * contact)
{
	/*
	//������ID
    int32 SrcPid=1;
    //��������
    int32 TargetPid=2;
    int32 SkillId=3;
    int32 BulletId=4;
    //��������
    Position ContactPos=5;
	*/
	//��ȡĿ��ID
	int targetID = contact->targetpid();

	//����Ҵ���
	if (contact->srcpid() != this->m_PlayerID)
	{
		return;
	}

	//��ѯ�������ߵ�GameRole
	GameRole *TargetRole = nullptr;
	auto players = mCurrentWorld->GetSurPlayers(this);
	for (auto r : players)
	{
		GameRole * role = dynamic_cast<GameRole *>(r);
		if (role->m_PlayerID == targetID)
		{
			TargetRole = role;
			break;
		}
	}
	if (nullptr == TargetRole)
	{
		return;
	}

	//����˺�ֵ
	int Damge = 300 + g_random_engine() % 300;

	//�������ߵ�Ѫ
	TargetRole->m_hp -= Damge;

	//�޸Ĵ������Ľṹ��
	auto pos = contact->mutable_contactpos();
	pos->set_bloodvalue(TargetRole->m_hp);

	//�����������Ѫ��С��0
	if (TargetRole->m_hp <= 0)
	{
		//�����������л�����һ������ �����˲����л�����
		if (TargetRole->mProtocol != nullptr)
		{
			TargetRole->ProcChangeWorld(mCurrentWorld->mWorldId, 1);
		}
	}
	//����
	else
	{
		//����������� �������ߵ�Ѫ
		for (auto r : players)
		{
			GameRole * role = dynamic_cast<GameRole *>(r);

			auto pbMsg = MakeSkillContact(contact);

			SendOut(pbMsg, role->mProtocol);
		}
	}
}

//��Ұ��ʧ
void GameRole::ViewDisappear(std::list<AOI_Player*>& oldList, std::list<AOI_Player*>& newList)
{
	vector<AOI_Player*> oldVec(oldList.begin(), oldList.end());
	vector<AOI_Player*> newVec(newList.begin(), newList.end());

	sort(oldVec.begin(), oldVec.end());
	sort(newVec.begin(), newVec.end());

	//Ӧ����ʧ�����
	vector<AOI_Player*> diff;

	//����
	std::set_difference(oldVec.begin(), oldVec.end(), newVec.begin(), newVec.end(),
		std::inserter(diff, diff.begin()));

	for (auto r : diff)
	{
		auto role = dynamic_cast<GameRole*>(r);
	
		//�����������Ұ����ʧ
		auto pMsg = MakeLogoffSyncPid();
		SendOut(pMsg, role->mProtocol);

		//����������Լ���Ұ����ʧ
		pMsg = role->MakeLogoffSyncPid();
		SendOut(pMsg, mProtocol);
	}
}

//��Ұ����
void GameRole::ViewAppear(std::list<AOI_Player*>& oldList, std::list<AOI_Player*>& newList)
{
	vector<AOI_Player*> oldVec(oldList.begin(), oldList.end());
	vector<AOI_Player*> newVec(newList.begin(), newList.end());

	sort(oldVec.begin(), oldVec.end());
	sort(newVec.begin(), newVec.end());

	//Ӧ����ʧ�����
	vector<AOI_Player*> diff;

	//����
	std::set_difference(newVec.begin(), newVec.end(), oldVec.begin(), oldVec.end(),
		std::inserter(diff, diff.begin()));

	for (auto r : diff)
	{
		auto role = dynamic_cast<GameRole*>(r);

		//�����������Ұ�г���
		auto pMsg = MakeInitPosBroadcast();
		SendOut(pMsg, role->mProtocol);

		//����������Լ���Ұ�г���
		pMsg = role->MakeInitPosBroadcast();
		SendOut(pMsg, mProtocol);
	}
}

void GameRole::SendOut(GameMsg * msg, GameProtocol * protocol)
{
	if (protocol == nullptr || msg == nullptr)
	{
		return;
	}

	ZinxKernel::Zinx_SendOut(*msg, *protocol);
}

//--------------------------- AOI_PLAYER ----------------------------//
int GameRole::GetX()
{
	return m_x;
}

int GameRole::GetY()
{
	return m_z;
}

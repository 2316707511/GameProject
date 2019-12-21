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

	// 通过 TimerOutProc 继承
	virtual void Proc() override
	{
		auto players = ZinxKernel::Zinx_GetAllRole();

		if (players.size() == 1)
		{
			//退出框架
			ZinxKernel::Zinx_Exit();
		}

	}
	virtual int GetTimerSec() override
	{
		return 20;
	}
} g_out;


int GameRole::sm_PlayerID = 0;

//设置当前时间为引擎种子
default_random_engine g_random_engine(time(nullptr));

//构造函数
GameRole::GameRole()
{

	sm_PlayerID++;
	//玩家ID
	m_PlayerID = sm_PlayerID;
	//玩家名字
	m_PlayerName = RandomName::GetInstance().GetName();
	//m_PlayerName = "Player";

	//玩家坐标
	m_x = 100 + g_random_engine() % 20;
	m_z = 100 + g_random_engine() % 20;
	m_y = 0;

	//玩家朝向
	m_v = 0;

	//玩家血量
	m_hp = 1000;
}

//析构函数
GameRole::~GameRole()
{
}

//角色初始化
bool GameRole::Init()
{
	//注销自动退出任务
	ZinxTimerDeliver::GetInstance().UnRegisterProcObject(g_out);

	//初始化角色ID 和 名字
	GameMsg * pMsg = MakeLogonSyncPid();
	SendOut(pMsg, mProtocol);

	//获取世界
	mCurrentWorld = WorldManager::GetInstance().GetWorld(1);
	
	//在当前世界添加自己
	mCurrentWorld->AddPlayer(this);

	//初始化登陆位置
	pMsg = MakeInitPosBroadcast();
	SendOut(pMsg, mProtocol);
	
	//初始化其他玩家登陆位置
	pMsg = MakeSurPlays();
	SendOut(pMsg, mProtocol);

	//发送给其他玩家上线信息
	auto players = mCurrentWorld->GetSurPlayers(this);

	//auto players = ZinxKernel::Zinx_GetAllRole();

	//将自己的位置发送给其他玩家	
	for (auto it : players)
	{
		if (this == it)
		{
			continue;
		}

		GameRole * role = dynamic_cast<GameRole *>(it);

		//向其他玩家广播位置
		pMsg = MakeInitPosBroadcast();
		SendOut(pMsg, role->mProtocol);
		
		//获取其他玩家位置
		//pMsg = role->MakeInitPosBroadcast();
		//SendOut(*pMsg, *mProtocol);
	}

	//将玩家名字记录在redis数据库中
	redisContext * pc = redisConnect("127.0.0.1", 6379);
	if (pc != nullptr)
	{
		freeReplyObject(redisCommand(pc, "lpush game_name %s", m_PlayerName.c_str()));
		redisFree(pc);
	}

	return true;
}

//接受协议层发过来的数据
UserData * GameRole::ProcMsg(UserData & _poUserData)
{
	GameMsg * pMsg = dynamic_cast<GameMsg *>(&_poUserData);
	
	for (auto single : pMsg->m_MsgList)
	{
		switch (single->m_MsgType)
		{
		//同步玩家位置 
		case GameSingleTLV::GAME_MSG_NEW_POSTION:
		{
			auto pbMsg = dynamic_cast<pb::Position*>(single->m_poGameMsg);
			
			cout << "GAME_MSG_NEW_POSTION x: " << pbMsg->x()
				<< " y: " << pbMsg->y()
				<< " z: " << pbMsg->z() << endl;
			
			this->ProcNewPosition(pbMsg->x(), pbMsg->y(), pbMsg->z(), pbMsg->v());
			break;
		}
		
		//获取广播消息
		case GameSingleTLV::GAME_MSG_TALK_CONTENT:
		{
			auto pbMsg = dynamic_cast<pb::Talk*>(single->m_poGameMsg);
			this->ProcTalkContent(pbMsg->content());
			break;
		}

		//获取切换世界场景请求
		case GameSingleTLV::GAME_MSG_CHANGE_WORLD:
		{
			auto pbMsg = dynamic_cast<pb::ChangeWorldRequest *>(single->m_poGameMsg);
			
			this->ProcChangeWorld(pbMsg->srcid(), pbMsg->targetid());

			break;
		}

		//获取技能触发请求
		case GameSingleTLV::GAME_MSG_SKILL_TRIGGER:
		{
			auto pbMsg = dynamic_cast<pb::SkillTrigger *>(single->m_poGameMsg);

			this->ProcSkillTrigger(pbMsg);

			break;
		}

		//获取技能命中请求
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

//当通道类析构时 会调用这个函数
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

		//创建当前玩家下线的消息
		auto pMsg = MakeLogoffSyncPid();

		//发送消息到当前所有的玩家
		SendOut(pMsg, role->mProtocol);
	}

	//从当前地图中删除人物
	mCurrentWorld->DelPlayer(this);

	if (this->mProtocol != nullptr)
	{
		RandomName::GetInstance().ReleaseName(m_PlayerName);
	}

	//将玩家名字从redis数据库中删除
	redisContext * pc = redisConnect("127.0.0.1", 6379);
	if (pc != nullptr)
	{
		freeReplyObject(redisCommand(pc, "lrem game_name 1 %s", m_PlayerName.c_str()));
		redisFree(pc);
	}

	//当玩家为最后一个玩家时 注册自动退出任务
	if (ZinxKernel::Zinx_GetAllRole().size() == 1)
	{
		ZinxTimerDeliver::GetInstance().RegisterProcObject(g_out);
	}
}

//组装上线名字 和 ID
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

//组装网络通信广播
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

//组装上线位置
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

//组装移动消息
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

//组装下线信息
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

//组装周围玩家信息
GameMsg * GameRole::MakeSurPlays()
{
	auto pbMsg = new SyncPlayers;

	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto it : players)
	{
		//跳过自己
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

//组装切换世界信息
GameMsg * GameRole::MakeChangeWorldResponse(int srcId, int targetId)
{
	/*
	int32 Pid=1;
    int32 ChangeRes=2;
    int32 SrcId=3;
    int32 TargetId=4;
    //切换场景后的出生点
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

//组装技能触发信息
GameMsg * GameRole::MakeSkillTrigger(pb::SkillTrigger * trigger)
{
	SkillTrigger * pbMsg = new SkillTrigger(*trigger);

	GameSingleTLV *single = new GameSingleTLV(GameSingleTLV::GAME_MSG_SKILL_BROAD, pbMsg);

	GameMsg * retMsg = new GameMsg;

	retMsg->m_MsgList.push_back(single);

	return retMsg;
}

//组装技能命中信息
GameMsg * GameRole::MakeSkillContact(pb::SkillContact * contact)
{
	SkillContact * pbMsg = new SkillContact(*contact);

	GameSingleTLV *single = new GameSingleTLV(GameSingleTLV::GAME_MSG_SKILL_CONTACT_BROAD, pbMsg);

	GameMsg * retMsg = new GameMsg;

	retMsg->m_MsgList.push_back(single);

	return retMsg;

}

//广播新的位置
void GameRole::ProcNewPosition(float _x, float _y, float _z, float _v)
{
	//判断位置是否改变
	if (mCurrentWorld->GridChanged(this, _x, _z))
	{
		//位置发生了改变

		//获取之前的周围玩家
		auto oldList = mCurrentWorld->GetSurPlayers(this);

		//从之前的格子中删除玩家
		mCurrentWorld->DelPlayer(this);

		m_x = _x;
		m_y = _y;
		m_z = _z;
		m_v = _v;

		//获取新的周围玩家
		auto newList = mCurrentWorld->GetSurPlayers(this);

		//消失视野范围外的玩家
		ViewDisappear(oldList, newList);

		//显示视野范围内的玩家
		ViewAppear(oldList, newList);

		//在新的格子中加入玩家
		mCurrentWorld->AddPlayer(this);

	}
	else
	{
		//位置没有发生改变
		m_x = _x;
		m_y = _y;
		m_z = _z;
		m_v = _v;
	}

	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto it : players)
	{
		GameRole *pRole = dynamic_cast<GameRole *>(it);

		//如果是自己就跳过
		if (it == this)
		{
			continue;
		}

		GameMsg * pMsg = MakeNewPosBroadcast();

		//将当前位置发给其他玩家
		SendOut(pMsg, pRole->mProtocol);
	}
}

//广播聊天信息
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

//切换世界场景
void GameRole::ProcChangeWorld(int srcId, int targetWorldId)
{
	//告诉周围玩家 你已经下线
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

	//从当前世界中移除该玩家
	mCurrentWorld->DelPlayer(this);

	//切换世界
	if (1 == targetWorldId)
	{
		//玩家坐标
		m_x = 100 + g_random_engine() % 20;
		m_z = 100 + g_random_engine() % 20;
		m_y = 0;

		//玩家朝向
		m_v = 0;

		//玩家血量
		m_hp = 1000;
	}
	else if (2 == targetWorldId)
	{
		//玩家坐标
		m_x = 50 + g_random_engine() % 40;
		m_z = 50 + g_random_engine() % 40;
		m_y = 0;

		//玩家朝向
		m_v = 0;

		//玩家血量
		m_hp = 1000;
	}

	//在新的世界中添加该玩家
	mCurrentWorld = WorldManager::GetInstance().GetWorld(targetWorldId);
	mCurrentWorld->AddPlayer(this);

	//告诉客户端切换场景
	auto pbMsg = MakeChangeWorldResponse(srcId, targetWorldId);
	SendOut(pbMsg, mProtocol);

	//告诉其他玩家自己的位置
	players = mCurrentWorld->GetSurPlayers(this);
	for (auto r : players)
	{
		GameRole * role = dynamic_cast<GameRole *>(r);
		pbMsg = MakeInitPosBroadcast();
		SendOut(pbMsg, role->mProtocol);
	}

	//获取其他玩家位置
	pbMsg = MakeSurPlays();
	SendOut(pbMsg, mProtocol);
	
}

//技能触发请求
void GameRole::ProcSkillTrigger(pb::SkillTrigger * trigger)
{
	//广播给其他玩家
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

//处理技能命中
void GameRole::ProcSkillContact(pb::SkillContact * contact)
{
	/*
	//攻击者ID
    int32 SrcPid=1;
    //被攻击者
    int32 TargetPid=2;
    int32 SkillId=3;
    int32 BulletId=4;
    //攻击坐标
    Position ContactPos=5;
	*/
	//获取目标ID
	int targetID = contact->targetpid();

	//防外挂处理
	if (contact->srcpid() != this->m_PlayerID)
	{
		return;
	}

	//查询被攻击者的GameRole
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

	//随机伤害值
	int Damge = 300 + g_random_engine() % 300;

	//被攻击者掉血
	TargetRole->m_hp -= Damge;

	//修改传过来的结构体
	auto pos = contact->mutable_contactpos();
	pos->set_bloodvalue(TargetRole->m_hp);

	//如果被攻击者血量小于0
	if (TargetRole->m_hp <= 0)
	{
		//将被攻击者切换到第一个场景 机器人不用切换场景
		if (TargetRole->mProtocol != nullptr)
		{
			TargetRole->ProcChangeWorld(mCurrentWorld->mWorldId, 1);
		}
	}
	//否则
	else
	{
		//告诉所有玩家 被攻击者掉血
		for (auto r : players)
		{
			GameRole * role = dynamic_cast<GameRole *>(r);

			auto pbMsg = MakeSkillContact(contact);

			SendOut(pbMsg, role->mProtocol);
		}
	}
}

//视野消失
void GameRole::ViewDisappear(std::list<AOI_Player*>& oldList, std::list<AOI_Player*>& newList)
{
	vector<AOI_Player*> oldVec(oldList.begin(), oldList.end());
	vector<AOI_Player*> newVec(newList.begin(), newList.end());

	sort(oldVec.begin(), oldVec.end());
	sort(newVec.begin(), newVec.end());

	//应该消失的玩家
	vector<AOI_Player*> diff;

	//计算差集
	std::set_difference(oldVec.begin(), oldVec.end(), newVec.begin(), newVec.end(),
		std::inserter(diff, diff.begin()));

	for (auto r : diff)
	{
		auto role = dynamic_cast<GameRole*>(r);
	
		//在其他玩家视野中消失
		auto pMsg = MakeLogoffSyncPid();
		SendOut(pMsg, role->mProtocol);

		//其他玩家在自己视野中消失
		pMsg = role->MakeLogoffSyncPid();
		SendOut(pMsg, mProtocol);
	}
}

//视野出现
void GameRole::ViewAppear(std::list<AOI_Player*>& oldList, std::list<AOI_Player*>& newList)
{
	vector<AOI_Player*> oldVec(oldList.begin(), oldList.end());
	vector<AOI_Player*> newVec(newList.begin(), newList.end());

	sort(oldVec.begin(), oldVec.end());
	sort(newVec.begin(), newVec.end());

	//应该消失的玩家
	vector<AOI_Player*> diff;

	//计算差集
	std::set_difference(newVec.begin(), newVec.end(), oldVec.begin(), oldVec.end(),
		std::inserter(diff, diff.begin()));

	for (auto r : diff)
	{
		auto role = dynamic_cast<GameRole*>(r);

		//在其他玩家视野中出现
		auto pMsg = MakeInitPosBroadcast();
		SendOut(pMsg, role->mProtocol);

		//其他玩家在自己视野中出现
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

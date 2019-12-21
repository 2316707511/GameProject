#include "GameRobot.h"
#include "GameChannel.h"
#include "GameProtocol.h"
#include "GameRole.h"
#include "GameMsg.h"
#include "zinx.h"

using namespace std;


GameRobot::GameRobot()
{
}

GameRobot::~GameRobot()
{
}

//自动攻击
void GameRobot::autoAttack()
{
	//获取周围玩家
	auto players = mCurrentWorld->GetSurPlayers(this);
	for (auto it : players)
	{
		GameRole * role = dynamic_cast<GameRole *>(it);
		//发送给周围玩家
		if (role != nullptr)
		{
			//组装技能触发信息
			auto p = new pb::SkillTrigger();
			p->set_pid(m_PlayerID);// 角色的id
			p->set_skillid(1);// 技能ID
			static int count = 1;
			p->set_bulletid(count++);// 子弹ID
			pb::Position* pos = p->mutable_p();// 位置信息 
			pos->set_x(m_x);
			pos->set_y(m_y);
			pos->set_z(m_z);
			pos->set_v(m_v);
			pos->set_bloodvalue(m_hp);
			pb::Velocity* pvel = p->mutable_v();//
			pvel->set_x(m_x);
			pvel->set_y(m_y);
			pvel->set_z(m_z);

			auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_SKILL_BROAD, p);

			GameMsg * retMsg = new GameMsg;
			retMsg->m_MsgList.push_back(single);
			
			SendOut(retMsg, role->mProtocol);
			if (role->mProtocol == nullptr)
			{
				delete retMsg;
			}
		}
	}
}

bool GameRobot::Init()
{
	mProtocol = nullptr;
	mChannel = nullptr;

	m_PlayerID = 10001;
	//玩家名字
	m_PlayerName = "Robot_One";
	//玩家坐标
	m_x = 70;
	m_y = 0;
	m_z = 70;
	//玩家朝向
	m_v = 0;
	//玩家血量
	m_hp = 1000;
	//当前世界
	mCurrentWorld = WorldManager::GetInstance().GetWorld(2);
	mCurrentWorld->AddPlayer(this);

	//向周围玩家显示机器人
	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto r : players)
	{
		if (r == this)
		{
			continue;
		}

		GameRole * role = dynamic_cast<GameRole *>(r);

		auto pbMsg = MakeInitPosBroadcast();

		if (nullptr != role)
		{
			ZinxKernel::Zinx_SendOut(*pbMsg, *role->mProtocol);
		}
	}

	//在时间轮盘注册任务
	ZinxTimerDeliver::GetInstance().RegisterProcObject(*this);

	return true;
}

//任务处理函数
void GameRobot::Proc()
{
	//自动攻击
	autoAttack();

	//自动移动
}

//2秒触发一次
int GameRobot::GetTimerSec()
{
	return 2;
}

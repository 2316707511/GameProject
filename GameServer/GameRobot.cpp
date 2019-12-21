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

//�Զ�����
void GameRobot::autoAttack()
{
	//��ȡ��Χ���
	auto players = mCurrentWorld->GetSurPlayers(this);
	for (auto it : players)
	{
		GameRole * role = dynamic_cast<GameRole *>(it);
		//���͸���Χ���
		if (role != nullptr)
		{
			//��װ���ܴ�����Ϣ
			auto p = new pb::SkillTrigger();
			p->set_pid(m_PlayerID);// ��ɫ��id
			p->set_skillid(1);// ����ID
			static int count = 1;
			p->set_bulletid(count++);// �ӵ�ID
			pb::Position* pos = p->mutable_p();// λ����Ϣ 
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
	//�������
	m_PlayerName = "Robot_One";
	//�������
	m_x = 70;
	m_y = 0;
	m_z = 70;
	//��ҳ���
	m_v = 0;
	//���Ѫ��
	m_hp = 1000;
	//��ǰ����
	mCurrentWorld = WorldManager::GetInstance().GetWorld(2);
	mCurrentWorld->AddPlayer(this);

	//����Χ�����ʾ������
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

	//��ʱ������ע������
	ZinxTimerDeliver::GetInstance().RegisterProcObject(*this);

	return true;
}

//��������
void GameRobot::Proc()
{
	//�Զ�����
	autoAttack();

	//�Զ��ƶ�
}

//2�봥��һ��
int GameRobot::GetTimerSec()
{
	return 2;
}

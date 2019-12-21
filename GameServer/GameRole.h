#pragma once
#include "zinx.h"
#include "AOI_World.h"
#include "GameMsg.h"
#include "WorldManager.h"

class GameProtocol;
class GameChannel;

class GameRole :
	public Irole,
	public AOI_Player
{
public:
	GameRole();
	virtual ~GameRole();

	// ͨ�� Irole �̳�
	virtual bool Init() override;
	virtual UserData * ProcMsg(UserData & _poUserData) override;
	virtual void Fini() override;

	/*��������ʱ��id��������Ϣ*/
	GameMsg *MakeLogonSyncPid();
	/*�����㲥������Ϣ*/
	GameMsg *MakeTalkBroadcast(std::string _talkContent);
	/*�����㲥����λ����Ϣ*/
	GameMsg *MakeInitPosBroadcast();
	/*�����㲥�ƶ�����λ����Ϣ*/
	GameMsg *MakeNewPosBroadcast();
	/*��������ʱ��id��������Ϣ*/
	GameMsg *MakeLogoffSyncPid();
	/*������Χ���λ����Ϣ*/
	GameMsg *MakeSurPlays();
	/*���볡��ȷ�ϵ���Ϣ*/
	GameMsg *MakeChangeWorldResponse(int srcId, int targetId);
	/*���ܴ�����Ϣ*/
	GameMsg *MakeSkillTrigger(pb::SkillTrigger *trigger);
	/*������ײ��Ϣ*/
	GameMsg *MakeSkillContact(pb::SkillContact *contact);

	//��������ƶ�����Ϣ
	void ProcNewPosition(float _x, float _y, float _z, float _v);
	void ProcTalkContent(std::string content);
	void ProcChangeWorld(int srcId, int targetWorldId);
	void ProcSkillTrigger(pb::SkillTrigger *trigger);
	void ProcSkillContact(pb::SkillContact *contact);

	//����AOI��Ұ��ʧ
	void ViewDisappear(std::list<AOI_Player*> &oldList, std::list<AOI_Player*> &newList);
	//����AOI��Ұ����
	void ViewAppear(std::list<AOI_Player*> &oldList, std::list<AOI_Player*> &newList);

	//����Zinx::SendOut ����
	void SendOut(GameMsg *, GameProtocol *);


public:
	GameProtocol * mProtocol;
	GameChannel *mChannel;

protected:
	static int sm_PlayerID;
	//���ID
	int m_PlayerID;
	//�������
	string m_PlayerName;
	//�������
	float m_x;
	float m_y;
	float m_z;

	//��ҳ���
	float m_v;

	//���Ѫ��
	int m_hp;


	// ͨ�� AOI_Player �̳�
	virtual int GetX() override;

	virtual int GetY() override;

	//��ǰ����
	AOI_World * mCurrentWorld;

};


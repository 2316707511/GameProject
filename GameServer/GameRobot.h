#pragma once
#include "GameRole.h"
#include "ZinxTimerDeliver.h"

class GameRobot :
	public GameRole , public TimerOutProc
{
public:
	GameRobot();
	virtual ~GameRobot();
	void autoAttack();

	// GameRole
	virtual bool Init() override;

	// ͨ�� TimerOutProc �̳�
	virtual void Proc() override;
	virtual int GetTimerSec() override;
};


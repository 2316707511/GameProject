#pragma once
#include "zinx.h"
#include <vector>
#include <list>

using namespace std;

class TimerOutProc
{
public:
	TimerOutProc(){}
	virtual ~TimerOutProc() {}
	virtual void Proc() = 0;
	virtual int GetTimerSec() = 0;
};

struct WheelNode
{
	int LastCount = -1;
	TimerOutProc *pProc = nullptr;
};



class ZinxTimerDeliver :
	public AZinxHandler
{
public:
	// Í¨¹ý AZinxHandler ¼Ì³Ð
	virtual IZinxMsg * InternalHandle(IZinxMsg & _oInput) override;
	virtual AZinxHandler * GetNextHandler(IZinxMsg & _oNextMsg) override;

	static ZinxTimerDeliver &GetInstance()
	{
		return m_single;
	}

	bool RegisterProcObject(TimerOutProc &_proc);
	void UnRegisterProcObject(TimerOutProc &_proc);

private:
	ZinxTimerDeliver();
	ZinxTimerDeliver(const ZinxTimerDeliver &){}
	~ZinxTimerDeliver();

private:
	static ZinxTimerDeliver m_single;

	int m_cur_index = 0;

	vector<std::list<WheelNode>> m_TimerWheel;
};


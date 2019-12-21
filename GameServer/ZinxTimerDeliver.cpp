#include "ZinxTimerDeliver.h"
#include <iostream>
#include <vector>

using namespace std;

#define ZINX_TIMER_WHEEL_SIZE 8

ZinxTimerDeliver ZinxTimerDeliver::m_single;

//注册任务
bool ZinxTimerDeliver::RegisterProcObject(TimerOutProc & _proc)
{

	int tmo = 0;
	int index = 0;
	int count = 0;
	WheelNode node;

	//超时时间
	tmo = _proc.GetTimerSec();
	if (tmo <= 0)
	{
		cout << "超时时间  不得小于1" << endl;
		return false;
	}

	//格子数
	index = (m_cur_index + tmo) % m_TimerWheel.size();

	//圈数
	count = tmo / m_TimerWheel.size();

	//任务指针
	node.pProc = &_proc;
	node.LastCount = count;
	m_TimerWheel[index].push_back(node);

	return true;
}

void ZinxTimerDeliver::UnRegisterProcObject(TimerOutProc & _proc)
{
	for (auto &i : m_TimerWheel)
	{
		for (auto it = i.begin(); it != i.end(); )
		{
			if (it->pProc == &_proc)
			{
				it = i.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
}

ZinxTimerDeliver::ZinxTimerDeliver()
{
	m_TimerWheel.resize(ZINX_TIMER_WHEEL_SIZE);
}


ZinxTimerDeliver::~ZinxTimerDeliver()
{
}

IZinxMsg * ZinxTimerDeliver::InternalHandle(IZinxMsg & _oInput)
{
	uint64_t val;
	BytesMsg *pMsg = dynamic_cast<BytesMsg *>(&_oInput);
	pMsg->szData.copy((char*)&val, sizeof(val));

	vector<WheelNode> nodes;

	for (uint64_t j = 0; j < val; j++)
	{
		m_cur_index = (m_cur_index + 1) % m_TimerWheel.size();
		for (auto i = m_TimerWheel[m_cur_index].begin() ; i!= m_TimerWheel[m_cur_index].end() ; )
		{
			i->LastCount--;
			if (i->LastCount < 0)
			{
				i->pProc->Proc();
				nodes.push_back(*i);
				i = m_TimerWheel[m_cur_index].erase(i);
			}
			else
			{
				i++;
			}
		}
	}

	for (auto i : nodes)
	{
		RegisterProcObject(*(i.pProc));
	}

	return nullptr;
}

AZinxHandler * ZinxTimerDeliver::GetNextHandler(IZinxMsg & _oNextMsg)
{
	return nullptr;
}

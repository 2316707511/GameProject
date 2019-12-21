#include "ZinxTimer.h"
#include "ZinxTimerDeliver.h"

#include <sys/timerfd.h>
#include <stdint.h>

#include <iostream>

using namespace std;

ZinxTimer::ZinxTimer()
{
}


ZinxTimer::~ZinxTimer()
{
}

bool ZinxTimer::Init()
{
	mFd = timerfd_create(CLOCK_MONOTONIC, 0);
	if (-1 == mFd)
	{
		perror("timer_create");
		return false;
	}

	itimerspec tmo = { {1,0} , {1,0} };
	int ret = timerfd_settime(mFd, 0, &tmo, NULL);
	if (-1 == ret)
	{
		perror("timerfd_settime");
		return false;
	}

	return true;
}

bool ZinxTimer::ReadFd(std::string & _input)
{
	uint64_t val;

	int ret = -1;

	ret = read(mFd, (char *)&val, sizeof(val));
	if (ret != sizeof(val))
	{
		perror("read");
		return false;
	}

	_input.append((char *)(&val), sizeof(val));

	return true;
}

bool ZinxTimer::WriteFd(std::string & _output)
{
	return false;
}

void ZinxTimer::Fini()
{
	if (mFd > 0)
	{
		close(mFd);
	}
}

int ZinxTimer::GetFd()
{
	return mFd;
}

std::string ZinxTimer::GetChannelInfo()
{
	return std::string("timer");
}

AZinxHandler * ZinxTimer::GetInputNextStage(BytesMsg & _oInput)
{
	return &ZinxTimerDeliver::GetInstance();
}

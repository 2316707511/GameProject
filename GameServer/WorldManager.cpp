#include "WorldManager.h"

WorldManager WorldManager::smManager;

//��ȡʵ��
WorldManager & WorldManager::GetInstance()
{
	// TODO: �ڴ˴����� return ���
	return smManager;
}

//��ȡ����
AOI_World * WorldManager::GetWorld(int id)
{
	return mVecWorld[id];
}

//���캯��
WorldManager::WorldManager()
{
	mVecWorld.reserve(3);

	mVecWorld[0] = nullptr;

	//��һ������
	mVecWorld[1] = new AOI_World(85, 410, 75, 400, 10, 20);
	mVecWorld[1]->mWorldId = 1;

	//�ڶ������� ս������
	mVecWorld[2] = new AOI_World(0, 140, 0, 140, 1, 1);
	mVecWorld[2]->mWorldId = 2;

}

//��������
WorldManager::~WorldManager()
{
	for (auto it : mVecWorld)
	{
		if (nullptr != it)
		{
			delete it;
		}
	}
}

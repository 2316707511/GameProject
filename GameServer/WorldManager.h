#pragma once

#include "AOI_World.h"

class WorldManager
{
public:
	static WorldManager &GetInstance();
	AOI_World *GetWorld(int id);

private:
	WorldManager();
	~WorldManager();
	static WorldManager smManager;
	std::vector<AOI_World*> mVecWorld;
};


#include "AOI_World.h"
#include <list>
#include <vector>
#include <algorithm>

using namespace std;


AOI_Grid::AOI_Grid(int _gid):iGID(_gid)
{
}

AOI_Grid::~AOI_Grid()
{
}


//---------------------------------------------------//

//构造函数
AOI_World::AOI_World(int _minx, int _maxx, int _miny, int _maxy, int _xcnt, int _ycnt)
{
	minX = _minx;
	maxX = _maxx;
	minY = _miny;
	maxY = _maxy;
	Xcnt = _xcnt;
	Ycnt = _ycnt;

	this->m_grids.reserve(_xcnt * _ycnt);
	for (int i = 0; i < _xcnt * _ycnt; ++i)
	{
		m_grids.push_back(new AOI_Grid(i));
	}
}

//析构函数
AOI_World::~AOI_World()
{
	for (auto it : m_grids)
	{
		delete it;
	}
}

//获取周围用户
std::list<AOI_Player*> AOI_World::GetSurPlayers(AOI_Player * _player)
{
	int row = 0;
	int col = 0;
	int index = 0;

	//列
	col = (_player->GetX() - minX) / ( (this->maxX - this->minX) / this->Xcnt);
	//行
	row = (_player->GetY() - minY) / ( (this->maxY - this->minY) / this->Ycnt);

	list<AOI_Player*> Players;

	pair<int, int> row_col[] =
	{
		make_pair(row - 1 , col - 1),
		make_pair(row - 1 , col),
		make_pair(row - 1 , col + 1),

		make_pair(row  , col - 1),
		make_pair(row  , col ),
		make_pair(row  , col + 1),

		make_pair(row + 1 , col - 1),
		make_pair(row + 1 , col ),
		make_pair(row + 1 , col + 1)
	};

	for (auto it : row_col)
	{
		//判断行合法
		if (it.first < 0 || it.first >= Ycnt)
		{
			continue;
		}

		//判断列合法
		if (it.second < 0 || it.second >= Xcnt)
		{
			continue;
		}

		index = it.first * this->Xcnt + it.second;

		for (auto p : m_grids[index]->m_players)
		{
			Players.push_back(p);
		}
	}

	return Players;
}

//添加用户
void AOI_World::AddPlayer(AOI_Player * _player)
{
	int index = Calculate_grid_idx(_player->GetX(), _player->GetY());

	if (index < 0 || index >= this->Xcnt * this->Ycnt)
	{
		std::cout << "玩家所在的格子数非法" << std::endl;
		return;
	}

	m_grids[index]->m_players.push_back(_player);

}

//删除用户
void AOI_World::DelPlayer(AOI_Player * _player)
{
	int index = Calculate_grid_idx(_player->GetX(), _player->GetY());
	if (index < 0 || index >= this->Xcnt * this->Ycnt)
	{
		std::cout << "玩家所在的格子数非法" << std::endl;
		return;
	}

	m_grids[index]->m_players.remove(_player);
}

//判断是否改变了格子
bool AOI_World::GridChanged(AOI_Player * _player, int _newX, int _newY)
{
	int oldIndex = Calculate_grid_idx(_player->GetX(), _player->GetY());

	int newIndex = Calculate_grid_idx(_newX, _newY);

	if (oldIndex == newIndex)
	{
		return false;
	}

	return true;
}

//计算格子索引
int AOI_World::Calculate_grid_idx(int x, int y)
{
	int row = -1;
	int col = -1;

	int index = -1;

	//列
	col = (x - minX) / ((this->maxX - this->minX) / this->Xcnt);
	//行
	row = (y - minY) / ((this->maxY - this->minY) / this->Ycnt);

	if (row < 0 || row >= this->Ycnt || col < 0 || col >= this->Xcnt)
	{
		cout << "玩家的所在的格子非法" << endl;
		return -1;
	}

	index = row * Xcnt + col;

	return index;
}

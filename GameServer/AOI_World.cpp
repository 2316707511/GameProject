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

//���캯��
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

//��������
AOI_World::~AOI_World()
{
	for (auto it : m_grids)
	{
		delete it;
	}
}

//��ȡ��Χ�û�
std::list<AOI_Player*> AOI_World::GetSurPlayers(AOI_Player * _player)
{
	int row = 0;
	int col = 0;
	int index = 0;

	//��
	col = (_player->GetX() - minX) / ( (this->maxX - this->minX) / this->Xcnt);
	//��
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
		//�ж��кϷ�
		if (it.first < 0 || it.first >= Ycnt)
		{
			continue;
		}

		//�ж��кϷ�
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

//����û�
void AOI_World::AddPlayer(AOI_Player * _player)
{
	int index = Calculate_grid_idx(_player->GetX(), _player->GetY());

	if (index < 0 || index >= this->Xcnt * this->Ycnt)
	{
		std::cout << "������ڵĸ������Ƿ�" << std::endl;
		return;
	}

	m_grids[index]->m_players.push_back(_player);

}

//ɾ���û�
void AOI_World::DelPlayer(AOI_Player * _player)
{
	int index = Calculate_grid_idx(_player->GetX(), _player->GetY());
	if (index < 0 || index >= this->Xcnt * this->Ycnt)
	{
		std::cout << "������ڵĸ������Ƿ�" << std::endl;
		return;
	}

	m_grids[index]->m_players.remove(_player);
}

//�ж��Ƿ�ı��˸���
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

//�����������
int AOI_World::Calculate_grid_idx(int x, int y)
{
	int row = -1;
	int col = -1;

	int index = -1;

	//��
	col = (x - minX) / ((this->maxX - this->minX) / this->Xcnt);
	//��
	row = (y - minY) / ((this->maxY - this->minY) / this->Ycnt);

	if (row < 0 || row >= this->Ycnt || col < 0 || col >= this->Xcnt)
	{
		cout << "��ҵ����ڵĸ��ӷǷ�" << endl;
		return -1;
	}

	index = row * Xcnt + col;

	return index;
}

#if 1
#include "RandomName.h"

#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

#define RANDOM_NAME_FIRST "random_first.txt"
#define RANDOM_NAME_LAST "random_last.txt"

RandomName RandomName::smInstance;

RandomName & RandomName::GetInstance()
{
	// TODO: 在此处插入 return 语句
	return smInstance;
}

RandomName::RandomName()
{
	LoadFile();
}

RandomName::~RandomName()
{
}

void RandomName::LoadFile()
{
	ifstream ifsFirst;
	ifstream ifsLast;

	//打开文件
	ifsFirst.open(RANDOM_NAME_FIRST);
	ifsLast.open(RANDOM_NAME_LAST);

	if (!ifsFirst.is_open() || !ifsLast.is_open())
	{
		cout << "文件打开失败" << endl;
	}

	//从姓读取一行数据
	string first_name;
	while (getline(ifsFirst, first_name))
	{
		//从名读取一行数据
		string last_name;
		while (getline(ifsLast, last_name))
		{
			//组合塞入容器中
			string name = first_name + last_name;
			m_names.push_back(name);
		}

		//重新定位文件流指针
		ifsLast.clear(ios::goodbit);
		ifsLast.seekg(ios::beg);
	}

	//乱序容器
	srand((unsigned int)time(NULL));
	random_shuffle(m_names.begin(), m_names.end());
	
	//关闭文件
	ifsFirst.close();
	ifsLast.close();
}

std::string RandomName::GetName()
{
	string name;

	name = m_names.front();
	m_names.pop_front();

	return name;
}

void RandomName::ReleaseName(std::string szName)
{
	m_names.push_back(szName);
}

#else

#include "RandomName.h"
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

#define RANDOM_NAME_FIRST "random_first.txt"
#define RANDOM_NAME_LAST  "random_last.txt"

RandomName RandomName::smInstance;

RandomName & RandomName::GetInstance()
{
	// TODO: 在此处插入 return 语句
	return smInstance;
}

//构造函数
RandomName::RandomName()
{
	//加载文件
	LoadFile();
}

//析构函数
RandomName::~RandomName()
{
}

//读取文件 生成姓名添加到队列中
void RandomName::LoadFile()
{
	ifstream iFirst;
	ifstream iLast;

	//打开文件
	iFirst.open(RANDOM_NAME_FIRST);
	iLast.open(RANDOM_NAME_LAST);

	//判断文件打开是否成功
	if (iFirst.is_open() && iLast.is_open())
	{
		string firstName;

		//获取一行
		while (getline(iFirst, firstName))
		{
			string lastName;
			while (getline(iLast, lastName))
			{
				string finalName;
				finalName = firstName + "_" + lastName;
				m_names.push_back(finalName);
			}

			//重新定位文件流指针
			iLast.clear(ios::goodbit);
			iLast.seekg(ios::beg);
		}
	}
	else
	{
		cout << "打开文件失败" << endl;
		return;
	}

	//设置随机种子
	srandom(time(nullptr));
	//随机洗牌
	random_shuffle(m_names.begin(), m_names.end());

	iFirst.close();
	iLast.close();
}

//获取一个姓名
std::string RandomName::GetName()
{
	string retName;

	retName = m_names.front();
	m_names.pop_front();

	return retName;
}

//归还名字
void RandomName::ReleaseName(std::string szName)
{
	m_names.push_back(szName);
}


#endif

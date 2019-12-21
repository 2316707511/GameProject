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
	// TODO: �ڴ˴����� return ���
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

	//���ļ�
	ifsFirst.open(RANDOM_NAME_FIRST);
	ifsLast.open(RANDOM_NAME_LAST);

	if (!ifsFirst.is_open() || !ifsLast.is_open())
	{
		cout << "�ļ���ʧ��" << endl;
	}

	//���ն�ȡһ������
	string first_name;
	while (getline(ifsFirst, first_name))
	{
		//������ȡһ������
		string last_name;
		while (getline(ifsLast, last_name))
		{
			//�������������
			string name = first_name + last_name;
			m_names.push_back(name);
		}

		//���¶�λ�ļ���ָ��
		ifsLast.clear(ios::goodbit);
		ifsLast.seekg(ios::beg);
	}

	//��������
	srand((unsigned int)time(NULL));
	random_shuffle(m_names.begin(), m_names.end());
	
	//�ر��ļ�
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
	// TODO: �ڴ˴����� return ���
	return smInstance;
}

//���캯��
RandomName::RandomName()
{
	//�����ļ�
	LoadFile();
}

//��������
RandomName::~RandomName()
{
}

//��ȡ�ļ� ����������ӵ�������
void RandomName::LoadFile()
{
	ifstream iFirst;
	ifstream iLast;

	//���ļ�
	iFirst.open(RANDOM_NAME_FIRST);
	iLast.open(RANDOM_NAME_LAST);

	//�ж��ļ����Ƿ�ɹ�
	if (iFirst.is_open() && iLast.is_open())
	{
		string firstName;

		//��ȡһ��
		while (getline(iFirst, firstName))
		{
			string lastName;
			while (getline(iLast, lastName))
			{
				string finalName;
				finalName = firstName + "_" + lastName;
				m_names.push_back(finalName);
			}

			//���¶�λ�ļ���ָ��
			iLast.clear(ios::goodbit);
			iLast.seekg(ios::beg);
		}
	}
	else
	{
		cout << "���ļ�ʧ��" << endl;
		return;
	}

	//�����������
	srandom(time(nullptr));
	//���ϴ��
	random_shuffle(m_names.begin(), m_names.end());

	iFirst.close();
	iLast.close();
}

//��ȡһ������
std::string RandomName::GetName()
{
	string retName;

	retName = m_names.front();
	m_names.pop_front();

	return retName;
}

//�黹����
void RandomName::ReleaseName(std::string szName)
{
	m_names.push_back(szName);
}


#endif

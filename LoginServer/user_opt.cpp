#include "user_opt.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <iostream>

#include <sys/wait.h>
#include <sys/types.h>
#include <hiredis/hiredis.h>

using namespace std;

void AddUser(string &username , string &password)
{
	char buf[256] = {0};

	sprintf(buf , "./add_user.sh %s %s" , username.c_str() , password.c_str());

	system(buf);
}

bool CheckUser(string &username , string * password)
{
	bool ret = false;

	if(fork() > 0)
	{
		//father
		int status = 1;

		wait(&status);

		if(status == 0)
		{
			ret = true;
		}	

	}
	else
	{
		//child
		if(password == nullptr)
		{
			execlp("./check_user.sh" , "./check_user.sh" , username.c_str() , NULL);
		}
		else
		{
			execlp("./check_user.sh" , "./check_user.sh" , username.c_str() , password->c_str() , NULL);
		}
	}

	return ret;
}

static int g_cur_server = 0;
string CreateRoom(int room_no)
{
	/*
	string ret;
	FILE * pf = popen("./create_room.sh" , "r");

	if(pf != NULL)
	{
		char buf[64];
		fread(buf , 1 , sizeof(buf) , pf);
		ret.append(buf);
		ret.pop_back();
		pclose(pf);
	}
	return ret;

	*/

	// 1
	string server_array[]= { "122.51.68.107" };
	string cur_server = server_array[g_cur_server];
	g_cur_server++;
	g_cur_server %= sizeof(server_array)/ sizeof(server_array[0]);

	
	// 2
	string port;
	auto pc = redisConnect("127.0.0.1" , 6379);
       	if(pc != NULL)
       	{
		freeReplyObject(redisCommand(pc , "publish create_room %s:%d" , cur_server.c_str() , room_no));
		freeReplyObject(redisCommand(pc , "subscribe server_port"));

		redisReply *rp = nullptr;
		if(REDIS_OK == redisGetReply(pc ,(void **)&rp))
		{
			port.append(rp->element[2]->str);
			freeReplyObject(rp);
		}
		redisFree(pc);
       	}	       

	//3
	string ret = cur_server + " " + port;

	return ret;
}

bool CheckRoom(string & room_no)
{
	bool ret = false;
	if(fork() > 0)
	{
		//father
		int status = 1;

		wait(&status);

		if(status == 0)
		{
			//find room
			ret = true;
		}	

	}
	else
	{
		execlp("./check_room.sh" , "./check_room.sh" , room_no.c_str() , NULL);
	}
	return ret;
}

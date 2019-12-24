#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>
#include <json/json.h>
#include <iostream>
#include <hiredis/hiredis.h>

#include "user_opt.h"

#include <fcgi_stdio.h>

using namespace std;
using namespace Json;

int main()
{
	while(FCGI_Accept() >= 0)
	{
		int len = atoi(getenv("CONTENT_LENGTH"));
		char * buf;
		buf = (char *)malloc(len * sizeof(char));
		memset(buf , 0 , len);

		fread(buf , 1 , len , stdin);

		Reader reader;

		Value val;

		reader.parse(buf , val);

		string username = val["username"].asString();
		string password = val["password"].asString();
		string opt = val["opt"].asString();		

		//response json obj;

		Value res_str;

		printf("Content-type: application/json\r\n");
		if(CheckUser(username , &password))
		{
			res_str["login_result"] = "success";
			//User Success
			if(opt == "create_room")
			{
				static int room_no = 0;
				room_no++;

				//cout << "success" << endl;
				string room_info = CreateRoom(room_no);
				int pos = room_info.find(" ");
				string serverIP = room_info.substr(0,pos);
				string serverPort = room_info.substr(pos+1 , room_info.size() - pos - 1);
				
				res_str["room_IP"] = serverIP.c_str();
				res_str["room_Port"] = serverPort.c_str();
				char buf_room[32] = {0};
				sprintf(buf_room , "%d" , room_no);
				res_str["room_no"] = buf_room;

				//redis
				auto pc = redisConnect("122.51.68.107" , 6379);
				if(pc != NULL)
				{
					freeReplyObject(redisCommand(pc , "set %d %s" , room_no , room_info.c_str()));
					redisFree(pc);
				}
				else
				{	
					res_str["login_result"] = "fail";
				}

			}
			else if(opt == "follow_room")
			{
				string room_no = val["room_no"].asString();

				//connect redis
				auto pc = redisConnect("122.51.68.107" , 6379);
				if(pc != NULL)
				{
					redisReply * reply = (redisReply*)redisCommand(pc, "get %s", room_no.c_str());

					if(reply->type == REDIS_REPLY_STRING)
					{
						string room_info = reply->str;
						int pos = room_info.find(" ");
						string ip = room_info.substr(0 , pos);
						string port = room_info.substr(pos + 1 , room_info.size() - pos - 1);

						res_str["follow_result"] = "success";
						res_str["room_IP"] = ip.c_str();
						res_str["room_Port"] = port.c_str();
					}
					else
					{
						res_str["follow_result"] = "fail";
					}

				}
				else
				{
					res_str["follow_result"] = "fail";
				}
			
			}
		}
		else
		{
			//User Fail
			//cout << "fail" << endl;
			res_str["login_result"] = "fail";
		}
		FastWriter fw;

		string out = fw.write(res_str);
		printf("Content-length:%d\r\n\r\n", out.size());

		//cout<<fw.write(res_str)<< endl;
		printf("%s\r\n" , out.c_str());
		free(buf);
	}

	return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include "user_opt.h"

#include <fcgi_stdio.h>

using namespace std;

int main()
{
	while(FCGI_Accept() >= 0)
	{
		char buf[512] = {0};
		fread(buf , 1 , sizeof(buf) , stdin);
		string tmp(buf);
		string username;
		string password;
		int pos = tmp.find('&');
		username = tmp.substr(9,pos-9);
		password = tmp.substr(pos + 1 + 9 , tmp.size()-pos-1-9);
		if(CheckUser(username))
		{
			printf("Content-type:text\r\n\r\n");
			printf("Register Fail\r\n");
			return 0;
		}
		AddUser(username , password);
		printf("Content-type:text\r\n\r\n");
		printf("Register Success\r\n");
	}
	return 0;
}

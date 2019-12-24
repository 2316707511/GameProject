#ifndef __USER_OPT_H
#define __USER_OPT_H

#include <string>

using namespace std;

void AddUser(string & username , string & password);

bool CheckUser(string & username , string * password = nullptr);

string CreateRoom(int room_no);

bool CheckRoom(string & room_no);

#endif

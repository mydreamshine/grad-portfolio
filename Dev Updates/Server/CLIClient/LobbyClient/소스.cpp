//#pragma comment(lib, "ws2_32")

//#include <WS2tcpip.h>
//#include <thread>
//#include "../..//LobbyServer/LobbyServer/lobby_protocol.h"
//#include "..//..//BattleServer/BattleServer/battle_protocol.h"

#include "NWMODULE.h"
#include "NWMODULE.cpp"

#include <iostream>
#include <conio.h>
#include <string>
#include <vector>
#include <sstream>
#define BUF_SIZE 200

using namespace std;

class testfunc
{
private:
	int cd;

public:
	NWMODULE<testfunc> nw;
	void login_ok() { cout << "LOGIN OK" << endl; };
	void enque() { cout << "ENQUEUED" << endl; };
	void deque() { cout << "DEQUEUED" << endl; };
	testfunc() : nw(*this) {
		/*nw.enroll_lobby_callback(SC_PACKET_LOGIN_OK, &testfunc::login_ok);
		nw.enroll_lobby_callback(SC_PACKET_MATCH_ENQUEUE, &testfunc::enque);
		nw.enroll_lobby_callback(SC_PACKET_MATCH_DEQUEUE, &testfunc::deque);*/
	}
};

vector<string> split(string& str, char delimiter) {
	vector<string> internal;
	stringstream ss(str);
	string temp;

	if (getline(ss, temp, delimiter))
		internal.push_back(temp);
	if (getline(ss, temp))
		internal.push_back(temp);
	return internal;
}

void print_help()
{
	printf("¸í·É¾î : /enqueue, /dequeue, /add friend_id, /accept\n");
}

int main()
{
	testfunc a;
	//NWMODULE<testfunc> nw(a);

	//
	//nw.enroll_lobby_callback(SC_PACKET_LOGIN_OK, &testfunc::login_ok);
	//nw.enroll_lobby_callback(SC_PACKET_MATCH_ENQUEUE, &testfunc::enque);
	//nw.enroll_lobby_callback(SC_PACKET_MATCH_DEQUEUE, &testfunc::deque);

	a.nw.connect_lobby(0);
	//a.nw.request_login("test001");

	cout << "[CLI CLIENT]" << endl;

	print_help();

	while (true)
	{
		a.nw.update();
		string chat;
		getline(cin, chat);
		
		if (chat[0] == '/')
		{
			vector<string> token = split(chat, ' ');
			//if (token[0] == "/enqueue") { a.nw.match_enqueue(); }
			//else if (token[0] == "/dequeue") { a.nw.match_dequeue(); }
			//else if (token[0] == "/add") { a.nw.add_friend(token[1].c_str()); }
			//else if (token[0] == "/accept") {
			//	//if (friend_flag == false) continue;
			//	//nw.accept_friend();
			//}
			//else if (token[0] == "/help") {
			//	print_help();
			//}
			//else if (token[0] == "/quit")
			//	a.nw.disconnect_lobby();
		}
		a.nw.update();
	}
}
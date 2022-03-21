#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define _AFXDLL
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <vector>
#include <process.h> 

using namespace std;


bool f_end = false;
bool fl;
void send_message(void* socket);
void eraseText(int n);

int main() {
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
		return -1;

	struct sockaddr_in peer;//адрес сервера
	peer.sin_family = AF_INET;
	peer.sin_port = htons(1280);
	peer.sin_addr.s_addr = inet_addr("127.0.0.1");

	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);//возвращает дескриптор сокета
	connect(s, (struct sockaddr*)&peer, sizeof(peer));//связывает клиента с сервером, второй аргумент - указатель на адрес сервера

	char p[200], user_inf[200];

	recv(s, p, sizeof(p), 0);//читаем данные из сокета
	cout << p << endl;
	p[0] = '\0';

	cout << "Enter your ID: ";
	cin.getline(user_inf, 200);
	send(s, user_inf, sizeof(user_inf), 0);
	cout << "Your name: ";
	cin.getline(user_inf, 200);
	send(s, user_inf, sizeof(user_inf), 0);

	_beginthread(send_message, 0, (void*)s);
	
	while (true) {
		if (recv(s, p, sizeof(p), 0)) {
			fl = true;

		}
		if (f_end) {
			break;
		}
		eraseText(20);
		cout << p << endl << "Command: ";
		p[0] = '\0';
	}
	closesocket(s);
	WSACleanup();
	system("pause");
}

void send_message(void* socket) {
	SOCKET client_socket = (SOCKET)socket;
	while (true) {
		cout << "\nCommand: ";
		char str[250];
		cin.getline(str, 250, '\n');
		cout.flush();
		send(client_socket, str, sizeof(str), 0);
		if (!strcmp(str, "quit ")) {
			f_end = true;
			return;
		}
	}
}


void eraseText(int n) {
	for (int i = 0; i < n; i++) {
		cout << '\b' << " " << '\b';
	}
}

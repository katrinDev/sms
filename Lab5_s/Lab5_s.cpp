#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define _AFXDLL
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include "afx.h"
#include <winsock2.h>
#include <map>
#include <vector>
#include <process.h>   
#include <string.h>
#include <time.h>
#include <fstream>
#include <process.h>

using namespace std;

CFile f;
CFileException ex;
clock_t start, finish;

class Client {
	string id;
	SOCKET socket;
	string name;
public:
	Client(SOCKET Socket, string ID = "Unknown", string Name = "Unknown") {
		id = ID;
		name = Name;
		socket = Socket;
	}
	SOCKET get_socket() { return socket;}
	string get_name() { return name; }
	string get_id() { return id; }
	void set_id(string Id) { id = Id; }
	void set_name(string Name) { name = Name; }
	void set_socket(SOCKET Socket) { socket = Socket; }
};

vector <Client> clients;

struct SMS {
	string sender;
	string recipient;
	string message;
};

void sms(char* p, int n, SOCKET s);
void SMSworking(void* client);
// удалить сообщение с номером
void write(SMS smska);
SMS del(SMS smska);
string GetClientName(string ID);
string GetClientName(SOCKET s);
string GetClientId(SOCKET s);
void removeClient(SOCKET s);



int main() {

	WORD wVersionRequested;
	WSADATA wsaData;//активируем использование сокетов в Windows
	wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData)) 
		return -1;

	sockaddr_in local;//структура имени сокета
	local.sin_family = AF_INET;//устройство использует глобальную сеть по протоколу IPv4, AF_INET6 — IPv6
	local.sin_port = htons(1280);//превращает hardware to network short при этом используется ushort
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

	int c = bind(s, (struct sockaddr*)&local, sizeof(local));//присваиваем сокету сервера имя, до ее выполнения ни один клиент не сможет получить доступ
	int r = listen(s, 5);//второй аргумент определяет, как много запросов связи может быть принято на сокет одновременно; информируем ОС, что готовы слушать, до этой функции всякое требование связи с этим сокетом будет отвергнуто

	while (true) {
		sockaddr_in remote;
		int j = sizeof(remote);
		SOCKET newS = accept(s, (struct sockaddr*)&remote, &j);//accept используется сервером для принятия связи на сокет, возвращает новый сокет-дескриптор, через который и происходит общение клиента с сервером. 
		//Пока устанавливается связь клиента с сервером, функция accept блокирует другие запросы связи с данным сервером, а после установления связи "прослушивание" запросов возобновляется.
		//remote хранит указатель на адрес клиента, который отправляет запрос
		cout << "New connection has been established!\n";
		Client cl(newS);
		_beginthread(SMSworking, 0, &cl);//функции SMSworking передается параметр - адрес структуры инфы о новом клиенте
		cout << "New thread was built!\n" << endl;
	}
	WSACleanup();
	system("pause");

	return 0;
}


void SMSworking(void* client) {

	char p[200], command[200], user_inf[200];
	command[0] = '\0'; p[0] = '\0';
	strcat(p, "SMS center connected...\n");
	char error[] = "Incorrect command! Could u please try again?\n";
	char menu[] = "To send a message: 'sms  <ID of the recipient>  <message>'\nTo stop messaging: 'quit '\n";
	Client* cl = (Client*)client;

	SOCKET newS = cl->get_socket();
	send(newS, p, sizeof(p), 0);
	recv(newS, user_inf, sizeof(user_inf), 0);//id нового клиента
	cl->set_id(user_inf);
	flush(cout);

	recv(newS, user_inf, sizeof(user_inf), 0);//имя нового клиента
	cl->set_name(user_inf);
	cl->set_socket(newS);
	cout << cl->get_name() << " was connected\n";
	send(newS, menu, sizeof(menu), 0);
	clients.push_back(*cl);

	while (true) {
		int pr = recv(newS, p, sizeof(p), 0);
		if (pr == 50) continue;
		int i = 0;
		bool f = 0;
		for (; i < strlen(p); i++) {
			if (p[i] == ' ')
				f = 1;
		}
		i = 0;
		if (!f) {
			send(newS, error, sizeof(error), 0);
		}
		else {
			while (p[i] != ' ') {
				command[i] = p[i];
				i++;
			};
			command[i] = ' ';
			command[++i] = '\0';

			if (!strcmp(command, "sms ")) {
				sms(p, i, newS);
				command[0] = '\0';
			}
			else if (!strcmp(command, "quit ")) {
				char buy[] = "Buy! See u!";
				cout << "User " << GetClientName(newS) << " already quit\n";
				removeClient(newS);
				send(newS, buy, sizeof(buy), 0);
				closesocket(newS);
				return;
			}
			else
				send(newS, error, sizeof(error), 0);
		}
	}
}


void sms(char* p, int n, SOCKET s) {
	char str[200], inf[200], str1[200];
	int i, j = 0;
	SMS smska;
	SMS sms_file;
	bool flag = false;

	for (i = n; p[i] != ' '; i++) {
		inf[j] = p[i];
		j++;
	}
	inf[j] = '\0';
	smska.recipient = inf;
	SOCKET r_socket = 0;

	for (int i = 0; i < clients.size(); i++) {
		if (clients[i].get_id() == smska.recipient) {
			cout << "Recipient exists\n";
			r_socket = clients[i].get_socket();
			flag = true;
			break;
		}
	}
	if (!flag) {
		strcpy_s(str, "There is no such client!");
		send(s, str, sizeof(str), 0);
		return;
	}
	j = 0;
	inf[0] = '\0';
	for (i = i + 1; p[i] != '\0'; i++) {
		inf[j] = p[i];
		j++;
	}
	inf[j] = '\0';
	smska.message = inf;
	smska.sender = GetClientId(s);
	write(smska);

	start = clock();

	while (true) {
		finish = clock();

		if (((double)finish - (double)start) / CLOCKS_PER_SEC > 3) {
			sms_file = del(smska);
			str[0] = '\0';
			strcat_s(str, "from ");
			strcat_s(str, GetClientName(s).c_str());
			strcat_s(str, " : ");
			strcat_s(str, sms_file.message.c_str());
			send(r_socket, str, sizeof(str), 0);
			str1[0] = '\0';
			strcat_s(str1, "That message was sent for ");
			strcat_s(str1, GetClientName(r_socket).c_str());
			strcat_s(str1, " : ' ");
			strcat_s(str1, sms_file.message.c_str());
			strcat_s(str1, " '");
			send(s, str1, sizeof(str1), 0);
			cout << "SMS '" << sms_file.message << "' from " << GetClientName(sms_file.sender) << " for " << GetClientName(r_socket) << " was sent!\n";
			break;
		}
	}
}


void write(SMS smska) {
	ofstream file("D:\\BSUIR\\3 semester\\KS\\LABS\\SMS.txt", ios::app);
	if (!file) {
		cerr << "The file can't be open!" << endl;
		return;
	}
	file << smska.sender << "  " << smska.recipient << "  " << smska.message << "\n";
	file.close();
	cout << "Message was succesfully saved in the SMS-senter" << endl;
}

SMS del(SMS smska) {
	string buf, sender, recipient, msg;
	SMS sms_file;
	bool flag = false;
	fstream file("D:\\BSUIR\\3 semester\\KS\\LABS\\SMS.txt", ios::in | ios::out);
	if (!file) {
		cerr << "The file can't be open!" << endl;
		sms_file.sender = sms_file.recipient = sms_file.message = '\0';
		return sms_file;
	}
	while (file >> sms_file.sender >> sms_file.recipient >> sms_file.message) {
		if (tie(sms_file.sender, sms_file.recipient, sms_file.message) == tie(smska.sender, smska.recipient, smska.message)) {
			flag = true;
		}
		else
			buf += sms_file.sender + "   " + sms_file.recipient + "   " + sms_file.message + "\n";
	}
	file.close();

	if (!flag)
		cerr << "There is no such message at the SMS-senter!\n";
	file.open("D:\\BSUIR\\3 semester\\KS\\LABS\\SMS.txt", ios::out);
	file << buf;
	file.close();
	cout << "----------------------\n";
	cout << "Message was succesfully deleted from the SMS-senter" << endl;

	return sms_file;
}


void removeClient(SOCKET s) {
	for (int i = 0; i < clients.size(); i++) {
		if (s == clients[i].get_socket()) {
			clients.erase(clients.begin() + i);
			return;
		}
	}
}

string GetClientName(string ID) {
	for (int i = 0; i < clients.size(); i++) {
		if (ID == clients[i].get_id()) {
			return clients[i].get_name();
		}
	}
	return "Unknown";
}

string GetClientName(SOCKET s) {
	for (int i = 0; i < clients.size(); i++) {
		if (s == clients[i].get_socket()) {
			return clients[i].get_name();
		}
	}
	return "Unknown";
}

string GetClientId(SOCKET s) {
	for (int i = 0; i < clients.size(); i++) {
		if (s == clients[i].get_socket()) {
			return clients[i].get_id();
		}
	}
	return "Unknown";
}


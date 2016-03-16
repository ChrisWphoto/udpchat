#pragma once
/*UDP Echo Client
UDPEchoClient.cpp
*/
#include <stdio.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "cipher.h"
#include <string>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <thread>
#include <process.h>
#pragma comment(lib, "Ws2_32") 

using namespace std;
SOCKET theSocket;
int notused = 0;
bool wait = true;
string username = "";
//global which will be updated by secondary thread
vector<string> recMsgs;

/*
encryption function
*/
string crypt(string msg){
	string eMsg = "";

	int i = 0;
	while (i < msg.length()){
		eMsg += encr[msg[i]];
		i++;
	}
	return eMsg;
}


/*
Assumption: There will always be 2 semicolons in msg param.
*/

string* deCrypt(char* msg){
	string * rMsg = new string[2];
	if (msg[0] == '\0'){
		rMsg[0] =  "empty reply";
		rMsg[1] = "empty reply";
		
	}
	
	//Get Rid of first 2 semi colons
	int i = 0;
	int numSemi = 0;
	while ( msg[i] != '\0' && numSemi < 2){
		rMsg[0] += msg[i];
		if (msg[i] == ';')
			numSemi++;

		i++;
	}

	while (msg[i] != '\0'){
		rMsg[1] += decr[msg[i]];
		i++;
	}
	return rMsg;
}


void l1(void * notused){	
	int recv_size;
	char server_reply[65000];

	

	while (true){
		//Receive a reply from the server	
		if ((recv_size = recvfrom(theSocket, server_reply, 65000, 0, NULL, NULL)) == SOCKET_ERROR)	{
			std::cout << "recv failed" << WSAGetLastError() << endl;
			
		}
		else {
			try{
				server_reply[recv_size] = '\0'; //null terminator
				string* headerAndMsg = deCrypt(server_reply);
				std::cout << std::endl << "Acknowledgement: "  << headerAndMsg[0] << endl << headerAndMsg[1] << endl;
				if (wait == true) wait = !wait;
			}
			catch (...){
				std::cout << GetLastError() << "error on 2nd thead" << std::endl;
			}
			
		}
	}
}



int main(int argc, char *argv[]){
	srand(time(NULL));
	int min = 10000;
	int randMsgNum = (rand() % (RAND_MAX - min) + min);


	//Server Vars
	WSADATA wsa;
	struct sockaddr_in server;
	char *message, server_reply[500];
	int recv_size;


	std::cout << "Initialising Winsock...." << endl;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)	{
		printf("Failed. Error Code : %d", WSAGetLastError());		return 1;
	}
	std::cout << "Initialised" << endl;
	//Create a socket	
	if ((theSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	std::cout << "socket created" << endl;
	server.sin_addr.s_addr = inet_addr("204.76.188.23");
	server.sin_family = AF_INET;
	server.sin_port = htons(23456);
	std::cout << "IP: 204.76.188.23 Port: 23456" << endl;
	std::cout << "Starting MSG number: " << randMsgNum << endl;

	//set SO_RESESADDR on a socket s to true	
	int optval = 1;
	setsockopt(theSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval);


	//Main Program Loop
	string username;
	int msgPlus = 0;
	while (true) {

		//set username if not set
		if (username == ""){
			std::cout << "Welcome to Chat: Enter your username:";
			std::cin >> username;


			//Send user info to server
			string sMsg = to_string(randMsgNum) + ";1;" + crypt(username);
			message = const_cast<char*>(sMsg.c_str());

			if (sendto(theSocket, message, strlen(message), 0, (struct sockaddr*)&server, sizeof(server)) < 0)	{
				cout << "Send Failed" << endl;
				return 1;
			}

			
			_beginthread(l1, 0, (void *)  notused);
			while (wait);

		}

		//Display user menu
		char req;
		cout << "Enter q (for quit), s (send msg), or c (check for msgs): ";
		std::cin >> req;

		//quit
		if (req == 'q') {
			//Send user info to server
			string sMsg = to_string(randMsgNum) + ";3;" + crypt(username);
			message = const_cast<char*>(sMsg.c_str());

			if (sendto(theSocket, message, strlen(message), 0, (struct sockaddr*)&server, sizeof(server)) < 0)	{
				cout << "Send Failed" << endl;
				return 1;
			}

		}

		//Send Dialog 
		if (req == 's'){
			cout << "Enter Buddy Name: ";
			string buddy;
			cin >> buddy;
			cout << "Enter message here: ";
			string userMsg;
			cin >> userMsg;

			//adding new line
			buddy += '\n';

			//increment msg
			int randMsgPlus = ++randMsgNum;
			//Construct Buddy Message
			string sMsg = to_string(randMsgPlus) + ";2;" + crypt((username + '\n')) + crypt(buddy) + crypt(userMsg);
			message = const_cast<char*>(sMsg.c_str());

			//Send Message
			if (sendto(theSocket, message, strlen(message), 0, (struct sockaddr*)&server, sizeof(server)) < 0)	{
				cout << "Send Failed" << endl;
				return 1;
			}
			
		}


		if (req == 'c'){
			//Send some data	
			string sMsg = to_string(randMsgNum++) + ";1;" + crypt(username);
			message = const_cast<char*>(sMsg.c_str());

			if (sendto(theSocket, message, strlen(message), 0, (struct sockaddr*)&server, sizeof(server)) < 0)	{
				cout << "Send Failed" << endl;
				return 1;
			}
			cout << "Msg Sent" << endl;
		}

		

	}

	closesocket(theSocket);
	WSACleanup();
	system("pause");
	return 0;
}




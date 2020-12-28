// TCPServerMultipleConnectionTutorial.cpp : Defines the entry point for the application.
//

#include "TCPServerMultipleConnectionTutorial.h"
#include <WS2tcpip.h> // This is the framework Windows uses to use tcp/ip
#pragma comment (lib, "ws2_32.lib") // This is the same as adding the library, but idk where it is in settings, so this does it for me

using namespace std;

int main()
{
	// Initialize winsock
	WSADATA wsData; // A struct for holding the socket information
	WORD ver = MAKEWORD(2, 2); // Version 2.2

	int wsOk = WSAStartup(ver, &wsData); // Starts up the stuff (Version we're using, Where we are storing our port implementation information)
	if (wsOk != 0) {
		cerr << "Can't initialize winsock!" << endl;
		return 1;
	}

	// Create a socket (just a number, it's a handle)
	SOCKET listening = socket(AF_INET /*A family of addresses, ipv4*/, SOCK_STREAM /*tcp*/, 0 /*flag?*/);
	if (listening == INVALID_SOCKET) {
		cerr << "Can't create socket!" << endl;
		WSACleanup(); // If failed, clean this
		return 2;
	}

	//Bind the ip address and port to the socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000 /*This is the port number*/); // HostToNetworkShort, networks are big endian and computers are little endian, need to switch
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // We'll bind to any address, could use inet_pton...

	bind(listening, (sockaddr*)&hint, sizeof(hint)); // Binding our "listening" socket to the port address so we know which port to connect to on client side

	// Tell the socket to start listening and the maximum number of connections allowed
	listen(listening, SOMAXCONN /*Maximum connections allowed*/);

	// When we hear something, create a client socket to communicate back with the client
	sockaddr_in client;
	int  clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize); // The accept function takes the socket that listening heard from and creates a new socket with the connection
	if (clientSocket == INVALID_SOCKET) {
		cerr << "Client socket invalid." << endl;
		return 3;
	}

	char host[NI_MAXHOST]; // Client's remote name
	char service[NI_MAXHOST]; // Service (i.e. port) the client is connected on

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXHOST);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		cout << host << " connected on port " << service << endl;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connected on port " << ntohs(client.sin_port) << endl;
	}

	// Close listening socket because we already accepted the communication with a new socket "clientSocket"
	closesocket(listening);

	// While loop: accept and echo message back to client
	char buf[4096];

	while (true) {
		ZeroMemory(buf, 4096);

		// Wait for client to send data
		int bytesReceived = recv(clientSocket, buf, 4096, 0);
		if (bytesReceived == SOCKET_ERROR) {
			cerr << "Error in recv(): " << SOCKET_ERROR << endl;
			return 4;
		}

		if (bytesReceived == 0) {
			cout << "Client Disconnected" << endl;
			break;
		}
		cout << "Message received: " << buf << endl;
		// Echo message back to client
		send(clientSocket, buf, bytesReceived + 1 /*When we get a message, we don't get our null termination character, so we have to put it back*/, 0);

	}

	// Close the socket
	closesocket(clientSocket);

	// Shutdown winsock
	WSACleanup();

	return 0;

}

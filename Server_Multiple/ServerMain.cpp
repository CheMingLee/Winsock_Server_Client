#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <iostream>
#include <winsock2.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

using namespace std;

#define DEFAULT_PORT 27015
#define DEFAULT_BUFLEN 512
#define MAX_CLIENTS 50

char recvbuf[DEFAULT_BUFLEN] = "";
int len, receiveres, clients_connected = 0;
bool active = TRUE;

SOCKET server_socket = INVALID_SOCKET;
SOCKET client_fd;
sockaddr_in server;

//table for clients sockets
struct _clients_b
{
	bool connected;
	SOCKET ss;
};

_clients_b clients[MAX_CLIENTS];

//function declarations
void start_server();

int main()
{
	cout << "Server starting..." << endl;
	
	//start the server and do basic tcp setup ------------------
	start_server();
	
	//the main loop
	while (active)
	{
		//start accepting clients ------------------------------
		len = sizeof(server);
		client_fd = accept(server_socket, (struct sockaddr*)&server, &len);
		
		//our client is a real thing?
		if (client_fd != INVALID_SOCKET)
		{
			//save client socket into our struct table
			clients[clients_connected].ss = client_fd;
			clients[clients_connected].connected = TRUE;
			//and of course we need a calculator too
			clients_connected++;
			cout << "New client: " << client_fd << endl;
		}
		
		//we might need to add some delays cause our code might be too fast
		//commenting this function will eat your cpu like the hungriest dog ever
		//plus we don't need to loop that fast anyways
		Sleep(1);
		
		//receiving and sending data ---------------------------
		//we have clients or no?
		if (clients_connected > 0) {
			//lets go through all our clients
			for (int cc = 0; cc < clients_connected; cc++)
			{
				memset(&recvbuf, 0, sizeof(recvbuf));
				if (clients[cc].connected)
				{
					//receive data
					receiveres = recv(clients[cc].ss, recvbuf,
						DEFAULT_BUFLEN, 0);
					//and echo it back if we get any
					if (receiveres > 0)
					{
						Sleep(10);
						cout << "Client data received: " << recvbuf
							<< endl;
						send(client_fd, recvbuf, strlen(recvbuf),
							0);
					}
					//how to close connection
					//this just for quick example
					//so you are just getting rid off client's socket data
					else if (receiveres == 0 || strcmp(recvbuf, "disconnect") == 0)
					{
						cout << "Client disconnected." << endl;
						clients[cc].connected = FALSE;
						clients_connected--;
						//delete [cc] clients;
					}
				}
			}
		}
	}
	
	//when we shut down our server
	closesocket(server_socket);
	
	// Clean up winsock
	WSACleanup();
	
	return 0;
}

void start_server()
{
	int wsaresult, i = 1;
	WSADATA wsaData;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(DEFAULT_PORT);
	
	// Initialize Winsock
	wsaresult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	
	//if error
	if (wsaresult != 0)
	{
		printf("WSAStartup failed with error: %d\n", wsaresult);
	}
	
	// Create a SOCKET for connecting to server
	server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
	}
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&i, sizeof(i));
	
	//Binding part
	wsaresult = bind(server_socket, (sockaddr*)&server, sizeof(server));
	if (wsaresult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(server_socket);
		WSACleanup();
	}
	
	// Setup the TCP listening socket
	wsaresult = listen(server_socket, 5);
	unsigned long b = 1;
	
	//make it non blocking
	ioctlsocket(server_socket, FIONBIO, &b);
	if (wsaresult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(server_socket);
		WSACleanup();
	}
}

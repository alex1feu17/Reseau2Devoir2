#include <iostream>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

int main()
{
	// Initialisation et validation de winsock
	WSADATA wsData;
	if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) {
		cerr << "Erreur d'initialisation." << endl;
		return 99;
	}

	// Paramètres de connexion
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(54000);
	address.sin_addr.S_un.S_addr = INADDR_ANY;

	//Création d'un set de sockets
	fd_set sockets;
	FD_ZERO(&sockets);

	// Création et validation d'un socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		cerr << "Socket invalide." << endl;
		return 99;
	}

	// Binding à l'adresse, mise en écoute & ajout au set
	bind(listening, (sockaddr*)&address, sizeof(address));
	listen(listening, SOMAXCONN);
	FD_SET(listening, &sockets);
	cout << "Serveur en ecoute de connexions..." << endl;

	while (true)
	{
	}
}

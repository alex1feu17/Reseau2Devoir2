#include <iostream>
#include <WS2tcpip.h>
#include <sstream>

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
	address.sin_port = htons(12500);
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

	while (true) {
		fd_set copy = sockets;
		int count = select(0, &copy, nullptr, nullptr, nullptr);

		// On loop à travers tous nos sockets
		for (int i = 0; i < count; i++) {
			SOCKET socketIn = copy.fd_array[i];

			// Si est une nouvelle connexion
			if (socketIn == listening) {

				// Création d'un socket client et ajout au set
				SOCKET client = accept(listening, nullptr, nullptr);
				FD_SET(client, &sockets);
				cout << "Client connecté!" << endl;

				// Envoie d'une réponse au client
				string msg = "Connecté au serveur!";
				send(client, msg.c_str(), msg.size() + 1, 0);
			}
			else {

				//Réception du message
				char buf[4096];
				ZeroMemory(buf, 4096);
				int bytes = recv(socketIn, buf, 4096, 0);
				if (bytes <= 0) {

					// On retire le client
					closesocket(socketIn);
					FD_CLR(socketIn, &sockets);
				}
				else {
					for (int i = 0; i < sockets.fd_count; i++) {
						SOCKET socketOut = sockets.fd_array[i];

						ostringstream os;
						if (socketOut == socketIn) os << "MOI >> " << buf << "\r\n";
						else os << "#" << socketIn << " >> " << buf << "\r\n";

						string msg = os.str();
						send(socketOut, msg.c_str(), msg.size() + 1, 0);
					}
				}
			}
		}
	}
}

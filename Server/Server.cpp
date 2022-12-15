#include <iostream>
#include <WS2tcpip.h>
#include <sstream>
#include <map>
#include <vector>
#include <set>

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

	map<SOCKET, string> names;

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
				cout << "Client connecte!" << endl;

				// Envoie d'une réponse au client
				string msg = "Connecte au serveur!";
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
					names.erase(socketIn);
					cout << "Client deconnecte!" << endl;
				}
				else {

					// Construction d'un string
					string message = "";
					for (int i = 0; i < bytes; i++) message += buf[i];
					while (message.front() == ' ') message = message.substr(0, 1);

					// Socket est-il enregistré?
					bool registered = false;
					for (auto const& element : names)
						if (element.first == socketIn) registered = true;

					// Sinon -> Premier message = nom
					if (!registered) {
						names[socketIn] = message;
					}
					else if (message == "list")
					{
						string s = "Connectes: [";
						for (auto const& element : names)
							s += element.second + ", ";
						s += "]";
						send(socketIn, s.c_str(), s.size() + 1, 0);
					}
					else {
						set<string> targets;
						string users = "";
						if (message.front() == '{') {
							int cmdEnd = message.find('}');
							if (cmdEnd != std::string::npos) {
								users = message.substr(1, cmdEnd - 1);
								users.erase(remove(users.begin(), users.end(), ' '), users.end());
							}
							message = message.substr(cmdEnd + 1, message.length() - 1);
							while (message.front() == ' ') message = message.substr(1, message.length() - 1);
							if (users.length() != 0) {
								stringstream ss(users);
								string user;
								while (!ss.eof()) {
									getline(ss, user, ',');
									targets.insert(user);
								}
							}
						}

						if (targets.size() == 0)
							for (auto const& element : names)
								if (element.first != socketIn)
									targets.insert(element.second);

						for (set<string>::iterator target = targets.begin(); target != targets.end(); ++target) {
							for (auto const& element : names) {
								if (element.second == *target && element.first != socketIn) {
									SOCKET socketOut = element.first;

									string name = "Unknown";
									for (auto const& element : names)
										if (element.first == socketIn)
											name = element.second;

									string msg = name + " >> " + message + "\r\n";
									send(socketOut, msg.c_str(), msg.size() + 1, 0);
								}
							}
						}

						string msg = "Moi >> " + message + "\r\n";
						send(socketIn, msg.c_str(), msg.size() + 1, 0);
					}
				}
			}
		}
	}
}

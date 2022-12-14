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
					// Sinon si message est "list"
					else if (message == "list")
					{
						string s = "Connectes: [";
						for (auto const& element : names)
							s += element.second + ", ";
						s += "]";
						send(socketIn, s.c_str(), s.size() + 1, 0);
					}
					else {
						// On créer un set d'utilisateurs cibles
						set<string> targets;
						string userList = "";
						
						// Si message commence par { et contient }
						int cmdEnd = message.find('}');
						if (message.length() > 0 && message.front() == '{' && cmdEnd != std::string::npos) {

							// On extrait la liste de cible du message et on y retire les espaces
							userList = message.substr(1, cmdEnd - 1);
							userList.erase(remove(userList.begin(), userList.end(), ' '), userList.end());

							// On extrait le reste du message et on retire les espaces au debut (Si l'utiisateur a mis un espace après la liste)
							message = message.substr(cmdEnd + 1, message.length() - 1);
							while (message.length() > 0 && message.front() == ' ') message = message.substr(1, message.length() - 1);

							// On sépare les élements de la liste et on les ajoute dans le set
							if (userList.length() > 0) {
								stringstream ss(userList);
								string user;
								while (!ss.eof()) {
									getline(ss, user, ',');
									targets.insert(user);
								}
							}
						}

						// Si le message n'est pas vide
						if (message.length() > 0) {

							// Si aucune cible n'a été envoyé, on ajoute tous les utilisateurs
							if (targets.size() == 0)
								for (auto const& element : names)
									if (element.first != socketIn)
										targets.insert(element.second);

							// On loop parmis toutes les cibles
							for (set<string>::iterator target = targets.begin(); target != targets.end(); ++target) {

								// On cherche la cible dans notre map de noms
								for (auto const& element : names) {

									//Si socket trouvé
									if (element.second == *target && element.first != socketIn) {
										SOCKET socketOut = element.first;

										// On trouve le nom de l'expéditeur
										string name = "Inconnu";
										for (auto const& element : names)
											if (element.first == socketIn)
												name = element.second;

										// On envoit le message
										string msg = name + " >> " + message + "\r\n";
										send(socketOut, msg.c_str(), msg.size() + 1, 0);
									}
								}
							}

							// On renvoit à l'expéditeur
							string msg = "Moi >> " + message + "\r\n";
							send(socketIn, msg.c_str(), msg.size() + 1, 0);
						}
					}
				}
			}
		}
	}
}

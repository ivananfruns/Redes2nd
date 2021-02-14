#pragma once

#include"SFML/Network.hpp"
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <iostream>
#include<string>
#include<vector>
#include <ctime>
#include<fstream>
#include <signal.h>
#include <stdio.h>
#include "rapidxml.hpp"
#include <thread>
#include <sstream>
#include <memory>

const int MAX_VIDA = 10;
const int PX_VENTANA = 600;
const float LADO_BALDOSA = 30.f;
const int BALDOSA_POR_LADO = PX_VENTANA / LADO_BALDOSA;
const int NUM_TRENES = 3;

enum class Commands {
	ASK_USERNAME, USERNAME, ASK_PASSWORD, PASSWORD, INVALID_PASSWORD, ASK_RACE, RACE,
	ASK_CHARACTER_NAME, CHARACTER_NAME, ASK_MAP, MAP, PLAY, ASK_GO_TO, GO_TO, ASK_PICK_UP, PICK_UP, ASK_STATE, STATE, ASK_ATTACK, ATTACK, ASK_SEE, SEE, AKNOWLEDGE_INFO, DISCONNECT, WIN
};


int main()
{

	sf::TcpSocket* socket = new sf::TcpSocket;
	sf::Socket::Status status = socket->connect("127.0.0.1", 52000, sf::seconds(2.f));

	if (status != sf::Socket::Done)
	{
		std::cout << "no se ha podido establecer la conexion" << std::endl;
		socket->disconnect();
	}
	sf::Packet packet;
	Commands command;
	int aux;
	bool checkInfo;
	std::string info;
	bool disconnect = false;
	while (!disconnect)
	{
		packet.clear();
		if (socket->receive(packet) != sf::Socket::Done)
		{
			std::cout << "Se ha perdido la conexión con el servidor" << std::endl;
			disconnect = true;
		}
		else
		{
			packet >> aux;
			command = (Commands)aux;
			switch (command) {
			case Commands::ASK_USERNAME: {
				std::cout << "Enter your username" << std::endl;
				std::cin >> info;
				packet.clear();
				packet << static_cast<int32_t>(Commands::USERNAME) << info;
				socket->send(packet);
				break;
			}
			case Commands::ASK_PASSWORD: {
				std::cout << "Enter your password" << std::endl;
				std::cin >> info;
				packet.clear();
				packet << static_cast<int32_t>(Commands::PASSWORD) << info;
				socket->send(packet);

				break;
			}

			case Commands::INVALID_PASSWORD: {
				std::cout << "Invalid password, enter your username again" << std::endl;
				std::cin >> info;
				packet.clear();
				packet << static_cast<int32_t>(Commands::USERNAME) << info;
				socket->send(packet);

				break;
			}
			case Commands::ASK_RACE: {
				std::cout << "Choose your race" << std::endl;
				std::cout << "1. HUMAN" << std::endl;
				std::cout << "2. ELF" << std::endl;
				std::cout << "3. DWARF" << std::endl;
				std::cout << "4. FAIRY" << std::endl;
				std::cout << "5. GIANT" << std::endl;

				std::cin >> aux;
				packet.clear();
				packet << static_cast<int32_t>(Commands::RACE) << aux;
				socket->send(packet);

				break;
			}

			case Commands::ASK_CHARACTER_NAME: {
				std::cout << "Enter your character name" << std::endl;
				std::cin >> info;
				packet.clear();
				packet << static_cast<int32_t>(Commands::CHARACTER_NAME) << info;
				socket->send(packet);

				break;
			}

			case Commands::ASK_MAP: {
				packet >> aux;
				std::cout << "Write the number map that you want to play in (invalid entry will open map 1) \n";
				for (int i = 0; i < aux; i++)
				{
					packet >> info;
					std::cout << i + 1 << ". " << info << std::endl;
				}
				std::cin >> aux;
				packet.clear();
				packet << static_cast<int32_t>(Commands::MAP) << aux;
				socket->send(packet);

				break;
			}

			case Commands::PLAY: {
				std::cout << "Es tu turno, utiliza N, E, O, S para moverte \n";
				bool correctInput = false;
				while (!correctInput)
				{
					
					std::cin >> info;
					if (info == "N" || info == "S" || info == "W" || info == "E")
					{
						packet.clear();
						packet << static_cast<int32_t>(Commands::ASK_GO_TO) << info;
						socket->send(packet);
						correctInput = true;
					}
					else if (info == "Pickup" || info == "pickup" || info == "PICKUP" || info == "PickUp")
					{
						packet.clear();
						packet << static_cast<int32_t>(Commands::ASK_PICK_UP);
						socket->send(packet);
						correctInput = true;
					}
					else if (info == "State" || info == "state" || info == "STATE")
					{
						correctInput = true;
						packet.clear();
						packet << static_cast<int32_t>(Commands::ASK_STATE);
						socket->send(packet);
					}
					else if (info == "Attack" || info == "attack" || info == "ATTACK")
					{
						correctInput = true;
						packet.clear();
						packet << static_cast<int32_t>(Commands::ASK_ATTACK);
						socket->send(packet);
					}
					else if (info == "See" || info == "see" || info == "SEE")
					{
						correctInput = true;
						packet.clear();
						packet << static_cast<int32_t>(Commands::ASK_SEE);
						socket->send(packet);
					}
					else if (info == "disconect")
					{
						correctInput = true;
						packet.clear();
						packet << static_cast<int32_t>(Commands::DISCONNECT);
						socket->send(packet);
						disconnect = true;

					}
					if(!correctInput)
						std::cout << "Invalid instruction, please enter a new one:" << std::endl;
				}
				break;
			}

			case Commands::GO_TO:
			{
				packet >> checkInfo;
				if (checkInfo)
				{
					std::cout << "You have entered a new room" << std::endl;
				}
				else
				{
					std::cout << "You can't go that way!" << std::endl;
				}
				packet.clear();
				packet << static_cast<int32_t>(Commands::AKNOWLEDGE_INFO);
				socket->send(packet);
				break;
			}

			case Commands::PICK_UP:
			{
				packet >> checkInfo;
				if (checkInfo)
				{
					packet >> info;
					std::cout << "You've found the object: " << info << std::endl;
				}
				else
				{
					std::cout << "There is no objects to collect in this room" << std::endl;
				}
				packet.clear();
				packet << static_cast<int32_t>(Commands::AKNOWLEDGE_INFO);
				socket->send(packet);
				break;
			}

			case Commands::STATE:
			{
				packet >> aux;
				std::cout << "Your Health points are: " << aux << std::endl;
				packet >> aux;
				int objects = aux;
				if (objects > 0)
				{
					for (int i = 0; i < objects; i++)
					{
						packet >> aux >> info;
						std::cout << aux << ". " << info << std::endl;
					}
				}
				else
				{
					std::cout << "Inventory empty" << std::endl;
				}
				packet.clear();
				packet << static_cast<int32_t>(Commands::AKNOWLEDGE_INFO);
				socket->send(packet);
				break;
			}

			case Commands::ATTACK:
			{
				packet >> checkInfo;
				if (checkInfo)
				{
					packet >> aux;
					std::cout << "You have Atacked! Your Remaining HP is " << aux << std::endl;
					packet >> aux;
					if (aux == 0)
					{
						std::cout << "You have defeated the enemy " << std::endl;
					}
					else
					{
						std::cout << "The remaining HP of the moster is " << aux << std::endl;
					}
				}
				else
				{
					std::cout << "There is no enemy to attack" << std::endl;
				}
				packet.clear();
				packet << static_cast<int32_t>(Commands::AKNOWLEDGE_INFO);
				socket->send(packet);
				break;
			}

			case Commands::SEE:
			{
				packet >> aux;
				std::cout << "Your are in room " << aux << std::endl;
				std::cout << "You can go:" << std::endl;
				for (int i = 0; i < 4; i++)
				{
					packet >> aux;
					if (aux>=0)
					{
						switch (i) {
						case 0:
							std::cout << "North" << std::endl;
							break;
						case 1:
							std::cout << "East" << std::endl;
							break;
						case 2:
							std::cout << "South" << std::endl;
							break;
						case 3:
							std::cout << "West" << std::endl;
							break;
						default:
							break;
						}
					}
				}

				packet >> checkInfo;
				if (checkInfo)
				{
					packet >> info;
					std::cout << "Object " << info << " in the room" << std::endl;
				}
				else
				{
					std::cout << "There are no objects in the room" << std::endl;
				}
				packet >> checkInfo;
				if (checkInfo)
				{
					packet >> info;
					std::cout << "Enemy " << info << " in the room" << std::endl;
				}
				else
				{
					std::cout << "There are no enemies in the room" << std::endl;
				}
				packet.clear();
				packet << static_cast<int32_t>(Commands::AKNOWLEDGE_INFO);
				socket->send(packet);
				break;
			}
			case Commands::WIN:
			{
				packet.clear();
				packet << static_cast<int32_t>(Commands::DISCONNECT);
				socket->send(packet);
				disconnect = true;

				std::cout << "you Won! press any key to exit";

				std::cin >> info;
				break;
			}
			default:
				break;
			}
		}
	}

	socket->disconnect();

}
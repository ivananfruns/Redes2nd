#include"SFML/Network.hpp"
/*
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
*/
#include <iostream>
#include<sstream>
#include<string>
#include<vector>
#include <ctime>
#include<fstream>
#include <signal.h>
#include <stdio.h>
#include "rapidxml.hpp"
#include <thread>
#include <memory>

enum class Commands {
	ASK_USERNAME, USERNAME, ASK_PASSWORD, PASSWORD, INVALID_PASSWORD, ASK_RACE, RACE, ASK_CHARACTER_NAME, CHARACTER_NAME, ASK_MAP,
	MAP, PLAY, ASK_GO_TO, GO_TO, ASK_PICK_UP, PICK_UP, ASK_STATE, STATE, ASK_ATTACK, ATTACK, ASK_SEE, SEE, AKNOWLEDGE_INFO, DISCONNECT, WIN
};

enum class Races { HUMAN, ELF, DWARF, FAIRY, GIANT };
enum class ObjectType { POTION, WEAPON, ARMOR };

struct enemy {
	std::string name;
	Races race;
	int atack;
	int def;
	int HP;
};
struct object {
	ObjectType type;
	std::string name;
	int power;
};
struct Character {
	std::string name;
	Races race;
	int HP;
	int atk;
	int def;
	int magicPoints;

	std::vector<object> inventario;
};
struct Player {
	std::string username;
	std::string password;
	Character character;
};

struct Room
{
	int roomNumber;
	std::string description;
	bool hasEnemy = false;
	bool hasObject = false;
	bool hasFinalTresure = false;

	int NorthConnection = 0;
	int SouthConnection = 0;
	int EastConnection = 0;
	int WestConnection = 0;

	object objectToPickup;
	enemy enemy_;
};

//carga el mapa en un vector que contiene habitaciones
std::vector<Room> LoadMapIntoVector(std::string mapName)
{
	rapidxml::xml_document<> doc;
	rapidxml::xml_node<> * root_node;
	// Read the xml file into a vector
	std::ifstream theFile(mapName);
	std::vector<char> buffer((std::istreambuf_iterator<char>(theFile)), std::istreambuf_iterator<char>());
	buffer.push_back('\0');
	// Parse the buffer using the xml file parsing library into doc 
	doc.parse<0>(&buffer[0]);
	// Find our root node
	root_node = doc.first_node("Map");
	std::vector<Room> RoomsVector;
	for (rapidxml::xml_node<> * RoomNode = root_node->first_node("Room"); RoomNode; RoomNode = RoomNode->next_sibling())
	{
		Room auxRoom;

		//Room Info
		auxRoom.roomNumber = std::stoi(RoomNode->first_attribute("RoomNumber")->value());
		auxRoom.description = RoomNode->first_attribute("description")->value();

		//Room Content
		auxRoom.hasEnemy = RoomNode->first_node("Enemy");
		if (auxRoom.hasEnemy) {
			//CARGAR ENEMIGO RANDOM DE BBDD, bypassed por ahora
			auxRoom.enemy_.name = "randomName";
			auxRoom.enemy_.atack = 5;
			auxRoom.enemy_.def = 5;
			auxRoom.enemy_.HP = 5;
			auxRoom.enemy_.race = Races::ELF;

		}
		auxRoom.hasObject = RoomNode->first_node("Object");
		if (auxRoom.hasObject) {

			//CARGAR UN OBJETO RANDOM DE BASE DE DATOS, bypassed por ahora
			auxRoom.objectToPickup.name = "random obj";
			auxRoom.objectToPickup.power = 3;
			auxRoom.objectToPickup.type = ObjectType::ARMOR;
		}

		auxRoom.hasFinalTresure = RoomNode->first_node("FinalTresure");

		//Room Connections
		auxRoom.NorthConnection = std::stoi(RoomNode->first_node("N")->value());
		auxRoom.EastConnection = std::stoi(RoomNode->first_node("E")->value());
		auxRoom.WestConnection = std::stoi(RoomNode->first_node("W")->value());
		auxRoom.SouthConnection = std::stoi(RoomNode->first_node("S")->value());


		RoomsVector.push_back(auxRoom);
	}
	return RoomsVector;
}
std::vector<sf::TcpSocket*> sockets;

void Connect(sf::TcpSocket* socket)
{
	//Colocamos al player en la sala 0
	int currentRoom = 0;
	std::vector<Room> MapRooms;
	Player player;
	sf::Packet packet;
	packet << static_cast<int32_t>(Commands::ASK_USERNAME);
	socket->send(packet);
	Commands command;
	int aux;
	int map;
	std::string receivedInfo;
	bool existingPlayer = false;
	bool connected = true;
	while (connected)
	{
		packet.clear();
		if (socket->receive(packet) != sf::Socket::Done)
		{
			std::cout << "The client has disconnected" << std::endl;
			connected = false;
		}
		else
		{
			packet >> aux;
			command = (Commands)aux;
			switch (command) {
			case Commands::USERNAME: {
				packet >> receivedInfo;
				std::cout << receivedInfo << std::endl;
				player.username = receivedInfo;
				//Comprobar si en base de datos existe el usuario recibido (almacenado en player.username)  

				//Si existe
				if (1 == 1)
				{
					existingPlayer = true;
				}
				//Si no existe
				else
				{
					existingPlayer = false;
				}
				packet.clear();
				packet << static_cast<int32_t>(Commands::ASK_PASSWORD);
				socket->send(packet);
				break;
			}
			case Commands::PASSWORD: {
				packet >> receivedInfo;
				std::cout << receivedInfo << std::endl;
				player.password = receivedInfo;
				if (existingPlayer)
				{
					//Comprobar si en base de datos si este password es el del username (almacenado en player.password) 

					//Si coincide
					if (1 == 1)
					{
						packet.clear();
						packet << static_cast<int32_t>(Commands::ASK_MAP);
						//Coger el número de mapas en base de datos
						int mapas = 2;
						packet << mapas;
						//Coger los nombres de todos los mapas en base de datos
						packet << "map 1" << "map 2";
						socket->send(packet);
					}
					//Si no coincide
					else
					{
						packet.clear();
						packet << static_cast<int32_t>(Commands::INVALID_PASSWORD);
						socket->send(packet);
					}

				}
				else
				{
					//Añadir el nuevo player a la base de datos (alamcenado en player)


					packet.clear();
					packet << static_cast<int32_t>(Commands::ASK_RACE);
					socket->send(packet);
				}

				break;
			}

			case Commands::RACE:
			{
				packet >> aux;
				player.character.race = (Races)aux;
				packet.clear();
				packet << static_cast<int32_t>(Commands::ASK_CHARACTER_NAME);
				socket->send(packet);
				break;
			}

			case Commands::CHARACTER_NAME:
			{
				packet >> receivedInfo;
				player.character.name = receivedInfo;
				std::cout << player.character.name << ", " << (int)player.character.race << std::endl;
				//Añadir el personaje a la base de datos (almacenado en player.character)


				packet.clear();
				packet << static_cast<int32_t>(Commands::ASK_MAP);
				//Coger el número de mapas en base de datos
				int mapas = 3;
				packet << mapas;
				//Coger los nombres de todos los mapas en base de datos
				packet << "map 1" << "map 2" << "map 3";
				socket->send(packet);
				break;
			}

			case Commands::MAP:
			{
				packet >> aux;
				map = aux;
				std::cout << map << std::endl;
				if (map == 2)
				{
					MapRooms = LoadMapIntoVector("Map2.xml");
				}
				else {
					MapRooms = LoadMapIntoVector("Map1.xml");
				}
				packet.clear();
				packet << static_cast<int32_t>(Commands::PLAY)<< true;
				socket->send(packet);
				break;
			}

			case Commands::ASK_GO_TO:
			{
				packet >> receivedInfo;
				//Mirar si se puede ir en la dirección indicada (almacenada en receivedInfo) s
				//ReceivedInFo contendrá 'N', 'E', 'S' o 'W'
				bool destinationSuccesfull = false;
				if (receivedInfo == "N" && MapRooms[currentRoom].NorthConnection >= 0)
				{
					currentRoom = MapRooms[currentRoom].NorthConnection;
					destinationSuccesfull = true;
				}
				if (receivedInfo == "S" && MapRooms[currentRoom].SouthConnection >= 0)
				{
					currentRoom = MapRooms[currentRoom].SouthConnection;
					destinationSuccesfull = true;
				}
				if (receivedInfo == "E" && MapRooms[currentRoom].EastConnection >= 0)
				{
					currentRoom = MapRooms[currentRoom].EastConnection;
					destinationSuccesfull = true;
				}
				if (receivedInfo == "W" && MapRooms[currentRoom].WestConnection >= 0)
				{
					currentRoom = MapRooms[currentRoom].WestConnection;
					destinationSuccesfull = true;
				}

				//Si se puede
				if (destinationSuccesfull)
				{
					//Moverse en la dirección indicada (Ir a la sala en esa dirección)
					packet.clear();
					packet << static_cast<int32_t>(Commands::GO_TO) << true << currentRoom << MapRooms[currentRoom].description;
					socket->send(packet);
				}
				//Si no se puede
				else
				{
					packet.clear();
					packet << static_cast<int32_t>(Commands::GO_TO) << false;
					socket->send(packet);
				}
				break;
			}

			case Commands::ASK_PICK_UP:
			{
				//Mirar si existe un objeto en la sala(Solo puede haber uno por sala)
				//Si lo hay
				if (MapRooms[currentRoom].hasFinalTresure)
				{
					packet.clear();
					packet << static_cast<int32_t>(Commands::WIN);
				}
				else if (MapRooms[currentRoom].hasObject)
				{
					
					
					if (MapRooms[currentRoom].objectToPickup.type == ObjectType::ARMOR)
						player.character.def += MapRooms[currentRoom].objectToPickup.power;
					if (MapRooms[currentRoom].objectToPickup.type == ObjectType::POTION)
						player.character.HP += MapRooms[currentRoom].objectToPickup.power;
					if (MapRooms[currentRoom].objectToPickup.type == ObjectType::WEAPON)
						player.character.atk += MapRooms[currentRoom].objectToPickup.power;


					packet.clear();
					packet << static_cast<int32_t>(Commands::PICK_UP) << true;
					packet << MapRooms[currentRoom].objectToPickup.name;
				}				
				else
				{
					packet.clear();
					packet << static_cast<int32_t>(Commands::PICK_UP) << false;
					
				}
				socket->send(packet);
				break;
			}

			case Commands::ASK_STATE:
			{
				//Coger estado del jugador de base de datos (o si se prefiere almacenarlo al princpio del juego y aqui solo coger los elementos)
				packet.clear();
				packet << static_cast<int32_t>(Commands::STATE);

				packet << player.character.HP<< (int) player.character.inventario.size(); //Sustituir 100 por vida de jugador y 2 por número de objetos en inventario
				for (int i = 0; i < player.character.inventario.size(); i++)//Sustituir 2 por número de objetos en inventario
				{
					packet << i + 1 << player.character.inventario[i].name; //Sustiruir key por nombre del objeto pertinente
				}
				socket->send(packet);
				break;
			}

			case Commands::ASK_ATTACK:
			{
				//Mirar si hay enemigo en la sala
				//si lo hay
				if (MapRooms[currentRoom].hasEnemy)
				{
					//Comprobar daño inflingido y daño recibido 

					MapRooms[currentRoom].enemy_.HP -= player.character.atk;
					if (MapRooms[currentRoom].enemy_.HP <= 0)
					{
						MapRooms[currentRoom].enemy_.HP = 0;
						MapRooms[currentRoom].hasEnemy = false;
					}
					
					player.character.HP -= MapRooms[currentRoom].enemy_.atack;
					if (player.character.HP <= 0)
					{
						player.character.HP = 0;
					}
					packet.clear();
					packet << static_cast<int32_t>(Commands::ATTACK) << true;
					//Pasar daño inflingido y daño recibido 
					packet  << player.character.HP << MapRooms[currentRoom].enemy_.HP;

					socket->send(packet);
				}
				//Si no lo hay
				else
				{
					packet.clear();
					packet << static_cast<int32_t>(Commands::ATTACK) << false;
					socket->send(packet);
				}
				break;
			}

			case Commands::ASK_SEE:
			{
				//Coger información de la sala de base de datos (o si se prefiere almacenarlo al princpio del juego y aqui solo coger la información)
				
				packet.clear();
				packet << static_cast<int32_t>(Commands::SEE);
				//Pasar descripción de la sala
				packet << currentRoom;
				packet << MapRooms[currentRoom].NorthConnection << MapRooms[currentRoom].EastConnection << MapRooms[currentRoom].SouthConnection << MapRooms[currentRoom].WestConnection; //Sustituir el primer true por la posibilidad de ir a Norte desde la sala actual y asi con los otros(orden: North, East, South, West)
				packet << (MapRooms[currentRoom].hasObject || MapRooms[currentRoom].hasFinalTresure);
				if (MapRooms[currentRoom].hasObject)
				{
					packet << MapRooms[currentRoom].objectToPickup.name; //Sustituir true en base a si hay objeto o no, y sustituir key por el nombre del objeto si lo hay (Si no lo hay, solo enviar el false)
				}
				else if(MapRooms[currentRoom].hasFinalTresure)
				{
					packet << "final treasure";
				}
				packet << MapRooms[currentRoom].hasEnemy;
				if (MapRooms[currentRoom].hasEnemy)
				{
					packet << MapRooms[currentRoom].enemy_.name; //Sustituir true en base a si hay objeto o no, y sustituir key por el nombre del objeto si lo hay (Si no lo hay, solo enviar el false)
				}
				socket->send(packet);
				break;
			}

			case Commands::AKNOWLEDGE_INFO:
			{
				packet.clear();
				packet << static_cast<int32_t>(Commands::PLAY)<< false;
				socket->send(packet);
				break;
			}
			case Commands::DISCONNECT:
			{
				connected = false;
				std::cout << "The client has disconnected \n";
				break;
			}
			default:
				break;


			}

		}
	}
	sockets.erase(std::remove(sockets.begin(), sockets.end(), socket));
	delete socket;
}

int main()
{
	sf::TcpListener listener;
	sf::Socket::Status status = listener.listen(52000);
	if (status != sf::Socket::Done)
	{
		std::cout << "Error in the creation of the listener" << std::endl;
		return 0;
	}


	while (true)
	{
		sf::TcpSocket* newSocket = new sf::TcpSocket;
		if (listener.accept(*newSocket) != sf::Socket::Done)
		{
			std::cout << "Connection error" << std::endl;
			newSocket->disconnect();
		}
		else
		{
			std::cout << "Connected" << std::endl;
			sockets.push_back(newSocket);
			std::thread connectionThread(&Connect, newSocket);
			connectionThread.detach();
		}
	}

	listener.close();
	std::cout << "Execution terminated";

}

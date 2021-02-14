#include"SFML/Network.hpp"
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
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

sql::Connection* conn;
sql::Driver* driver;
std::string initTime;

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
			auto stmt = conn->createStatement();
			auto res = stmt->executeQuery("SELECT * FROM enemigos ORDER BY RAND() LIMIT 1");
			if (res->next())
			{
				auxRoom.enemy_.name = res->getString("Name");
				auxRoom.enemy_.atack = res->getInt("Attack");
				auxRoom.enemy_.def = res->getInt("Defense");
				auxRoom.enemy_.HP = res->getInt("Life");
				auxRoom.enemy_.race = static_cast<Races>(res->getInt("Race"));
			}
			delete stmt;
		}
		auxRoom.hasObject = RoomNode->first_node("Object");
		if (auxRoom.hasObject) {

			//CARGAR UN OBJETO RANDOM DE BASE DE DATOS, bypassed por ahora
			//CARGAR UN OBJETO RANDOM DE BASE DE DATOS
			auto stmt = conn->createStatement();
			auto res = stmt->executeQuery("SELECT * FROM objetos ORDER BY RAND() LIMIT 1");
			if (res->next())
			{
				auxRoom.objectToPickup.name = res->getString("Nombre");
				auxRoom.objectToPickup.power = res->getInt("Power");
				auxRoom.objectToPickup.type = static_cast<ObjectType>(res->getInt("ObjectType"));
			}
			delete stmt;
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
	try
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
					auto stmt = conn->createStatement();
					auto res = stmt->executeQuery("SELECT 1 FROM users WHERE Username='" + player.username +"'" );
					//Si existe
					if (res->next())
					{
						existingPlayer = true;
					}
					//Si no existe
					else
					{
						existingPlayer = false;
					}
					delete stmt;

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
						//Comprobar si en base de datos si este password es el del username 
						auto stmt = conn->createStatement();
						auto res = stmt->executeQuery("SELECT 1 FROM users WHERE Username='" + player.username + "' AND Password='" + player.password + "'");
						//Si coincide
						if (res->next())
						{
							delete stmt;

							auto stmt = conn->createStatement();
							auto res = stmt->executeQuery("SELECT * FROM personajes WHERE Username_FK='" + player.username +"'");

							if (res->next())
							{
								player.character.name = res->getString("Name");
								player.character.race = static_cast<Races>(res->getInt("Race"));
								player.character.HP = res->getInt("Life");
								player.character.atk = res->getInt("Attack");
								player.character.def = res->getInt("Defense");
								player.character.magicPoints = res->getInt("Magica");

								auto stmt1 = conn->createStatement();
								auto res1 = stmt1->executeQuery("SELECT * FROM inventarios WHERE Username_FK='" + player.username +"'");
								while (res1->next())
								{
									object ob;
									ob.name = res->getString("ObjectName");
									ob.power = res->getInt("Power");
									ob.type = static_cast<ObjectType>(res->getInt("ObjectType"));
									player.character.inventario.push_back(ob);
								}
								delete stmt1;
							}
							delete stmt;

							struct tm newtime;
							time_t now = time(0);
							localtime_s(&newtime, &now);
							initTime = std::to_string(1900 + newtime.tm_year) + "-" + std::to_string(newtime.tm_mon + 1) + "-" + std::to_string(newtime.tm_mday) + " " + std::to_string(newtime.tm_hour) + ":" + std::to_string(newtime.tm_min) + ":" + std::to_string(newtime.tm_sec);

							packet.clear();
							packet << static_cast<int32_t>(Commands::ASK_MAP);
							int mapas = 2;
							packet << mapas;
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
						//Añadir el nuevo player a la base de datos
						auto stmt = conn->createStatement();
						stmt->executeUpdate("INSERT INTO users (Username, Password) VALUES ('" + player.username + "','" + player.password + "')");
						delete stmt;

						struct tm newtime;
						time_t now = time(0);
						localtime_s(&newtime, &now);
						initTime = std::to_string(1900 + newtime.tm_year) + "-" + std::to_string(newtime.tm_mon + 1) + "-" + std::to_string(newtime.tm_mday) + " " + std::to_string(newtime.tm_hour) + ":" + std::to_string(newtime.tm_min) + ":" + std::to_string(newtime.tm_sec);

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
					player.character.atk = 1 + rand() % 4;
					player.character.def = 1 + rand() % 4;
					player.character.HP = 1 + rand() % 4;
					player.character.magicPoints = 1 + rand() % 4;
					std::cout << player.character.name << ", " << (int)player.character.race << std::endl;
					//Añadir el personaje a la base de datos (almacenado en player.character)
					auto stmt = conn->createStatement();
					stmt->executeUpdate("INSERT INTO personajes (Username_FK, Race, Name, Life, Attack, Defense, Magica) VALUES ('" + player.username + "'," + std::to_string((int)player.character.race) + ",'" + player.character.name + "'," + std::to_string(player.character.HP) + "," + std::to_string(player.character.atk) + "," + std::to_string(player.character.def) + "," + std::to_string(player.character.magicPoints) + ")");
					delete stmt;

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
					packet << static_cast<int32_t>(Commands::PLAY) << true;
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

						auto stmt = conn->createStatement();
						stmt->executeUpdate("INSERT INTO inventarios (Username_FK, ObjectType, ObjectName, Power) VALUES ('" + player.username + "'," + std::to_string((int)(MapRooms[currentRoom].objectToPickup.type)) + ",'" + MapRooms[currentRoom].objectToPickup.name + "'," + std::to_string(MapRooms[currentRoom].objectToPickup.power) + ")");
						delete stmt;
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

					packet << player.character.HP << (int)player.character.inventario.size(); //Sustituir 100 por vida de jugador y 2 por número de objetos en inventario
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
							auto stmt = conn->createStatement();
							stmt->executeUpdate("UPDATE users SET NumEnemigosMatados=NumEnemigosMatados+1");
							delete stmt;
						}

						player.character.HP -= MapRooms[currentRoom].enemy_.atack;
						if (player.character.HP <= 0)
						{
							player.character.HP = 0;
							auto stmt = conn->createStatement();
							stmt->executeUpdate("UPDATE users SET NumMuertes=NumMuertes+1");
							delete stmt;
						}
						/*auto stmt = conn->createStatement();
						stmt->executeUpdate("UPDATE personajes SET Life=" + std::to_string(player.character.HP));
						delete stmt;*/

						packet.clear();
						packet << static_cast<int32_t>(Commands::ATTACK) << true;
						//Pasar daño inflingido y daño recibido 
						packet << player.character.HP << MapRooms[currentRoom].enemy_.HP;

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
					else if (MapRooms[currentRoom].hasFinalTresure)
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
					packet << static_cast<int32_t>(Commands::PLAY) << false;
					socket->send(packet);
					break;
				}
				case Commands::DISCONNECT:
				{
					connected = false;
					std::cout << "The client has disconnected \n";

					auto stmt = conn->createStatement();
					stmt->executeUpdate("INSERT INTO usersesiontimes (Username, InicioSesion) VALUES ('" + player.username + "','" + initTime + "')");
					auto stmt1 = conn->createStatement();
					stmt1->executeUpdate("INSERT INTO usermaptimes (Username, MapID, InicioSesion) VALUES ('" + player.username + "'," + std::to_string(map) + ",'" + initTime + "')");
					delete stmt;
					delete stmt1;

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
	catch (sql::SQLException &e)
	{

		std::string s1(__FILE__);
		std::string s2(e.what());
		std::string s3(std::to_string(e.getErrorCode()));
		std::string s4(e.getSQLState());
		std::cout<<"ERROR: SQLException in " + s1 + " ERROR: " + s2 + " (MySQL error code: " + s3 + ", SQLState: " + s4 + ")";
	}
}

int main()
{
	driver = sql::mysql::get_driver_instance();
	conn = driver->connect("localhost", "root", "(SecphoLand7113114)");
	conn->setSchema("balzhur");

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

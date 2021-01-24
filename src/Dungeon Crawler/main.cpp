#include <random>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "Player.h"
#include "Coin.h"
#include "Ground.h"
#include "Enemy.h"
#include "QuadTree.h"

namespace
{
	std::random_device rd;
	std::mt19937 mt(rd());

	int randomInt(int exclusiveMax)
	{
		std::uniform_int_distribution<> dist(0, exclusiveMax - 1);
		return dist(mt);
	}

	int randomInt(int min, int max) // inclusive min/max
	{
		std::uniform_int_distribution<> dist(0, max - min);
		return dist(mt) + min;
	}

	bool randomBool(double probability = 0.5)
	{
		std::bernoulli_distribution dist(probability);
		return dist(mt);
	}
}

struct Rect
{
	int x, y;
	int width, height;
};

class Dungeon
{
public:
	enum Tile
	{
		Unused = ' ',
		Floor = '.',
		Corridor = ',',
		Wall = '#',
		ClosedDoor = '+',
		OpenDoor = '-',
		UpStairs = '<',
		DownStairs = '>'
	};

	enum Direction
	{
		North,
		South,
		West,
		East,
		DirectionCount
	};

public:
	Dungeon(int width, int height)
		: _width(width)
		, _height(height)
		, _tiles(width* height, Unused)
		, _rooms()
		, _exits()
	{
	}

	void generate(int maxFeatures)
	{
		// place the first room in the center
		if (!makeRoom(_width / 2, _height / 2, static_cast<Direction>(randomInt(4), true)))
		{
			std::cout << "Unable to place the first room.\n";
			return;
		}

		// we already placed 1 feature (the first room)
		for (int i = 1; i < maxFeatures; ++i)
		{
			if (!createFeature())
			{
				std::cout << "Unable to place more features (placed " << i << ").\n";
				break;
			}
		}

		if (!placeObject(UpStairs))
		{
			std::cout << "Unable to place up stairs.\n";
			return;
		}

		if (!placeObject(DownStairs))
		{
			std::cout << "Unable to place down stairs.\n";
			return;
		}

		for (char& tile : _tiles)
		{
			if (tile == Unused)
				tile = '.';
			else if (tile == Floor || tile == Corridor)
				tile = ' ';
		}
	}

	void print()
	{
		for (int y = 0; y < _height; ++y)
		{
			for (int x = 0; x < _width; ++x)
				std::cout << getTile(x, y);

			std::cout << std::endl;
		}
	}

	std::vector<char> getTiles() {
		return this->_tiles;
	}

private:
	char getTile(int x, int y) const
	{
		if (x < 0 || y < 0 || x >= _width || y >= _height)
			return Unused;

		return _tiles[x + y * _width];
	}

	void setTile(int x, int y, char tile)
	{
		_tiles[x + y * _width] = tile;
	}

	bool createFeature()
	{
		for (int i = 0; i < 1000; ++i)
		{
			if (_exits.empty())
				break;

			// choose a random side of a random room or corridor
			int r = randomInt(_exits.size());
			int x = randomInt(_exits[r].x, _exits[r].x + _exits[r].width - 1);
			int y = randomInt(_exits[r].y, _exits[r].y + _exits[r].height - 1);

			// north, south, west, east
			for (int j = 0; j < DirectionCount; ++j)
			{
				if (createFeature(x, y, static_cast<Direction>(j)))
				{
					_exits.erase(_exits.begin() + r);
					return true;
				}
			}
		}

		return false;
	}

	bool createFeature(int x, int y, Direction dir)
	{
		static const int roomChance = 50; // corridorChance = 100 - roomChance

		int dx = 0;
		int dy = 0;

		if (dir == North)
			dy = 1;
		else if (dir == South)
			dy = -1;
		else if (dir == West)
			dx = 1;
		else if (dir == East)
			dx = -1;

		if (getTile(x + dx, y + dy) != Floor && getTile(x + dx, y + dy) != Corridor)
			return false;

		if (randomInt(100) < roomChance)
		{
			if (makeRoom(x, y, dir))
			{
				setTile(x, y, ClosedDoor);

				return true;
			}
		}

		else
		{
			if (makeCorridor(x, y, dir))
			{
				if (getTile(x + dx, y + dy) == Floor)
					setTile(x, y, ClosedDoor);
				else // don't place a door between corridors
					setTile(x, y, Corridor);

				return true;
			}
		}

		return false;
	}

	bool makeRoom(int x, int y, Direction dir, bool firstRoom = false)
	{
		static const int minRoomSize = 3;
		static const int maxRoomSize = 6;

		Rect room;
		room.width = randomInt(minRoomSize, maxRoomSize);
		room.height = randomInt(minRoomSize, maxRoomSize);

		if (dir == North)
		{
			room.x = x - room.width / 2;
			room.y = y - room.height;
		}

		else if (dir == South)
		{
			room.x = x - room.width / 2;
			room.y = y + 1;
		}

		else if (dir == West)
		{
			room.x = x - room.width;
			room.y = y - room.height / 2;
		}

		else if (dir == East)
		{
			room.x = x + 1;
			room.y = y - room.height / 2;
		}

		if (placeRect(room, Floor))
		{
			_rooms.emplace_back(room);

			if (dir != South || firstRoom) // north side
				_exits.emplace_back(Rect{ room.x, room.y - 1, room.width, 1 });
			if (dir != North || firstRoom) // south side
				_exits.emplace_back(Rect{ room.x, room.y + room.height, room.width, 1 });
			if (dir != East || firstRoom) // west side
				_exits.emplace_back(Rect{ room.x - 1, room.y, 1, room.height });
			if (dir != West || firstRoom) // east side
				_exits.emplace_back(Rect{ room.x + room.width, room.y, 1, room.height });

			return true;
		}

		return false;
	}

	bool makeCorridor(int x, int y, Direction dir)
	{
		static const int minCorridorLength = 3;
		static const int maxCorridorLength = 6;

		Rect corridor;
		corridor.x = x;
		corridor.y = y;

		if (randomBool()) // horizontal corridor
		{
			corridor.width = randomInt(minCorridorLength, maxCorridorLength);
			corridor.height = 1;

			if (dir == North)
			{
				corridor.y = y - 1;

				if (randomBool()) // west
					corridor.x = x - corridor.width + 1;
			}

			else if (dir == South)
			{
				corridor.y = y + 1;

				if (randomBool()) // west
					corridor.x = x - corridor.width + 1;
			}

			else if (dir == West)
				corridor.x = x - corridor.width;

			else if (dir == East)
				corridor.x = x + 1;
		}

		else // vertical corridor
		{
			corridor.width = 1;
			corridor.height = randomInt(minCorridorLength, maxCorridorLength);

			if (dir == North)
				corridor.y = y - corridor.height;

			else if (dir == South)
				corridor.y = y + 1;

			else if (dir == West)
			{
				corridor.x = x - 1;

				if (randomBool()) // north
					corridor.y = y - corridor.height + 1;
			}

			else if (dir == East)
			{
				corridor.x = x + 1;

				if (randomBool()) // north
					corridor.y = y - corridor.height + 1;
			}
		}

		if (placeRect(corridor, Corridor))
		{
			if (dir != South && corridor.width != 1) // north side
				_exits.emplace_back(Rect{ corridor.x, corridor.y - 1, corridor.width, 1 });
			if (dir != North && corridor.width != 1) // south side
				_exits.emplace_back(Rect{ corridor.x, corridor.y + corridor.height, corridor.width, 1 });
			if (dir != East && corridor.height != 1) // west side
				_exits.emplace_back(Rect{ corridor.x - 1, corridor.y, 1, corridor.height });
			if (dir != West && corridor.height != 1) // east side
				_exits.emplace_back(Rect{ corridor.x + corridor.width, corridor.y, 1, corridor.height });

			return true;
		}

		return false;
	}

	bool placeRect(const Rect& rect, char tile)
	{
		if (rect.x < 1 || rect.y < 1 || rect.x + rect.width > _width - 1 || rect.y + rect.height > _height - 1)
			return false;

		for (int y = rect.y; y < rect.y + rect.height; ++y)
			for (int x = rect.x; x < rect.x + rect.width; ++x)
			{
				if (getTile(x, y) != Unused)
					return false; // the area already used
			}

		for (int y = rect.y - 1; y < rect.y + rect.height + 1; ++y)
			for (int x = rect.x - 1; x < rect.x + rect.width + 1; ++x)
			{
				if (x == rect.x - 1 || y == rect.y - 1 || x == rect.x + rect.width || y == rect.y + rect.height)
					setTile(x, y, Wall);
				else
					setTile(x, y, tile);
			}

		return true;
	}

	bool placeObject(char tile)
	{
		if (_rooms.empty())
			return false;

		int r = randomInt(_rooms.size()); // choose a random room
		int x = randomInt(_rooms[r].x + 1, _rooms[r].x + _rooms[r].width - 2);
		int y = randomInt(_rooms[r].y + 1, _rooms[r].y + _rooms[r].height - 2);

		if (getTile(x, y) == Floor)
		{
			setTile(x, y, tile);

			// place one object in one room (optional)
			_rooms.erase(_rooms.begin() + r);

			return true;
		}

		return false;
	}

private:
	int _width, _height;
	std::vector<char> _tiles;
	std::vector<Rect> _rooms; // rooms for place stairs or monsters
	std::vector<Rect> _exits; // 4 sides of rooms or corridors
};

int main()
{

	Dungeon d(70, 20);
	d.generate(15);
	d.print();
	std::vector<char> allTiles = d.getTiles();

	std::cout << "Press Enter to quit... ";
	std::cin.get();
	//std::cout << "test = " << _tiles << std::endl;
	const int screenDimensionX = 700;
	const int screenDimensionY = 500;

	//sf::RenderWindow window(sf::VideoMode(700, 500), "Dungeon Crawler");
	sf::RenderWindow window(sf::VideoMode(screenDimensionX, screenDimensionY), "Dungeon Crawler");
	window.setFramerateLimit(120);
	window.setVerticalSyncEnabled(true);
	sf::Vector2f screenPosition(screenDimensionX / 2, screenDimensionY / 2);

	//VIEW
	sf::View view;

	view.reset(sf::FloatRect(0, 0, screenDimensionX, screenDimensionY));
	view.setViewport(sf::FloatRect(0, 0, 1.0f, 1.0f));

	const int globalBlocSizeX = 40;
	const int globalBlocSizeY = 40;
	int lineCheck = 0;
	std::vector<Ground*> collisionTab;

	bool isRotating = false;
	float rotationStep = 5.f;
	float currentRotation = 0.f;
	sf::Vector2f topDirections[4];
	topDirections[0] = sf::Vector2f(0.f, -1.f);
	topDirections[1] = sf::Vector2f(1.f, 0.f);
	topDirections[2] = sf::Vector2f(0.f, 1.f);
	topDirections[3] = sf::Vector2f(-1.f, 0.f);
	int topDirectionIndex = 0;

	//Wall texture
	sf::Texture wallTexture;
	if (!wallTexture.loadFromFile("res/img/wall.png")) {
		std::cout << "Load failed" << std::endl;

		system("pause");
	}

	//Player instance
	sf::Texture playertexture;

	if (!playertexture.loadFromFile("res/img/slime.jpg")) {
		std::cout << "Load failed" << std::endl;

		system("pause");
	}
	Player player({ 30,30 }, &playertexture);
	player.setPos({ 50,500 });

	//Handle all items from generated map
	float LayerX = 0;
	float LayerY = 0;
	std::vector<Ground*> wallVector;

	for (int i = 0; i < allTiles.size(); i++) {
		//Go to a new line
		if (lineCheck == 70) {
			lineCheck = 0;
			LayerY = 0;
			LayerX = LayerX + 40;
		}
		//If Tile = Wall
		if (allTiles[i] == '#') {
			Ground* wall = new Ground({ globalBlocSizeX,globalBlocSizeY }, &wallTexture);
			wallVector.push_back(wall);
			collisionTab.push_back(wall);
			wall->setPos({ LayerY, LayerX });
			std::cout << wall->getX() << " is X" << std::endl;
			std::cout << wall->getY() << " is Y" << std::endl;
		}
		//If Tile = Character
		if (allTiles[i] == '>') {
			player.setPos({ LayerY, LayerX });
		}
		else {
			//Empty for now
		}
		lineCheck = lineCheck++;

		LayerY = LayerY + 40;
	}

	//create quadTree to check collisions
	//QuadTree quadTree(sf::FloatRect(player.getX() - screenPosition.x, player.getY() - screenPosition.y, screenDimensionX, screenDimensionY),0);// fait la taille de l'ecran
	QuadTree quadTree(sf::FloatRect(0.f, 0.f, 70 * 40, 20 * 40), 0);//le quadtree fait la taille du fichier

	for (int i = 0; i < wallVector.size(); i++) {
		// Add object in the quadTree
		quadTree.insert(wallVector[i]);
	}

	while (window.isOpen())
	{
		sf::Event event;
		//Movement Handling /!\ TEMPORARY FOR TEST PURPOSE ONLY
		const float moveSpeed = 0.8;

		/*if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			player.move({ 0, -moveSpeed });
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			player.move({ 0, moveSpeed });
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
			player.move({ moveSpeed, 0 });
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
			player.move({ -moveSpeed, 0 });
		}*/

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && !isRotating) {
			isRotating = true;
		}
		if (isRotating) {
			currentRotation += rotationStep;
			view.rotate(rotationStep); // TODO: * deltaTime
			player.rotate(rotationStep); // TODO: * deltaTime
			if (currentRotation >= 90) {
				isRotating = false;
				currentRotation = 0;
				topDirectionIndex == 3 ? topDirectionIndex = 0 : topDirectionIndex += 1;
			}
		}
		else {
			// We move only if the game isn't rotating
			player.move(moveSpeed * topDirections[topDirectionIndex]);

			// check possible collisions in this view
			// bug au lancement tant qu'on ne tourne pas le perso il n'y a jamais aucune collision detecté
			// bug les collisions se font que dans un certain rayon puis plus rien il faudrait refaire le quadtree à chaque fois que le player en sort ?
			std::vector<Ground*> groundVector = quadTree.getObjects(sf::FloatRect(player.getX(), player.getY(), player.getGlobalBounds().height, player.getGlobalBounds().width));

			for (Ground* ground : groundVector) {
				if (ground > 0) {
					std::cout << "possible collisions ! " << player.isCollidingWithGround(ground) << std::endl;
					if (player.isCollidingWithGround(ground)) {
						player.move(-(moveSpeed * topDirections[topDirectionIndex]));
					}
				}
			}

			//quadTree.Clear();
		}


		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}


		window.setView(view);
		window.clear();

		//Screen Position follow player
		if (player.getX() + 10 > screenDimensionX / 2)
			screenPosition.x = player.getX() + 10;
		else
			screenPosition.x = screenDimensionX / 2;

		if (player.getY() + 10 > screenDimensionY / 2)
			screenPosition.y = player.getY() + 10;
		else
			screenPosition.y = screenDimensionY / 2;

		view.setCenter(screenPosition);
		//Player Tile
		player.drawTo(window);
		//Block Tile
		for (int i = 0; i < wallVector.size(); i++) {
			wallVector[i]->drawTo(window);
		}
		//Quadtree
		quadTree.Draw(window);

		window.setView(window.getDefaultView());

		window.display();
	}


	return 0;
}

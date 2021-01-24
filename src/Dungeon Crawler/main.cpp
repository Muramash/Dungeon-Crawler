#include <random>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <sstream>
#include "Player.h"
#include "Coin.h"
#include "Ground.h"
#include "Enemy.h"
#include "QuadTree.h"
#include "EmptySpace.h"
#include "Stairs.h"


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
		DownStairs = '>',
		Coin = '*',
		Enemy = '/'
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
	//d.print();
	std::vector<char> allTiles = d.getTiles();

	//std::cout << "Press Enter to quit... ";
	//std::cin.get();
	//std::cout << "test = " << _tiles << std::endl;
	const int screenDimensionX = 700;
	const int screenDimensionY = 500;
	sf::Clock clock = sf::Clock();
	float deltaTime;
	int nbSpacePressed = 0;
	bool spaceReleased = true;

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
	float rotationStep = 240.f;
	float currentRotation = 0.f;
	sf::Vector2f topDirections[4];
	topDirections[0] = sf::Vector2f(0.f, -1.f);
	topDirections[1] = sf::Vector2f(1.f, 0.f);
	topDirections[2] = sf::Vector2f(0.f, 1.f);
	topDirections[3] = sf::Vector2f(-1.f, 0.f);
	int topDirectionIndex = 0;
	//Empty textyre
	sf::Texture emptyTexture;

	//Wall texture
	sf::Texture wallTexture;
	if (!wallTexture.loadFromFile("res/img/wall.png")) {
		std::cout << "Load failed" << std::endl;

		system("pause");
	}

	//Stairs texture
	sf::Texture stairsTexture;
	if (!stairsTexture.loadFromFile("res/img/stairs.png")) {
		std::cout << "Load failed" << std::endl;

		system("pause");
	}
	Stairs stairs({ globalBlocSizeX,globalBlocSizeY }, &stairsTexture);

	//Coin texture
	sf::Texture coinTexture;
	if (!coinTexture.loadFromFile("res/img/coin.png")) {
		std::cout << "Load failed" << std::endl;

		system("pause");
	}

	//Enemy texture
	sf::Texture enemyTexture;
	if (!enemyTexture.loadFromFile("res/img/slime.jpg")) {
		std::cout << "Load failed" << std::endl;

		system("pause");
	}

	//Player instance
	sf::Texture playertexture;

	if (!playertexture.loadFromFile("res/img/chara.png")) {
		std::cout << "Load failed" << std::endl;

		system("pause");
	}
	Player player({ 20,20 }, &playertexture);
	player.setPos({ 50,500 });

	//Game Over object
	sf::Font minecraft;
	minecraft.loadFromFile("res/fonts/Minecraft.ttf");

	std::ostringstream ssBigMessage;
	ssBigMessage << "";

	sf::Text lblBigMessage;
	lblBigMessage.setCharacterSize(100);
	lblBigMessage.setPosition({ 50,200 });
	lblBigMessage.setFont(minecraft);
	lblBigMessage.setString(ssBigMessage.str());

	//Score object
	int score = 0;

	std::ostringstream ssScore;
	ssScore << "Score: " << score;

	sf::Text lblScore;
	lblScore.setCharacterSize(30);
	lblScore.setPosition({ 10,10 });
	lblScore.setFont(minecraft);
	lblScore.setString(ssScore.str());

	//Life object
	int life = 3;

	std::ostringstream ssLife;
	ssLife << "HP: " << life;

	sf::Text lblLife;
	lblLife.setCharacterSize(30);
	lblLife.setPosition({ 600,10 });
	lblLife.setFont(minecraft);
	lblLife.setString(ssLife.str());


	//Handle all items from generated map
	float LayerX = 0;
	float LayerY = 0;
	std::vector<Ground*> wallVector;
	std::vector<Coin*> coinVector;
	std::vector<Enemy*> enemyVector;
	std::vector<EmptySpace*> emptyVector;

	float playerX = 0.f, playerY = 0.f;

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
			playerX = LayerY;
			playerY = LayerX;
		}
		//If Tile = Stairs
		if (allTiles[i] == '<') {
			stairs.setPos({ LayerY, LayerX });
		}
		//Generate coin or monster on empty space
		if (allTiles[i] == ' ') {
			EmptySpace* emptySpace = new EmptySpace({ globalBlocSizeX, globalBlocSizeY }, &emptyTexture);
			emptyVector.push_back(emptySpace);
			emptySpace->setPos({ LayerY, LayerX });

		}
		else {
			//Empty for now
		}
		lineCheck = lineCheck++;

		LayerY = LayerY + 40;
	}
	int emptyBlocNum = 0;
	int coinChecker = 0;
	int enemyChecker = 0;
	int maxEnemyPerLevel = randomInt(5);
	int maxCoinPerLevel = randomInt(5);

	for (int i = 0; i < emptyVector.size(); i++) {
		emptyBlocNum = emptyBlocNum + 1;
	}
	for (int i = 0; i < emptyVector.size(); i++) {
		if (coinChecker <= maxCoinPerLevel) {
			int randomEmpty = randomInt(emptyBlocNum);
			float EmptyPosX = emptyVector[randomEmpty]->getX();
			float EmptyPosY = emptyVector[randomEmpty]->getY();
			coinChecker = coinChecker++;

			Coin* coin = new Coin({ globalBlocSizeX, globalBlocSizeY }, &coinTexture);
			coinVector.push_back(coin);
			coin->setPos({ EmptyPosX, EmptyPosY });
		}

	}
	for (int i = 0; i < emptyVector.size(); i++) {

		if (enemyChecker <= maxEnemyPerLevel) {
			std::cout << "i'm in ???" << std::endl;
			int randomEmpty = randomInt(emptyBlocNum);
			float EmptyPosX = emptyVector[randomEmpty]->getX();
			float EmptyPosY = emptyVector[randomEmpty]->getY();
			enemyChecker = enemyChecker++;

			Enemy* enemy = new Enemy({ globalBlocSizeX, globalBlocSizeY }, &enemyTexture);
			enemyVector.push_back(enemy);
			enemy->setPos({ EmptyPosX, EmptyPosY });
		}
	}

	//create quadTree to check collisions
	QuadTree quadTree(sf::FloatRect(0.f, 0.f, 70 * 40, 20 * 40), 0);

	// Add objects in the quadTree
	for (int i = 0; i < wallVector.size(); i++) {
		quadTree.insert(wallVector[i]);
	}

	bool isPlaying = true, first = false, mainMenu = true;
	float posX = 0.f, posY = 0.f;

	while (window.isOpen())
	{
		deltaTime = clock.restart().asSeconds();
		//Enemy logic
		for (int i = 0; i < enemyVector.size(); i++) {
			if (player.isCollidingWithEnemy(enemyVector[i])) {
				score = score + 10;
				life = life - 1;
				ssScore.str("");
				ssScore << "Score: " << score;
				lblScore.setString(ssScore.str());
				ssLife.str("");
				ssLife << "HP:" << life;
				lblLife.setString(ssLife.str());
				enemyVector[i]->setPos({ 999999,999999 });
			}
		}

		//Coin logic
		for (int i = 0; i < coinVector.size(); i++) {
			if (player.isCollidingWithCoin(coinVector[i])) {
				coinVector[i]->setPos({ 999999,999999 });
				score++;
				ssScore.str("");
				ssScore << "Score: " << score;
				lblScore.setString(ssScore.str());
			}
		}

		if (life <= 0) {
			player.setPos({ 99999,999999 });
			ssBigMessage.str("");
			ssBigMessage << "GAME OVER";
			lblBigMessage.setString(ssBigMessage.str());
		}

		sf::Event event;

		if (mainMenu) {
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
			}

			window.setView(view);
			window.clear();

			view.setCenter(screenPosition);

			//Name of the game
			sf::Text message;
			message.setString("Dungeon Crawler");
			sf::Font font;
			font.loadFromFile("res/fonts/minecraft.ttf");
			message.setFont(font);
			message.setCharacterSize(70);
			sf::FloatRect messageRect = message.getLocalBounds();
			message.setOrigin(messageRect.left + messageRect.width / 2.0f, messageRect.top + messageRect.height / 2.0f);
			message.setPosition(sf::Vector2f(screenPosition.x, screenPosition.y - 100.f));

			//Instruction to start the game
			sf::Text scoreMessage;
			scoreMessage.setString("Press enter to start !");
			scoreMessage.setFont(font);
			scoreMessage.setCharacterSize(40);
			sf::FloatRect scoreMessageRect = scoreMessage.getLocalBounds();
			scoreMessage.setOrigin(scoreMessageRect.left + scoreMessageRect.width / 2.0f, scoreMessageRect.top + scoreMessageRect.height / 2.0f);
			scoreMessage.setPosition(sf::Vector2f(screenPosition.x, screenPosition.y + 60.f));

			// animation of the player in the main menu
			posX += 1.f;
			if (posX > view.getSize().x) {
				posX = 0.f;
			}
			if (posX >= view.getCenter().x - 200.f && posX <= view.getCenter().x - 180.f || posX >= view.getCenter().x - 20.f && posX <= view.getCenter().x || posX >= view.getCenter().x + 180.f && posX <= view.getCenter().x + 200.f) {
				posY -= 1.f;
			}
			else if (posX > view.getCenter().x - 180.f && posX <= view.getCenter().x - 160.f || posX > view.getCenter().x && posX <= view.getCenter().x + 20.f || posX > view.getCenter().x + 200.f && posX <= view.getCenter().x + 220.f) {
				posY += 1.f;
			}
			else {
				posY = view.getCenter().y + 170.f;
			}
			player.setPos({ posX, posY });

			window.draw(message);
			window.draw(scoreMessage);
			player.drawTo(window);

			window.setView(window.getDefaultView());

			window.display();

			// launch the game
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
				player.setPos({ playerX, playerY });
				mainMenu = false;
			}
		}
		else {
			if (isPlaying) {
				//Movement Handling /!\ TEMPORARY FOR TEST PURPOSE ONLY
				const float moveSpeed = 150.f;

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

				if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && spaceReleased) {
					if (nbSpacePressed <= 3)
						++nbSpacePressed;
					spaceReleased = false;
					//isRotating = true;
				}
				if (nbSpacePressed >= 1) {
					float currentStep = rotationStep * deltaTime;
					if (currentRotation + currentStep > 90.f)
					{
						currentStep = 90.f - currentRotation;
					}
					currentRotation += currentStep;
					view.rotate(currentStep);
					player.rotate(currentStep);
					if (currentRotation == 90.f) {
						--nbSpacePressed;
						//isRotating = false;
						currentRotation = 0;
						topDirectionIndex == 3 ? topDirectionIndex = 0 : topDirectionIndex += 1;
					}
				}
				else {
					// We move only if the game isn't rotating
					player.move((moveSpeed * deltaTime) * topDirections[topDirectionIndex]);

					if (player.isCollidingWithStairs(stairs)) {
						isPlaying = false;
					}

					std::vector<Ground*> groundVector = quadTree.getObjects(sf::FloatRect(player.getX(), player.getY(), player.getGlobalBounds().height, player.getGlobalBounds().width));

					// check collisions with ground
					for (Ground* ground : groundVector) {
						if (ground > 0) {
							std::cout << "possible collisions ! " << player.isCollidingWithGround(ground) << std::endl;
							if (player.isCollidingWithGround(ground)) {
								player.move(-((moveSpeed * deltaTime) * topDirections[topDirectionIndex]));
							}
						}
					}
				}


				while (window.pollEvent(event))
				{
					if (event.type == sf::Event::Closed)
						window.close();
					if (event.type == sf::Event::MouseButtonReleased)
					{
						if (event.mouseButton.button == sf::Mouse::Left)
						{
							spaceReleased = true;
						}
					}
				}

				window.clear();

				window.draw(lblScore);
				window.draw(lblLife);
				window.draw(lblBigMessage);

				window.setView(view);


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
				//Stairs Tile
				stairs.drawTo(window);
				//Block Tile
				for (int i = 0; i < wallVector.size(); i++) {
					wallVector[i]->drawTo(window);
				}
				for (int i = 0; i < coinVector.size(); i++) {
					coinVector[i]->drawTo(window);
				}
				for (int i = 0; i < enemyVector.size(); i++) {
					enemyVector[i]->drawTo(window);
				}
				//Quadtree
				//quadTree.Draw(window);

				window.setView(window.getDefaultView());

				window.display();
			}
			else {
				while (window.pollEvent(event))
				{
					if (event.type == sf::Event::Closed)
						window.close();
				}

				if (!first) {
					// we put the screen back in the original direction
					while (topDirectionIndex != 0) {

						float currentStep = rotationStep * deltaTime;
						if (currentRotation + currentStep > 90.f)
						{
							currentStep = 90.f - currentRotation;
						}
						currentRotation += currentStep;
						view.rotate(currentStep);
						player.rotate(currentStep);
						if (currentRotation == 90.f) {
							--nbSpacePressed;
							//isRotating = false;
							currentRotation = 0;
							topDirectionIndex == 3 ? topDirectionIndex = 0 : topDirectionIndex += 1;
						}
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

					player.setPos({ (float)player.getX() + 35.f, (float)player.getY() - 90.f });

					//Victory Message
					sf::Text message;
					message.setString("VICTORY !");
					sf::Font font;
					font.loadFromFile("res/fonts/minecraft.ttf");
					message.setFont(font);
					message.setCharacterSize(100);
					sf::FloatRect messageRect = message.getLocalBounds();
					message.setOrigin(messageRect.left + messageRect.width / 2.0f, messageRect.top + messageRect.height / 2.0f);
					message.setPosition(sf::Vector2f(screenPosition.x, screenPosition.y - 100.f));

					//Score message
					sf::Text scoreMessage;
					scoreMessage.setString(ssScore.str());
					scoreMessage.setFont(font);
					scoreMessage.setCharacterSize(40);
					sf::FloatRect scoreMessageRect = scoreMessage.getLocalBounds();
					scoreMessage.setOrigin(scoreMessageRect.left + scoreMessageRect.width / 2.0f, scoreMessageRect.top + scoreMessageRect.height / 2.0f);
					scoreMessage.setPosition(sf::Vector2f(screenPosition.x - 20.f, screenPosition.y + 60.f));

					window.draw(message);
					window.draw(scoreMessage);
					player.drawTo(window);

					window.setView(window.getDefaultView());

					window.display();

					first = true;
				}


			}
		}
	}


	return 0;
}

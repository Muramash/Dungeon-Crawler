#pragma once

#include <iostream>
#include <SFML\Graphics.hpp>
#include "Coin.h"
#include "Enemy.h"
#include "Ground.h"

class Player
{
public:
	Player(sf::Vector2f size, sf::Texture *texture) {
		player.setSize(size);
		player.setTexture(texture);
	}

	void drawTo(sf::RenderWindow& window) {
		window.draw(player);
	}

	void move(sf::Vector2f distance) {
		player.move(distance);
	}

	void setPos(sf::Vector2f newPos) {
		player.setPosition(newPos);
	}

	sf::FloatRect getGlobalBounds() {
		return player.getGlobalBounds();
	}
	int getY() {
		return player.getPosition().y;
	}
	int getX() {
		return player.getPosition().x;
	}

	bool isCollidingWithCoin(Coin *coin) {
		if (player.getGlobalBounds().intersects(coin->getGlobalBounds())) {
			return true;
		}
		return false;
	}
	bool isCollidingWithEnemy(Enemy* enemy) {
		if (player.getGlobalBounds().intersects(enemy->getGlobalBounds())) {
			return true;
		}
		return false;
	}
	bool isCollidingWithGround(Ground* ground) {
		if (player.getGlobalBounds().intersects(ground->getGlobalBounds())) {
			return true;
		}
		return false;
	}


private:
	sf::RectangleShape player;
};


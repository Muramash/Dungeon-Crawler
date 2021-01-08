#pragma once

#include <iostream>
#include <SFML\Graphics.hpp>
#include "Player.h"

class Enemy
{
public:
	Enemy(sf::Vector2f size, sf::Texture* texture) {
		enemy.setSize(size);
		enemy.setTexture(texture);
		enemy.setFillColor(sf::Color::Red);
	}

	void drawTo(sf::RenderWindow& window) {
		window.draw(enemy);
	}

	void move(sf::Vector2f distance) {
		enemy.move(distance);
	}

	void setPos(sf::Vector2f newPos) {
		enemy.setPosition(newPos);

	}
	sf::FloatRect getGlobalBounds() {
		return enemy.getGlobalBounds();
	}
	int getY() {
		return enemy.getPosition().y;
	}
	int getX() {
		return enemy.getPosition().x;
	}

private:
	sf::RectangleShape enemy;
};


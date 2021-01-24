#pragma once

#include <iostream>
#include <SFML\Graphics.hpp>

class Stairs
{
public:
	Stairs(sf::Vector2f size, sf::Texture* texture) {
		stairs.setSize(size);
		stairs.setTexture(texture);
		stairs.setOrigin(size.x / 2, size.y / 2);
	}

	void drawTo(sf::RenderWindow& window) {
		window.draw(stairs);
	}

	void move(sf::Vector2f distance) {
		stairs.move(distance);
	}

	void setPos(sf::Vector2f newPos) {
		stairs.setPosition(newPos);
	}

	sf::FloatRect getGlobalBounds() {
		return stairs.getGlobalBounds();
	}
	int getY() {
		return stairs.getPosition().y;
	}
	int getX() {
		return stairs.getPosition().x;
	}

private:
	sf::RectangleShape stairs;
};


#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>

class EmptySpace {
public:
	EmptySpace(sf::Vector2f size, sf::Texture* texture) {
		emptySpace.setSize(size);
		emptySpace.setTexture(texture);
	}

	void drawTo(sf::RenderWindow& window) {
		window.draw(emptySpace);
	}

	sf::FloatRect getGlobalBounds() {
		return emptySpace.getGlobalBounds();
	}

	void setPos(sf::Vector2f newPos) {
		emptySpace.setPosition(newPos);
	}
	int getY() {
		return emptySpace.getPosition().y;
	}
	int getX() {
		return emptySpace.getPosition().x;
	}
private:
	sf::RectangleShape emptySpace;
};
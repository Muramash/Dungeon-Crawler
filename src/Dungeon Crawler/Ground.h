#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>

class Ground {
public:
	Ground(sf::Vector2f size, sf::Texture* texture) {
		ground.setSize(size);
		ground.setTexture(texture);
	}

	void drawTo(sf::RenderWindow& window) {
		window.draw(ground);
	}

	sf::FloatRect getGlobalBounds() {
		return ground.getGlobalBounds();
	}

	void setPos(sf::Vector2f newPos) {
		ground.setPosition(newPos);
	}
	int getY() {
		return ground.getPosition().y;
	}
	int getX() {
		return ground.getPosition().x;
	}

private:
	sf::RectangleShape ground;
};
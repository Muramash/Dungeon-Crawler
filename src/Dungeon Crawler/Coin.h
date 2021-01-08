#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>

class Coin {
public:
	Coin(sf::Vector2f size, sf::Texture *texture) {
		coin.setSize(size);
		coin.setTexture(texture);
	}

	void drawTo(sf::RenderWindow &window) {
		window.draw(coin);
	}

	sf::FloatRect getGlobalBounds() {
		return coin.getGlobalBounds();
	}

	void setPos(sf::Vector2f newPos) {
		coin.setPosition(newPos);
	}
private:
	sf::RectangleShape coin;
};
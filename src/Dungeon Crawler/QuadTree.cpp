#include "QuadTree.h"
#include <sstream>

QuadTree::QuadTree(sf::FloatRect _boundary,int _level) :
	boundary(_boundary),
	level(_level)
{
	// To draw the quadtree
	shape.setPosition(boundary.left, boundary.top);
	shape.setSize(sf::Vector2f(boundary.width, boundary.height));
	shape.setFillColor(sf::Color(0, 0, 0, 0));
	shape.setOutlineThickness(1.0f);
	shape.setOutlineColor(sf::Color(64, 128, 255));
	text.setPosition(boundary.left, boundary.top + level * 16);
	text.setCharacterSize(12);

	// creating subsections while the max level hasn't been reached
	if (level != maxLevel) {
		northWest = new QuadTree(sf::FloatRect(boundary.left, boundary.top, boundary.width / 2.0f, boundary.height / 2.0f), level + 1);
		northEast = new QuadTree(sf::FloatRect(boundary.left + boundary.width / 2.0f, boundary.top, boundary.width / 2.0f, boundary.height / 2.0f), level + 1);
		southWest = new QuadTree(sf::FloatRect(boundary.left, boundary.top + boundary.height / 2.0f, boundary.width / 2.0f, boundary.height / 2.0f), level + 1);
		southEast = new QuadTree(sf::FloatRect(boundary.left + boundary.width / 2.0f, boundary.top + boundary.height / 2.0f, boundary.width / 2.0f, boundary.height / 2.0f), level + 1);
	}
}

QuadTree::~QuadTree()
{
	// deleting every subsections
	if (level == maxLevel) {
		return;
	}

	delete northWest;
	delete northEast;
	delete southWest;
	delete southEast;
}

void QuadTree::insert(Ground *object) {
	
	// Add the object when on the last level
	if (level == maxLevel) {
		objects.push_back(object);
		return;
	}

	// Check in which section the object belongs
	if (contains(northWest, object)) {
		northWest->insert(object); return;
	}
	else if (contains(northEast, object)) {
		northEast->insert(object); return;
	}
	else if (contains(southWest, object)) {
		southWest->insert(object); return;
	}
	else if (contains(southEast, object)) {
		southEast->insert(object); return;
	}

	// Add the element if this section contains it
	if (contains(this, object)) {
		objects.push_back(object);
	}

}

vector<Ground*> QuadTree::getObjects(sf::FloatRect range) {

	// when on the last level, return objects
	if (level == maxLevel) {
		return objects;
	}

	// Prepare an array of results
	vector<Ground*> objectsInRange, childObjectsInRange;

	if (!objects.empty()) {
		objectsInRange = objects;
	}

	// if the object is in the right side of the quadtree
	if (range.left >= boundary.left + boundary.width / 2.0f && range.left < boundary.left + boundary.width) {
		// if the object is in the bottom side of the quadtree
		if (range.top >= boundary.top + boundary.height / 2.0f && range.top < boundary.top + boundary.height) {
			childObjectsInRange = southEast->getObjects(range);
			for (Ground* o : childObjectsInRange) {
				objectsInRange.push_back(o);
			}
			return objectsInRange;
		}
		// if the object is in the top side of the quadtree
		else if (range.top > boundary.top && range.top < boundary.top + boundary.height / 2.0f) {
			childObjectsInRange = northEast->getObjects(range);
			for (Ground* o : childObjectsInRange) {
				objectsInRange.push_back(o);
			}
			return objectsInRange;
		}
	}
	// if the object is in the left side of the quadtree
	else if (range.left > boundary.left && range.left < boundary.left + boundary.width / 2.0f) {
		// if the object is in the bottom side of the quadtree
		if (range.top >= boundary.top + boundary.height / 2.0f && range.top < boundary.top + boundary.height) {
			childObjectsInRange = southWest->getObjects(range);
			for (Ground* o : childObjectsInRange) {
				objectsInRange.push_back(o);
			}
			return objectsInRange;
		}
		// if the object is in the top side of the quadtree
		else if (range.top > boundary.top && range.top < boundary.top + boundary.height / 2.0f) {
			childObjectsInRange = northWest->getObjects(range);
			for (Ground* o : childObjectsInRange) {
				objectsInRange.push_back(o);
			}
			return objectsInRange;
		}
	}

	return objectsInRange;
}

void QuadTree::Draw(sf::RenderTarget &canvas) {

	stringstream ss;
	ss << objects.size();
	string numObjectsStr = ss.str();
	text.setString(numObjectsStr);
	canvas.draw(shape);
	canvas.draw(text);

	if (level != maxLevel) {
		northWest->Draw(canvas);
		northEast->Draw(canvas);
		southWest->Draw(canvas);
		southEast->Draw(canvas);
	}
}

bool QuadTree::contains(QuadTree *child, Ground *object) {
				
	// check if the object is inside the child borders
	bool res = (object->getX() >= child->boundary.left &&
				object->getY() >= child->boundary.top &&
				object->getX() < child->boundary.left + child->boundary.width &&
				object->getY() < child->boundary.top + child->boundary.height &&
				object->getX() + object->getGlobalBounds().width > child->boundary.left &&
				object->getY() + object->getGlobalBounds().height > child->boundary.top &&
				object->getX() + object->getGlobalBounds().width <= child->boundary.left + child->boundary.width &&
				object->getY() + object->getGlobalBounds().height <= child->boundary.top + child->boundary.height);

	return res;
}
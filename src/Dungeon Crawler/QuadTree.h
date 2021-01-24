#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include <vector>
#include "Ground.h"

using namespace std;

class QuadTree
{
public:
	QuadTree(sf::FloatRect boundary, int level);
	~QuadTree();

	void insert(Ground* object);
	vector<Ground*> getObjects(sf::FloatRect range);
	void Draw(sf::RenderTarget& canvas);

private :
	bool contains(QuadTree* child, Ground* object);

private:
	const int maxLevel = 3;
	int level;
	sf::FloatRect boundary;
	vector<Ground*>	objects;

	// To Draw the quadtree
	sf::RectangleShape shape;
	sf::Text text;

	// QuadTree contains 4 children
	QuadTree* northWest;
	QuadTree* northEast;
	QuadTree* southWest;
	QuadTree* southEast;
};

#endif

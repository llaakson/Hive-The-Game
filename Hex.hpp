#pragma once
#include <SFML/Graphics.hpp>
#include "Tile.hpp"

struct HexCell
{
	int q;
	int r;

	sf::CircleShape shape;
	sf::Color color = sf::Color::White; // are we even using this anymore?? color is inside the shape

	bool is_piece = false;
	bool permanent = false;
	bool selected = false;

	int piece_type = 0;
	TileType tile_type; //This two are essentially the same thing??

	//matrix coordinates for grid indexing
	int x;
	int y;
};

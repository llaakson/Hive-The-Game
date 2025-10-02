#pragma once
#include <SFML/Graphics.hpp>

struct HexCell
{
	int q;
	int r;

	sf::CircleShape shape;
	sf::Color color = sf::Color::White;

	bool is_piece = false;
	bool permanent = false;
	bool selected = false;

	int piece_type = 0;

	//matrix coordinates for grid indexing
	int x;
	int y;
};

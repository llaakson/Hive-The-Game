#pragma once
#include "Hex.hpp"
#include <vector>
#include <array>

class Board
{
	public:
		Board(int rows, int cols, float radius, sf::RenderWindow& window);
		void draw(sf::RenderWindow& window);
		void printMatrix() const;
		std::vector<HexCell>& getCells() { return cells; }
		HexCell* getCellAt(sf::Vector2f mousePos);

	private:
		int rows;
		int cols;
		float radius;
		std::vector<HexCell> cells;
		std::array<std::array<int, 10>, 5> game_matrix;
};


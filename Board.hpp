#pragma once
#include "Hex.hpp"
#include <vector>
#include <array>
#include <unordered_map>
#include <vector>

// --- Neighbor Directions ---
// Axial directions for flat-topped hexes
// now handles column offset
const std::array<std::pair<int,int>,6> EVEN_COL_OFFSETS {{
    {+1,0},{0,-1},{-1,-1},{-1,0},{-1,+1},{0,+1}
}};

const std::array<std::pair<int,int>,6> ODD_COL_OFFSETS {{
    {+1,0},{+1,-1},{0,-1},{-1,0},{0,+1},{+1,+1}
}};

class Board
{
	public:
		Board(int rows, int cols, float radius, sf::RenderWindow& window);
		void draw(sf::RenderWindow& window);
		void printMatrix() const;
		std::vector<HexCell>& getCells() { return cells; }
		HexCell* getCellAt(sf::Vector2f mousePos);
		HexCell* getCellAtAxial(int q, int r);
		std::vector<HexCell*> getNeighbours(const HexCell& cell);
		std::array<std::array<int, 10>, 5> game_matrix;
		HexCell* init_old_cell();
		void SetPieceChar(HexCell *c);
	private:
		int rows;
		int cols;
		float radius;
		std::vector<HexCell> cells;
		//std::array<std::array<int, 10>, 5> game_matrix;
		sf::Text piece_char;
		sf::Font font;
		std::vector<sf::Text> piece_chars;
};


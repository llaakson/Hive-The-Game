#include "Board.hpp"
#include <iostream>
#include <cmath>

static sf::Vector2f hexToPixelOffset(int col, int row, float radius)
{
	float x = radius * 1.5f * col;
	float y = std::sqrt(3.f) * radius * (row + 0.5f * (col & 1));
	return {x, y};
}

Board::Board(int rows, int cols, float radius, sf::RenderWindow& window) : rows(rows), cols(cols), radius(radius)
{
	game_matrix.fill({0});

	std::vector<sf::Vector2f> positions;
	positions.reserve(cols * rows);

	for (int row = 0; row < rows; ++row)
		for (int col = 0; col < cols; ++col)
			positions.push_back(hexToPixelOffset(col, row, radius));

	float minX = positions[0].x, maxX = positions[0].x;
	float minY = positions[0].y, maxY = positions[0].y;

	for (auto& p : positions)
	{
		minX = std::min(minX, p.x);
		maxX = std::max(maxX, p.x);
		minY = std::min(minY, p.y);
		maxY = std::max(maxY, p.y);
	}
	float gridWidth = maxX - minX;
	float gridHeight = maxY - minY;
	
	float offsetX = (window.getSize().x - gridWidth) / 2.f - minX;
    	float offsetY = (window.getSize().y - gridHeight) / 2.f - minY;
	
	int idx = 0;
	for (int row = 0; row < rows; ++row) 
	{
        	for (int col = 0; col < cols; ++col)
		{
			HexCell cell;
			cell.q = col - row / 2;
			cell.r = row;
			cell.x = row;
			cell.y = col;
			
			cell.shape = sf::CircleShape(radius, 6);
			cell.shape.setRotation(30);
			cell.shape.setFillColor(sf::Color::White);
			cell.shape.setOutlineThickness(-2);
			cell.shape.setOutlineColor(sf::Color::Black);

			sf::Vector2f pos = positions[idx++];
			pos.x += offsetX;
			pos.y += offsetY;
			cell.shape.setPosition(pos);
			cells.push_back(cell);
		}
	}
}

void Board::draw(sf::RenderWindow& window)
{
	for (auto& c : cells)
		window.draw(c.shape);
}

HexCell* Board::getCellAt(sf::Vector2f mousePos) 
{
	for (auto& cell : cells) 
	{
        	sf::Transform transform = cell.shape.getTransform();
        	std::vector<sf::Vector2f> vertices;
        	for (size_t i = 0; i < cell.shape.getPointCount(); ++i)
            		vertices.push_back(transform.transformPoint(cell.shape.getPoint(i)));

        	bool inside = false;
        	for (size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) 
		{
            		const sf::Vector2f& vi = vertices[i];
            		const sf::Vector2f& vj = vertices[j];
            		bool intersect = ((vi.y > mousePos.y) != (vj.y > mousePos.y)) &&
                             (mousePos.x < (vj.x - vi.x) * (mousePos.y - vi.y) / (vj.y - vi.y) + vi.x);
            		if (intersect)
				inside = !inside;
        	}
		if (inside) 
			return &cell;
	}
	return nullptr;
}

HexCell* Board::getCellAtAxial(int q, int r)
{
	for (auto& cell : cells)
	{
		if (cell.q == q && cell.r == r)
			return &cell;
	}
	return nullptr;
}

std::vector<HexCell*> Board::getNeighbours(const HexCell& cell)
{
	std::vector<HexCell*> neighbours;
	for (auto [dq, dr] : HEX_DIRECTIONS)
	{
		int nq = cell.q + dq;
		int nr = cell.r + dr;
		if (HexCell* neighbour = getCellAtAxial(nq, nr))
			neighbours.push_back(neighbour);
	}
	return neighbours;
}

void Board::printMatrix() const
{
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
			std::cout << game_matrix[i][j] << "";
		std::cout << "\n";
	}
}

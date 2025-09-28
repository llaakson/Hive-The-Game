#include "Hive.hpp"

struct HexCell
{
	int q; // horizontal axis
	int r; // diagonal axis
	sf::CircleShape shape; //visual representation
	bool selected = false;
};

sf::Vector2f hexToPixelOffset(int col, int row, float radius) {
    float x = radius * 1.5f * col;
    float y = std::sqrt(3.f) * radius * (row + 0.5f * (col & 1));
    return {x, y};
}

// --- Neighbor Directions ---
// Axial directions for flat-topped hexes
const std::array<std::pair<int,int>, 6> HEX_DIRECTIONS 
{{
    {+1, 0}, {+1, -1}, {0, -1},
    {-1, 0}, {-1, +1}, {0, +1}
}};

bool pointInHex(const sf::CircleShape& hex, sf::Vector2f point)
{
    sf::Transform transform = hex.getTransform();

    std::vector<sf::Vector2f> vertices;
    vertices.reserve(hex.getPointCount());

    for (size_t i = 0; i < hex.getPointCount(); ++i)
        vertices.push_back(transform.transformPoint(hex.getPoint(i)));

    bool inside = false;
    for (size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) 
    {
        const sf::Vector2f& vi = vertices[i];
        const sf::Vector2f& vj = vertices[j];

        bool intersect = ((vi.y > point.y) != (vj.y > point.y)) &&
                         (point.x < (vj.x - vi.x) * (point.y - vi.y) / (vj.y - vi.y) + vi.x);
        if (intersect)
            inside = !inside;
    }

    return inside;
}

int main() 
{
    const float radius = 40.f;
    const int cols = 10;
    const int rows = 5;

    sf::RenderWindow window(sf::VideoMode(800, 600), "Hive");

    // generating grid based on columns and rows
    std::vector<HexCell> cells;
    cells.reserve(cols * rows);

    std::vector<sf::Vector2f> positions;
    positions.reserve(cols * rows);

    for (int row = 0; row < rows; ++row)
    {
	for (int col = 0; col < cols; ++col)
	{
        	sf::Vector2f pos = hexToPixelOffset(col, row, radius);
        	positions.push_back(pos);
    	}
    }

    // centering
    float minX = positions[0].x, maxX = positions[0].x;
    float minY = positions[0].y, maxY = positions[0].y;
    for (auto& p : positions) {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }
    float gridWidth = maxX - minX;
    float gridHeight = maxY - minY;

    float offsetX = (window.getSize().x - gridWidth) / 2.f - minX;
    float offsetY = (window.getSize().y - gridHeight) / 2.f - minY;

    // hex generation - replaced basic hexes  with structs HEXES (holding coordinates)
    int idx = 0;
    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            HexCell cell;

            // Convert offset coords to axial coordinates for Hive logic
            cell.q = col - row / 2;
            cell.r = row;

            cell.shape = sf::CircleShape(radius, 6);
            cell.shape.setRotation(30); // flat-topped
            cell.shape.setFillColor(sf::Color(200, 200, 255));
            cell.shape.setOutlineThickness(-2);
            cell.shape.setOutlineColor(sf::Color::Black);

            sf::Vector2f pos = positions[idx++];
            pos.x += offsetX;
            pos.y += offsetY;
            cell.shape.setPosition(pos);

            cells.push_back(cell);
        }
    }

// main loop
    while (window.isOpen()) 
    {
        sf::Event event;
        while (window.pollEvent(event)) 
	{
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) 
	    {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                for (auto& cell : cells) 
		{
                    if (pointInHex(cell.shape, mousePos)) 
		    {
                        cell.selected = !cell.selected;
                        std::cout << "Clicked hex q=" << cell.q << " r=" << cell.r
                                  << " (s=" << -cell.q - cell.r << ")\n";
                    }
                }
            }
        }

        window.clear(sf::Color::White);
        for (auto& cell : cells) 
	{
            cell.shape.setFillColor(cell.selected ? sf::Color::Yellow
                                                  : sf::Color(200, 200, 255));
            window.draw(cell.shape);
        }
        window.display();
    }

    return 0;
}

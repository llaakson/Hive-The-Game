#include "Hive.hpp"
//#include "Tile.hpp"
//#include "Queen.hpp"

constexpr int cols = 10;
constexpr int rows = 5;
int turn_counter = 0;

struct HexCell
{
	int q; // horizontal axis
	int r; // diagonal axis
	sf::CircleShape shape; //visual representation
	bool selected = false;
	sf::Color color = sf::Color(255,255,255);
	bool is_piece = false;

	//matrix goordinates
	int x;
	int y;
	int piece_type = 0;
	
	bool permanent = false;
};

struct StoredColor {
	sf::Color color = sf::Color(255,255,255);
	int piece_type = 0;
	HexCell *oldpiece;
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

void print_game_matrix(std::array<std::array<int,cols>, rows> game_matrix)
{
	for (size_t i = 0; i < game_matrix.size(); ++i) {
        for (size_t j = 0; j < game_matrix[i].size(); ++j) {
            std::cout << game_matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

bool isFirstPiece(sf::Color playerColor, const std::vector<HexCell>& cells) 
{
    for (const auto& c : cells) 
    {
        if (c.is_piece && !c.permanent && c.color == playerColor)
            return false;
    }
    return true;
}

bool hasSameColorNeighbour(HexCell& cell, std::vector<HexCell>& cells, sf::Color playerColor)
{
	for (auto [dq, dr] : HEX_DIRECTIONS)
	{
		int nq = cell.q + dq;
		int nr = cell.r + dr;

		for (const auto& neighbor : cells)
		{
			if (neighbor.q == nq && neighbor.r == nr)
			{
				if (neighbor.is_piece && neighbor.color == playerColor)
				{
					std::cout << "same color as neighbour!\n";
					return true;
				}
			}
		}
	}
	return false;
}

int main() 
{
    const float radius = 40.f;
    //const int cols = 10; // moved top of the file to be globals for the print function
    //const int rows = 5;
    std::array<std::array<int,cols>, rows> game_matrix = {0};

    sf::RenderWindow window(sf::VideoMode(900, 600), "Hive");

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
            cell.shape.setFillColor(sf::Color(255, 255, 255));
            cell.shape.setOutlineThickness(-2);
            cell.shape.setOutlineColor(sf::Color::Black);
		
	    cell.x = row;
	    cell.y = col;
            sf::Vector2f pos = positions[idx++];
            pos.x += offsetX;
            pos.y += offsetY;
            cell.shape.setPosition(pos);

            cells.push_back(cell);
        }
    }

     HexCell cell;
     cell.shape = sf::CircleShape(radius, 6);
     cell.shape.setRotation(30); // flat-topped
     cell.shape.setFillColor(sf::Color(0, 0, 255));
     cell.color = sf::Color(0,0,255);
     cell.shape.setOutlineThickness(-2);
     cell.shape.setOutlineColor(sf::Color::Black);
     cell.piece_type = 1;
     cell.permanent = true;

     sf::Vector2f pos = positions[idx++];
     pos.x += 50;
     pos.y += 50;
     cell.shape.setPosition(pos);
     cell.is_piece = true;
     cells.push_back(cell);
     cell.shape = sf::CircleShape(radius, 6);
     cell.shape.setRotation(30); // flat-topped
     cell.shape.setFillColor(sf::Color(0, 255, 0));
     cell.color = sf::Color(0,255,0);
     cell.shape.setOutlineThickness(-2);
     cell.shape.setOutlineColor(sf::Color::Black);

     //sf::Vector2f pos = positions[idx++];
     pos.x += 50;
     pos.y += 70;
     cell.shape.setPosition(pos);
     cell.is_piece = true;
     cell.piece_type = 2;
     cells.push_back(cell);

// main loop
    StoredColor storedcolor;
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
			if (cell.is_piece == true){
				storedcolor.piece_type = cell.piece_type;
				storedcolor.color = cell.color;
				storedcolor.oldpiece = &cell;

			}
			// all conditions checks are happening here
			else if (cell.color == sf::Color::White && storedcolor.color != sf::Color::White)
			{
				if (isFirstPiece(storedcolor.color, cells) || hasSameColorNeighbour(cell, cells, storedcolor.color))
				{
				cell.color = storedcolor.color;
                        	cell.selected = !cell.selected;
                        	std::cout << "Clicked hex q=" << cell.q << " r=" << cell.r << " (s=" << -cell.q - cell.r << ")\n";
				cell.shape.setFillColor(cell.color);
				cell.piece_type = storedcolor.piece_type;
				storedcolor.color = sf::Color(255,255,255);
				storedcolor.piece_type = 0;
				game_matrix[cell.x][cell.y] = cell.piece_type; // testing to update game matrix when piece is placed
				if (cell.color != sf::Color::White)
					cell.is_piece = true; // cell now has a game piece on it
				if (storedcolor.oldpiece->permanent == false){
				storedcolor.oldpiece->color = sf::Color::White;
				storedcolor.oldpiece->piece_type = 0;
				storedcolor.oldpiece->is_piece = false;
				storedcolor.oldpiece->shape.setFillColor(sf::Color::White);
				game_matrix[storedcolor.oldpiece->x][storedcolor.oldpiece->y] = storedcolor.oldpiece->piece_type;
				print_game_matrix(game_matrix);}
				}
				else std::cout << "Place next to the same color!\n";
				turn_counter++;
			}
                    }
                }
            }
        }

        window.clear(sf::Color::White);
        for (auto& cell : cells) 
	{
            //cell.shape.setFillColor(cell.selected ? sf::Color::Yellow
             //                                     : sf::Color(200, 200, 255));
            window.draw(cell.shape);
        }
        window.display();
    }

    return 0;
}


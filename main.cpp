#include "Hive.hpp"
#include "Board.hpp"

int main() 
{
	sf::RenderWindow window(sf::VideoMode(900, 600), "Hive");
	Board board(5, 10, 40.f, window);

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
				HexCell* clicked = board.getCellAt(mousePos);
				if (clicked)
					clicked->shape.setFillColor(sf::Color::Yellow);
			}
		}
		window.clear(sf::Color::White);
		board.draw(window);
		window.display();
	}
}


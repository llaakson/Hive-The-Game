#include "Hive.hpp"
#include "Engine.hpp"

int main() 
{
	sf::RenderWindow window(sf::VideoMode(900, 600), "Hive");
	Engine engine(window);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event)) 
		{
			if (event.type == sf::Event::Closed)
				window.close();
			engine.handleEvent(event);
		}
		engine.update();
		window.clear(sf::Color::White);
		engine.draw();
		window.display();
	}
}


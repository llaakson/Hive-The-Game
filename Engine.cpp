#include "Engine.hpp"
#include <iostream>

Engine::Engine(sf::RenderWindow& window) 
    : window(window), 
      board(5, 10, 40.f, window), 
      player1(sf::Color::Blue),
      player2(sf::Color::Green),
      currentIndex(0)
{}

Player& Engine::currentPlayer() 
{
    return (currentIndex == 0 ? player1 : player2);
}

void Engine::nextTurn() 
{
    currentIndex = 1 - currentIndex;
}

void Engine::handleEvent(const sf::Event& event)
{
	if (event.type == sf::Event::MouseButtonPressed) 
	{
		sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
		HexCell* clicked = board.getCellAt(mousePos);
		if (clicked) 
		{
			if (currentPlayer().hasAvailable(TileType::QUEEN)) 
			{
				clicked->shape.setFillColor(currentPlayer().getColor());
				clicked->is_piece = true;
				clicked->piece_type = static_cast<int>(TileType::QUEEN);
				currentPlayer().usePiece(TileType::QUEEN);
				nextTurn();
			}
			else
		                std::cout << "No more Queens left for this player!\n";
            	}
        }
}

void Engine::update() 
{
    // for now nothing
}

void Engine::draw() 
{
    board.draw(window);
}


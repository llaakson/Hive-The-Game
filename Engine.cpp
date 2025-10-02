#include "Engine.hpp"
#include <iostream>

Engine::Engine(sf::RenderWindow& window) 
	: window(window), 
	board(5, 10, 40.f, window), 
	player1(sf::Color::Blue),
	player2(sf::Color::Green),
	currentIndex(0),
	selected(TileType::QUEEN)
{
	if (!font.loadFromFile("arial.ttf")) 
		std::cerr << "Failed to load font!\n";
    uiText.setFont(font);
    uiText.setCharacterSize(18);
    uiText.setFillColor(sf::Color::Black);
    uiText.setPosition(10, 10);
}

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
	if (event.type == sf::Event::KeyPressed)
	{
		switch(event.key.code)
		{
			case sf::Keyboard::Num1:
				selected = TileType::QUEEN;
				std::cout << "choice: QUEEN\n";
				break;
			case sf::Keyboard::Num2:
				selected = TileType::BEETLE;
				std::cout << "choice: BEETLE\n";
				break;
			case sf::Keyboard::Num3:
				selected = TileType::SPIDER;
				std::cout << "choice: SPIDER\n";
				break;
			case sf::Keyboard::Num4:
				selected = TileType::ANT;
				std::cout << "choice: ANT\n";
				break;
			case sf::Keyboard::Num5:
				selected = TileType::GRASSHOPPER;
				std::cout << "choice: GRASSHOPPER\n";
				break;
			default: break;
		}
	}
	if (event.type == sf::Event::MouseButtonPressed) 
	{
		sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
		HexCell* clicked = board.getCellAt(mousePos);
		if (!clicked)
			return;
		if (clicked->is_piece)
		{
			std::cout << "already taken\n";
			return;
		}
		
		if (!currentPlayer().hasAvailable(selected))
		{
			std::cout << "not available anymore\n";
			return;
		}
		bool firstMove = true;
		for (auto& c : board.getCells())
		{
			if (c.is_piece)
			{
				firstMove = false;
				break;
			}
		}
		
		auto nbrs = board.getNeighbours(*clicked);
		bool touchesOwn = false;
		bool touchesOpponent = false;
		for (auto* n : nbrs)
		{
			if (!n->is_piece)
				continue;
			if (n->shape.getFillColor() == currentPlayer().getColor())
				touchesOwn = true;
			else
				touchesOpponent = true;
		}
		bool canPlace = false;
		if (firstMove)
			canPlace = true;
		else if (!touchesOwn && touchesOpponent)
			canPlace = true;
		else if (touchesOwn)
			canPlace = true;
		if (!canPlace)
		{
			std::cout << "Invalid move\n";
			return;
		}

		clicked->shape.setFillColor(currentPlayer().getColor());
		clicked->is_piece = true;
		clicked->piece_type = static_cast<int>(selected);
		currentPlayer().usePiece(selected);
		board.game_matrix[clicked->x][clicked->y] = clicked->piece_type;
		board.printMatrix();
		nextTurn();
        }
}

void Engine::update() 
{
    std::string text = "Player " + std::string(currentIndex == 0 ? "Blue" : "Green") + "\n";
    text += "Selected: ";
    switch (selected) 
	{
        case TileType::QUEEN: text += "Queen\n"; break;
        case TileType::BEETLE: text += "Beetle\n"; break;
        case TileType::SPIDER: text += "Spider\n"; break;
        case TileType::ANT: text += "Ant\n"; break;
        case TileType::GRASSHOPPER: text += "Grasshopper\n"; break;
        default: text += "None\n"; break;
    }

    // show how many left
    text += "Reserves:\n";
    text += "Queen: " + std::to_string(currentPlayer().getReserveCount(TileType::QUEEN)) + "\n";
    text += "Beetle: " + std::to_string(currentPlayer().getReserveCount(TileType::BEETLE)) + "\n";
    text += "Spider: " + std::to_string(currentPlayer().getReserveCount(TileType::SPIDER)) + "\n";
    text += "Ant: " + std::to_string(currentPlayer().getReserveCount(TileType::ANT)) + "\n";
    text += "Grasshopper: " + std::to_string(currentPlayer().getReserveCount(TileType::GRASSHOPPER)) + "\n";

    uiText.setString(text);
}

void Engine::draw() 
{
    board.draw(window);
	window.draw(uiText);
}


#pragma once
#include "Hive.hpp"
#include "Board.hpp"
#include "Player.hpp"

class Engine
{
	public:
		Engine(sf::RenderWindow& window);

		void handleEvent(const sf::Event& event);
		void update();
		void draw();

	private:
		sf::RenderWindow& window;
		Board board;
		Player player1;
		Player player2;
		int currentIndex;
		Player& currentPlayer();
		void nextTurn();
		TileType selected;
		sf::Font font;
		sf::Text uiText;
};

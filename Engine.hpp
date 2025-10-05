#pragma once
#include "Hive.hpp"
#include "Board.hpp"
#include "Player.hpp"
#include "Tile.hpp"

class Engine
{
	public:
		Engine(sf::RenderWindow& window);

		void handleEvent(const sf::Event& event);
		void update();
		void draw();
		void clear_tile();
		bool getDraw_chars();
		bool oneHive(HexCell* cell);

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
		HexCell *old_cell;
		bool draw_chars;
};

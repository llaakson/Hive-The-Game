#pragma once
#include "Hive.hpp"
#include <map>
#include "Tile.hpp"

class Player
{
	public:
		Player(const sf::Color& color);
		void usePiece(TileType type);
		void returnPiece(TileType type);
		bool hasAvailable(TileType type) const;

		const sf::Color& getColor() const;
		int getReserveCount(TileType type) const;

	private:
		sf::Color color;
		std::map<TileType, int> reserve;
};

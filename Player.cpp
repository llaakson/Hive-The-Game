#include "Player.hpp"


const sf::Color& Player::getColor() const
{
	return color;
}

Player::Player(const sf::Color& color) : color(color)
{
	reserve[TileType::QUEEN] = 1;
	reserve[TileType::BEETLE] = 2;
	reserve[TileType::SPIDER] = 2;
	reserve[TileType::GRASSHOPPER] = 3;
	reserve[TileType::ANT] = 3;
}

bool Player::hasAvailable(TileType type) const
{
	auto it = reserve.find(type);
	if (it != reserve.end() && it->second > 0)
		return true;
	return false;
}

void Player::usePiece(TileType type)
{
	if (hasAvailable(type))
		reserve[type]--;
}

void Player::returnPiece(TileType type)
{
	reserve[type]++;
}

int Player::getReserveCount(TileType type) const
{
	auto it = reserve.find(type);
	if (it != reserve.end())
		return it->second;
	else
		return 0;
}

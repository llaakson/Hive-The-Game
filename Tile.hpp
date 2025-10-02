#pragma once

enum class TileType
{
	NONE = 0,
	QUEEN,
	BEETLE,
	SPIDER,
	ANT,
	GRASSHOPPER
};

struct Tile
{
	TileType type;
	int playerId;
};


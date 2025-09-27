#pragma once

class Tile
{
	public:
		int type;
		int blocked = 6;
		bool isPlaced = false;
		Tile();
		virtual ~Tile();
}

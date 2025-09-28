#pragma once

class Tile
{
	public:
		int type;
		int blocked = 6;
		bool isPlaced = false;
		Tile();
		virtual ~Tile();
		Player owner;
		//add a lock - if locked, piece cannot be moved
}

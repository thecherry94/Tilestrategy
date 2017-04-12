#include "Tile.hpp"


Tile::Tile(int img_id, bool is_obstacle = false)
{
	_img_id = img_id;

	_is_obstacle = is_obstacle;
}

Tile* Tile::create_empty()
{
	return new Tile(TS_GRASS);
}
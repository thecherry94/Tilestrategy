#include "Tile.hpp"


Tile::Tile(int img_id, bool is_obstacle = false)
{
	_img_id = img_id;
	_is_obstacle = is_obstacle;
}

std::unique_ptr<Tile> Tile::create_empty()
{
	return std::make_unique<Tile>(TS_GRASS);
}


int Tile::get_image_id()
{
	return _img_id;
}

bool Tile::is_obstacle()
{
	return is_obstacle;
}
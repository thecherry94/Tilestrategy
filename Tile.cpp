#include "Tile.hpp"


Tile::Tile(int img_id, bool is_obstacle = false)
{
	_img_id = img_id;
	_is_obstacle = is_obstacle;
}

std::shared_ptr<Tile> Tile::create_empty()
{
	return std::make_shared<Tile>(TS_GRASS);
}

std::shared_ptr<Tile> Tile::create_wall()
{
	return std::make_shared<Tile>(TS_WALL, true);
}


int Tile::get_image_id()
{
	return _img_id;
}

bool Tile::is_obstacle()
{
	return _is_obstacle;
}
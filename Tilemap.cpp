#include "Tilemap.hpp"


Tilemap::Tilemap(sf::Vector2<int> map_size, sf::String image_path)
{
	_map = std::vector<std::vector<std::vector<Tile*>>>();
	_size = map_size;

	// Reserve 
	_map.reserve(_size.y);
	for (int i = 0; i < _size.y; i++)
	{
		_map[i].reserve(_size.x);
	}

	for (int y = 0; y < _size.y; y++)
	{
		for (int x = 0; y < _size.x; x++)
		{
			_map[y][x][0] = Tile::create_empty();
		}
	}


}
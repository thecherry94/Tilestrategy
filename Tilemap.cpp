#include "Tilemap.hpp"



Tilemap::Tilemap(sf::Vector2<int> map_size, sf::String image_path, int num_layers = 1)
{
	_num_layers = num_layers;
	_size = map_size;	

	if (!_texture.loadFromFile(image_path))
	{
		throw std::exception("[FATAL ERROR]\nTilemap: Failed to load texture.");
	}
	
	int width = _size.x;
	int height = _size.y;


	// Reserve memory to save time for later computations
	//
	_map.reserve(width * height);
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			_map[y * width + x].reserve(_num_layers);

	// Fill the map with empty tiles by default
	//
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			_map[y * width + x][0] = Tile::create_empty();

	// Update the texture map
	_texmap = get_texture_map();

	// Update the obstacle map
	_obstmap = get_obstacle_map();


	_map_vertices.setPrimitiveType(sf::Quads);
	_map_vertices.resize(width * height * 4);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int tile_img_id = _texmap[y * width + x][0];

			int tu = tile_img_id % (_texture.getSize().x / _tilesize);
			int tv = tile_img_id / (_texture.getSize().y / _tilesize);

			sf::Vertex* quad = &_map_vertices[tile_img_id * 4];

			quad[0].position = sf::Vector2f(x * _tilesize, y * _tilesize);
			quad[1].position = sf::Vector2f((x + 1) * _tilesize, y * _tilesize);
			quad[2].position = sf::Vector2f((x + 1) * _tilesize, (y + 1) * _tilesize);
			quad[3].position = sf::Vector2f(x * _tilesize, (y + 1) * _tilesize);


			quad[0].texCoords = sf::Vector2f(tu * _tilesize, tv * _tilesize);
			quad[1].texCoords = sf::Vector2f((tu + 1) * _tilesize, tv * _tilesize);
			quad[2].texCoords = sf::Vector2f((tu + 1) * _tilesize, (tv + 1) * _tilesize);
			quad[3].texCoords = sf::Vector2f(tu * _tilesize, (tv + 1) * _tilesize);
		}
	}
}

void Tilemap::draw(sf::RenderTarget& target, sf::RenderStates states)
{
	states.transform *= getTransform();

	states.texture = &_texture;

	target.draw(_map_vertices, states);
}


Tilemap::~Tilemap()
{
	// Originally needed for deleting the tile pointers
	// However, the use of smart pointers make it obsolete
}


/*
 * Returns a 2d-vector
 * first vector is the 2d tilemap condensed to a single array
 * the second vector holds the layers
 * the values of the second vector are the texture id
 */
TextureMap Tilemap::get_texture_map()
{
	TextureMap texmap;
	

	int width = _size.x;
	int height = _size.y;

	// Reserve memory beforehand to save computation time
	//
	texmap.reserve(width * height);
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			texmap[y * width + x].reserve(_num_layers);

	// Fill texmap with information
	//
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			for (int l = 0; l < _num_layers; l++)
				texmap[y * width + x][l] = _map[y * width + x][l]->get_image_id();

	return texmap;
}


/*
 * Returns a 1d-vector
 * This method goes over each single tile and checks for every layer
 * whether any of them are defined as obstacle
 * if yes -> field = true else false
 */
ObstacleMap Tilemap::get_obstacle_map()
{
	ObstacleMap obstmap;


	int width = _size.x;
	int height = _size.y;

	// Reserve memory beforehand to save computation time
	obstmap.reserve(width * height);

	// Fill obstmap with information
	//
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			for (int l = 0; l < _num_layers; l++)
			{
				obstmap[y * width + x] = false;
				if (_map[y * width + x][l]->is_obstacle())
				{
					obstmap[y * width + x] = true;
					break;	// On one of the layers, there is an obstacle, so we don't need to see the rest
				}
			}
		}
	}

	return obstmap;
}
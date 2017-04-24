#include "Tilemap.hpp"



Tilemap::Tilemap(sf::RenderWindow* win, sf::Vector2<int> map_size, int tilesize, sf::String image_path, int num_layers = 1)
{
	_p_win = win;
	_num_layers = num_layers;
	_size = map_size;
	_tilesize = tilesize;

	if (!_texture.loadFromFile(image_path))
	{
		throw std::exception("[FATAL ERROR]\nTilemap: Failed to load texture.");
	}

	if (_texture.getSize().x % _tilesize != 0 || _texture.getSize().y % _tilesize != 0)
	{
		throw std::exception("[FATAL ERROR]\nTilemap: Tile size and tileset size do not match.");
	}

	int width = _size.x;
	int height = _size.y;


	// resize memory and initialize values with empty tiles
	//
	_map.resize(width * height);
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			_map[y * width + x].resize(_num_layers, Tile::create_empty());

	// Update the texture map
	_texmap = generate_texture_map();

	// Update the obstacle map
	_obstmap = generate_obstacle_map();





	/*
	 **************************************************************
	 | Initialize and setup the rendering vertices of the tilemap |
	 **************************************************************
	*/

	_render_vertices.resize(_num_layers);
	_render_vertices[0].setPrimitiveType(sf::Quads);
	_render_vertices[0].resize(width * height * 4);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// Get the tile image id from the texture map on layer 0
			int tile_img_id = _texmap[y * width + x][0];

			// Calculate the texture coordinates for that tile
			int tu = tile_img_id % (_texture.getSize().x / _tilesize);
			int tv = tile_img_id / (_texture.getSize().y / _tilesize);

			sf::Vertex* quad = &_render_vertices[0][(y * width + x) * 4];

			// Calculate the quads position
			// 
			quad[0].position = sf::Vector2f(x * _tilesize, y * _tilesize);
			quad[1].position = sf::Vector2f((x + 1) * _tilesize, y * _tilesize);
			quad[2].position = sf::Vector2f((x + 1) * _tilesize, (y + 1) * _tilesize);
			quad[3].position = sf::Vector2f(x * _tilesize, (y + 1) * _tilesize);

			// Calculate the texture coordinates 
			quad[0].texCoords = sf::Vector2f(tu * _tilesize, tv * _tilesize);
			quad[1].texCoords = sf::Vector2f((tu + 1) * _tilesize, tv * _tilesize);
			quad[2].texCoords = sf::Vector2f((tu + 1) * _tilesize, (tv + 1) * _tilesize);
			quad[3].texCoords = sf::Vector2f(tu * _tilesize, (tv + 1) * _tilesize);
		}
	}
}

void Tilemap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	states.texture = &_texture;

	for (int l = 0; l < _num_layers; l++)
		target.draw(_visible_vertices[l], states);
}


/*
 * This method adds a new tile into the tilemap.
 * A new layer will be added as long as the field specified is not transparent.
 * Use set_tile to change a tile on a layer specified.
 */
void Tilemap::add_tile(sf::Vector2<int> pos, Tile tile)
{
	int x = pos.x;
	int y = pos.y;

	int width = _size.x;
	int height = _size.y;

	// If the tile on the position specified is not transparent, add a new layer
	//
	if (_map[y * width + x][_num_layers - 1]->get_image_id() != TS_TRANSPARENT)
		add_layer();

	// Insert the new tile into the tilemap
	_map[y * width + x][_num_layers - 1] = std::make_shared<Tile>(tile);
	update_maps();
}

void Tilemap::update_maps(sf::Vector2u at)
{
	update_maps(at.x, at.y);
}



void Tilemap::update_maps(unsigned int at_x, unsigned int at_y)
{

}



/*
 * Instead of adding a tile (and potentially a new layer) this method
 * replaces the specified tile (position, layer) with a new one.
 */
void Tilemap::set_tile(sf::Vector2<int> pos, Tile tile, int layer)
{
	int x = pos.x;
	int y = pos.y;

	int width = _size.x;
	int height = _size.y;

	if (layer >= _num_layers || layer < 0)
	{
		std::stringstream str;
		str << "[ERROR] The layer specified does not exist [layer = " << layer << "]";
		throw std::exception(str.str().c_str());
	}

	_map[y * width + x][layer] = std::make_shared<Tile>(tile);
	update_maps();
}


// Overload this method for single changes
//
void Tilemap::update_maps()
{
	_texmap = generate_texture_map();
	_obstmap = generate_obstacle_map();
	update_map_vertices();
	update_visible_vertices(_render_offset);
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
 * the values of the second vector are the texture id.
 */
TextureMap Tilemap::generate_texture_map()
{
	TextureMap texmap;
	

	int width = _size.x;
	int height = _size.y;

	// resize memory beforehand to save computation time
	// 
	texmap.resize(width * height);
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			texmap[y * width + x].resize(_num_layers);
			
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
 * whether any of them are defined as obstacle.
 * if yes -> field = true else false
 */
ObstacleMap Tilemap::generate_obstacle_map()
{
	ObstacleMap obstmap;


	int width = _size.x;
	int height = _size.y;

	// resize memory beforehand to save computation time
	obstmap.resize(width * height);

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


void Tilemap::update_map_vertices()
{
	int width = _size.x;
	int height = _size.y;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			for (int l = 0; l < _num_layers; l++)
			{
				// Get the tile image id from the texture map on layer 0
				int tile_img_id = _texmap[y * width + x][l];

				// Calculate the texture coordinates for that tile
				int tu = tile_img_id % (_texture.getSize().x / _tilesize);
				int tv = tile_img_id / (_texture.getSize().y / _tilesize);

				sf::Vertex* quad = &_render_vertices[l][(y * width + x) * 4];

				// Calculate the texture coordinates 
				quad[0].texCoords = sf::Vector2f(tu * _tilesize, tv * _tilesize);
				quad[1].texCoords = sf::Vector2f((tu + 1) * _tilesize, tv * _tilesize);
				quad[2].texCoords = sf::Vector2f((tu + 1) * _tilesize, (tv + 1) * _tilesize);
				quad[3].texCoords = sf::Vector2f(tu * _tilesize, (tv + 1) * _tilesize);
			}
		}
	}
}


void Tilemap::add_layer()
{
	int width = _size.x;
	int height = _size.y;

	_num_layers++;

	_render_vertices.resize(_num_layers);

	_render_vertices[_num_layers - 1] = sf::VertexArray();
	_render_vertices[_num_layers - 1].setPrimitiveType(sf::Quads);
	_render_vertices[_num_layers - 1].resize(width * height * 4);

	

	// Fill the new layer with transparent tiles first
	//
	for (int ly = 0; ly < height; ly++)
	{
		for (int lx = 0; lx < width; lx++)
		{
			// Let the main map know about the new layer
			//
			_map[ly * width + lx].resize(_num_layers, std::make_shared<Tile>(Tile(TS_TRANSPARENT, false)));

			// Update the texture map manually
			//
			_texmap[ly * width + lx].resize(_num_layers, _map[ly * width + lx][_num_layers - 1]->get_image_id());



			// Copy paste from initialization in constructor above
			//
			int tile_img_id = _texmap[ly * width + lx][_num_layers-1];

			// Calculate the texture coordinates for that tile
			int tu = tile_img_id % (_texture.getSize().x / _tilesize);
			int tv = tile_img_id / (_texture.getSize().y / _tilesize);

			sf::Vertex* quad = &_render_vertices[_num_layers - 1][(ly * width + lx) * 4];

			// Calculate the quads position
			// 
			quad[0].position = sf::Vector2f(lx * _tilesize, ly * _tilesize);
			quad[1].position = sf::Vector2f((lx + 1) * _tilesize, ly * _tilesize);
			quad[2].position = sf::Vector2f((lx + 1) * _tilesize, (ly + 1) * _tilesize);
			quad[3].position = sf::Vector2f(lx * _tilesize, (ly + 1) * _tilesize);

			// Calculate the texture coordinates 
			quad[0].texCoords = sf::Vector2f(tu * _tilesize, tv * _tilesize);
			quad[1].texCoords = sf::Vector2f((tu + 1) * _tilesize, tv * _tilesize);
			quad[2].texCoords = sf::Vector2f((tu + 1) * _tilesize, (tv + 1) * _tilesize);
			quad[3].texCoords = sf::Vector2f(tu * _tilesize, (tv + 1) * _tilesize);
		}
	}

	update_maps();
}




void Tilemap::update_visible_vertices(sf::Vector2u offset)
{
	int width = _size.x;
	int height = _size.y;

	int win_width = _p_win->getView().getSize().x;
	int win_height = _p_win->getView().getSize().y;
	
	// number of visible tiles in each direction 
	// Add one extra to avoid rendering glitches
	//
	int tiles_x = win_width / _tilesize + 1;
	int tiles_y = win_height / _tilesize + 1;

	// The rendering offset (can't be negative)
	//
	int off_x = offset.x;
	int off_y = offset.y;


	// This block could be transfered to a seperate method
	// since this only needs to be done once the resolution 
	// or the number of layers changes
	// (Better just leave it here though)
	//
	
	_visible_vertices.resize(_num_layers);
	for (int y = 0; y < tiles_y; y++)
		for (int x = 0; x < tiles_x; x++)
			for (int l = 0; l < _num_layers; l++)
			{
				_visible_vertices[l].setPrimitiveType(sf::Quads);
				_visible_vertices[l].resize(tiles_x * tiles_y * 4);
			}
	

	// Copy all visible render_vertices from all layers into the visible_vertices
	//
	for (int l = 0; l < _num_layers; l++)
	{
		for (int y = 0; y < tiles_y; y++)
		{
			for (int x = 0; x < tiles_x; x++)
			{
				// Calculate indices of starting vertices in both vectors
				//
				int idx_render = ((off_y + y) * width + off_x + x) * 4;
				int idx_visible = (y * tiles_x + x) * 4;

				// Pick starting vertices
				sf::Vertex* render_vertex = &_render_vertices[l][idx_render];
				sf::Vertex* visible_vertex = &_visible_vertices[l][idx_visible];

				// Copy the next 4 vertices
				// and apply offset
				//
				for (int i = 0; i < 4; i++)
				{
					visible_vertex[i] = render_vertex[i];
					visible_vertex[i].position -= sf::Vector2f(offset) * (float)_tilesize;
				}
			}
			
		}
	}

}


void Tilemap::set_render_offset(sf::Vector2u offset)
{
	_render_offset = offset;
	update_visible_vertices(_render_offset);
}


void Tilemap::set_render_offset(unsigned int x, unsigned int y)
{
	set_render_offset(sf::Vector2u(x, y));
}


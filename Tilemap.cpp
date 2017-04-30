#include "Tilemap.hpp"

#define SQRT2 1.414f

// THIS IS JUST A TEMPORARY WORKAROUND
float __heuristic = 0.0f;

Tilemap::Tilemap(sf::RenderWindow* win, sf::Vector2u map_size, int tilesize, sf::String image_path, int num_layers = 1)
	: _p_win(win), _num_layers(num_layers), _size(map_size), _tilesize(tilesize)
{
	int win_width = _p_win->getView().getSize().x;
	int win_height = _p_win->getView().getSize().y;

	// number of visible tiles in each direction 
	// Add one extra to avoid rendering glitches
	//
	int tiles_x = win_width / _tilesize + 1;
	int tiles_y = win_height / _tilesize + 1;


	// There can't be more visible tiles than tiles in existence
	//
	if (tiles_x > _size.x)
		tiles_x = _size.x;

	if (tiles_y > _size.y)
		tiles_y = _size.y;

	_visible_size = sf::Vector2u(tiles_x, tiles_y);


	// Texture failed to load
	//
	if (!_texture.loadFromFile(image_path))
	{
		throw std::exception("[FATAL ERROR]\nTilemap: Failed to load texture.");
	}

	// Texture size and tile size don't match 
	//
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

	_map.shrink_to_fit();

	// Update the obstacle map
	_obstmap = generate_obstacle_map();





	/*
	 **************************************************************
	 | Initialize and setup the rendering vertices of the tilemap |
	 **************************************************************
	*/

	_visible_vertices.resize(_num_layers);
	_visible_vertices[0].setPrimitiveType(sf::Quads);
	_visible_vertices[0].resize(tiles_x * tiles_y * 4);

	for (int y = 0; y < tiles_y; y++)
	{
		for (int x = 0; x < tiles_x; x++)
		{
			// Get the tile image id from the texture map on layer 0
			int tile_img_id = _map[y * tiles_x + x][0]->get_image_id();

			// Calculate the texture coordinates for that tile
			int tu = tile_img_id % (_texture.getSize().x / _tilesize);
			int tv = tile_img_id / (_texture.getSize().y / _tilesize);

			sf::Vertex* quad = &_visible_vertices[0][(y * tiles_x + x) * 4];

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


// SFML sf::Drawable override
//
void Tilemap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	states.texture = &_texture;

	for (int l = 0; l < _num_layers; l++)
		target.draw(_visible_vertices[l], states);
}



// Updates all maps at specified point
//
void Tilemap::update_maps(sf::Vector2u at)
{
	update_maps(at.x, at.y);
}


// Updates all maps at specified point
//
void Tilemap::update_maps(unsigned int at_x, unsigned int at_y)
{
	int width = _size.x;
	int height = _size.y;

	std::vector<std::shared_ptr<Tile>> tiles;
	tiles.resize(_num_layers);

	for (int l = 0; l < _num_layers; l++)
		tiles[l] = _map[at_y * width + at_x][l];
	
	update_obstacle_map(at_x, at_y, tiles);
	update_visible_vertices(_render_offset);
}



// Updates all obstacle maü at specified point
//
void Tilemap::update_obstacle_map(unsigned int at_x, unsigned int at_y, std::vector<std::shared_ptr<Tile>> tiles)
{
	int width = _size.x;
	int height = _size.y;

	bool obst = false;
	for (int l = 0; l < _num_layers; l++)
	{
		if (tiles[l]->is_obstacle())
		{
			obst = true;
			break;
		}
	}

	_obstmap[at_y * width + at_x] = obst;
}

// Method overload from above
//
void Tilemap::update_obstacle_map(sf::Vector2u pos, std::vector<std::shared_ptr<Tile>> tile)
{
	update_obstacle_map(pos.x, pos.y, tile);
}


// Add a new layer to the map and fill it with transparent tiles
// Update the whole map afterwars
//
void Tilemap::add_layer()
{
	int width = _size.x;
	int height = _size.y;

	_num_layers++;

	_visible_vertices.resize(_num_layers);

	_visible_vertices[_num_layers - 1] = sf::VertexArray();
	_visible_vertices[_num_layers - 1].setPrimitiveType(sf::Quads);
	_visible_vertices[_num_layers - 1].resize(_visible_size.x * _visible_size.y * 4);



	// Fill the new layer with transparent tiles first
	//
	for (int ly = 0; ly < height; ly++)
	{
		for (int lx = 0; lx < width; lx++)
		{
			// Let the main map know about the new layer
			//
			_map[ly * width + lx].resize(_num_layers, std::make_shared<Tile>(Tile(TS_TRANSPARENT, false)));
		}
	}

	update_maps();
}


/*
* This method adds a new tile into the tilemap.
* A new layer will be added as long as the field specified is not transparent.
* Use set_tile to change a tile on a layer specified.
*/
void Tilemap::add_tile(sf::Vector2u pos, Tile tile)
{
	int x = pos.x;
	int y = pos.y;

	int w_visible = _visible_size.x;
	int h_visible = _visible_size.y;

	int width = _size.x;
	int height = _size.y;

	// If the tile on the position specified is not transparent, add a new layer
	//
	if (_map[y * width + x][_num_layers - 1]->get_image_id() != TS_TRANSPARENT)
		add_layer();

	// Insert the new tile into the tilemap
	_map[y * width + x][_num_layers - 1] = std::make_shared<Tile>(tile);
	update_maps(x, y);
}


/*
 * Instead of adding a tile (and potentially a new layer) this method
 * replaces the specified tile (position, layer) with a new one.
 */
void Tilemap::set_tile(sf::Vector2u pos, Tile tile, int layer)
{
	int x = pos.x;
	int y = pos.y;

	int width = _size.x;
	int height = _size.y;

	// Check bounds
	//
	if (x < 0 || y < 0 || x >= width || y >= width)
		return;

	// Check layer
	//
	if (layer >= _num_layers || layer < 0)
	{
		std::stringstream str;
		str << "[ERROR] The layer specified does not exist [layer = " << layer << "]";
		throw std::exception(str.str().c_str());
	}

	// Replace current tile on position with new tiles
	_map[y * width + x][layer] = std::make_shared<Tile>(tile);

	// Update all maps at that location
	update_maps(x, y);
}




void Tilemap::update_maps()
{
	//_texmap = generate_texture_map();
	_obstmap = generate_obstacle_map();

	//update_map_vertices();
	update_visible_vertices(_render_offset);
}


Tilemap::~Tilemap()
{
	// Originally needed for deleting the tile pointers
	// However, the use of smart pointers make it obsolete
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

	obstmap.shrink_to_fit();
	return obstmap;
}



void Tilemap::update_visible_vertices(sf::Vector2u offset)
{
	int width = _size.x;
	int height = _size.y;
	
	// number of visible tiles in each direction 
	// Add one extra to avoid rendering glitches
	//
	int tiles_x = _visible_size.x;
	int tiles_y = _visible_size.y;

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
	

	// Fill _visible_vertices with new values
	// This part 
	for (int l = 0; l < _num_layers; l++)
	{
		for (int y = 0; y < tiles_y; y++)
		{
			for (int x = 0; x < tiles_x; x++)
			{
				// Get the tile image id from the texture map on layer
				int map_idx = (y + off_y) * width + x + off_x;
				int tile_img_id = _map[map_idx][l]->get_image_id();

				// Calculate the texture coordinates for that tile
				int tu = tile_img_id % (_texture.getSize().x / _tilesize);
				int tv = tile_img_id / (_texture.getSize().y / _tilesize);

				sf::Vertex* quad = &_visible_vertices[l][(y * tiles_x + x) * 4];

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

	_visible_vertices.shrink_to_fit();
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


sf::Vector2u Tilemap::get_map_size()
{
	return _size;
}

/*
 * Converts the global coordinates to the map coordinates
 * IMPORTANT:
 * This only takes scale into account. Other transformations will be ignored
 * Has to be implemented if needed
 */
sf::Vector2u Tilemap::screen_to_map_coords(sf::Vector2f pos)
{
	float x = pos.x;
	float y = pos.y;

	int x_off = _render_offset.x;
	int y_off = _render_offset.y;

	return sf::Vector2u(((int)(x / _tilesize) + x_off) / getScale().x, ((int)(y / _tilesize) + y_off) / getScale().y);
}


/*
 * Converts the global coordinates to the map coordinates
 * IMPORTANT:
 * This only takes scale into account. Other transformations will be ignored
 * Has to be implemented if needed
 */
sf::Vector2u Tilemap::screen_to_map_coords(sf::Vector2i pos)
{
	int x = pos.x;
	int y = pos.y;

	int x_off = _render_offset.x;
	int y_off = _render_offset.y;

	return sf::Vector2u((x / _tilesize + x_off) / getScale().x, (y / _tilesize + y_off) / getScale().y);
}



/*
 * Utilizes the A* Pathfinding algorithm to find a path between two map nodes
 * This is still the first implementation and it might not work in some edge cases
 * More testing needs to be done
 */
std::vector<sf::Vector2u> Tilemap::get_path(sf::Vector2u _start, sf::Vector2u _goal, bool diagonal)
{
	// THIS IS JUST A TEMPORARY WORKAROUND
	__heuristic = 0.0f;

	Pathnode start(_start);
	Pathnode goal(_goal);

	std::vector<Pathnode> closed_set = get_all_obstacles();
	std::vector<Pathnode> open_set;
	open_set.push_back(start);

	std::map<Pathnode, Pathnode> came_from;
	
	std::map<Pathnode, bool> open_check;
	std::map<Pathnode, bool> closed_check;

	// Nope
	/*
	for (int y = 0; y < _size.y; y++)
		for (int x = 0; x < _size.x; x++)
		{
			open_check[Pathnode(x, y)] = false;
			closed_check[Pathnode(x, y)] = false;
		}
	*/

	open_check[start] = true;
	closed_check[start] = false;

	while (!open_set.empty())
	{
		Pathnode current = find_lowest_score_node(open_set);

		// A path has been found, stop algorithm and return path
		//
		if (current == goal)
			return reconstruct_path(came_from, current);

		// Remove the current element from the open set
		open_set.erase(std::find(open_set.begin(), open_set.end(), current));
		open_check[current] = false;

		// Set nodes status to closed
		current.closed = true;
		current.open = false;

		// and assign it to the closed set
		closed_set.push_back(current);
		closed_check[current] = true;

		// Loop through all neighbors
		//
		std::vector<Pathnode> current_neighbors = get_neighbors(current, diagonal);	
		std::vector<Pathnode>::iterator it_nb = current_neighbors.begin();
		for (it_nb; it_nb != current_neighbors.end(); it_nb++)
		{
			// If the current neighbor belongs to the closed set,
			// continue with the next element
			//if (std::find(closed_set.begin(), closed_set.end(), *it_nb) != closed_set.end()) <-- GARBAGE INEFFICIENT CODE
			if (closed_check[*it_nb])
				continue;

			// Since all nodes on the map are equidistantly spaced, I only need to check if their x and y values differ
			// If both x and y differ, they are diagonally neighoring 
			// If only x or only y differ, they are neighboring in a straight line
			// It's faster than pythagoras
			float tent_g_score = current.g_score_current + (it_nb->x != current.x && it_nb->y != current.y) ? 1.414 : 1;  //(float)sqrt(pow<float>((it_nb->x - current.x), 2) + pow<float>((it_nb->y - current.y), 2));

			// If the current neighbor is not in the open_set yet
			//if (std::find(open_set.begin(), open_set.end(), *it_nb) == open_set.end()) <-- GARBAGE INEFFICIENT CODE
			if (!open_check[*it_nb])
			{
				it_nb->open = true;
				open_set.push_back(*it_nb);	
				open_check[*it_nb] = true;
			}
			// Is it in the open set?
			else if (tent_g_score > it_nb->g_score_current)
				// yes, but not a better path, continue
				continue;

			it_nb->g_score_current = tent_g_score;
			it_nb->f_score_current = tent_g_score + steven_van_dijk_heuristic(start, goal, current);
			came_from[*it_nb] = current;
			
		}
	}

	// No path found, return empty vector
	return std::vector<sf::Vector2u>();
}

std::vector<sf::Vector2u> Tilemap::reconstruct_path(std::map<Pathnode, Pathnode> cf, Pathnode current)
{
	std::vector<sf::Vector2u> path;
	//path.reserve(cf.size());

	std::vector<Pathnode> keys;
	std::map<Pathnode, Pathnode>::iterator it = cf.begin();
	for (it; it != cf.end(); it++)
		keys.push_back(it->first);
	
	path.push_back(current);
	while (std::find(keys.begin(), keys.end(), current) != keys.end())
	{
		current = cf[current];
		path.push_back(current);
	}

	path.shrink_to_fit();
	return path;
	
}

bool Tilemap::is_obstacle_at(Pathnode pos)
{
	int x = pos.x;
	int y = pos.y;

	int width = _size.x;

	return _obstmap[y * width + x];
}


std::vector<Pathnode> Tilemap::get_all_obstacles()
{
	std::vector<Pathnode> retval;

	int width = _size.x;
	int height = _size.y;

	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			if (_obstmap[y * width + x])
				retval.push_back(Pathnode(x, y, true));

	return retval;
}



/*
 * This method finds the lowest f score node within the node list specified
 */
Pathnode find_lowest_score_node(std::vector<Pathnode> nodes, std::map<Pathnode, float> scores)
{
	std::pair<Pathnode, float> lowest(Pathnode(0, 0), 1000000);

	
	// Loop through all map pairs
	//
	std::map<Pathnode, float>::iterator it = scores.begin();
	for (it; it != scores.end(); it++)
	{
		std::pair<Pathnode, float> p = *it;

		// If the value is lower than the current lowest value
		// and is contained within the passed nodes vector
		//
		if (p.second < lowest.second && std::find(nodes.begin(), nodes.end(), p.first) != nodes.end())
			// Make it the new lowest value
			lowest = p;
	}

	return lowest.first;
}

/*
 * Finds the the node with the lowest f score in the vector
 */
Pathnode Tilemap::find_lowest_score_node(std::vector<Pathnode> nodes)
{
	std::vector<Pathnode>::iterator lowest = nodes.begin();
	std::vector<Pathnode>::iterator it = nodes.begin() + 1;
	for (it; it != nodes.end(); it++)
	{
		if (lowest->f_score_current > it->f_score_current)
			lowest = it;
	}

	return *lowest;
}

/*
 * Returns at least the 4 directly neighboring tiles
 * If specified, the 4 diagonal neighbors will be returned well
 * 
 */
std::vector<Pathnode> Tilemap::get_neighbors(Pathnode node, bool diagonal)
{
	int x = node.x;
	int y = node.y;

	int width = _size.x;
	int height = _size.y;

	std::vector<Pathnode> neighbors;


	// Add the next 4 neighbouring tiles
	//

	if (x > 0)
	{
		if (!_obstmap[y * width + x - 1])
			neighbors.push_back(Pathnode(x - 1, y));
	}
	
	if (x + 1 < width)
	{
		if (!_obstmap[y * width + x + 1])
			neighbors.push_back(Pathnode(x + 1, y));
	}
	
	if (y > 0)
	{
		if (!_obstmap[(y - 1) * width + x])
			neighbors.push_back(Pathnode(x, y - 1));
	}

	if (y + 1 < height)
	{
		if (!_obstmap[(y + 1) * width + x])
			neighbors.push_back(Pathnode(x, y + 1));
	}


	// If diagonal tiles as well 
	// add 4 more
	//

	if (diagonal)
	{
		if (x > 0 && y > 0)
		{
			if (!_obstmap[(y - 1) * width + x - 1])
				neighbors.push_back(Pathnode(x - 1, y));
		}

		if (x > 0 && y + 1 < height)
		{
			if (!_obstmap[(y + 1) * width + x - 1])
				neighbors.push_back(Pathnode(x - 1, y + 1));
		}

		if (x + 1 < width && y > 0)
		{
			if (!_obstmap[(y - 1) * width + x + 1])
				neighbors.push_back(Pathnode(x + 1, y - 1));
		}

		if (x + 1 < width && y + 1 < height)
		{
			if (!_obstmap[(y + 1) * width + x + 1])
				neighbors.push_back(Pathnode(x + 1, y + 1));
		}
	}
	
	return neighbors;
}


float Tilemap::steven_van_dijk_heuristic(Pathnode& start, Pathnode& goal, Pathnode& current)
{
	const float C = 0.001f;

	float dx1 = (float)current.x - (float)goal.x;
	float dx2 = (float)start.x - (float)goal.x;

	float dy1 = (float)current.y - (float)goal.y;
	float dy2 = (float)start.y - (float)goal.y;

	float cross = abs(dx1 * dy2 - dy1 * dx2);

	__heuristic += C * cross;

	return __heuristic;
}




int Tilemap::get_tile_size(bool with_transform)
{
	return with_transform ? _tilesize * getScale().x : _tilesize;
}

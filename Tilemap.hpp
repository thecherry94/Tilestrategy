#pragma once

#include "Tile.hpp"

#include <iostream>

#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <vector>
#include <memory>
#include <sstream>

typedef std::vector<std::vector<std::shared_ptr<Tile>>> TilemapContainer;
typedef std::vector<bool>								ObstacleMap;
typedef std::vector<sf::VertexArray>					RenderVertices;

typedef std::vector<sf::Vector2u>						Path;

class Pathnode : public sf::Vector2u
{
	public:

		Pathnode() : sf::Vector2u() {}

		Pathnode(sf::Vector2u v, bool c = false) : sf::Vector2u(v), 
			closed(c), open(c), 
			g_score_cost(1), g_score_current(0) { }

		Pathnode(unsigned int x, unsigned int y, bool c = false) : sf::Vector2u(x, y), 
			closed(c), open(c), 
			g_score_cost(1), g_score_current(0) {}

		// Overloading < operator so this class can be used in 
		// std::map as key
		//
		bool operator <(const Pathnode& other)
		{
			if (this->x == other.x)
				return this->y < other.y;

			return this->x < other.x;
		}

		/* possibly not needed anymore */
		bool closed;
		bool open;
		/*******************************/

		float g_score_cost;
		float g_score_current;

		float f_score_current;
};

// Overloading < operator for Pathnode to Pathnode comparison
// so a Pathnode can be used as std::map as key
//
inline bool operator<(const Pathnode& lhs, const Pathnode& rhs)
{
	if (lhs.x == rhs.x)
		return lhs.y < rhs.y;

	return lhs.x < rhs.x;
}


/*
 * Tilemap class
 * This class holds all the information necessary to draw and navigate through the map.
 * It inherits from sf::Drawable and sf::Transformable to ensure (rendering) compatibility with the SFML framework.
 * 
 * The main information is stored in the TilemapContainer _map, 
 * which is a vector of a vectors of shared pointers of the type Tile.
 * -> The first vector holds the information of the position of a tile on the map.
 * -> The second vector is responsible for the layer ordering.
 * -> The shared pointer makes life easier by not worrying about memory leaks too much 
 *	  (Still not sure if it should be replaced by a unique_ptr or something different)
 *
 * This class also contains sub maps that in turn just contain bits of information of the main _map.
 * -> The ObstacleMap only contains obstacle information.
 * -> The VisibileVertices only contain information necessary about ingame position and texture position for the visible tiles.
 *
 * 
 * Things that have to be changed/bugs:
 * 1. [99% SURE DONE] 
 *	  The layer system is still not fully functional.
 *	  Adding new layers is not implemented.
 *
 * 2. If the offset is too large (so that the outside of the map would be visible) 
 *	  the array index will go out of bounds and causes a crash.
 *	  
 *
 * Possible improvements:
 * 1. [MAYBE DONE] (Still needs more testing)
	  Instead of having to update all submaps and all of their fields when a single change is made,
 *	  only do updates where it is really necessary [Saves computation time quadratically with map size]
 *
 * 2. The add_tile method checks for transparent tiles whether it should add a new layer or not.
 *	  I think there is a better method to do this.
 *
 * 3. [DONE]
 *	  At the moment, the rendering method loops through the whole map. Parts that are not visible should be ignored.
 *	  A possibility would be to add an additional vertex array with only enough space to fit in whats inside the screen.
 *
 * 4. The tilemap takes up too much RAM. There is some redundant data stored inside the vectors.
 *	  I should get rid of those.
 *	  [UPDATE] Reduced RAM allocation by 2/3. Maybe I can reduce it even more?
 *			   If memory allocation is again posing to be a problem 
 *			   I should consider replacing the smart pointers with plain old raw ones.
 *			   This also means I have to implement additional logic for deallocation.
 *			   Maybe it's just not worth it. We'll see.
 *	  [DONE] One way would be to completely remove the member _render_vertices. It contains ALL render information.
 *			 But why store render information of something that is not visible?
 * 
 * 5. Adding a new layer is super performance intensive. I should see to improve it.
 *
 * 7. The patfinding algorithm should be more generalized since I want to graph the map into submaps
 *	  and always walk from submap to submap for larger paths. This won't be as performance hungry and
 *	  might look more continous to the player.
 *
 *	  [DONE] This worked better than expected
 *	  Checking if nodes are in an open or closed set is worst case O(N)
 *	  Maybe I can introduce an additional map that holds these informations separately.
 *	  Accessing this map would be worst case O(ln N)
 *
 *	  Maybe create an array of all possible Pathnodes and just point to them for the rest of the algo
 *
 *    Maybe spawn a separate, detached thread for the calculations?
 * 
 * TL;DR:
 * This code sucks ass but it works.
 * >>> I should refactor soon before it grows too big.
 */
class Tilemap : public sf::Drawable, public sf::Transformable
{
	

	private:
		TilemapContainer _map;
		ObstacleMap _obstmap;
		sf::Texture _texture;
		RenderVertices _visible_vertices;
		sf::RenderWindow* _p_win;


		int _tilesize;
		sf::Vector2u _size;
		sf::Vector2u _visible_size;
		int _num_layers;

		void update_maps();
		void update_maps(sf::Vector2u at);
		void update_maps(unsigned int, unsigned int);
		
		void add_layer();
		void update_visible_vertices(sf::Vector2u);

		std::vector<Pathnode> get_neighbors(Pathnode node, bool diagonal = false);
		Pathnode find_lowest_score_node(std::vector<Pathnode> nodes);
		std::vector<sf::Vector2u> reconstruct_path(std::map<Pathnode, Pathnode> cf, Pathnode current);
		float steven_van_dijk_heuristic(Pathnode& start, Pathnode& goal, Pathnode& current);

		sf::Vector2u _render_offset;

		


	public:
		Tilemap(sf::RenderWindow*, sf::Vector2u, int, sf::String, int);
		void draw(sf::RenderTarget & target, sf::RenderStates states) const;
		~Tilemap();
		
		void add_tile(sf::Vector2u, Tile);
		void set_tile(sf::Vector2u, Tile, int);

		ObstacleMap generate_obstacle_map();

		void update_obstacle_map(unsigned int at_x, unsigned int at_y, std::vector<std::shared_ptr<Tile>> tile);
		void update_obstacle_map(sf::Vector2u, std::vector<std::shared_ptr<Tile>> tile);
		
		void set_render_offset(sf::Vector2u offset);
		void set_render_offset(unsigned int, unsigned int);

		sf::Vector2u screen_to_map_coords(sf::Vector2f pos);
		sf::Vector2u screen_to_map_coords(sf::Vector2i pos);

		sf::Vector2u get_map_size();

		bool is_obstacle_at(Pathnode pos);
		std::vector<Pathnode> get_all_obstacles();

		std::vector<sf::Vector2u> get_path(sf::Vector2u start, sf::Vector2u goal, bool diagonal = false);


		int get_tile_size(bool with_transform = false);
};



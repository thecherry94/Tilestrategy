#pragma once

#include "Tile.hpp"

#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <vector>
#include <memory>
#include <sstream>

typedef std::vector<std::vector<std::shared_ptr<Tile>>> TilemapContainer;
typedef std::vector<std::vector<int>>					TextureMap;
typedef std::vector<bool>								ObstacleMap;
typedef std::vector<sf::VertexArray>					RenderVertices;



/*
 * Tilemap class
 * This class holds all the information necessary to draw and navigate through the map.
 * 
 * The main information is stored in the TilemapContainer _map, 
 * which is a vector of a vector of shared pointer of the type Tile.
 * -> The first vector holds the information of the position of a tile on the map.
 * -> The second vector is responsible for the layer ordering.
 * -> The shared pointer makes life easier by not worrying about memory leaks too much 
 *	  (Still not sure if it should be replaced by a unique_ptr or something different)
 *
 * This class also contains sub maps that in turn just contain bits of information of the main _map.
 * The TexturMap only contains the image IDs .
 * The ObstacleMap only contains obstacle information.
 * The RenderVertices only contain information necessary about ingame position and texture position for the rendering process.
 *
 * The advantage here is that these objects will be accessed multiple times per iteration and it's easier to tell these things apart.
 * It is also (maybe) faster.
 *
 * The disadvante is that everytime there is a change in the TilemapContainer, every submap has to be updated for things to work correctly.
 * This can take a long time depending on map size.
 * However, I don't know how to do it better since I don't know much about SFML.
 * The most important thing is it works for now.
 * 
 * Things that have to implemented:
 * 1. The layer system is still not fully functional.
 *	  Adding new layers is not implemented.
 *
 * Possible improvements:
 * 1. Instead of having to update all submaps and all of their fields when a single change is made,
 *	  only do updates where it is really necessary [Saves computation time]
 *
 * 2. The add_tile method checks for transparent tiles whether it should add a new layer or not.
 *	  I think there is a better method to do this.
 *
 *
 * TL;DR:
 * This code sucks ass but it works
 */
class Tilemap : public sf::Drawable, public sf::Transformable
{
	

	private:
		TilemapContainer _map;
		TextureMap _texmap;
		ObstacleMap _obstmap;
		sf::Texture _texture;
		RenderVertices _render_vertices;


		int _tilesize;
		sf::Vector2<int> _size;
		int _num_layers;

		void update_maps();

		


	public:
		Tilemap(sf::Vector2<int>, int, sf::String, int);
		void draw(sf::RenderTarget & target, sf::RenderStates states) const;
		~Tilemap();

		void add_tile(sf::Vector2<int>, Tile);
		void set_tile(sf::Vector2<int>, Tile, int);

		TextureMap get_texture_map();
		ObstacleMap get_obstacle_map();
		void update_map_vertices();


};
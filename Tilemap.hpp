#pragma once

#include "Tile.hpp"

#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <vector>
#include <memory>

typedef std::vector<std::vector<std::unique_ptr<Tile>>> TilemapContainer;
typedef std::vector<std::vector<int>>					TextureMap;
typedef std::vector<bool>								ObstacleMap;

class Tilemap : public sf::Drawable, sf::Transformable
{
	

	private:
		TilemapContainer _map;
		TextureMap _texmap;
		ObstacleMap _obstmap;
		sf::Texture _texture;
		sf::VertexArray _map_vertices;


		int _tilesize;
		sf::Vector2<int> _size;
		int _num_layers;

		


	public:
		Tilemap(sf::Vector2<int>, sf::String, int);
		virtual void draw(sf::RenderTarget & target, sf::RenderStates states);
		~Tilemap();

		TextureMap get_texture_map();
		ObstacleMap get_obstacle_map();
		


};
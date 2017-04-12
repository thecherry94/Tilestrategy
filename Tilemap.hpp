#pragma once

#include "Tile.hpp"

#include <SFML\System.hpp>
#include <vector>

class Tilemap
{
	private:
		std::vector<std::vector<std::vector<Tile*>>> _map;

		int _tilesize;
		sf::Vector2<int> _size;


	public:
		Tilemap(sf::Vector2<int>, sf::String);

		


};
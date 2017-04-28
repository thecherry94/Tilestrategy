#pragma once

#include <SFML\Graphics.hpp>
#include <SFML\System.hpp>

#include "Tilemap.hpp"

class MapWalker : public sf::Drawable, public sf::Transformable
{
	private:
		Tilemap* _p_map;
		sf::Texture _texture;
		sf::Sprite _sprite;


	public:
		MapWalker(Tilemap* p_map, sf::Vector2u pos, sf::String img_path);

		void draw(sf::RenderTarget & target, sf::RenderStates states) const;
};
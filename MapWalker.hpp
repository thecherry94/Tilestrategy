#pragma once

#include <SFML\Graphics.hpp>
#include <SFML\System.hpp>

#include "Tilemap.hpp"

enum PF_State
{
	NONE,
	CALCULATING,
	WALKING
};

class MapWalker : public sf::Drawable, public sf::Transformable
{
	private:
		Tilemap* _p_map;
		sf::Texture _texture;
		sf::Sprite _sprite;

		float _walk_speed;
		int _walk_idx;
		Path _current_path;
		PF_State _pfstate;

	public:
		MapWalker(Tilemap* p_map, sf::Vector2u pos, sf::String img_path);

		void draw(sf::RenderTarget & target, sf::RenderStates states) const;
		
		void update(float dt);

		void move_to(sf::Vector2u pos);
};
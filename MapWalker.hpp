#pragma once

#include <SFML\Graphics.hpp>
#include <SFML\System.hpp>

#include "Tilemap.hpp"

#include <thread>
#include <future>
#include <chrono>

enum PF_State
{
	NONE,
	CALCULATING,
	WALKING
};


/*
 * Mapwalker - Can walk on a map to the specified location
 * This class utilizies the stl for running parallel async processes
 * in order to maximize the performance on a multicore system
 * incase there have to run multiple algorithms at once
 *
 */
class MapWalker : public sf::Drawable, public sf::Transformable
{
	private:
		Tilemap* _p_map;
		sf::Texture _texture;
		sf::Sprite _sprite;
		sf::Clock _clk;

		std::thread _thr_calcpath;
		std::shared_ptr<std::future<Path>> _future_path;

		float _walk_speed;
		int _walk_idx;
		Path _current_path;
		PF_State _pfstate;

	public:
		MapWalker();
		MapWalker(Tilemap* p_map, sf::Vector2u pos, sf::Sprite& sprite, float speed);

		void draw(sf::RenderTarget & target, sf::RenderStates states) const;
		
		void update(float dt);

		void move_to(sf::Vector2u pos, bool diagonal = false);
};
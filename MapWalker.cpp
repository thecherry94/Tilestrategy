#include "MapWalker.hpp"

MapWalker::MapWalker()
{

}

MapWalker::MapWalker(Tilemap* p_map, sf::Vector2u pos, sf::Sprite& sprite, float speed)
	: _p_map(p_map), _pfstate(NONE), _walk_idx(0), _walk_speed(speed)
{
	setPosition((sf::Vector2f)pos);
	setScale(_p_map->getScale());

	_sprite = sprite;
}

void MapWalker::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	target.draw(_sprite, states);
}

/*
 * Calculates a path asynchronously and makes the walker move 
 * as soon as path is available
 * Walker must be idle to be executed
 */
void MapWalker::move_to(sf::Vector2u pos, bool diagonal)
{
	// Only do something new if idle
	if (_pfstate != NONE)
		return;

	

	// Transform from world pos to map pos
	sf::Vector2u map_pos = _p_map->screen_to_map_coords(getPosition());

	// Create a functor
	auto f = [](Tilemap* m, sf::Vector2u mp, sf::Vector2u p, bool dgl)
	{
		return m->get_path(mp, p, dgl);
	};

	// Run the async thread and find a path
	_future_path = std::make_shared<std::future<Path>>(std::async(std::launch::async, f, _p_map, map_pos, pos, diagonal));

	// Set state to calculating
	_pfstate = CALCULATING;
}


void MapWalker::update(float dt)
{
	// Nothing to do
	if (_pfstate == NONE)
		return;

	// Execute this while the pathfinding is in progress
	if (_pfstate == CALCULATING)
	{
		// Check if it's done
		auto status = _future_path->wait_for(std::chrono::microseconds(10));

		// If async call done
		if (status == std::future_status::ready)
		{
			// Get the calculated path
			_current_path = _future_path->get();
			_future_path.reset();

			// No path could be found
			if (_current_path.size() < 1)
			{
				_pfstate = NONE;
				return;
			}

			// The path is in backward order
			_walk_idx = _current_path.size();
			_pfstate = WALKING;
			_clk.restart();
		}
	}

	// A path is available and it's walking
	if (_pfstate == WALKING)
	{
		// Did walker already wait long enough?
		if (_clk.getElapsedTime().asMilliseconds() <  1000 / _walk_speed)
			return;

		// Restart the timer clock and reduce the position index
		_clk.restart();
		_walk_idx--;

		// Did walker reach the end?
		if (_walk_idx < 0)
		{
			// Add event for when action is done
			_current_path.clear();

			_pfstate = NONE;
			return;
		}

		// Set it's position on the map
		//
		int tilesize = _p_map->get_tile_size(true);
		sf::Vector2f p_path = (sf::Vector2f)_current_path[_walk_idx];
		p_path.x *= tilesize;
		p_path.y *= tilesize;

		setPosition(p_path);
	}
}
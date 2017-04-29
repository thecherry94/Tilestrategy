#include "MapWalker.hpp"


MapWalker::MapWalker(Tilemap* p_map, sf::Vector2u pos, sf::String img_path)
{
	_p_map = p_map;
	setPosition((sf::Vector2f)pos);

	if (!_texture.loadFromFile(img_path))
	{
		throw std::exception("Failed to load walker image");
	}

	_sprite.setTexture(_texture);

	_pfstate = NONE;
	_walk_idx = 0;
}

void MapWalker::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	target.draw(_sprite, states);
}


void MapWalker::move_to(sf::Vector2u pos)
{
	_pfstate = CALCULATING;

	// Transform from world pos to map pos
	sf::Vector2u map_pos = _p_map->screen_to_map_coords(getPosition());

	// Try to find a path
	_current_path = _p_map->get_path(map_pos, pos);

	// No path could be found
	if (_current_path.size() < 1)
	{
		_pfstate = NONE;
		return;
	}

	// The path is in backward order
	_walk_idx = _current_path.size() - 1;
	_pfstate = WALKING;
}


void MapWalker::update(float dt)
{
	
}
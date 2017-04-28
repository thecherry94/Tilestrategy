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
}

void MapWalker::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	target.draw(_sprite, states);
}
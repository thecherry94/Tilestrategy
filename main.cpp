#include <SFML\Graphics.hpp>
#include <SFML\System.hpp>

#include <iostream>

#include "Tilemap.hpp"

int main()
{
	sf::RenderWindow win(sf::VideoMode(800, 600), "Tilestrategy");

	Tilemap map(sf::Vector2<int>(500, 500), 16, "tiles.png", 1);
	
	map.set_tile(sf::Vector2<int>(0, 0), Tile(TS_CHEST, false), 0);
	
	
	while (win.isOpen())
	{
		sf::Event ev;

		while (win.pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed)
				win.close();
		}

		win.clear();
		win.draw(map);
		win.display();
	}

	return 0;
}
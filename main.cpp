#include <SFML\Graphics.hpp>
#include <SFML\System.hpp>

#include <iostream>

#include "Tilemap.hpp"

int main()
{
	sf::RenderWindow win(sf::VideoMode(800, 600), "Tilestrategy");

	Tilemap* map = new Tilemap(sf::Vector2<int>(1000, 1000), 16, "tiles.png", 1);
	
	map->add_tile(sf::Vector2<int>(0, 0), Tile(TS_CHEST, false));
	
	while (win.isOpen())
	{
		clock_t start = clock();
		sf::Event ev;

		while (win.pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed)
				win.close();
		}

		win.clear();
		win.draw(*map);
		win.display();
		clock_t end = clock();

		std::cout << (double)(end - start) / CLOCKS_PER_SEC << std::endl;
	}

	delete map;
	map = NULL;

	
	std::system("pause");

	return 0;
}
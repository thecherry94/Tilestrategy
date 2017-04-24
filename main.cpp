#include <SFML\Graphics.hpp>
#include <SFML\System.hpp>

#include <sstream>
#include <iostream>
#include <conio.h>

#include "Tilemap.hpp"

int main()
{
	sf::RenderWindow win(sf::VideoMode(800, 600), "Tilestrategy");

	Tilemap* map = new Tilemap(&win, sf::Vector2<int>(1000, 1000), 16, "tiles.png", 1);
	map->add_tile(sf::Vector2<int>(3, 3), Tile(TS_CHEST, false));
	map->set_render_offset(0, 0);

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
		
		std::stringstream s;
		s << "Elapsed time: " << 1000.0 * (double)(end - start) / CLOCKS_PER_SEC << " ms ";
		std::cout << s.str();
		for (int i = 0; i < s.str().size(); i++)
			std::cout << '\b';
	}

	delete map;
	map = NULL;

	std::system("pause");

	return 0;
}
#include <Windows.h>

#include <SFML\Graphics.hpp>
#include <SFML\System.hpp>

#include <sstream>
#include <iostream>
#include <conio.h>

#include "Tilemap.hpp"
#include "MapWalker.hpp"

#define SPEEDTEST

void gotoxy(int x, int y);

int main()
{
#ifdef SPEEDTEST
	clock_t start, end;
	double dt;
#endif

	sf::RenderWindow win(sf::VideoMode(1280, 960), "Tilestrategy");

	// Performance tests
	//

#ifdef SPEEDTEST
	start = clock();
#endif
	Tilemap* map = new Tilemap(&win, sf::Vector2u(200, 200), 16, "tiles.png", 1);
#ifdef SPEEDTEST
	end = clock();
	dt = 1000.0 * (double)(end - start) / CLOCKS_PER_SEC;
	std::cout << "Tilemap init (" << map->get_map_size().x << "x" << map->get_map_size().y << ")\t\t\t[" << dt << " ms]" << std::endl;
#endif
	
#ifdef SPEEDTEST
	start = clock();
#endif
	map->add_tile(sf::Vector2u(3, 3), Tile(TS_CHEST, false));
#ifdef SPEEDTEST
	end = clock();
	dt = 1000.0 * (double)(end - start) / CLOCKS_PER_SEC;
	std::cout << "Tilemap::add_tile (with adding new layer)\t[" << dt << " ms]" << std::endl;
#endif

#ifdef SPEEDTEST
	start = clock();
#endif
	map->add_tile(sf::Vector2u(4, 3), Tile(TS_CHEST, false));
#ifdef SPEEDTEST
	end = clock();
	dt = 1000.0 * (double)(end - start) / CLOCKS_PER_SEC;
	std::cout << "Tilemap::add_tile (without adding new layer)\t[" << dt << " ms]" << std::endl;
#endif

/*
#ifdef SPEEDTEST
	start = clock();
#endif
	map->set_render_offset(3, 3);
#ifdef SPEEDTEST
	end = clock();
	dt = 1000.0 * (double)(end - start) / CLOCKS_PER_SEC;
	std::cout << "Tilemap::set_render_offset\t\t\t[" << dt << " ms]" << std::endl;
#endif
*/


	map->setScale(2.0f, 2.0f);

	MapWalker walker(map, sf::Vector2u(0, 0), "player.png");
	walker.setScale(2.0f, 2.0f);

	while (win.isOpen())
	{
		clock_t start = clock();
		sf::Event ev;

		while (win.pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed)
				win.close();
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			map->set_tile(map->screen_to_map_coords(sf::Mouse::getPosition(win)), Tile(TS_WALL, true), 0);
		}
		else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
		{
			map->set_tile(map->screen_to_map_coords(sf::Mouse::getPosition(win)), Tile(TS_GRASS, false), 0);
		}

		win.clear();
		win.draw(*map);
		win.draw(walker);
		win.display();
		clock_t end = clock();
		
		/*
		double dt = (double)(end - start) / CLOCKS_PER_SEC;

		std::stringstream s;
		s << "Elapsed time: " << 1000.0 * dt << " ms "
			<< std::endl
			<< "FPS: " << 1.0 / dt << " ";
		std::cout << s.str();

		gotoxy(0, 0);
		*/
	}

	delete map;
	map = NULL;

	//std::system("pause");

	return 0;
}


void gotoxy(int x, int y) {
	COORD pos = { x, y };
	HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(output, pos);
}
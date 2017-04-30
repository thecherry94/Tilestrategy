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

inline sf::Vector2u operator*(sf::Vector2u& lhs, int rhs)
{
	return sf::Vector2u(lhs.x * rhs, lhs.y * rhs);
}

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
	Tilemap* map = new Tilemap(&win, sf::Vector2u(100, 100), 16, "tiles.png", 1);
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


	map->setScale(1.0f, 1.0f);


	sf::Texture walker_tex;
	walker_tex.loadFromFile("player.png");

	sf::Sprite walker_sprite;
	walker_sprite.setTexture(walker_tex);


	std::vector<MapWalker> walkers;
	int tilesize = map->get_tile_size(true);

	walkers.push_back(MapWalker(map, sf::Vector2u(5, 5) * tilesize, walker_sprite, 20));

	walkers.push_back(MapWalker(map, sf::Vector2u(4, 5) * tilesize, walker_sprite, 20));
	walkers.push_back(MapWalker(map, sf::Vector2u(6, 5) * tilesize, walker_sprite, 20));

	walkers.push_back(MapWalker(map, sf::Vector2u(5, 4) * tilesize, walker_sprite, 20));
	walkers.push_back(MapWalker(map, sf::Vector2u(5, 6) * tilesize, walker_sprite, 20));

	sf::Vector2u test_pos(map->get_map_size().x - 10, map->get_map_size().x - 10);

	// Include this to stress test
	/*
	walkers[0].move_to(sf::Vector2u(test_pos.x, test_pos.y), true);

	walkers[1].move_to(sf::Vector2u(test_pos.x - 1, test_pos.y), true);
	walkers[2].move_to(sf::Vector2u(test_pos.x + 1, test_pos.y), true);

	walkers[3].move_to(sf::Vector2u(test_pos.x, test_pos.y - 1), true);
	walkers[4].move_to(sf::Vector2u(test_pos.x, test_pos.y + 1), true);
	*/


	//std::vector<sf::Vector2u> path = map->get_path(sf::Vector2u(0, 0), sf::Vector2u(99, 99), true);
	int i = 0;
	
	sf::Clock clk;
	sf::Time delta_t;

	win.setFramerateLimit(144);

	while (win.isOpen())
	{
		sf::Event ev;
		sf::Vector2u mouse_pos_map = map->screen_to_map_coords(sf::Mouse::getPosition(win));


		while (win.pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed)
				win.close();

			if (ev.type == sf::Event::KeyPressed)
			{
				if (ev.key.code == sf::Keyboard::Space)
				{
					walkers[0].move_to(sf::Vector2u(mouse_pos_map.x, mouse_pos_map.y), true);

					walkers[1].move_to(sf::Vector2u(mouse_pos_map.x - 1, mouse_pos_map.y), true);
					walkers[2].move_to(sf::Vector2u(mouse_pos_map.x + 1, mouse_pos_map.y), true);

					walkers[3].move_to(sf::Vector2u(mouse_pos_map.x, mouse_pos_map.y - 1), true);
					walkers[4].move_to(sf::Vector2u(mouse_pos_map.x, mouse_pos_map.y + 1), true);
				}
			}
		}

		// Left Mousebutton -> change tile on mouse pos to impassible terrain
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			map->set_tile(mouse_pos_map, Tile(TS_WALL, true), 0);
		}
		// Right Mousebutton -> change tile on mouse pos to passable terrain
		else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
		{
			map->set_tile(map->screen_to_map_coords(sf::Mouse::getPosition(win)), Tile(TS_GRASS, false), 0);
		}

		

		std::vector<MapWalker>::iterator it_w = walkers.begin();
		for (it_w; it_w != walkers.end(); it_w++)
		{
			it_w->update(delta_t.asMilliseconds());
		}

		win.clear();
		win.draw(*map);

		it_w = walkers.begin();
		for (it_w; it_w != walkers.end(); it_w++)
		{
			win.draw(*it_w);
		}
		win.display();


		delta_t = clk.getElapsedTime();
		clk.restart();

		// Include this block to check program execution blockades
		/*
		i++;
		std::cout << i << std::endl;

		if (i > 500) i = 0;
		*/
		
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

	//std::system("pause")

	return 0;
}


void gotoxy(int x, int y) {
	COORD pos = { x, y };
	HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(output, pos);
}
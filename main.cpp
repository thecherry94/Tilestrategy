#include <Windows.h>

#include <SFML\Graphics.hpp>
#include <SFML\System.hpp>

#include <sstream>
#include <iostream>
#include <conio.h>

#include "Tilemap.hpp"

void gotoxy(int x, int y);

int main()
{
	sf::RenderWindow win(sf::VideoMode(1280, 960), "Tilestrategy");

	Tilemap* map = new Tilemap(&win, sf::Vector2u(100, 100), 16, "tiles.png", 1);
	map->add_tile(sf::Vector2u(3, 3), Tile(TS_CHEST, false));
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

	std::system("pause");

	return 0;
}


void gotoxy(int x, int y) {
	COORD pos = { x, y };
	HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(output, pos);
}
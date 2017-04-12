#include <SFML\Graphics.hpp>
#include <SFML\System.hpp>

int main()
{
	sf::RenderWindow win(sf::VideoMode(800, 600), "Tilestrategy");
	
	while (win.isOpen())
	{
		sf::Event ev;

		while (win.pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed)
				win.close();
		}

		win.clear();
		win.display();
	}

	return 0;
}
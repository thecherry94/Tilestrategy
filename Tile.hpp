#pragma once

#include <memory>

#define TS_GRASS 24
#define TS_TRANSPARENT 5
#define TS_CHEST 27
#define TS_WALL 7


/*
 * TODO:
 * Implement signals for events such as property changes (obstacle, img_id) etc.
 */

class Tile
{
	private:
		int _img_id;
		int _d;
		bool _is_obstacle;


	public:
		Tile(int, bool);
		
		static std::shared_ptr<Tile> create_empty();
		static std::shared_ptr<Tile> create_wall();

		// Setter
		void set_pos(int, int);
		void set_image_id(int);
		void set_obstacle(bool);

		// Getter
		int get_image_id();
		bool is_obstacle();



};
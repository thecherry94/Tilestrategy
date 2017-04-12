#pragma once


#define TS_GRASS 5


class Tile
{
	private:
		int _img_id;

		bool _is_obstacle;


	public:
		Tile(int, bool);
		
		static Tile* create_empty();

		// Setter
		void set_pos(int, int);
		void set_image_id(int);
		void set_obstacle(bool);

		// Getter
		int get_image_id();
		bool is_obstacle();



};
#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <allegro5/allegro.h>
#include "UI.h"

/**
 * @brief Main class that runs the whole game.
 * @details All game procedures must be processed through this class.
 */
class Game
{
public:
	void execute();
public:
	Game();
	~Game();
	void game_init();
	bool game_update();
	void game_draw();
private:
	/**
	 * @brief States of the game process in game_update.
	 * @see Game::game_update()
	 */
	enum class STATE {
		CHOOSELV,
		START, // -> LEVEL
		LEVEL, // -> PAUSE, END
		PAUSE, // -> LEVEL
		WIN,
		LOSE,
		END
	};
	STATE state;
	ALLEGRO_EVENT event;
	ALLEGRO_BITMAP *game_icon;
	ALLEGRO_BITMAP *background;
	ALLEGRO_BITMAP *level_background;
	ALLEGRO_BITMAP *level1_background;
	ALLEGRO_BITMAP *level2_background;
	ALLEGRO_BITMAP *level3_background;
	ALLEGRO_BITMAP *select_button;

private:
	ALLEGRO_DISPLAY *display;
	ALLEGRO_TIMER *timer;
	ALLEGRO_EVENT_QUEUE *event_queue;
	UI *ui;
};

#endif

#include "Game.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/OperationCenter.h"
#include "data/SoundCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include "Player.h"
#include "Level.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_acodec.h>
#include <vector>
#include <cstring>
#include "towers/Tower.h"

// fixed settings
constexpr char game_icon_img_path[] = "./assets/image/game_icon.png";
constexpr char game_start_sound_path[] = "./assets/sound/growl.wav";
constexpr char background_img_path[] = "./assets/image/StartBackground.jpg";
constexpr char level_background_img_path[] = "./assets/image/main_menu.png";
constexpr char level1_background_img_path[] = "./assets/image/Level1Background.jpg";
constexpr char level2_background_img_path[] = "./assets/image/Level2Background.jpg";
constexpr char level3_background_img_path[] = "./assets/image/Level3Background.jpg";
constexpr char background_sound_path[] = "./assets/sound/mainbgm.ogg";
constexpr char select_button_img_path[] = "./assets/image/select_button.png";
//constexpr char minimum_button_path[] = "./assets/image/Level2Background.jpg";
//constexpr char hard_button_path[] = "./assets/image/Level2Background.jpg";
static int current_bgm_index = 0;
std::vector<std::string> bgm_list = {
    "./assets/sound/mainbgm.ogg",
    "./assets/sound/battle_cats_bgm.ogg",
    "./assets/sound/BackgroundMusic.ogg"
};
/**
 * @brief Game entry.
 * @details The function processes all allegro events and update the event state to a generic data storage (i.e. DataCenter).
 * For timer event, the game_update and game_draw function will be called if and only if the current is timer.
 */


float pause_button_width = 190;
float pause_button_height = 60;
float pause_button_x = 605;
float pause_button_y = 530;

float start_button_width = 190;
float start_button_height = 60;
float start_button_x = 605;
float start_button_y = 530;

float continue_button_width = 190;
float continue_button_height = 60;
float continue_button_x = 605;
float continue_button_y = 530;

float mute_button_width = 190;
float mute_button_height = 60;
float mute_button_x = 605;
float mute_button_y = 450;

void
Game::execute() {
	DataCenter *DC = DataCenter::get_instance();
	// main game loop
	bool run = true;
	while(run) {
		// process all events here
		al_wait_for_event(event_queue, &event);
		switch(event.type) {
			case ALLEGRO_EVENT_TIMER: {
				run &= game_update();
				game_draw();
				break;
			} case ALLEGRO_EVENT_DISPLAY_CLOSE: { // stop game
				run = false;
				break;
			} case ALLEGRO_EVENT_KEY_DOWN: {
				DC->key_state[event.keyboard.keycode] = true;
				break;
			} case ALLEGRO_EVENT_KEY_UP: {
				DC->key_state[event.keyboard.keycode] = false;
				break;
			} case ALLEGRO_EVENT_MOUSE_AXES: {
				DC->mouse.x = event.mouse.x;
				DC->mouse.y = event.mouse.y;
				break;
			} case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: {
				DC->mouse_state[event.mouse.button] = true;
				break;
			} case ALLEGRO_EVENT_MOUSE_BUTTON_UP: {
				DC->mouse_state[event.mouse.button] = false;
				break;
			} default: break;
		}
	}
}

/**
 * @brief Initialize all allegro addons and the game body.
 * @details Only one timer is created since a game and all its data should be processed synchronously.
 */
Game::Game() {
	DataCenter *DC = DataCenter::get_instance();
	GAME_ASSERT(al_init(), "failed to initialize allegro.");

	// initialize allegro addons
	bool addon_init = true;
	addon_init &= al_init_primitives_addon();
	addon_init &= al_init_font_addon();
	addon_init &= al_init_ttf_addon();
	addon_init &= al_init_image_addon();
	addon_init &= al_init_acodec_addon();
	GAME_ASSERT(addon_init, "failed to initialize allegro addons.");

	// initialize events
	bool event_init = true;
	event_init &= al_install_keyboard();
	event_init &= al_install_mouse();
	event_init &= al_install_audio();
	GAME_ASSERT(event_init, "failed to initialize allegro events.");

	// initialize game body
	GAME_ASSERT(
		display = al_create_display(DC->window_width, DC->window_height),
		"failed to create display.");
	GAME_ASSERT(
		timer = al_create_timer(1.0 / DC->FPS),
		"failed to create timer.");
	GAME_ASSERT(
		event_queue = al_create_event_queue(),
		"failed to create event queue.");

	debug_log("Game initialized.\n");
	game_init();
}

/**
 * @brief Initialize all auxiliary resources.
 */
void
Game::game_init() {
	DataCenter *DC = DataCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();
	// set window icon
	game_icon = IC->get(game_icon_img_path);
	al_set_display_icon(display, game_icon);

	// register events to event_queue
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    
    if (background) {
        al_destroy_bitmap(background);
        background = nullptr;
    }
	// init sound setting
	SC->init();
	// init font setting
	FC->init();
	// init UI setting
	ui = new UI();
	ui->init();
	DC->level->init();
	// game start
	background = IC->get(background_img_path);
	level_background = IC->get(level_background_img_path);
	level1_background = IC->get(level1_background_img_path);
	level2_background = IC->get(level2_background_img_path);
	level3_background = IC->get(level3_background_img_path);
	select_button = IC-> get(select_button_img_path);
	debug_log("Game state: change to CHOOSELV\n");
	state = STATE::CHOOSELV;
	al_stop_timer(timer);
	al_start_timer(timer);
}


/**
 * @brief The function processes all data update.
 * @details The behavior of the whole game body is determined by its state.
 * @return Whether the game should keep running (true) or reaches the termination criteria (false).
 * @see Game::STATE
 */
bool
Game::game_update() {
	DataCenter *DC = DataCenter::get_instance();
	OperationCenter *OC = OperationCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	static ALLEGRO_SAMPLE_INSTANCE *background = nullptr;

	switch(state) {
	    
		case STATE::START: {
			static bool is_played = false;
			static ALLEGRO_SAMPLE_INSTANCE *instance = nullptr;
			if(!is_played) {
				instance = SC->play(game_start_sound_path, ALLEGRO_PLAYMODE_ONCE);
				is_played = true;
			}
            if (DC->mouse_state[1] && !DC->prev_mouse_state[1]// press level
                && DC->mouse.x >= start_button_x && DC->mouse.x <= start_button_x + start_button_width
                && DC->mouse.y >= start_button_y && DC->mouse.y <= start_button_y + start_button_height) {
                if(!SC->is_playing(instance)) {
				    debug_log("<Game> state: change to LEVEL\n");
				    state = STATE::LEVEL;
			    }
            }
			
			break;
		} case STATE::CHOOSELV: {
			
			float button_width = 150;
            float button_height = 50;
			float button_x = DC->window_width / 2 - button_width / 2;
            float button_y_1 = 370;
			float button_y_2 = 440;
			float button_y_3 = 510;
			if (DC->mouse_state[1] && !DC->prev_mouse_state[1]// press level1
                && DC->mouse.x >= button_x && DC->mouse.x <= button_x + button_width
                && DC->mouse.y >= button_y_1 && DC->mouse.y <= button_y_1 + button_height) {
                debug_log("<Game> state: change to START\n");
				DC->level->load_level(1);
                state = STATE::START; 
            }
			
			if (DC->mouse_state[1] && !DC->prev_mouse_state[1]// press level2
                && DC->mouse.x >= button_x && DC->mouse.x <= button_x + button_width
                && DC->mouse.y >= button_y_2 && DC->mouse.y <= button_y_2 + button_height) {
                debug_log("<Game> state: change to START\n");
				DC->level->load_level(2);
                state = STATE::START; 
            }
			
			if (DC->mouse_state[1] && !DC->prev_mouse_state[1]// press level3
                && DC->mouse.x >= button_x && DC->mouse.x <= button_x + button_width
                && DC->mouse.y >= button_y_3 && DC->mouse.y <= button_y_3 + button_height) {
                debug_log("<Game> state: change to START\n");
				DC->level->load_level(3);
                state = STATE::START; 
            }	
			
			break;
		} case STATE::LEVEL: {
			
			static bool BGM_played = false;
			if(!BGM_played) {
				background = SC->play(bgm_list[current_bgm_index], ALLEGRO_PLAYMODE_LOOP);
				BGM_played = true;
			}


            if (DC->mouse_state[1] && !DC->prev_mouse_state[1]
                && DC->mouse.x >= pause_button_x && DC->mouse.x <= pause_button_x + pause_button_width
                && DC->mouse.y >= pause_button_y && DC->mouse.y <= pause_button_y + pause_button_height) {
				debug_log("<Game> state: change to PAUSE\n");
                state = STATE::PAUSE; 
            }
			
			if(DC->level->remain_monsters() == 0 && DC->monsters.size() == 0) {
				debug_log("<Game> state: change to WIN\n");
				state = STATE::WIN;
			}
			if(DC->player->HP == 0) {
				debug_log("<Game> state: change to LOSE\n");
				state = STATE::LOSE;
			}
			break;
		} case STATE::PAUSE: {
			static bool isMute = false;
            float sound_button_width = 200;
            float sound_button_height = 50;
            float sound_button_x = 50;
            float sound_button_y = 400;
			if (DC->mouse_state[1] && !DC->prev_mouse_state[1]
                && DC->mouse.x >= sound_button_x && DC->mouse.x <= sound_button_x + sound_button_width
                && DC->mouse.y >= sound_button_y && DC->mouse.y <= sound_button_y + sound_button_height) {
                current_bgm_index = (current_bgm_index + 1) % bgm_list.size();                
				SC->toggle_playing(background);
                background = SC->play(bgm_list[current_bgm_index], ALLEGRO_PLAYMODE_LOOP);
                debug_log("<Game> BGM switched to index %d\n", current_bgm_index);
            }

            if (DC->mouse_state[1] && !DC->prev_mouse_state[1]
                && DC->mouse.x >= continue_button_x && DC->mouse.x <= continue_button_x + continue_button_width
                && DC->mouse.y >= continue_button_y && DC->mouse.y <= continue_button_y + continue_button_height) {
				debug_log("<Game> state: CONTINUE GAME\n");
                state = STATE::LEVEL; 
            }
			if (DC->mouse_state[1] && !DC->prev_mouse_state[1]
    			&& DC->mouse.x >= mute_button_x && DC->mouse.x <= mute_button_x + mute_button_width
    			&& DC->mouse.y >= mute_button_y && DC->mouse.y <= mute_button_y + mute_button_height) {

    			if (!isMute) { // Currently not muted, so mute it
        			debug_log("<Game> state: MUTE\n");
        			al_set_sample_instance_playing(background, false); // Pause the music
        			isMute = true; // Update state to muted
    			} else { // Currently muted, so unmute it
        				debug_log("<Game> state: UNMUTE\n");
        				al_set_sample_instance_playing(background, true); // Resume the music
        				isMute = false; // Update state to unmuted
    			}
			}

			break;
		} case STATE::WIN: {
			static double win_time = -1;
            if (win_time < 0) {
                win_time = al_get_time();
            }

            if (al_get_time() - win_time >= 3.0) {
               if (DC->level->current_level() < 3) {
                    debug_log("<Game> state: change to LEVEL %d\n", DC->level->current_level() + 1);
                    DC->level->load_level(DC->level->current_level() + 1);
                    state = STATE::LEVEL;
               } else {
                    debug_log("<Game> state: change to END\n");
                    state = STATE::END;
                }
            }
            break;
		} case STATE::LOSE: {
			static double lose_time = -1;
			if(lose_time < 0) {
				lose_time = al_get_time();
			}
			if(al_get_time() - lose_time >= 3.0){
                state = STATE::END;
			}
			break;
		} case STATE::END: {
			return false;
		}
	}
	// If the game is not paused, we should progress update.
	if(state != STATE::PAUSE && state != STATE::CHOOSELV) {
		DC->player->update();
		SC->update();
		ui->update();
		if(state != STATE::START) {
			DC->level->update();
			OC->update();
		}
	}
	// game_update is finished. The states of current frame will be previous states of the next frame.
	memcpy(DC->prev_key_state, DC->key_state, sizeof(DC->key_state));
	memcpy(DC->prev_mouse_state, DC->mouse_state, sizeof(DC->mouse_state));
	return true;
}

/**
 * @brief Draw the whole game and objects.
 */
void
Game::game_draw() {
	DataCenter *DC = DataCenter::get_instance();
	OperationCenter *OC = OperationCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();

	// Flush the screen first.
	al_clear_to_color(al_map_rgb(100, 100, 100));
	if(state != STATE::END) {
		// background
		if(DC->level->current_level() == 1) {
			al_draw_bitmap(level1_background, 0, 0, 0);
		} else if(DC->level->current_level() == 2) {
			al_draw_bitmap(level2_background, 0, 0, 0);
		} else if(DC->level->current_level() == 3) {
			al_draw_bitmap(level3_background, 0, 0, 0);
		}
		if(DC->game_field_length < DC->window_width)
			al_draw_filled_rectangle(
				DC->game_field_length, 0,
				DC->window_width, DC->window_height,
				al_map_rgb(100, 100, 100));
		if(DC->game_field_length < DC->window_height)
			al_draw_filled_rectangle(
				0, DC->game_field_length,
				DC->window_width, DC->window_height,
				al_map_rgb(100, 100, 100));
		// user interface
		if(state != STATE::START) {
			DC->level->draw();
			ui->draw();
			OC->draw();
		}
	}
	switch(state) {
		case STATE::START: {
    		float bitmap_width = al_get_bitmap_width(select_button);
    		float bitmap_height = al_get_bitmap_height(select_button);

			float bitmap_x = start_button_x + (start_button_width - bitmap_width) / 2; // Center horizontally
    		float bitmap_y = start_button_y + (start_button_height - bitmap_height) / 2; // Center vertically

			bool isHovering;

			al_draw_bitmap(select_button, bitmap_x, bitmap_y, 0);
			if(DC->mouse.x >= start_button_x && DC->mouse.x <= start_button_x + start_button_width
                && DC->mouse.y >= start_button_y && DC->mouse.y <= start_button_y + start_button_height){
					isHovering = true;
                //al_draw_bitmap(level1_background, 0, 0, 0);
			}
			else {
				isHovering = false;
			}
            // al_draw_filled_rectangle(start_button_x, start_button_y, 
            //                  start_button_x + start_button_width, 
            //                  start_button_y + start_button_height, 
            //                  al_map_rgb(100, 200, 100));
            // al_draw_rectangle(start_button_x, start_button_y, 
            //           start_button_x + start_button_width, 
            //           start_button_y + start_button_height, 
            //           al_map_rgb(0, 0, 0), 2);
            al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
                 start_button_x + start_button_width / 2, 
                 start_button_y + start_button_height / 2 - 15,
                 ALLEGRO_ALIGN_CENTRE, "START");
			if (isHovering) {
				al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(50, 50, 50),
                	start_button_x + start_button_width / 2 + 2, 
                 	start_button_y + start_button_height / 2 - 13,
                 	ALLEGRO_ALIGN_CENTRE, "START");
				al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 0),
                 	start_button_x + start_button_width / 2, 
                 	start_button_y + start_button_height / 2 - 15,
                 	ALLEGRO_ALIGN_CENTRE, "START");
			}
			break;
		} case STATE::CHOOSELV: {
            // draw choose level button 
            float button_width = 150;
            float button_height = 50;
            
			float button_x = DC->window_width / 2 - button_width / 2;
            float button_y_1 = 370;
			float button_y_2 = 440;
			float button_y_3 = 510;

			bool isHovering1 = false;
			bool isHovering2 = false;
			bool isHovering3 = false;

			if(DC->mouse.x >= button_x && DC->mouse.x <= button_x + button_width
                && DC->mouse.y >= button_y_1 && DC->mouse.y <= button_y_1 + button_height){
					isHovering1 = true;
                al_draw_bitmap(level1_background, 0, 0, 0);
				al_draw_filled_rectangle(600, 0, 800, 600, al_map_rgb(100, 100, 100));
				const char* text[] = {"GREEN VALLEY", "RELAX!", "THIS IS", "ONLY THE", "FIRST LEVEL"};
				for (int i = 0; i < 5; i++) {
        			al_draw_text(
            			FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255), // Normal color
            			700, 15 + 30 * i,
            			ALLEGRO_ALIGN_CENTRE, text[i]);
				}
			} else if(DC->mouse.x >= button_x && DC->mouse.x <= button_x + button_width
                && DC->mouse.y >= button_y_2 && DC->mouse.y <= button_y_2 + button_height){
					isHovering2 = true;
                al_draw_bitmap(level2_background, 0, 0, 0);
				al_draw_filled_rectangle(600, 0, 800, 600, al_map_rgb(100, 100, 100));
				const char* text[] = {"BLUE TUNDRA", "REMEMBER", "TO SPEND", "YOUR MONEY"};
				for (int i = 0; i < 4; i++) {
        			al_draw_text(
            			FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255), // Normal color
            			700, 15 + 30 * i,
            			ALLEGRO_ALIGN_CENTRE, text[i]);
				}
			} else if(DC->mouse.x >= button_x && DC->mouse.x <= button_x + button_width
                && DC->mouse.y >= button_y_3 && DC->mouse.y <= button_y_3 + button_height){
					isHovering3 = true;
                al_draw_bitmap(level3_background, 0, 0, 0);
				al_draw_filled_rectangle(600, 0, 800, 600, al_map_rgb(100, 100, 100));
				const char* text[] = {"RED MAZE", "WARNING", "LOTS OF", "MONSTERS AHEAD"};
				for (int i = 0; i < 4; i++) {
        			al_draw_text(
            			FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255), // Normal color
            			700, 15 + 30 * i,
            			ALLEGRO_ALIGN_CENTRE, text[i]);
				}
			} else{
				isHovering1 = false;
				isHovering2 = false;
				isHovering3 = false;
                al_draw_bitmap(level_background, 0, 0, 0); 
			}
			const char* level_names[] = {"EASY", "MEDIUM", "HARD"}; 
            for (int i = 0; i < 3; i++) {
                float button_x = DC->window_width / 2 - button_width / 2;
                float button_y = 370 + i * 70;

				// Calculate the center alignment for the select_button bitmap
    			float bitmap_width = al_get_bitmap_width(select_button);
    			float bitmap_height = al_get_bitmap_height(select_button);

				float bitmap_x = button_x + (button_width - bitmap_width) / 2; // Center horizontally
    			float bitmap_y = button_y + (button_height - bitmap_height) / 2; // Center vertically
				al_draw_bitmap(select_button, bitmap_x, bitmap_y, 0);

                //al_draw_filled_rectangle(button_x, button_y, button_x + button_width, button_y + button_height, al_map_rgb(100, 100, 200));
                //al_draw_rectangle(button_x, button_y, button_x + button_width, button_y + button_height, al_map_rgb(0, 0, 0), 5);


        		al_draw_text(
            		FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255), // Normal color
            		button_x + button_width / 2, button_y + button_height / 2 - 15,
            		ALLEGRO_ALIGN_CENTRE, level_names[i]);
            }
			if (isHovering1) {
        			al_draw_text(
            			FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(50, 50, 50), // Shadow color
            			button_x + button_width / 2 + 2, button_y_1 + button_height / 2 - 13,
            			ALLEGRO_ALIGN_CENTRE, "EASY");
				    al_draw_text(
            			FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 0), // Normal color
            			button_x + button_width / 2, button_y_1 + button_height / 2 - 15,
            			ALLEGRO_ALIGN_CENTRE, "EASY");
					

			}
			if (isHovering2) {
	        		al_draw_text(
            			FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(50, 50, 50), // Shadow color
            			button_x + button_width / 2 + 2, button_y_2 + button_height / 2 - 13,
            			ALLEGRO_ALIGN_CENTRE, "MEDIUM");
				    al_draw_text(
            			FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 0), // Normal color
            			button_x + button_width / 2, button_y_2 + button_height / 2 - 15,
            			ALLEGRO_ALIGN_CENTRE, "MEDIUM");

			}
			if (isHovering3) {
	        		al_draw_text(
            			FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(50, 50, 50), // Shadow color
            			button_x + button_width / 2 + 2, button_y_3 + button_height / 2 - 13,
            			ALLEGRO_ALIGN_CENTRE, "HARD");
				    al_draw_text(
            			FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 0), // Normal color
            			button_x + button_width / 2, button_y_3 + button_height / 2 - 15,
            			ALLEGRO_ALIGN_CENTRE, "HARD");
			}
            break;
		} case STATE::LEVEL: {

    		float bitmap_width = al_get_bitmap_width(select_button);
    		float bitmap_height = al_get_bitmap_height(select_button);

			float bitmap_x = pause_button_x + (pause_button_width - bitmap_width) / 2; // Center horizontally
    		float bitmap_y = pause_button_y + (pause_button_height - bitmap_height) / 2; // Center vertically
			al_draw_bitmap(select_button, bitmap_x, bitmap_y, 0);
			bool isHovering = false;
			if(DC->mouse.x >= pause_button_x && DC->mouse.x <= pause_button_x + pause_button_width
                && DC->mouse.y >= pause_button_y && DC->mouse.y <= pause_button_y + pause_button_height){
					isHovering = true;
			}
			else {
				isHovering = false;
			}
            //al_draw_filled_rectangle(button_x, button_y, button_x + button_width, button_y + button_height, al_map_rgba(255, 255, 255, 128));
            al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
                 pause_button_x + pause_button_width / 2, 
                 pause_button_y + pause_button_height / 2 - 15,
                 ALLEGRO_ALIGN_CENTRE, "PAUSE");
			if (isHovering) {
				al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(50, 50, 50),
                	pause_button_x + pause_button_width / 2 + 2, 
                 	pause_button_y + pause_button_height / 2 - 13,
                 	ALLEGRO_ALIGN_CENTRE, "PAUSE");
				al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 0),
                 	pause_button_x + pause_button_width / 2, 
                 	pause_button_y + pause_button_height / 2 - 15,
                 	ALLEGRO_ALIGN_CENTRE, "PAUSE");
			}
			break;
		} case STATE::PAUSE: {
			// game layout cover
			al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(50, 50, 50, 64));
			al_draw_text(
				FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
				DC->window_width/2., DC->window_height/2.,
				ALLEGRO_ALIGN_CENTRE, "GAME PAUSED");

    		float bitmap_width = al_get_bitmap_width(select_button);
    		float bitmap_height = al_get_bitmap_height(select_button);

			float bitmap_x = pause_button_x + (pause_button_width - bitmap_width) / 2; // Center horizontally
    		float bitmap_y = pause_button_y + (pause_button_height - bitmap_height) / 2; // Center vertically
			al_draw_bitmap(select_button, bitmap_x, bitmap_y, 0);
			bool isHovering1 = false;
			bool isHovering2 = false;
			if(DC->mouse.x >= pause_button_x && DC->mouse.x <= pause_button_x + pause_button_width
                && DC->mouse.y >= pause_button_y && DC->mouse.y <= pause_button_y + pause_button_height){
					isHovering1 = true;
			}
			else {
				isHovering1 = false;
			}
            //al_draw_filled_rectangle(button_x, button_y, button_x + button_width, button_y + button_height, al_map_rgba(255, 255, 255, 128));
            al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
                 pause_button_x + pause_button_width / 2, 
                 pause_button_y + pause_button_height / 2 - 15,
                 ALLEGRO_ALIGN_CENTRE, "CONTINUE");
			if (isHovering1) {
				al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(50, 50, 50),
                	pause_button_x + pause_button_width / 2 + 2, 
                 	pause_button_y + pause_button_height / 2 - 13,
                 	ALLEGRO_ALIGN_CENTRE, "CONTINUE");
				al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 0),
                 	pause_button_x + pause_button_width / 2, 
                 	pause_button_y + pause_button_height / 2 - 15,
                 	ALLEGRO_ALIGN_CENTRE, "CONTINUE");
			}

			bitmap_x = mute_button_x + (mute_button_width - bitmap_width) / 2; // Center horizontally
    		bitmap_y = mute_button_y + (mute_button_height - bitmap_height) / 2; // Center vertically

			al_draw_bitmap(select_button, bitmap_x, bitmap_y, 0);
			if(DC->mouse.x >= mute_button_x && DC->mouse.x <= mute_button_x + mute_button_width
                && DC->mouse.y >= mute_button_y && DC->mouse.y <= mute_button_y + mute_button_height){
					isHovering2 = true;
			}
			else {
				isHovering2 = false;
			}
            //al_draw_filled_rectangle(button_x, button_y, button_x + button_width, button_y + button_height, al_map_rgba(255, 255, 255, 128));
            al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
                 mute_button_x + mute_button_width / 2, 
                 mute_button_y + mute_button_height / 2 - 15,
                 ALLEGRO_ALIGN_CENTRE, "MUTE/UNMUTE");
			if (isHovering2) {
				al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(50, 50, 50),
                	mute_button_x + mute_button_width / 2 + 2, 
                 	mute_button_y + mute_button_height / 2 - 13,
                 	ALLEGRO_ALIGN_CENTRE, "MUTE/UNMUTE");
				al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 0),
                 	mute_button_x + mute_button_width / 2, 
                 	mute_button_y + mute_button_height / 2 - 15,
                 	ALLEGRO_ALIGN_CENTRE, "MUTE/UNMUTE");
			}
			break;
		} case STATE::WIN:{
            al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(50, 50, 50, 64));
			const char *passed_text = "LEVEL%d passed!!!";
            int number = DC->level->current_level(); 
			al_draw_textf(
				FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
				DC->window_width/2., DC->window_height/2.,
				ALLEGRO_ALIGN_CENTRE, passed_text, number);
    		for (Tower *tower : DC->towers) {
        		delete tower; // Free memory for each tower
    		}
    		DC->towers.clear(); // Remove all entries from the vector
			break;
		} case STATE::LOSE: {
            al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(50, 50, 50, 64));
			al_draw_text(
				FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
				DC->window_width/2., DC->window_height/2.,
				ALLEGRO_ALIGN_CENTRE, "YOU LOSE !!!");
			break;
		} case STATE::END: {

		}
	}
	al_flip_display();
}

Game::~Game() {
	al_destroy_display(display);
	al_destroy_timer(timer);
	al_destroy_event_queue(event_queue);
}

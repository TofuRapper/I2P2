#include "UI.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include <algorithm>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include "shapes/Point.h"
#include "shapes/Rectangle.h"
#include "Player.h"
#include "towers/Tower.h"
#include "Level.h"

// fixed settings
constexpr char love_img_path[] = "./assets/image/love.png";
constexpr int love_img_padding = 5;
constexpr int tower_img_left_padding = 30;
constexpr int tower_img_top_padding = 30;

void
UI::init() {
	DataCenter *DC = DataCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();
	love = IC->get(love_img_path);
	int tl_x = DC->game_field_length + tower_img_left_padding;
	int tl_y = tower_img_top_padding;
	int max_height = 0;
	int offset_x = -15; // Shift all items left
    int offset_y = 100; // Shift all items down
	// arrange tower shop
	for(size_t i = 0; i < (size_t)(TowerType::TOWERTYPE_MAX); ++i) {
		ALLEGRO_BITMAP *bitmap = IC->get(TowerSetting::tower_menu_img_path[i]);
		int w = al_get_bitmap_width(bitmap);
		int h = al_get_bitmap_height(bitmap);
		if(tl_x + w > DC->window_width) {
			tl_x = DC->game_field_length + tower_img_left_padding;
			tl_y += max_height + tower_img_top_padding;
			max_height = 0;
		}
		tower_items.emplace_back(bitmap, Point{tl_x + offset_x, tl_y + offset_y}, TowerSetting::tower_price[i]);
		tl_x += w + tower_img_left_padding;
		max_height = std::max(max_height, h);
	}

	// constexpr int towers_per_row = 10; // æ¯?è¡?å¡?æ¨???¸é??
    // const int horizontal_spacing = 20; // å¡?æ¨?ä¹???????æ°´å¹³???è·?
    // const int vertical_spacing = 10;   // è¡?ä¹????????????´é??è·?

    // // ???å§?ä½?ç½®ï??åº???¨é??å§???ºæ?¾ï??
    // int start_x = tower_img_left_padding;
    // int start_y = DC->window_height - tower_img_top_padding;
    // int current_row = 0; // ??¶å??è¡????

    // for (size_t i = 0; i < static_cast<size_t>(TowerType::TOWERTYPE_MAX); ++i) {
    //     ALLEGRO_BITMAP *bitmap = IC->get(TowerSetting::tower_menu_img_path[i]);
    //     int w = al_get_bitmap_width(bitmap);
    //     int h = al_get_bitmap_height(bitmap);

    //     // è¨?ç®???¶å??å¡?æ¨???? X ??? Y ä½?ç½?
    //     int tl_x = start_x + (i % towers_per_row) * (w + horizontal_spacing);
    //     int tl_y = start_y - (current_row * (h + vertical_spacing));

    //     // æ·»å??å¡?æ¨???????
    //     tower_items.emplace_back(bitmap, Point{tl_x, tl_y - h}, TowerSetting::tower_price[i]);
    // }
	debug_log("<UI> state: change to HALT\n");
	
	state = STATE::HALT;
	on_item = -1;
}

void
UI::update() {
	DataCenter *DC = DataCenter::get_instance();
	const Point &mouse = DC->mouse;

	switch(state) {
		case STATE::HALT: {
			for(size_t i = 0; i < tower_items.size(); ++i) {
				auto &[bitmap, p, price] = tower_items[i];
				int w = al_get_bitmap_width(bitmap);
				int h = al_get_bitmap_height(bitmap);
				// hover on a shop tower item
				if(mouse.overlap(Rectangle{p.x, p.y, p.x+w, p.y+h})) {
					on_item = i;
					debug_log("<UI> state: change to HOVER\n");
					state = STATE::HOVER;
					break;
				}
			}
			break;
		} case STATE::HOVER: {
			auto &[bitmap, p, price] = tower_items[on_item];
			int w = al_get_bitmap_width(bitmap);
			int h = al_get_bitmap_height(bitmap);
			if(!mouse.overlap(Rectangle{p.x, p.y, p.x+w, p.y+h})) {
				on_item = -1;
				debug_log("<UI> state: change to HALT\n");
				state = STATE::HALT;
				break;
			}
			// click mouse left button
			if(DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
				// no money
				if(price > DC->player->coin) {
					debug_log("<UI> Not enough money to buy tower %d.\n", on_item);
					break;
				}
				debug_log("<UI> state: change to SELECT\n");
				state = STATE::SELECT;
			}
			break;
			
		} case STATE::SELECT: {
			// click mouse left button: place
			if(DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
				debug_log("<UI> state: change to PLACE\n");
				state = STATE::PLACE;
			}
			// click mouse right button: cancel
			if(DC->mouse_state[2] && !DC->prev_mouse_state[2]) {
				on_item = -1;
				debug_log("<UI> state: change to HALT\n");
				state = STATE::HALT;
			}
			break;
		} case STATE::PLACE: {
			// check placement legality
			ALLEGRO_BITMAP *bitmap = Tower::get_bitmap(static_cast<TowerType>(on_item));
			int w = al_get_bitmap_width(bitmap);
			int h = al_get_bitmap_height(bitmap);
			Rectangle place_region{mouse.x - w / 2, mouse.y - h / 2, DC->mouse.x + w / 2, DC->mouse.y + h / 2};
			bool place = true;
			// tower cannot be placed on the road
            place &= (!DC->level->is_onroad(place_region));
            // tower cannot intersect with other towers
            for(Tower *tower : DC->towers) {
                place &= (!place_region.overlap(tower->get_region()));
            }
			if(!place) {
				debug_log("<UI> Tower place failed.\n");
			} else {
				DC->towers.emplace_back(Tower::create_tower(static_cast<TowerType>(on_item), mouse));
				DC->player->coin -= std::get<2>(tower_items[on_item]);
			}
			debug_log("<UI> state: change to HALT\n");
			state = STATE::HALT;
			break;
		}
	}
}

void
UI::draw() {
	DataCenter *DC = DataCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();
	const Point &mouse = DC->mouse;
	// draw HP
	const int &game_field_length = DC->game_field_length;
	const int &player_HP = DC->player->HP;
	int love_width = al_get_bitmap_width(love);
	for(int i = 1; i <= player_HP; ++i) {
		al_draw_bitmap(love, game_field_length - (love_width + love_img_padding) * i, love_img_padding, 0);
	}
	// draw coin
	const int &player_coin = DC->player->coin;
	al_draw_textf(
		FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
		610, 10,
		ALLEGRO_ALIGN_LEFT, "MONEY:%3d", player_coin);
	// draw tower shop items
	for (size_t i = 0; i < tower_items.size(); ++i) {
        auto &[bitmap, p, price] = tower_items[i];
        int w = al_get_bitmap_width(bitmap);
        int h = al_get_bitmap_height(bitmap);
        al_draw_bitmap(bitmap, p.x, p.y, 0);
        
        al_draw_rectangle(
            p.x - 1, p.y - 1,
            p.x  + w + 1, p.y  + h + 1,
            al_map_rgb(0, 0, 0), 5);

    //     al_draw_textf(
    //         FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
    //         p.x + w / 2, p.y + h + 5,
    //         ALLEGRO_ALIGN_CENTRE, "$%d", price);
    }
	
	switch(state) {
		static Tower *selected_tower = nullptr;
		case STATE::HALT: {
			// No tower should be selected for HALT state.
			if(selected_tower != nullptr) {
				delete selected_tower;
				selected_tower = nullptr;
			}
			break;
		} case STATE::HOVER: {
			auto &[bitmap, p, price] = tower_items[on_item];
			int w = al_get_bitmap_width(bitmap);
			int h = al_get_bitmap_height(bitmap);
			// Create a semitransparent mask covered on the hovered tower.
			al_draw_filled_rectangle(p.x , p.y , p.x + w, p.y + h, al_map_rgba(50, 50, 50, 64));
			// Draw tower attributes
			//al_draw_filled_rectangle(610, 300, 790, 550, al_map_rgba(50, 50, 50, 64));
            Tower *temp_tower = Tower::create_tower(static_cast<TowerType>(on_item), Point(0, 0));
            FontCenter *FC = FontCenter::get_instance();
			std::string tower_name = Tower::get_name(static_cast<TowerType>(on_item));
			al_draw_textf(
                FC->courier_new[FontSize::MEDIUM], 
                al_map_rgb(255, 255, 255),  
				620,
                390,                                 
                ALLEGRO_ALIGN_LEFT, 
                "%s", 
                tower_name.c_str()
            );
			al_draw_textf(
                FC->courier_new[FontSize::MEDIUM], 
                al_map_rgb(255, 255, 255),  
                620,                
                400,                  
                ALLEGRO_ALIGN_LEFT, 
                "%s", 
                "___________"
            );
            al_draw_textf(
                FC->courier_new[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
                620, 430, 
                ALLEGRO_ALIGN_LEFT, "Range:%d", temp_tower->get_range());

            al_draw_textf(
                FC->courier_new[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
                620, 450,
                ALLEGRO_ALIGN_LEFT, "Damage:%d", temp_tower->get_dmg());

            al_draw_textf(
                FC->courier_new[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
                620, 470,
                ALLEGRO_ALIGN_LEFT, "Freq:%d", temp_tower->get_speed());
    		// Tower Price
    		al_draw_textf(
        		FC->courier_new[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
        		620, 490,
        		ALLEGRO_ALIGN_LEFT, "Price: %d", price
    		);

			break;
		}
		case STATE::SELECT: {
			// If a tower is selected, we new a corresponding tower for previewing purpose.
			if(selected_tower == nullptr) {
				selected_tower = Tower::create_tower(static_cast<TowerType>(on_item), mouse);
			} else {
				selected_tower->shape->update_center_x(mouse.x);
				selected_tower->shape->update_center_y(mouse.y);
			}
		}
		case STATE::PLACE: {
			// If we select a tower from menu, we need to preview where the tower will be built and its attack range.
			ALLEGRO_BITMAP *bitmap = Tower::get_bitmap(static_cast<TowerType>(on_item));
			al_draw_filled_circle(mouse.x, mouse.y, selected_tower->attack_range(), al_map_rgba(255, 0, 0, 32));
			int w = al_get_bitmap_width(bitmap);
			int h = al_get_bitmap_height(bitmap);
			al_draw_bitmap(bitmap, mouse.x - w / 2, mouse.y - h / 2, 0);
			break;
		}
	}
}

#ifndef TOWER_H_INCLUDED
#define TOWER_H_INCLUDED

#include "../Object.h"
#include "../shapes/Rectangle.h"
#include <allegro5/bitmap.h>
#include <string>
#include <array>

class Bullet;

// fixed settings
enum class TowerType {
	ARCANE, ARCHER, CANON, CHOU, POISON, STORM, TOWERTYPE_MAX
};
namespace TowerSetting {
	const std::array<std::string, static_cast<int>(TowerType::TOWERTYPE_MAX)> tower_full_img_path = {
		"./assets/image/tower/Arcane.png",
		"./assets/image/tower/Archer.png",
		"./assets/image/tower/Canon.png",
		"./assets/image/tower/Poison.png",
		"./assets/image/tower/Storm.png",
		"./assets/image/tower/Chou.png"
	};
	const std::array<std::string, static_cast<int>(TowerType::TOWERTYPE_MAX)> tower_menu_img_path = {
		"./assets/image/tower/Arcane_Menu.png",
		"./assets/image/tower/Archer_Menu.png",
		"./assets/image/tower/Canon_Menu.png",
		"./assets/image/tower/Poison_Menu.png",
		"./assets/image/tower/Storm_Menu.png",
		"./assets/image/tower/Chou_Menu.png",
	};
	const std::array<std::string, static_cast<int>(TowerType::TOWERTYPE_MAX)> tower_bullet_img_path = {
		"./assets/image/tower/Arcane_Beam.png",
		"./assets/image/tower/Archer_Beam.png",
		"./assets/image/tower/Canon_Beam.png",
		"./assets/image/tower/Poison_Beam.png",
		"./assets/image/tower/Storm_Beam.png",
		"./assets/image/tower/Chou_Beam.png"
	};
	const std::array<int, static_cast<int>(TowerType::TOWERTYPE_MAX)> tower_price = {10, 50, 100, 150, 200, 250};
};

class Tower : public Object
{
public:
	/**
	 * @brief Get the ALLEGRO_BITMAP* instance of the full image of a specific TowerType.
	 */
	static ALLEGRO_BITMAP *get_bitmap(TowerType type);
	/**
	 * @brief Create a Tower* instance by the type.
	 * @param type the type of a tower.
	 * @param p center point of the tower.
	 */
	static std::string get_name(TowerType type);
	static Tower *create_tower(TowerType type, const Point &p);
	int center_x; // å¡????ä¸­å??é»? X åº§æ??
    int center_y; // å¡????ä¸­å??é»? Y åº§æ??
    int width;    // å¡????å¯¬åº¦
    int height;   // å¡????é«?åº?

    Rectangle get_bounding_box() const {
        int left = center_x - width / 2;
        int top = center_y - height / 2;
        int right = center_x + width / 2;
        int bottom = center_y + height / 2;

        return Rectangle(left, top, right, bottom);
    }
	
public:
	Tower(const Point &p, double attack_range, int attack_freq, TowerType type);
	virtual ~Tower() {}
	void update();
	virtual bool attack(Object *target);
	void draw();
	Rectangle get_region() const;
	
	virtual int get_dmg() const = 0;
	virtual int get_speed() const = 0;
	virtual int get_range() const = 0;
	virtual Bullet *create_bullet(Object *target) = 0;
	virtual const double attack_range() const = 0;
	TowerType type;
    double calculate_rotation_angle(const Object* target) const;
	
private:
	/**
	 * @var attack_freq
	 * @brief Tower attack frequency. This variable will be set by its child classes.
	 **
	 * @var counter
	 * @brief Tower attack cooldown.
	 */
	int attack_freq;
	int counter;
	ALLEGRO_BITMAP *bitmap;
};

#endif

#ifndef TOWERCHOU_H_INCLUDED
#define TOWERCHOU_H_INCLUDED

#include "Tower.h"
#include "Bullet.h"
#include "../shapes/Point.h"

// fixed settings: TowerChou attributes
class TowerChou : public Tower
{
public:
	TowerChou(const Point &p) : 
		Tower(p, attack_range(), 10, TowerType::CHOU)
        
    {}
	Bullet *create_bullet(Object *target) {
		const Point &p = Point(shape->center_x(), shape->center_y());
		const Point &t = Point(target->shape->center_x(), target->shape->center_y());
		return new Bullet(p, t, TowerSetting::tower_bullet_img_path[static_cast<int>(type)], 480, dmg, attack_range());
	}
	const double attack_range() const { return 250; }
	int get_dmg() const override { return dmg; }
	int get_speed() const override { return speed; }
	int get_range() const override { return range; }
private:
    int dmg = 2;
	int range = 250;
	int speed = 10;
};

#endif

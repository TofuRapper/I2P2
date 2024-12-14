#ifndef TOWERCANON_H_INCLUDED
#define TOWERCANON_H_INCLUDED

#include "Tower.h"
#include "Bullet.h"
#include "../shapes/Point.h"

// fixed settings: TowerCanon attributes
class TowerCanon : public Tower
{
public:
	TowerCanon(const Point &p) : 
		Tower(p, attack_range(), 10, TowerType::CANON)
    {}
	Bullet *create_bullet(Object *target) {
		const Point &p = Point(shape->center_x(), shape->center_y());
		const Point &t = Point(target->shape->center_x(), target->shape->center_y());
		return new Bullet(p, t, TowerSetting::tower_bullet_img_path[static_cast<int>(type)], 300, dmg, attack_range());
	}
	const double attack_range() const { return 200; }
	int get_dmg() const override { return dmg; }
	int get_speed() const override { return speed; }
	int get_range() const override { return range; }
private:
    int dmg = 5;
	int range = 200;
	int speed = 5;
};

#endif

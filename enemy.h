#ifndef _ENEMY_H
#define _ENEMY_H
#include<string>
#include<vector>
#include "player.h" // includes pile.h

class enemy {
public:
    std::string name;
    int poison = 0;
    int maxmana = 4; // Enemies have mana, may be useful some time
    int mana;
    int hp;
    int maxhp;
    int vulnerable = 0;
    int barricade = 0; // don't discard block for x turns
    int weak = 0;
    int block = 0;
    int strength = 0;
    int level;
    int frail = 0; // Lower block effectiveness
    std::string intention;
    std::vector<std::string> actions;
    int intention_counter_max;
    int intention_counter = 0;

    enemy(std::string n, int mhp, int l, std::vector<std::string> ac);

    int mult_dmg_from(int dmg);

    int mult_dmg_to(int dmg);

    void clear_block();

    int mult_block(int blc);

    void take_damage(int dmg);

    // piercing damage
    void take_damage_forced(int dmg);

    bool check_hp();

    void addblock(int blc);

    void remove_mana(int n);

    std::string get_intention();

    void commit_intention(player* pl, pile* plc);

    void begin_turn();

    void decrease_counters();

};
#endif

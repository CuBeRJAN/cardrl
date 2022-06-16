#ifndef _PLAYER_H
#define _PLAYER_H

#include<vector>
#include<string>
#include "pile.h"

class player { // TODO: There surely is a cleaner way than having 20000 variables for status effects
public:
    std::string name;
    int vulnerable = 0;
    int drawcards = 4;
    int nlevel = 0; // level of enemies (not of descent!)
    int drawlimit = 9; // how many cards can be in hand at once
    int barricade = 0; // don't lose block for x turns
    int gold = 0;
    int level = 0; // each act has a number of levels, stored here
    int dont_discard_hand = 0; // don't discard hand for x turns
    int dont_draw = 0;
    int act = 0; // game is split into 3 acts, this stores the act number
    int poison = 0;
    int frail = 0; // weaken block cards
    int maxmana = 2;
    int mana;
    int weak = 0; // -1 weak each turn
    int maxhp = 50;
    int hp = maxhp;
    int block = 0;
    int strength = 0;
    bool confused = false; // confusion effect

    void begin_turn(pile*);
    void end_turn();
    void take_damage(int);
    void take_damage_forced(int); // piercing damage / poison
    int mult_dmg_from(int dmg); // multiply outgoing damage
    int mult_dmg_to(int);
    void addblock(int);
    void remove_mana(int);
    void clear_block();
    int mult_block(int blc);
    void decrease_counters();
    void pl_discard_hand(pile* plc);
};
#endif

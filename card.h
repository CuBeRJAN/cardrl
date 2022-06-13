#ifndef _CARD_H
#define _CARD_H
#include<vector>
#include<string>

#define EFFECT_LENGTH 100

class card {
public:
    card(std::string n, std::string d, std::vector<int> vec, std::string ef, int c, int r, std::string cl, int tp);

    std::vector<int> values;
    int rarity; // 0 common, 1 uncommon, 2 rare, 3 very rare, 4 - upgraded cards
    int type; // 0 attack, 1 skill
    int cost;
    std::string color;
    std::string name;
    std::string desc;
    char effect[EFFECT_LENGTH];
};
#endif

#include<vector>
#include<string>
#include<cstring>
#include "card.h"

card::card(std::string n, std::string d, std::vector<int> vec, std::string ef, int c, int r, std::string cl, int tp) {
    name = n;
    type = tp;
    color = cl;
    rarity = r;
    desc = d;
    cost = c;
    values = vec;
    strcpy(effect, ef.c_str());
}

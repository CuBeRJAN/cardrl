#ifndef _PILE_H
#define _PILE_H
#include<vector>
#include "card.h"

// Player card piles
class pile {
public:
    std::vector<card> hand;
    std::vector<card> discard;
    std::vector<card> deck;
    std::vector<card> draw;
};
#endif

#include<iostream>
#include<chrono>
#include<algorithm>
#include<random>
#include<stdlib.h>
#include<string>
#include<cstdlib>
#include<string.h>
#include<vector>

#define EFFECT_LENGTH 100

using std::cout;    using std::cin;
using std::string;  using std::vector;


class card {
public:
    card (string n, string d, string ef) {
        name = n;
        desc = d;
        strcpy(effect, ef.c_str());
    }

    string name;
    string desc;
    char effect[EFFECT_LENGTH];
};

class player {
public:
    string name;
    int drawcards = 4;
    int drawlimit = 10;
    int poison = 0;
    int maxmana = 4;
    int mana;
    int hp = 25;
    int block = 0;
    vector<card> deck;

    void damage(int dmg) {
        hp -= dmg;
    }

    void addblock(int blc) {
        block += blc;
    }

    void remove_mana(int n) {
        mana -= n;
    }
};


class enemy {
public:
    string name;
    int poison = 0;
    int maxmana = 4; // Enemies have mana, may be useful some day
    int mana;
    int hp = 25;
    int block = 0;

    void damage(int dmg) {
        hp -= dmg;
    }

    void addblock(int blc) {
        block += blc;
    }

    void remove_mana(int n) {
        mana -= n;
    }
};


class pile {
public:
    vector<card> hand;
    vector<card> discard;
    vector<card> deck;
    vector<card> draw;
};

// Evaluate effect of card
// d = dmg
// p = pois
// b = block
// last letter is return value ([D]iscard, [E]xhaust, [R]eturn)
// 92d3b3p means 11 damage, 3 block and 3 poison
char eval_effect(char effect[EFFECT_LENGTH], player* plr, enemy* enemy) {
    int tmpnum = 0;
    for (int i = 0; i < EFFECT_LENGTH-1; i++) {
        if (effect[i] == '\0') break;
        if (isdigit(effect[i])) tmpnum += (effect[i] - '0');
        else {
            if (effect[i] == 'd') { enemy->damage(tmpnum); tmpnum = 0; }
            else if (effect[i] == 'b') { plr->addblock(tmpnum); tmpnum = 0; }
        }
    }
    return effect[EFFECT_LENGTH];
}

int efnum_to_int(string efnum) {
    int ret = 0;
    for (long unsigned int i = 0; i < efnum.size(); i++) {
        ret += (int)(efnum[i] - '0');
    }
    return ret;
}

string int_to_efnum(int real) {
    string ri = std::to_string(real);
    string concat = "";
    for (int i = 0; i < ((real-(real % 9)) / 9); i++) {
        concat += "9";
    }
    concat += std::to_string(real % 9);
    return concat;
}

void shuffle_deck(vector<card>* dc) {
    std::random_shuffle(dc->begin(), dc->end());
}

void start_fight(player* pl, pile* pl_cards) {
    pl_cards->hand.clear();
    pl_cards->discard.clear();
    pl_cards->draw = pl_cards->deck;
    shuffle_deck(&pl_cards->deck);
}

void draw_hand(player* pl, pile* pl_cards) {
    for (int i = 0; i < pl->drawcards; i++) {
        if (pl_cards->draw.size() > 0) {
            pl_cards->hand.push_back(pl_cards->draw.at(0));
            pl_cards->draw.erase(pl_cards->draw.begin());
        }
        else {
            pl_cards->draw = pl_cards->discard;
            shuffle_deck(&pl_cards->discard);
            pl_cards->hand.push_back(pl_cards->draw.at(0));
            pl_cards->draw.erase(pl_cards->draw.begin());

        }
    }
}

void discard_hand(player* pl, pile* pl_cards) {
    while (pl_cards->hand.size() > 0) {
        pl_cards->discard.push_back(pl_cards->hand.at(0));
        pl_cards->hand.erase(pl_cards->hand.begin());
    }
}

void print_game(player* pl, pile* pl_cards, enemy* en) {
    cout << "HP: " << pl->hp << "\t\tEnemy HP: " << en->hp << "\n";
    cout << "Block: " << pl->block << "\n";
    cout << "Mana: " << pl->mana << "\n";
    for (unsigned long int i = 0; i < pl_cards->hand.size(); i++) {
        cout << "(" << i+1 << ") " << pl_cards->hand.at(i).name << " :: " << pl_cards->hand.at(i).desc << std::endl;
    }
}

void start_turn(player* pl, pile* pl_cards) {
    discard_hand(pl, pl_cards);
    draw_hand(pl, pl_cards);
    pl->block = 0;
    pl->mana = pl->maxmana;
}

char eval_card(player* pl, enemy* en, card crd) {
    return eval_effect(crd.effect, pl, en);
}

void discard_from_hand(pile* pl_cards, int index) {
    pl_cards->discard.push_back(pl_cards->hand.at(index));
    pl_cards->hand.erase(pl_cards->hand.begin() + index);
}

void play_card_from_hand(player* pl, pile* pl_cards, enemy* en, int index) {
    if (eval_card(pl, en, pl_cards->hand.at(index)) == 'D')
        discard_from_hand(pl_cards, index);
}

int main() {
    /* Define cards TODO: read from a database file */
    vector<card> cards;
    cards.push_back(card("Strike", "Deal 6 damage","6dD"));
    cards.push_back(card("Defend", "Get 6 block","6bD"));
    cards.push_back(card("Iron mask", "Get 10 block and discard another card", "10bD"));

    // Initialize player and deck
    player pl;
    pile pl_pile;
    for (int i = 0; i < 3; i++)
        pl_pile.deck.push_back(cards.at(0));
    for (int i = 0; i < 3; i++)
        pl_pile.deck.push_back(cards.at(1));
    pl_pile.deck.push_back(cards.at(2));
    pl_pile.deck.push_back(cards.at(2));
    pl_pile.deck.push_back(cards.at(2));


    start_fight(&pl, &pl_pile);
    enemy en_main;
    bool fight = true;
    char choice;
    pl.damage(3);
    while (fight) {
        start_turn(&pl, &pl_pile);
        print_game(&pl, &pl_pile, &en_main);
        cin >> choice;
        play_card_from_hand(&pl, &pl_pile, &en_main, (int)(choice - '0')-1);
    }
}

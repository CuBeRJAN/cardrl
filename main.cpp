#include<iostream>
#include<chrono>
#include<algorithm>
#include<random>
#include<stdlib.h>
#include<string>
#include<cstdlib>
#include<string.h>
#include<vector>
#include "cpptree.h" // taken from https://github.com/CuBeRJAN/cpptree (it's not stealing if it's my own code)

#define EFFECT_LENGTH 100

// This game is a Slay the Spire ripoff

using std::cout;    using std::cin;
using std::string;  using std::vector;


void cls() {
    //system("clear");
}

class card {
public:
    card (string n, string d, string ef, int c) {
        name = n;
        desc = d;
        cost = c;
        strcpy(effect, ef.c_str());
    }

    int cost;
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
    int maxmana = 3;
    int mana;
    int hp = 25;
    int maxhp = 25;
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
    int maxmana = 4; // Enemies have mana, may be useful some time
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

// Player card piles
class pile {
public:
    vector<card> hand;
    vector<card> discard;
    vector<card> deck;
    vector<card> draw;
};

void discard_from_hand(pile* pl_cards, int index);
void exhaust_from_hand(pile* pl_cards, int index);

void buffer_send(string tosend) {
    cout << tosend;
}

char get_onechar() {
    char ret;
    cin >> ret;
    return ret;
}

int select_from_hand(pile* plc, string msg) {
    buffer_send(msg);
    char inp = get_onechar();
    return (inp - '0' - 1);
}

// Evaluate effect of card
// d = dmg
// p = pois
// b = block
// last letter is return value ([D]iscard, [E]xhaust, [R]eturn)
// 92d3b3p means 11 damage, 3 block and 3 poison
// [b]lock, [d]amage, dis[c]ard other card(s), [p]oison
// [h]eal, e[x]haust other card(s)
char eval_effect(char effect[EFFECT_LENGTH], player* plr, enemy* enemy, pile* pl_pile) {
    int tmpnum = 0;
    int mx;
    for (int i = 0; i < EFFECT_LENGTH-1; i++) {
        if (effect[i] == '\0') break;
        if (isdigit(effect[i])) tmpnum += (effect[i] - '0');
        else {
            if (effect[i] == 'd') { enemy->damage(tmpnum); tmpnum = 0; }
            else if (effect[i] == 'b') { plr->addblock(tmpnum); tmpnum = 0; }
            else if (effect[i] == 'c') {
                for (int i = 0; i < tmpnum; i++) {
                    if (pl_pile->hand.size() > 1) {
                        int choice = select_from_hand(pl_pile, "Select a card to discard: ");
                        discard_from_hand(pl_pile, choice);
                    }
                    else break;
                }
                tmpnum = 0;
            }
        }
        mx = i;
    }

    return effect[mx];
}

// convert effect number to int (991 to 19 etc.)
int efnum_to_int(string efnum) {
    int ret = 0;
    for (long unsigned int i = 0; i < efnum.size(); i++) {
        ret += (int)(efnum[i] - '0');
    }
    return ret;
}

// convert int to effect number (19 to 991 etc.)
string int_to_efnum(int real) {
    string ri = std::to_string(real);
    string concat = "";
    for (int i = 0; i < ((real-(real % 9)) / 9); i++) {
        concat += "9";
    }
    concat += std::to_string(real % 9);
    return concat;
}

// doesn't work?
void shuffle_deck(vector<card>* dc) {
    std::random_shuffle(dc->begin(), dc->end());
}

// initialize some stuff at the beginning of a fight
void start_fight(player* pl, pile* pl_cards) {
    pl_cards->hand.clear();
    pl_cards->discard.clear();
    pl_cards->draw = pl_cards->deck;
    shuffle_deck(&pl_cards->deck);
}

// draw a full hand of cards
// TODO: breaks if not enough cards in draw+discard pile
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

// discard entire hand
void discard_hand(player* pl, pile* pl_cards) {
    while (pl_cards->hand.size() > 0) {
        pl_cards->discard.push_back(pl_cards->hand.at(0));
        pl_cards->hand.erase(pl_cards->hand.begin());
    }
}

void print_game(player* pl, pile* pl_cards, enemy* en) {
    cls();
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
    pl->block = 0; // clear block at the start of turn
    pl->mana = pl->maxmana;
}

char eval_card(player* pl, pile* pl_pile, enemy* en, card crd) {
    pl->mana-=crd.cost;
    return eval_effect(crd.effect, pl, en, pl_pile);
}

void discard_from_hand(pile* pl_cards, int index) {
    pl_cards->discard.push_back(pl_cards->hand.at(index));
    pl_cards->hand.erase(pl_cards->hand.begin() + index);
}


void exhaust_from_hand(pile* pl_cards, int index) {
    pl_cards->discard.push_back(pl_cards->hand.at(index));
    pl_cards->hand.erase(pl_cards->hand.begin() + index);
}

void play_card_from_hand(player* pl, pile* pl_cards, enemy* en, int index) {
    if (pl->mana >= pl_cards->hand.at(index).cost)
        if (eval_card(pl, pl_cards, en, pl_cards->hand.at(index)) == 'D')
            discard_from_hand(pl_cards, index);
}

int main() {
    /* Define cards TODO: read from a database file */
    vector<card> cards;
    // TODO: define card pairings for upgraded variants
    //       possibly use a vector which will point to each card's variants?
    cards.push_back(card("Strike", "Deal 6 damage","6dD",1)); // last value is mana cost
    cards.push_back(card("Defend", "Get 5 block","5bD",1));
    cards.push_back(card("Iron mask", "Get 10 block and discard another card", "10b1cD",1));
    cards.push_back(card("Strike+", "Deal 9 damage","9dD",1));
    cards.push_back(card("Defend+", "Get 8 block","8bD",1));
    cards.push_back(card("Iron mask+", "Get 13 block and discard another card", "13b1cD",1));


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

    // Initialize enemy
    enemy en_main;


    start_fight(&pl, &pl_pile);
    bool fight = true;
    char choice;
    while (fight) {
        start_turn(&pl, &pl_pile);
        print_game(&pl, &pl_pile, &en_main);
        while (choice != 'q') {
            cin >> choice;
            if (choice > '0' && choice < (char)('1' + pl_pile.hand.size()))
                play_card_from_hand(&pl, &pl_pile, &en_main, (int)(choice - '0')-1);
            print_game(&pl, &pl_pile, &en_main);
        }
        choice = '0';
    }
}

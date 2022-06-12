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

// This game is a Slay the Spire ripoff

using std::cout;    using std::cin;
using std::string;  using std::vector;

#include  <iterator>

// randomization code here is copied from stackoverflow
template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

void cls() {
    //system("clear");
}

void end_game() {
    cls();
    cout << "You lost!\n";
    exit(0);
}


class card {
public:
    card (string n, string d, string ef, int c, int r) {
        name = n;
        rarity = r;
        desc = d;
        cost = c;
        strcpy(effect, ef.c_str());
    }

    int rarity; // 0 common, 1 uncommon, 2 rare, 3 very rare, 4 - upgraded cards
    int cost;
    string name;
    string desc;
    char effect[EFFECT_LENGTH];
};

// Player card piles
class pile {
public:
    vector<card> hand;
    vector<card> discard;
    vector<card> deck;
    vector<card> draw;
};

class player {
public:
    string name;
    int drawcards = 4;
    int drawlimit = 10;
    int barricade = 0; // don't lose block for x turns
    int blockdiscard = 0; // don't discard hand for x turns
    int poison = 0;
    int frail = 0; // weaken block cards
    int maxmana = 3;
    int mana;
    int weak; // -1 weak each turn
    int hp = 25;
    int maxhp = 25;
    int block = 0;
    int strength = 0;

    void damage(int dmg, int strength) {
        hp -= (dmg + (dmg * (0.2 * strength)));
    }

    void addblock(int blc) {
        if (!frail)
            block += blc;
        else
            block += blc * 0.7;
    }

    void remove_mana(int n) {
        mana -= n;
    }
};

class enemy;
char eval_effect(char effect[EFFECT_LENGTH], player* plr, enemy* enemy, pile* pl_pile);
class enemy {
public:
    string name;
    int poison = 0;
    int maxmana = 4; // Enemies have mana, may be useful some time
    int mana;
    int hp;
    int maxhp;
    int barricade = 0;
    int block = 0;
    int strength = 0;
    int level;
    int frail = 0; // Lower block effectiveness
    string intention;
    vector<string> actions;

    enemy(string n, int mhp, int l, vector<string> ac) {
        name = n;
        maxhp = mhp;
        level = l;
        actions = ac;
        hp = maxhp;
    }

    // void do_random_action(player* pl, pile* plc);

    void damage(int dmg, int strength) {
        hp -= (dmg + (dmg * (0.2 * strength)));
    }

    void addblock(int blc) {
        if (!frail)
            block += blc;
        else
            block += blc * 0.7;
    }

    void remove_mana(int n) {
        mana -= n;
    }

    string get_intention() {
        intention = *select_randomly(actions.begin(),actions.end());
        return intention;
    }

    void commit_intention(player* pl, pile* plc) {
        char ef[EFFECT_LENGTH];
        strcpy(ef,intention.c_str());
        eval_effect(ef,pl,this,plc);
    }

    void end_turn() {
        if (barricade) barricade--;
        else block = 0;
    }
};


void discard_from_hand(pile* pl_cards, int index);
void exhaust_from_hand(pile* pl_cards, int index);

void buffer_send(string tosend) {
    cout << tosend;
}

string msgbuffer; // global variable!!
void buffer_queue(string q) {
    msgbuffer = q;
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
// last char is return value ([D]iscard, [E]xhaust, [R]eturn to hand)
// 92d3b3p means 11 damage, 3 block and 3 poison
// [b]lock, [d]amage enemy, dis[c]ard other card(s), [p]oison
// [h]eal, e[x]haust other card(s), [H]eal enemy, [D]amage player
// [B]lock enemy
char eval_effect(char effect[EFFECT_LENGTH], player* plr, enemy* en, pile* pl_pile) {
    int tmpnum = 0;
    int mx;
    for (int i = 0; i < EFFECT_LENGTH-1; i++) {
        if (effect[i] == '\0') break;
        if (isdigit(effect[i])) tmpnum += (effect[i] - '0');
        else {
            if (effect[i] == 'd') { en->damage(tmpnum, plr->strength); tmpnum = 0; }
            else if (effect[i] == 'D') { plr->damage(tmpnum, en->strength); tmpnum = 0; }
            else if (effect[i] == 'b') { plr->addblock(tmpnum); tmpnum = 0; }
            else if (effect[i] == 'B') { en->addblock(tmpnum); tmpnum = 0; }
            else if (effect[i] == 'c') {
                for (int i = 0; i < tmpnum; i++) {
                    if (pl_pile->hand.size() > 0) {
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
    //string ri = std::to_string(real);
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

// make intention into readable string
string enemy_intention_to_string(string intend) {
    int tmpnum = 0;
    string ret = "";
    int mx;
    for (int i = 0; i < EFFECT_LENGTH-1; i++) {
        if (intend[i] == '\0') break;
        if (isdigit(intend[i])) tmpnum += (intend[i] - '0');
        else {
            if (intend[i] == 'D') { ret += ("Attack for " + std::to_string(tmpnum) + " damage. || "); tmpnum = 0; }
            else if (intend[i] == 'B') { ret += ("Block for " + std::to_string(tmpnum) + " damage. || "); tmpnum = 0; }
        }
        mx = i;
    }

    return ret;
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
    cout << "HP: " << pl->hp << "\t\t\t\t\tEnemy HP: " << en->hp << "\n";
    cout << "Block: " << pl->block << "\t\t\t\tEnemy block:" << en->block << "\n";
    cout << "Mana: " << pl->mana << "\t\t\t\t\tEnemy intention: " << enemy_intention_to_string(en->intention) << "\n";
    for (unsigned long int i = 0; i < pl_cards->hand.size(); i++) {
        cout << "(" << i+1 << ") " << pl_cards->hand.at(i).name << " ::\t" << pl_cards->hand.at(i).desc
             << "\t\t:: Mana cost: " << pl_cards->hand.at(i).cost << std::endl;
    }
}


// Start of turn function
void start_turn(player* pl, pile* pl_cards) {
    if (!pl->blockdiscard) // discard hand unless discard is blocked by effect
        discard_hand(pl, pl_cards);
    else
        pl->blockdiscard--;
    draw_hand(pl, pl_cards); // draw new cards
    pl->hp -= pl->poison; // apply poison
    if (pl->poison) pl->poison--; // decrease poison if any is applied
    if (pl->weak) pl->weak--; // decrease weaken
    if (pl->barricade) pl->barricade--; // decrease barricade turns left
    else pl->block = 0; // else remove all block
    if (pl->hp < 1) end_game(); // end game if hp <= 0
    pl->mana = pl->maxmana; // refresh mana
}

char eval_card(player* pl, pile* pl_pile, enemy* en, card crd) {
    pl->mana-=crd.cost;
    print_game(pl, pl_pile, en);
    return eval_effect(crd.effect, pl, en, pl_pile);
}

// discard x card from hand
void discard_from_hand(pile* pl_cards, int index) {
    pl_cards->discard.push_back(pl_cards->hand.at(index));
    pl_cards->hand.erase(pl_cards->hand.begin() + index);
}

// same thing but exhaust
void exhaust_from_hand(pile* pl_cards, int index) {
    pl_cards->discard.push_back(pl_cards->hand.at(index));
    pl_cards->hand.erase(pl_cards->hand.begin() + index);
}

// activate card from hand
void play_card_from_hand(player* pl, pile* pl_cards, enemy* en, int index) {
    if (pl->mana >= pl_cards->hand.at(index).cost) {
        card cr = pl_cards->hand.at(index); // Card has to be discarded / exhausted before effect is evaluated, so we make a copy we use to evaluate
        cr = pl_cards->hand.at(index);
        if (pl_cards->hand.at(index).effect[strlen(pl_cards->hand.at(index).effect)-1] == 'D')
            discard_from_hand(pl_cards, index);
        eval_card(pl, pl_cards, en, cr);
    }
    else {
        buffer_queue("Not enough mana to play card.");
    }
}

int main() {
    std::vector<enemy> enemies;
    // Name, HP, level, effects
    enemies.push_back(enemy("Goblin",20,1,{"6D","5B"}));
    /* Define cards TODO: read from a database file */
    vector<card> cards;
    // TODO: define card pairings for upgraded variants
    //       possibly use a vector which will point to each card's variants?
    cards.push_back(card("Strike", "Deal 6 damage","6dD",1,0)); // before last value is mana cost, last is rarity
    cards.push_back(card("Defend", "Get 5 block","5bD",1,0));
    cards.push_back(card("Iron mask", "Get 10 block and discard another card", "91b1cD",1,0));
    cards.push_back(card("Strike+", "Deal 9 damage","9dD",1,4));
    cards.push_back(card("Defend+", "Get 8 block","8bD",1,4));
    cards.push_back(card("Iron mask+", "Get 13 block and discard another card", "94b1cD",1,4));


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
    enemy en_main = enemies.at(0);


    start_fight(&pl, &pl_pile);
    bool fight = true;
    char choice;
    while (fight) {
        start_turn(&pl, &pl_pile);
        en_main.get_intention();
        print_game(&pl, &pl_pile, &en_main);
        while (choice != 'q') {
            cin >> choice;
            if (choice > '0' && choice < (char)('1' + pl_pile.hand.size()))
                play_card_from_hand(&pl, &pl_pile, &en_main, (int)(choice - '0')-1);
            print_game(&pl, &pl_pile, &en_main);
        }
        en_main.commit_intention(&pl, &pl_pile);
        en_main.end_turn();
        choice = '0';
    }
}

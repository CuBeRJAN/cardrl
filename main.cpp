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

struct {
    const string cyan = "\033[36m";
    const string magenta = "\033[35m";
    const string red = "\033[91m";
    const string gray = "\033[8m";
    const string green = "\033[92m";
    const string yellow = "\033[33m";
    const string end = "\033[0m";
} colors;

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
    system("clear");
}

void end_game() {
    cls();
    cout << "You lost!\n";
    exit(0);
}


class card {
public:
    card (string n, string d, string ef, int c, int r, string cl, int tp) {
        name = n;
        type = tp;
        color = cl;
        rarity = r;
        desc = d;
        cost = c;
        strcpy(effect, ef.c_str());
    }

    int rarity; // 0 common, 1 uncommon, 2 rare, 3 very rare, 4 - upgraded cards
    int type; // 0 attack, 1 skill
    int cost;
    string color;
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
    int drawlimit = 10; // how many cards can be in hand at once
    int barricade = 0; // don't lose block for x turns
    int gold = 0;
    int level = 0; // each act has a number of levels, stored here
    int blockdiscard = 0; // don't discard hand for x turns
    int act = 0; // game is split into 3 acts, this stores the act number
    int poison = 0;
    int frail = 0; // weaken block cards
    int maxmana = 3;
    int mana;
    int weak; // -1 weak each turn
    int hp = 50;
    int maxhp = 50;
    int block = 0;
    int strength = 0;

    void damage(int dmg, int strength) {
        int rdmg = (dmg + (dmg * (0.2 * strength)));
        int obl = block;
        if (rdmg >= block) {
            block = 0;
            hp -= rdmg-obl;
        }
        else
            block -= rdmg;
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

void shuffle_stringvec(vector<string>*);

class enemy;
void eval_effect(char effect[EFFECT_LENGTH], player* plr, enemy* enemy, pile* pl_pile);
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
    int intention_counter_max;
    int intention_counter = 0;

    enemy(string n, int mhp, int l, vector<string> ac) {
        name = n;
        maxhp = mhp;
        level = l;
        actions = ac;
        intention_counter_max = ac.size();
        hp = maxhp;
    }

    void damage(int dmg, int strength) {
        int rdmg = (dmg + (dmg * (0.2 * strength)));
        int obl = block;
        if (rdmg >= block) {
            block = 0;
            hp -= rdmg-obl;
        }
        else
            block -= rdmg;
    }

    bool check_hp() {
        if (hp < 1) {
            return false;;
        }
        return true;
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
        if (intention_counter >= intention_counter_max) {
            shuffle_stringvec(&actions);
            intention_counter = 0;
        }
        else {
            intention = actions.at(intention_counter);
            intention_counter++;
        }
        intention = *select_randomly(actions.begin(),actions.end());
        return intention;
    }

    void commit_intention(player* pl, pile* plc) {
        char ef[EFFECT_LENGTH];
        strcpy(ef,intention.c_str());
        eval_effect(ef,pl,this,plc);
    }

    void start_turn() {
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

void buffer_flush() {
    msgbuffer = "";
}
void buffer_queue();
// Evaluate effect of card
// d = dmg
// p = pois
// b = block
// last char is return value ([D]iscard, [E]xhaust, [R]eturn to hand)
// 92d3b3p means 11 damage, 3 block and 3 poison
// [b]lock, [d]amage enemy, dis[c]ard other card(s), [p]oison
// [h]eal, e[x]haust other card(s), [H]eal enemy, [D]amage player
// [B]lock enemy
void eval_effect(char effect[EFFECT_LENGTH], player* plr, enemy* en, pile* pl_pile) {
    buffer_flush();
    int tmpnum = 0;
    for (int i = 0; i < EFFECT_LENGTH-1; i++) {
        if (effect[i] == '\0') break;

        if (isdigit(effect[i])) tmpnum += (effect[i] - '0');
        else {
            if (effect[i] == 'd') { en->damage(tmpnum, en->strength);
                buffer_queue(colors.red+"You hit for "+std::to_string(tmpnum)+" damage"+colors.end); tmpnum = 0; }
            else if (effect[i] == 'D') { plr->damage(tmpnum, en->strength);
                buffer_queue(colors.red+"You take "+std::to_string(tmpnum)+" damage"+colors.end); tmpnum = 0; }
            else if (effect[i] == 'b') { plr->addblock(tmpnum );
                buffer_queue(colors.cyan+"You gain "+std::to_string(tmpnum)+" block"+colors.end); tmpnum = 0; }
            else if (effect[i] == 'B') { en->addblock(tmpnum );
                buffer_queue(colors.cyan+"Enemy gains "+std::to_string(tmpnum)+" block"+colors.end); tmpnum = 0; }
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
    }

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

void shuffle_stringvec(vector<string>* dc) {
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
            else if (intend[i] == 'B') { ret += ("Apply " + std::to_string(tmpnum) + " block. || "); tmpnum = 0; }
        }
        mx = i;
    }

    ret.erase(ret.end() - 3, ret.end());
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
    cout << colors.green << "Act: " << pl->act+1 << "/3" << "\t\t\t" << "Level: " << pl->level << colors.red
         << "\t\t\tDeck: " << pl_cards->deck.size() << " cards";
    cout << colors.yellow << "\t\t\tGold: " << pl->gold << colors.end << "\n";
    cout << string(111, '-') << "\n";
    cout << colors.yellow << pl->name << " the ironclad\t\t\t\t\t\t\t\t\t\t" << en->name << colors.end << std::endl;
    cout << "HP: " << colors.green << pl->hp << colors.end << "\t\t\t\t\t\t\t\t\t\t\t\tEnemy HP: " << colors.green << en->hp << colors.end << "\n";
    cout << "Block: " << colors.cyan << pl->block << colors.end << "\t\t\t\t\t\t\t\t\t\t\tEnemy block: " << colors.cyan << en->block << colors.end << "\n";
    cout << "Mana: " << colors.magenta << pl->mana << colors.end << "\t\t\t\t\t\t\t\t\t\t\t\tEnemy intent: " << colors.magenta << enemy_intention_to_string(en->intention) << colors.end << "\n";
    cout << string(111, '-') << "\n";
    string mydesc;
    for (unsigned long int i = 0; i < pl_cards->hand.size(); i++) {
        mydesc = pl_cards->hand.at(i).desc;
        while (mydesc.length() < 60) mydesc += " ";
        cout << pl_cards->hand.at(i).color << "(" << i+1 << ") " << pl_cards->hand.at(i).name << colors.end << "\t\t\t:: "
             << mydesc << colors.magenta
             << "\t" << ":: Mana cost: " << pl_cards->hand.at(i).cost << colors.end << std::endl;
    }
    cout << "\n\n" << msgbuffer << "\n";
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

void eval_card(player* pl, pile* pl_pile, enemy* en, card crd) {
    pl->mana-=crd.cost;
    print_game(pl, pl_pile, en);
    char tmpef[EFFECT_LENGTH];
    strcpy(tmpef, crd.effect);
    tmpef[strlen(tmpef)-1] = '\0';
    eval_effect(tmpef, pl, en, pl_pile);
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
        buffer_flush();
        buffer_queue("Not enough mana to play card.");
    }
}

void create_fight(player* pl, pile* plc, enemy* en_main) {
    start_fight(pl, plc);
    bool fight = true;
    char choice;
    while (fight) {
        start_turn(pl, plc);
        en_main->get_intention();
        print_game(pl, plc, en_main);
        while (choice != 'q') {
            cin >> choice;
            if (choice > '0' && choice < (char)('1' + plc->hand.size()))
                play_card_from_hand(pl, plc, en_main, (int)(choice - '0')-1);
            if (!en_main->check_hp()) {
                cout << colors.red << "You kill the " << en_main->name << "!\n" << colors.end;
                fight = false;
                break;
            }
            print_game(pl, plc, en_main);
        }
        en_main->start_turn();
        en_main->commit_intention(pl, plc);
        choice = '0';
    }

}

// TODO: define card pairings for upgraded variants
//       possibly use a vector which will point to each card's variants?
void init_game(vector<enemy>* env, vector<card>* crds) {
    // Name, HP, level, effects
    env->push_back(enemy("Goblin",20,1,{"6D","5B"}));
    /* Define cards TODO: read from a database file */
    // TODO: define card pairings for upgraded variants
    //       possibly use a vector which will point to each card's variants?
    //       name - desc - mana - rarity - color - type (0 attack, 1 skill)
    crds->push_back(card("Strike", "Deal 6 damage","6dD",1,0, colors.red,0));
    crds->push_back(card("Defend", "Get 5 block","5bD",1,0, colors.cyan,1));
    crds->push_back(card("Iron mask", "Get 10 block and discard another card", "91b1cD",1,0, colors.cyan,1));
    crds->push_back(card("Strike+", "Deal 9 damage","9dD",1,4,colors.red,0));
    crds->push_back(card("Defend+", "Get 8 block","8bD",1,4,colors.cyan,1));
    crds->push_back(card("Iron mask+", "Get 13 block and discard another card", "94b1cD",1,4,colors.cyan,1));

}

// Create deck at the beginning of the game
pile create_deck(vector<card> crds) {
    pile pl_pile;
    for (int i = 0; i < 3; i++)
        pl_pile.deck.push_back(crds.at(0));
    for (int i = 0; i < 3; i++)
        pl_pile.deck.push_back(crds.at(1));
    pl_pile.deck.push_back(crds.at(2));
    return pl_pile;
}

// Create player at the beginning of the game
player create_player() {
    player pl;
    cout << "Enter your name:\n";
    cin >> pl.name;
    return pl;
}

int main() {
    std::vector<enemy> enemies;
    vector<card> cards;

    init_game(&enemies, &cards);

    // Initialize player and deck
    player pl = create_player();
    pile pl_pile = create_deck(cards);
    // Initialize enemy
    enemy en_main = enemies.at(0); // Goblin enemy


    create_fight(&pl, &pl_pile, &en_main);
}

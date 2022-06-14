// komentáře jsou v angličtině protože jsem na to zvyklý
// projekt je možná zbytečně dlouhý, ale jen protože mě docela baví na něm pracovat
// některé části kódu jsou "vypůjčené" ale jsou to asi jen 2 funkce a je u nich komentář
#include<iostream>
#include<sstream>
#include<time.h>
#include<chrono>
#include<algorithm>
#include<random>
#include<stdlib.h>
#include<string>
#include<cstdlib>
#include<string.h>
#include<vector>
#include<fstream>
#include<iterator>
#include "cpptree.h" // https://github.com/CuBeRJAN/cpptree
                     // it's not stealing if it's my own library
                     //
                     // I know full definitions shouldn't be in headers,
                     // but it's not possible to declare a template class otherwise
                     // https://isocpp.org/wiki/faq/templates#templates-defn-vs-decl
#include "cardrl.h"
#include "card.h"
#include "pile.h"
#include "player.h"
#include "enemy.h"


// global variables
// defining in header cause issues for some reason
std::vector<card> cards; // global variable, all the cards in the game
std::vector<enemy> enemies; // all enemies in the game
// randomness seed
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine shuffle_myseed(seed);
std::string msgbuffer;



using std::cout;    using std::cin;
using std::string;  using std::vector;

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

void end_game() {
    cls();
    cout << "You lost!\n";
    exit(0);
}

void buffer_send(string tosend) {
    cout << tosend;
}

void buffer_queue(string q) {
    check_bufferlen();
    msgbuffer += q + "\n";
}

char get_onechar() {
    char ret;
    cin >> ret;
    return ret;
}

int select_from_hand(pile* plc, string msg) {
    buffer_send(msg);
    char inp = key_press();
    return (inp - '0' - 1);
}

void buffer_flush() {
    msgbuffer = "";
}

void check_bufferlen() {
    int lines = 0;
    for (unsigned long int i = 0; i < msgbuffer.length(); i++) {
        if (msgbuffer.at(i) == '\n') lines++;
    }
    int it = msgbuffer.length() - 2;
    bool lessline = false;
    bool r = false;
    while (lines > 4) {
        r = true;
        if (msgbuffer.at(it) == '\n') lessline = true;
        msgbuffer.erase(msgbuffer.begin() + it, msgbuffer.end());
        if (lessline) lines--;
        lessline = false;
        it--;
    }
    if (r)
        msgbuffer += '\n';
}

// Evaluate effect
// handles effects for cards, enemy attacks, random encounter effects, pretty much every effect in the game
// Effect is a C-style string, the description of how the effect string is interpreted is in the comment below
// d = dmg
// p = pois
// b = block
// 92da3bw3p means 11 damage, 3 block if enemy is weak, and 3 poison
// [b]lock, [d]amage enemy, dis[c]ard other cards, [p]oison
// [h]eal, e[x]haust other card(s), [H]eal enemy, [D]amage player
// [B]lock enemy, add [s]trength, add [S]trength to enemy
// [w]eaken player, [W]eaken enemy, gain [m]ana
// draw a [C]ard, enemy [l]ose str, player [L]ose str

// there are 2 letters at the end, the second is a condition!
// w means do only if enemy is [w]eak etc.
// [a]lways
void eval_effect(char effect[EFFECT_LENGTH], player* plr, enemy* en, pile* pl_pile) {
    check_bufferlen();
    int tmpnum = 0;
    for (int i = 0; i < EFFECT_LENGTH - 1; i++) {
        if (effect[i] == '\0') break;

        if (isdigit(effect[i])) tmpnum += (effect[i] - '0');
        // Here we check the conditions
        else if (effect[i + 1] == '\0' || (isdigit(effect[i - 1]) && ( // Implement conditions here (yes, it's ugly)
                                                                       (effect[i + 1] == 'a') || // "always" condition i.e. no condition
                                                                       (effect[i + 1] == 'w' && en->weak) || // check for enemy weaken condition
                                                                       (effect[i + 1] == 'W' && plr->weak)
            ))) {
            switch (effect[i]) {
                case 'd':
                    en->take_damage(plr->mult_dmg_from(tmpnum));
                    buffer_queue(colors.red + "You hit for " + std::to_string(en->mult_dmg_to(plr->mult_dmg_from(tmpnum))) + " damage" + colors.end); tmpnum = 0;
                    break;
                case 'D':
                    plr->take_damage(en->mult_dmg_from(tmpnum));
                    buffer_queue(colors.red + "You take " + std::to_string(plr->mult_dmg_to(en->mult_dmg_from(tmpnum))) + " damage" + colors.end); tmpnum = 0;
                    break;
                case 'b':
                    plr->addblock(tmpnum);
                    buffer_queue(colors.cyan + "You gain " + std::to_string(plr->mult_block(tmpnum)) + " block" + colors.end); tmpnum = 0;
                    break;
                case 'B':
                    en->addblock(tmpnum);
                    buffer_queue(colors.cyan + "Enemy gains " + std::to_string(en->mult_block(tmpnum)) + " block" + colors.end); tmpnum = 0;
                    break;
                case 's':
                    plr->strength += tmpnum;
                    buffer_queue(colors.magenta + "You gain " + std::to_string(tmpnum) + " strength" + colors.end); tmpnum = 0;
                    break;
                case 'S':
                    en->strength += tmpnum;
                    buffer_queue(colors.magenta + "Enemy gains " + std::to_string(tmpnum) + " strength" + colors.end); tmpnum = 0;
                    break;
                case 'w':
                    plr->weak += tmpnum + 1; // +1 because weaken gets removed at the start of player turn
                    buffer_queue(colors.magenta + "Enemy weakens you for " + std::to_string(tmpnum) + " more turn(s)" + colors.end); tmpnum = 0;
                    break;
                case 'W':
                    en->weak += tmpnum + 1; // +1 because weaken gets removed at start of enemy turn
                    buffer_queue(colors.magenta + "You weaken enemy for " + std::to_string(tmpnum) + " more turn(s)" + colors.end); tmpnum = 0;
                    break;
                case 'l':
                    en->strength -= tmpnum;
                    buffer_queue(colors.red + "Enemy loses " + std::to_string(tmpnum) + " strength" + colors.end); tmpnum = 0;
                    break;
                case 'L':
                    plr->strength -= tmpnum;
                    buffer_queue(colors.red + "You lose " + std::to_string(tmpnum) + " strength" + colors.end); tmpnum = 0;
                    break;
                case 'p':
                    en->poison += tmpnum;
                    buffer_queue(colors.green + std::to_string(tmpnum) + " poison has been applied to enemy" + colors.end); tmpnum = 0;
                    break;
                case 'h':
                    plr->hp += tmpnum;
                    if (plr->hp > plr->maxhp) plr->hp = plr->maxhp; // HP cap
                    buffer_queue(colors.green + "You heal " + std::to_string(tmpnum) + " health" + colors.end); tmpnum = 0;
                    break;
                case 'H':
                    en->hp += tmpnum;
                    if (en->hp > en->maxhp) en->hp = en->maxhp; // HP cap
                    buffer_queue(colors.green + "Enemy heals " + std::to_string(tmpnum) + " health" + colors.end); tmpnum = 0;
                    break;
                case 'm':
                    plr->mana += 1; // mana is allowed over maxmana counter
                    tmpnum = 0;
                    break;
                case 'c':
                    for (int i = 0; i < tmpnum; i++) {
                        if (pl_pile->hand.size() > 0) {
                            int choice = select_from_hand(pl_pile, "Select a card to discard: ");
                            discard_from_hand(pl_pile, choice);
                        }
                        else break;
                    }
                    tmpnum = 0;
                    break;
            }
        }
        else tmpnum = 0;
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
    for (int i = 0; i < ((real - (real % 9)) / 9); i++) {
        concat += "9";
    }
    concat += std::to_string(real % 9);
    return concat;
}

// shuffle deck
void shuffle_deck(vector<card>* dc) {
    //unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(dc->begin(), dc->end(), shuffle_myseed);
}

void shuffle_stringvec(vector<string>* dc) {
    std::shuffle(dc->begin(), dc->end(), shuffle_myseed);
}

// initialize some stuff at the beginning of a fight
void start_fight(player* pl, pile* pl_cards) {
    pl->weak = 0;
    pl->poison = 0;
    pl->block = 0;
    pl->barricade = 0;
    pl_cards->hand.clear();
    pl_cards->discard.clear();
    pl_cards->draw = pl_cards->deck;
    shuffle_deck(&pl_cards->draw);
}

// draw a full hand of cards
void draw_hand(player* pl, pile* pl_cards) {
    for (int i = 0; i < pl->drawcards; i++) {
        if (pl_cards->draw.size() > 0) {
            pl_cards->hand.push_back(pl_cards->draw.at(0));
            pl_cards->draw.erase(pl_cards->draw.begin());
        }
        else if (pl_cards->discard.size() > 0) {
            pl_cards->draw = pl_cards->discard;
            shuffle_deck(&pl_cards->draw);
            pl_cards->hand.push_back(pl_cards->draw.at(0));
            pl_cards->draw.erase(pl_cards->draw.begin());
            pl_cards->discard.erase(pl_cards->discard.begin(), pl_cards->discard.end());
        }
        else break;
    }
}

// make intention into readable string
string enemy_intention_to_string(player* pl, enemy* en) {
    string intend = en->intention;
    int tmpnum = 0;
    string ret = "";
    for (int i = 0; i < EFFECT_LENGTH - 1; i++) {
        if (intend[i] == '\0') break;
        if (isdigit(intend[i])) tmpnum += (intend[i] - '0');
        else {                                                           // multiply enemy damage by player taken damage
            if (intend[i] == 'D') { ret += ("Attack for " + std::to_string(pl->mult_dmg_to(en->mult_dmg_from(tmpnum))) + " damage. || "); tmpnum = 0; }
            else if (intend[i] == 'B') { ret += ("Apply " + std::to_string(en->mult_block(tmpnum)) + " block. || "); tmpnum = 0; }
            else if (intend[i] == 'S') { ret += ("Gain " + std::to_string(tmpnum) + " strength. || "); tmpnum = 0; }
            else if (intend[i] == 'w') { ret += ("Weaken " + std::to_string(tmpnum) + " turns. || "); tmpnum = 0; }
        }
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

// Check if element is present in vector
bool is_in_vector(vector<int> vec, int c) {
    bool found = false;
    for (int i = 0; i < vec.size(); i++) {
        if (vec.at(i) == c) {
            found = true;
            break;
        }
    }
    return found;
}

// get indexes of matching substrings
vector<int> get_substring_index(string data, string sub) {
    bool f;
    vector<int> ret;
    for (int i = 0; i < data.size() - sub.size(); i++) {
        f = true;
        for (int j = 0; j < sub.size(); j++) {
            if (data.at(i + j) != sub.at(j)) f = false;
        }
        if (f)
            ret.push_back(i);
    }
    return ret;
}

// Get card description
string get_card_desc(player* pl, enemy* en, card cr) {
    vector<int> vecdmg;
    vector<int> vecblc;
    string desc = cr.desc;
    vecdmg = get_substring_index(cr.desc, "_d_");
    vecblc = get_substring_index(cr.desc, "_b_");
    vector<int> vec = vecdmg;
    vec.insert(vec.begin(), vecblc.begin(), vecblc.end());
    std::sort(vec.begin(), vec.end());
    for (int i = vec.size()-1; i >= 0; i--) {
        if (is_in_vector(vecdmg, vec.at(i))) {
            desc.insert(vec.at(i)+3, std::to_string(en->mult_dmg_to(pl->mult_dmg_from(cr.values.at(i))))); // Multiply damage
        }
        if (is_in_vector(vecblc, vec.at(i))) {
            desc.insert(vec.at(i)+3, std::to_string(pl->mult_block(cr.values.at(i)))); // Multiply block
        }
    }
    for (int i = 0; i < vec.size(); i++) {
        for (int j = 0; j < 3; j++) {
            if (!isdigit(desc.at(vec.at(i))))
                desc.erase(desc.begin() + vec.at(i), desc.begin() + vec.at(i)+1);
        }
    }
    return desc;
}

// Prints the entire game screen
// This function is really ugly
void print_game(player* pl, pile* pl_cards, enemy* en) {
    cls();
    vector<card> tmpdraw = pl_cards->draw;
    vector<int> vdmg;
    vector<int> vblc;
    vector<string> draw_descs;
    for (long unsigned int i = 0; i < pl_cards->hand.size(); i++) {
         draw_descs.push_back(get_card_desc(pl, en, pl_cards->hand.at(i)));
    }
    cout << colors.green << "Act: " << pl->act + 1 << "/3" << "\t\t\t" << "Level: " << pl->level + 1 << colors.red
        << "\t\t\tDeck: " << pl_cards->deck.size() << " cards";
    cout << colors.yellow << "\t\t\tGold: " << pl->gold << colors.end << "\n";
    cout << string(111, '-') << "\n";
    cout << colors.yellow << pl->name << " the ironclad\t\t\t\t\t\t\t\t\t\t" << en->name << colors.end << std::endl;
    cout << "Draw pile: " << colors.yellow << pl_cards->draw.size() << " cards\t\t\t\t\t\t\t\t\t\t" << colors.end << "Discard pile: "
        << colors.yellow << pl_cards->discard.size() << " cards\n" << colors.end;
    cout << "HP: " << colors.green << pl->hp << colors.end << "\t\t\t\t\t\t\t\t\t\t\t\tEnemy HP: " << colors.green << en->hp << colors.end << "\n";
    cout << "Block: " << colors.cyan << pl->block << colors.end << "\t\t\t\t\t\t\t\t\t\t\tEnemy block: " << colors.cyan << en->block << colors.end << "\n";
    cout << "Mana: " << colors.magenta << pl->mana << colors.end << "\t\t\t\t\t\t\t\t\t\t\t\tEnemy intent: " << colors.magenta << enemy_intention_to_string(pl, en) << colors.end << "\n";
    cout << string(111, '-') << "\n";
    string mydesc;
    int cnt = 0; // Count number of cards so that message buffer is always at the same height
    for (unsigned long int i = 0; i < pl_cards->hand.size(); i++) {
        cnt++;
        mydesc = draw_descs.at(i);
        while (mydesc.length() < 60) mydesc += " ";
        cout << pl_cards->hand.at(i).color << "(" << i + 1 << ") " << pl_cards->hand.at(i).name << colors.end << "\t\t\t:: "
            << mydesc << colors.magenta
            << "\t" << ":: Mana cost: " << pl_cards->hand.at(i).cost << colors.end << std::endl;
    }
    cout << "\n\n\n";
    cout << "weak: " << colors.magenta << pl->weak << colors.end << "\t\tstr: " << colors.red << pl->strength << colors.end
        << "\t\tpoison: " << colors.green << pl->poison << colors.end << "\n";
    cout << "enemy weak: " << colors.magenta << en->weak << colors.end << "\tenemy str: "
        << colors.red << en->strength << colors.end << "\tenemy poison: " << colors.green << en->poison << colors.end << "\n";

    cout << string(pl->drawlimit - cnt, '\n') << msgbuffer << "\n";
}

// Start of turn function
void start_turn(player* pl, pile* pl_cards) {
    pl->begin_turn(pl_cards);
}

void eval_card(player* pl, pile* pl_pile, enemy* en, card crd) {
    pl->mana -= crd.cost;
    print_game(pl, pl_pile, en);
    char tmpef[EFFECT_LENGTH];
    strcpy(tmpef, crd.effect);
    tmpef[strlen(tmpef) - 1] = '\0'; // We need to remove the last char of the string, since it stores return value of card and is not part of effect
    eval_effect(tmpef, pl, en, pl_pile);
}

// discard x card from hand
void discard_from_hand(pile* pl_cards, int index) {
    pl_cards->discard.push_back(pl_cards->hand.at(index));
    pl_cards->hand.erase(pl_cards->hand.begin() + index);
}

// same thing but exhaust
void exhaust_from_hand(pile* pl_cards, int index) {
    pl_cards->hand.erase(pl_cards->hand.begin() + index);
}

// last char is return value ([D]iscard, [E]xhaust, [R]eturn to hand) -- NOT IN THIS FUNC ANYMORE
// activate card from hand
void play_card_from_hand(player* pl, pile* pl_cards, enemy* en, int index) {
    if (pl->mana >= pl_cards->hand.at(index).cost) {
        card cr = pl_cards->hand.at(index); // Card has to be discarded / exhausted before effect is evaluated, so we make a copy we use to evaluate
        cr = pl_cards->hand.at(index);
        if (pl_cards->hand.at(index).effect[strlen(pl_cards->hand.at(index).effect) - 1] == 'D') // Discard card
            discard_from_hand(pl_cards, index);
        else if (pl_cards->hand.at(index).effect[strlen(pl_cards->hand.at(index).effect) - 1] == 'E') // Exhaust card
            exhaust_from_hand(pl_cards, index);
        eval_card(pl, pl_cards, en, cr);
    }
    else {
        buffer_queue("Not enough mana to play card.");
    }
}

void create_fight(player* pl, pile* plc, enemy* en_main) {
    start_fight(pl, plc);
    bool fight = true;
    char choice = '0';
    while (fight) {
        start_turn(pl, plc);
        en_main->get_intention();
        pl->decrease_counters();
        print_game(pl, plc, en_main);
        while (choice != 'q') {
            choice = key_press();
            if (choice > '0' && choice < (char)('1' + plc->hand.size()))
                play_card_from_hand(pl, plc, en_main, (int)(choice - '0') - 1);
            if (!en_main->check_hp()) {
                cout << colors.red << "You kill the " << en_main->name << "!\n" << colors.end;
                fight = false;
                break;
            }
            print_game(pl, plc, en_main);
        }
        en_main->decrease_counters();
        en_main->begin_turn();
        en_main->commit_intention(pl, plc);
        choice = '0';
    }
}

// TODO: * read from some kind of proper database
//       * balance the game properly
void init_game(vector<enemy>* env, vector<card>* crds) {
    // +100 to level is elite
    // Levels 0-2 are all act one, just different difficulties
    // Name, HP, level, effects
    env->push_back(enemy("Goblin", 20, 0, { "6Da","5Ba" }));
    env->push_back(enemy("Goblin", 30, 1, { "8Da","9Ba","9Da" }));
    env->push_back(enemy("Cultist", 30, 1, { "8Da","7Ba","2Sa" }));
    env->push_back(enemy("Cultist", 30, 2, { "12Da","10Ba","2Sa" }));
    env->push_back(enemy("Hobgoblin", 30, 1, { "8Da","7Ba","2wa" }));
    env->push_back(enemy("Strong goblin", 70, 101, { "93Da","91Ba","93Da","93Da","91Ba","6Sa" }));

    // Add after all non-upgraded cards!
    // name - desc - effect - mana - rarity - color - type (0 attack, 1 skill)
    // '+' after card name means upgraded ! each upgraded card has to follow this naming !
    // TODO: shuffle curse, vulnerable, can't draw more this turn
    crds->push_back(card("Strike", "Deal _d_ damage", {5}, "5daD", 1, 0, colors.red, 0));
    crds->push_back(card("Defend", "Get _b_ block", {5}, "5baD", 1, 0, colors.cyan, 1));
    crds->push_back(card("Iron mask", "Get _b_ block and discard another card", {10}, "91ba1caD", 1, 1, colors.cyan, 1));
    crds->push_back(card("Fear strike", "Deal _d_ damage and apply 1 weak", {3}, "3da1WaD", 0, 0, colors.magenta, 0));
    crds->push_back(card("Instinct", "Deal _d_ damage, if enemy is weak gain 1 mana", {4}, "4d1mwD", 1, 1, colors.red, 0));
    crds->push_back(card("Pain", "Deal _d_ damage and apply 2 weak", {12}, "93da1WaD", 1, 1, colors.red, 0));
    crds->push_back(card("Shockwave", "Deal _d_ damage and gain _b_ block", {4,4}, "4da4baD", 1, 1, colors.cyan, 1));
    crds->push_back(card("Clean strike", "Deal _d_ damage and draw one card", {6}, "6da1CaD", 1, 1, colors.red, 0));
    crds->push_back(card("Boomerang", "Deal _d_ damage 3 times and take 3 damage", {3}, "3da3da3da3DaD", 1, 1, colors.red, 0));
    crds->push_back(card("Claw", "Deal _d_ damage twice", {4},  "4da4daD", 1, 1, colors.red, 0));
    crds->push_back(card("Sacrifice", "Gain 2 mana, lose 3 health", {}, "2ma3DaD", 1, 1, colors.magenta, 1));
    crds->push_back(card("Crack the sky", "Deal _d_ damage, exhaust", {25}, "997daE", 1, 1, colors.red, 0));
    crds->push_back(card("Disarm", "Enemy loses 4 strength", {}, "4laD", 1, 1, colors.red, 0));
}

void upgrade_card(pile* plc, int index) {
    for (long unsigned int i = 0; i < cards.size(); i++) {
        if (cards.at(i).name == plc->deck.at(index).name + "+")
            plc->deck.at(index) = cards.at(i);
    }
}

// Pick random enemy from act
enemy pick_enemy(player* pl) {
    enemy en = *select_randomly(enemies.begin(), enemies.end());
    while (en.level != pl->nlevel)
        en = *select_randomly(enemies.begin(), enemies.end());
    return en;
}

bool one_chance_in(int max) {
    if (rand() % (max - 1 + 1) + 1) return true;
    else return false;
}

card select_random_card() {
    card cr = *select_randomly(cards.begin(), cards.end());
    while (true) {
        if (cr.rarity == 0) // don't select started cards
            cr = *select_randomly(cards.begin(), cards.end());
        if (cr.rarity == 2) {
            if (!one_chance_in(3))
                cr = *select_randomly(cards.begin(), cards.end());
            else break;
        }
        if (cr.rarity == 3) {
            if (one_chance_in(3))
                cr = *select_randomly(cards.begin(), cards.end());
            else break;
        }
    }
    return cr;
}

// Pick random elite enemy from act
enemy pick_elite_enemy(player* pl) {
    enemy en = *select_randomly(enemies.begin(), enemies.end());
    while (en.level != pl->nlevel + 100) // +100 is elites
        en = *select_randomly(enemies.begin(), enemies.end());
    return en;
}

// Create deck at the beginning of the game
pile create_deck(vector<card> crds) {
    pile pl_pile;
    for (int i = 0; i < 3; i++)
        pl_pile.deck.push_back(crds.at(0)); // strike
    for (int i = 0; i < 3; i++)
        pl_pile.deck.push_back(crds.at(1)); // defend
    pl_pile.deck.push_back(crds.at(2)); // iron mask
    pl_pile.deck.push_back(crds.at(3)); // fear strike
    return pl_pile;
}

// Create player at the beginning of the game
player create_player() {
    player pl;
    cout << "Enter your name:\n";
    cin >> pl.name;
    return pl;
}

// Split a string into a vector of strings by char split
vector<string> split_string(string data, char split) {
    vector<string> v;
    string tmp;
    std::stringstream ss(data);
    while(getline(ss, tmp, split)){
        v.push_back(tmp);
    }
    return v;
}


bool isNumber(const string& str)
{
    for (char const &c : str) {
        if (std::isdigit(c) == 0 && c != '-') return false;
    }
    return true;
}

// same thing but get integer vector returned
vector<int> split_string_get_int(string data, char split) {
    vector<int> v;
    string tmp;
    std::stringstream ss(data);
    while(getline(ss, tmp, split)){
        if (isNumber(tmp))
            v.push_back(stoi(tmp));
    }
    return v;
}

int count_lines_in_file(string filename) {
    int nl = 0;
    std::string line;
    std::ifstream file(filename);
    while (std::getline(file, line))
        ++nl;
    return nl;
}

vector_tree<string> read_tree_from_file(string filename, int offset){
	std::ifstream file;
	file.open(filename);
	string line;

    for (int i = 0; i < offset; i++) getline(file, line);

    vector<int> relations;
    vector<string> names;

	getline(file, line);
    relations = split_string_get_int(line, '^'); // the '^' symbol is used to separate items in the vector file output
	getline(file, line);                         // don't use '^' in the random encounter texts or bad things will happen
    names = split_string(line, '^');

    vector_tree<string> ret;
    ret.setVectors(relations, names);
	file.close();
    return ret;
}

// just get a random number in a range
int get_random_in_range(int min, int max) {
    return min + (rand() % max);
}

// Pick a random encounter tree from database
vector_tree<string> get_random_encounter_tree() {
    string fi = ENCOUNTERS_PATH;
    int lines = count_lines_in_file(fi); // a slightly ugly solution
    int halflines = lines/2;
    int r = get_random_in_range(0,halflines);
    vector_tree<string> enc = read_tree_from_file(fi, (r*2));
    return enc;
}

void eval_encounter(player* pl, pile* plc, vector_tree<string>* enc) {
    enemy en = enemy("", 0, 0, { }); // Need dummy enemy to evaluate effect
    int pos = 0;
    bool isEven = true;
    vector<int> nodes;
    char choice;
    string ef;
    int c;
    cin.ignore();
    int addpos = 0;
    while (true) {
        if (enc->getChildren(pos).size()) {
            if (enc->getName(enc->getChildren(pos).at(0)).at(0) == '_') {
                ef = enc->getName(enc->getChildren(pos).at(0));
                ef.erase(ef.begin(),ef.begin()+1);
                break;
            }
            else {
                if (isEven) {
                    cout << "Act:" << pl->act+1 << "/3" << "\t\t\tLevel: " << pl->level+1 << "\n";
                    cout << "HP: " << colors.green << pl->hp << "/" << pl->maxhp << colors.end;
                    cout << "\t\tGold: " << colors.yellow << pl->gold << colors.end;
                    cout << "\n\n";
                    cout << enc->getName(pos) << "\n\n";
                }
                else {
                    nodes = enc->getChildren(pos);
                    for (int i = 0; i < nodes.size(); i++) {
                        cout << "(" << i+1 << ") " << enc->getName(nodes.at(i)) << "\n";
                    }
                    choice = key_press();
                    c = choice - '0' - 1;
                    //pos = nodes.at(enc->getChildren(c).at(enc->getChildren(0).at(0)));
                    pos = enc->getChildren(enc->getChildren(pos).at(c))).at(0);
                    cout << pos;
                    //addpos+=c;
                }
                isEven = !isEven;
            }
        }
    }
    cout << "end";
    //cout << enc->getName(enc->getChildren(pos).at(0));
    cin.ignore();
    if (ef != "") {
        char e[EFFECT_LENGTH];
        strcpy(e, ef.c_str());
        eval_effect(e, pl, &en ,plc);
    }
}

// Each encounter is a tree of interactions, ending with some effect (or no effect at all), marked by a '_'
// The encounters are taken from the database
// Theres an interface for adding new encounters, 'make_encounter.cpp', once an encounter is prepared you can append to the end
// of database with 'q' on the keyboard
// it's a bit complex, but I really don't like implementing 20 000 different functions for different encounters
// also I wanted to use my tree library
void random_encounter(player* pl, pile* plc) {
    vector_tree<string> enc = get_random_encounter_tree();
    eval_encounter(pl, plc, &enc);
}

// TODO: get card after battle
//       get special card after elite
// encounters, shops
int main() {
    srand(time(NULL));
    init_game(&enemies, &cards); // global variables

    // Initialize player and deck
    player pl = create_player();
    pile pl_pile = create_deck(cards);
    // Initialize enemy
    enemy en_main = pick_enemy(&pl);

    while (pl.act != 2) {
        while (pl.level != 10) {
            random_encounter(&pl, &pl_pile); // Run a random encounter
            cout << "end";
            cin.ignore();
            cin.ignore();
            //create_fight(&pl, &pl_pile, &en_main);
            getchar();
            getchar();
            en_main = pick_enemy(&pl);
            pl.level++;
            if (pl.level == 2 || pl.level == 6)
                pl.nlevel++;
        }
        pl.act++;
    }
}

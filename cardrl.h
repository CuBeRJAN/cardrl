#ifndef _CARDRL_H
#define _CARDRL_H
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

#include "player.h"
#include "enemy.h"

#define EFFECT_LENGTH 100 // max length of effect
#define ENCOUNTERS_PATH "./encounters" // define where the random encounter database is stored

struct {
    const std::string cyan = "\033[36m";
    const std::string magenta = "\033[35m";
    const std::string red = "\033[91m";
    const std::string gray = "\033[8m";
    const std::string green = "\033[92m";
    const std::string yellow = "\033[33m";
    const std::string end = "\033[0m";
} colors;

// random select templates
template<typename Iter, typename RandomGenerator>
    Iter select_randomly(Iter start, Iter end, RandomGenerator& g);

template<typename Iter>
    Iter select_randomly(Iter start, Iter end);


// some functions
void cls();
void game_quit();
int key_press();
void end_game();
void shuffle_stringvec(std::vector<std::string>*);
void eval_effect(char effect[EFFECT_LENGTH], player*, enemy*, pile*);
void discard_from_hand(pile* pl_cards, int index);
void exhaust_from_hand(pile* pl_cards, int index);
void buffer_send(std::string);
void buffer_queue(std::string);
char get_onechar(); // unused
int select_from_hand(pile*, std::string);
void buffer_flush(); // clear buffer
void check_bufferlen(); // make buffer 5 lines at most

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
void eval_effect(char effect[EFFECT_LENGTH], player*, enemy*, pile*); // evaluate effect

int efnum_to_int(std::string); // sum digits (991 to 19 etc.)
std::string int_to_efnum(int); // number to sum of digits

void shuffle_deck(std::vector<card>*);
void shuffle_stringvec(std::vector<std::string>*);

void start_fight(player*, pile*); // initialize stuff at the beginning of a fight
void draw_one_card(player*, pile*); // draw a single card
void draw_hand(player*, pile*); // draw a full hand of cards

// make enemy intent into readable string
std::string enemy_intention_to_string(player*, enemy*);

void discard_hand(player*, pile*);
bool is_in_vector(std::vector<int>, int);
std::vector<int> get_substring_index(std::string, std::string); // get indexes of matching substrings
std::string get_card_desc(player*, enemy*, card); // get description of card in readable format

void print_game(player*, pile*, enemy*); // print the game
void start_turn(player*, pile*); // start of turn function
void eval_card(player*, pile*, enemy*, card); // evaluate card
void discard_from_hand(pile*, int); // discard card from hand by index
void exhaust_from_hand(pile*, int); // exhaust from hand by index
void play_card_from_hand(pile*, int); // play from hand by index
void removeSubstrs(std::string&, std::string);
void create_fight(player*, pile*, enemy*); // start the fight loop
void create_shop(player*, pile*); // create a shop with cards
void init_game(std::vector<enemy>*, std::vector<card>*); // initialize games cards and enemies
void upgrade_card(pile*, int index); // upgrade card from deck by index
enemy pick_enemy(player*); // pick enemy by player level, etc.
bool one_chance_in(int); // probability one in x
card select_random_card(); // select random card from all cards
enemy pick_elite_enemy(player*); // pick appropriate elite enemy
pile create_deck(std::vector<card>); // create deck at the beginning of the game
player create_player(); // create player at the beginning of the game
std::vector<std::string> split_string(std::string, char); // split string by char
bool isNumber(const std::string&); // check if string is number
std::vector<int> split_string_get_int(std::string, char); // split string into integers
int count_lines_in_file(std::string);
vector_tree<std::string> read_tree_from_file(std::string, int offset); // read tree from db file
int get_random_in_range(int,int); // get random number in range
vector_tree<std::string> get_random_encounter_tree(); // get a random encounter tree
void eval_encounter(player*, pile*, vector_tree<std::string>*); // evaluate an encounter tree
void random_encounter(player*, pile*); // select a random encounter and evaluate it
#endif

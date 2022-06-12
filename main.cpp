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

class player;
void discard_hand(player*, pile*);
void draw_hand(player*, pile*);

class player { // TODO: There surely is a cleaner way than having 20000 variables for status effects
public:
    string name;
    int drawcards = 4;
    int nlevel = 0; // level of enemies (not of descent!)
    int drawlimit = 10; // how many cards can be in hand at once
    int barricade = 0; // don't lose block for x turns
    int gold = 0;
    int level = 0; // each act has a number of levels, stored here
    int dont_discard_hand = 0; // don't discard hand for x turns
    int dont_draw = 0;
    int act = 0; // game is split into 3 acts, this stores the act number
    int poison = 0;
    int frail = 0; // weaken block cards
    int maxmana = 3;
    int mana;
    int weak = 0; // -1 weak each turn
    int hp = 50;
    int maxhp = 50;
    int block = 0;
    int strength = 0;
    bool confused = false; // confusion effect

    void begin_turn(pile* pl_cards) {
        pl_discard_hand(pl_cards);
        if (!dont_draw)
            draw_hand(this, pl_cards); // draw new cards
        hp -= poison; // apply poison
        if (hp < 1) end_game(); // end game if hp <= 0
        mana = maxmana; // refresh mana
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

    void addblock(int blc) {
        if (!frail)
            block += blc;
        else
            block += blc * 0.7;
    }

    void remove_mana(int n) {
        mana -= n;
    }

    void clear_block() {
        if (!barricade) block = 0;
    }

    void decrease_counters() {
        clear_block();
        if (poison) poison--;
        if (barricade) barricade--;
        if (weak) weak--;
        if (dont_discard_hand) dont_discard_hand--;
        if (dont_draw) dont_draw--;
    }

    void pl_discard_hand(pile* plc) {
        if (!barricade)
            discard_hand(this, plc);
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
    int weak = 0;
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


    void clear_block() {
        if (!barricade) block = 0;
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
        if (intention_counter >= intention_counter_max-1) {
            shuffle_stringvec(&actions);
            intention_counter = 0;
        }
        else {
            intention_counter++;
        }
        intention = actions.at(intention_counter);
        return intention;
    }

    void commit_intention(player* pl, pile* plc) {
        char ef[EFFECT_LENGTH];
        strcpy(ef,intention.c_str());
        eval_effect(ef,pl,this,plc);
    }

    void begin_turn() {
        if (barricade) barricade--;
        else block = 0;
    }

    void decrease_counters() {
        clear_block();
        if (poison) poison--;
        if (barricade) barricade--;
        if (weak) weak--;
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
// 92d3b3p means 11 damage, 3 block and 3 poison
// [b]lock, [d]amage enemy, dis[c]ard other cards, [p]oison
// [h]eal, e[x]haust other card(s), [H]eal enemy, [D]amage player
// [B]lock enemy, add [s]trength, add [S]trength to enemy
// [w]eaken player, [W]eaken enemy
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
            else if (effect[i] == 's') { plr->strength += tmpnum;
                buffer_queue(colors.magenta+"You gain "+std::to_string(tmpnum)+" strength"+colors.end); tmpnum = 0; }
            else if (effect[i] == 'S') { en->strength += tmpnum;
                buffer_queue(colors.magenta+"Enemy gains "+std::to_string(tmpnum)+" strength"+colors.end); tmpnum = 0; }
            else if (effect[i] == 'w') { plr->weak += tmpnum+1; // +1 because weaken gets removed at the start of player turn
                buffer_queue(colors.magenta+"Enemy weakens you for "+std::to_string(tmpnum)+" turns"+colors.end); tmpnum = 0; }
            else if (effect[i] == 'W') { en->weak += tmpnum+1; // +1 because weaken gets removed at start of enemy turn
                buffer_queue(colors.magenta+"You weaken enemy for "+std::to_string(tmpnum)+" turns"+colors.end); tmpnum = 0; }
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

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine shuffle_myseed(seed);

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
    pl_cards->hand.clear();
    pl_cards->discard.clear();
    pl_cards->draw = pl_cards->deck;
    shuffle_deck(&pl_cards->draw);
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
            shuffle_deck(&pl_cards->draw);
            pl_cards->hand.push_back(pl_cards->draw.at(0));
            pl_cards->draw.erase(pl_cards->draw.begin());
            pl_cards->discard.erase(pl_cards->discard.begin(), pl_cards->discard.end());
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
            else if (intend[i] == 'S') { ret += ("Gain " + std::to_string(tmpnum) + " strength. || "); tmpnum = 0; }
            else if (intend[i] == 'w') { ret += ("Weaken " + std::to_string(tmpnum) + " turns. || "); tmpnum = 0; }
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
    cout << "Draw pile: " << colors.yellow << pl_cards->draw.size() << " cards\t\t\t\t\t\t\t\t\t\t" << colors.end << "Discard pile: "
         << colors.yellow << pl_cards->discard.size() << " cards\n" << colors.end;
    cout << "HP: " << colors.green << pl->hp << colors.end << "\t\t\t\t\t\t\t\t\t\t\t\tEnemy HP: " << colors.green << en->hp << colors.end << "\n";
    cout << "Block: " << colors.cyan << pl->block << colors.end << "\t\t\t\t\t\t\t\t\t\t\tEnemy block: " << colors.cyan << en->block << colors.end << "\n";
    cout << "Mana: " << colors.magenta << pl->mana << colors.end << "\t\t\t\t\t\t\t\t\t\t\t\tEnemy intent: " << colors.magenta << enemy_intention_to_string(en->intention) << colors.end << "\n";
    cout << string(111, '-') << "\n";
    string mydesc;
    int cnt = 0; // Count number of cards so that message buffer is always at the same height
    for (unsigned long int i = 0; i < pl_cards->hand.size(); i++) {
        cnt++;
        mydesc = pl_cards->hand.at(i).desc;
        while (mydesc.length() < 60) mydesc += " ";
        cout << pl_cards->hand.at(i).color << "(" << i+1 << ") " << pl_cards->hand.at(i).name << colors.end << "\t\t\t:: "
             << mydesc << colors.magenta
             << "\t" << ":: Mana cost: " << pl_cards->hand.at(i).cost << colors.end << std::endl;
    }
    cout << "\n\n\n";
    cout << "weak: " << colors.magenta << pl->weak << colors.end << "\t\tstr: " << colors.red << pl->strength << colors.end
         << "\t\tpoison: " << colors.green << pl->poison << colors.end << "\n";
    cout << "enemy weak: " << colors.magenta << en->weak << colors.end << "\tenemy str: "
         << colors.red << en->strength << colors.end << "\tenemy poison: " << colors.green << en->poison << colors.end << "\n";

    cout << string(pl->drawlimit-cnt,'\n') << msgbuffer << "\n";
}

// Start of turn function
void start_turn(player* pl, pile* pl_cards) {
    pl->begin_turn(pl_cards);
}

void eval_card(player* pl, pile* pl_pile, enemy* en, card crd) {
    pl->mana-=crd.cost;
    print_game(pl, pl_pile, en);
    char tmpef[EFFECT_LENGTH];
    strcpy(tmpef, crd.effect);
    tmpef[strlen(tmpef)-1] = '\0'; // We need to remove the last char of the string, since it stores return value of card and is not part of effect
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
        if (pl_cards->hand.at(index).effect[strlen(pl_cards->hand.at(index).effect)-1] == 'D') // Discard card
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
    while(fight) {
        start_turn(pl, plc);
        en_main->get_intention();
        en_main->decrease_counters();
        pl->decrease_counters();
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
        en_main->begin_turn();
        en_main->commit_intention(pl, plc);
        choice = '0';
    }

}

// TODO: read from some kind of proper database
void init_game(vector<enemy>* env, vector<card>* crds) {
    // +100 to level is elite
    // Levels 0-2 are all act one, just different difficulties
    // Name, HP, level, effects
    env->push_back(enemy("Goblin",20,0,{"6D","5B",})); // goblin level 0
    env->push_back(enemy("Goblin",30,1,{"8D","7B","8D"})); // goblin level 1
    env->push_back(enemy("Hobgoblin",30,1,{"8D","7B","2w"})); // hobgoblin level 1
    env->push_back(enemy("Strong goblin",35,101,{"93D","91B","93D","91B","93D","91B","1S"})); // elite goblin, level 1

    // Add after all non-upgraded cards!
    // name - desc - effect - mana - rarity - color - type (0 attack, 1 skill)
    // + after card name means upgraded ! each upgraded card has to follow this naming !
    crds->push_back(card("Strike", "Deal 6 damage","6dD",1,0, colors.red,0));
    crds->push_back(card("Defend", "Get 5 block","5bD",1,0, colors.cyan,1));
    crds->push_back(card("Iron mask", "Get 10 block and discard another card", "91b1cD",1,0, colors.cyan,1));
    crds->push_back(card("Fear strike", "Deal 3 damage and apply 1 weak","3d1WD",0,0, colors.magenta,0));
    crds->push_back(card("Strike+", "Deal 9 damage","9dD",1,4,colors.red,0));
    crds->push_back(card("Defend+", "Get 8 block","8bD",1,4,colors.cyan,1));
    crds->push_back(card("Iron mask+", "Get 13 block and discard another card", "94b1cD",1,4,colors.cyan,1));
    crds->push_back(card("Fear strike+", "Deal 5 damage and apply 1 weak","5d1WD",0,0, colors.magenta,0));
}

vector<card> cards; // another global variable...
std::vector<enemy> enemies;
void upgrade_card(pile* plc, int index) {
    for (long unsigned int i = 0; i < cards.size(); i++) {
        if (cards.at(i).name == plc->deck.at(index).name+"+")
            plc->deck.at(index) = cards.at(i);
    }
}

// Pick random enemy from act
enemy pick_enemy(player* pl) {
    enemy en = *select_randomly(enemies.begin(),enemies.end());
    while (en.level != pl->nlevel)
        en = *select_randomly(enemies.begin(),enemies.end());
    return en;
}

// Pick random elite enemy from act
enemy pick_elite_enemy(player* pl) {
    enemy en = *select_randomly(enemies.begin(),enemies.end());
    while (en.level != pl->nlevel+100) // +100 is elites
        en = *select_randomly(enemies.begin(),enemies.end());
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


int main() {
    srand(0);
    init_game(&enemies, &cards); // global variables

    // Initialize player and deck
    player pl = create_player();
    pile pl_pile = create_deck(cards);
    // Initialize enemy
    enemy en_main = pick_enemy(&pl);

    while (pl.act != 2) {
        while (pl.level != 10) {
            create_fight(&pl, &pl_pile, &en_main);
            getchar();
            getchar();
            en_main = pick_enemy(&pl);
            pl.level++;
            if (pl.level == 2 || pl.level == 6)
                pl.nlevel++;
        }
    }
}

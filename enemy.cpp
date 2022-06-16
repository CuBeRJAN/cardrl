#include "enemy.h"
#include "cardrl.h"

enemy::enemy(std::string n, int mhp, int l, std::vector<std::string> ac) {
    name = n;
    maxhp = mhp;
    level = l;
    actions = ac;
    intention_counter_max = ac.size();
    hp = maxhp;
}


int enemy::mult_dmg_from(int dmg) {
    if (weak) return (dmg + (dmg * (0.1 * strength))) * 0.8;
    return (dmg + (dmg * (0.1 * strength)));
}

void enemy::end_turn() {
    if (weak) weak--;
}

int enemy::mult_dmg_to(int dmg) {
    if (vulnerable) return dmg * 1.5;
    return dmg;
}

void enemy::clear_block() {
    if (!barricade) block = 0;
}

int enemy::mult_block(int blc) {
    if (frail)
        blc *= 0.7;
    return blc;
}

void enemy::take_damage(int dmg) {
    int rdmg = dmg;
    if (vulnerable) rdmg *= 1.5;
    int obl = block;
    if (rdmg >= block) {
        block = 0;
        hp -= rdmg - obl;
    }
    else
        block -= rdmg;
}

// piercing damage
void enemy::take_damage_forced(int dmg) {
    int rdmg = dmg;
    if (vulnerable) rdmg *= 1.5;
    hp -= rdmg;
}

bool enemy::check_hp() {
    if (hp < 1) {
        return false;
    }
    return true;
}

void enemy::addblock(int blc) {
    if (!frail)
        block += blc;
    else
        block += blc * 0.7;
}

void enemy::remove_mana(int n) {
    mana -= n;
}

std::string enemy::get_intention() {
    if (intention_counter >= intention_counter_max - 1) {
        shuffle_stringvec(&actions);
        intention_counter = 0;
    }
    else {
        intention_counter++;
    }
    intention = actions.at(intention_counter);
    return intention;
}

void enemy::commit_intention(player* pl, pile* plc) {
    char ef[EFFECT_LENGTH];
    strcpy(ef, intention.c_str());
    eval_effect(ef, pl, this, plc);
}

void enemy::begin_turn() {
    decrease_counters();
    if (barricade) barricade--;
    if (poison)
        take_damage_forced(poison);
    else block = 0;
}

void enemy::decrease_counters() {
    clear_block();
    if (poison) poison--;
    if (vulnerable) vulnerable--;
    if (barricade) barricade--;
}

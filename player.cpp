#include "player.h"
#include "enemy.h"
#include "cardrl.h"

void player::begin_turn(pile* pl_cards) {
    pl_discard_hand(pl_cards);
    if (!dont_draw)
        draw_hand(this, pl_cards); // draw new cards
    hp -= poison; // apply poison
    if (hp < 1) end_game(); // end game if hp <= 0
    mana = maxmana; // refresh mana
}

void player::take_damage(int dmg) {
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


// piercing damage / poison
void player::take_damage_forced(int dmg) {
    int rdmg = dmg;
    if (vulnerable) rdmg *= 1.5;
    hp -= rdmg;
}

// multiply outgoing damage
int player::mult_dmg_from(int dmg) {
    if (weak) return (dmg + (dmg * (0.2 * strength))) * 0.6;
    return (dmg + (dmg * (0.2 * strength))); // maybe change this calculation somehow, strength is way too impactful
}

int player::mult_dmg_to(int dmg) {
    if (vulnerable) return dmg * 1.5;
    return dmg;
}

void player::addblock(int blc) {
    if (!frail)
        block += blc;
    else
        block += blc * 0.7;
}

void player::remove_mana(int n) {
    mana -= n;
}

void player::clear_block() {
    if (!barricade) block = 0;
}

int player::mult_block(int blc) {
    if (frail)
        blc *= 0.7;
    return blc;
}

void player::decrease_counters() {
    if (poison)
        take_damage_forced(poison);
    clear_block();
    if (poison) poison--;
    if (vulnerable) vulnerable--;
    if (barricade) barricade--;
    if (weak) weak--;
    if (dont_discard_hand) dont_discard_hand--;
    if (dont_draw) dont_draw--;
}

void player::pl_discard_hand(pile* plc) {
    if (!dont_discard_hand)
        discard_hand(this, plc);
}

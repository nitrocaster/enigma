#pragma once
#include "config.h"
#include "common.h"

#define ENIGMA_CHAR_COUNT 256

typedef struct
{
    uint8_t substs[ENIGMA_CHAR_COUNT];
    uint8_t rsubsts[ENIGMA_CHAR_COUNT];
    uint8_t notches[ENIGMA_CHAR_COUNT];
    uint8_t pos;
} enigma_rotor_t;

typedef struct
{
    uint8_t first;
    uint8_t second;
} byte_pair_t;

typedef struct
{
    uint8_t conv[ENIGMA_CHAR_COUNT];
} enigma_plugboard_t;

typedef struct
{
    enigma_plugboard_t plugboard;
    uint8_t rotor_count;
    enigma_rotor_t rotors[]; // last rotor is reflector
} enigma_state_t;

void enigma_rotor_init(enigma_rotor_t *rotor, uint8_t init_pos,
    const uint8_t *substs, const uint8_t *notches);
enigma_state_t *enigma_state_init(const enigma_rotor_t *rotors,
    uint8_t r_count, const byte_pair_t *pb_pairs, uint8_t pb_count);
void enigma_state_deinit(enigma_state_t *enigma);
void enigma_state_reset(enigma_state_t *enigma, const uint8_t *r_positions);
void enigma_state_transform(enigma_state_t *enigma, const void *src,
    void *dst, size_t length);

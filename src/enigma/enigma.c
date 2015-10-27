#include "config.h"
#include "enigma.h"
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>

enigma_rotor_t *enigma_rotor_alloc()
{ return malloc(sizeof(enigma_rotor_t)); }

void enigma_rotor_free(enigma_rotor_t *rotor)
{ free(rotor); }

void enigma_rotor_init(enigma_rotor_t *rotor, uint8_t init_pos, int type,
    const uint8_t *substs, const uint8_t *notches)
{
    memcpy(rotor->substs, substs, sizeof(rotor->substs));
    // init notches
    if (type&ENIGMA_ROTOR_TYPE_PLAIN)
        memset(rotor->notches, 1, sizeof(rotor->notches));
    else
        memcpy(rotor->notches, notches, sizeof(rotor->notches));
    // init other params
    rotor->pos = init_pos;
    rotor->type = type;
}

static uint8_t enigma_rotor_transform(const enigma_rotor_t *rotor,
    uint8_t input)
{
    uint8_t rpos = rotor->pos;
    return rotor->substs[input-rpos]+rpos;
}

static int enigma_rotor_has_notch(const enigma_rotor_t *rotor)
{ return rotor->notches[rotor->pos]; }

static void enigma_rotor_rotate(enigma_rotor_t *rotor)
{ rotor->pos++; }

static void enigma_state_rotate(enigma_state_t *enigma)
{
    uint8_t rcount = enigma->rotor_count;
    uint8_t *r = alloca(rcount);
    r[0] = TRUE;
    for (int i = 1; i<rcount; i++)
        r[i] = enigma_rotor_has_notch(&enigma->rotors[i-1]);
    for (int i = 0; i < rcount; i++)
    {
        if (r[i])
            enigma_rotor_rotate(&enigma->rotors[i]);
    }
}

static void enigma_plugboard_init(enigma_plugboard_t *pb,
    const byte_pair_t *pb_pairs, uint8_t pb_count)
{
    for (int i = 0; i<ENIGMA_CHAR_COUNT; i++)
        pb->conv[i] = i;
    for (int i = 0; i<pb_count; i++)
    {
        const byte_pair_t *pair = &pb_pairs[i];
        pb->conv[pair->first] = pair->second;
    }
}

enigma_state_t *enigma_state_init(const enigma_rotor_t *rotors,
    uint8_t r_count, const byte_pair_t *pb_pairs, uint8_t pb_count)
{
    size_t state_size = sizeof(enigma_state_t) +
        sizeof(enigma_rotor_t)*r_count;
    enigma_state_t *enigma = malloc(state_size);
    enigma_plugboard_init(&enigma->plugboard, pb_pairs, pb_count);
    enigma->rotor_count = r_count;
    memcpy(enigma->rotors, rotors, sizeof(enigma_rotor_t)*r_count);
    return enigma;
}

void enigma_state_deinit(enigma_state_t* enigma)
{
    size_t state_size = sizeof(enigma_state_t) +
        sizeof(enigma_rotor_t)*enigma->rotor_count;
    memset(enigma, 0, state_size);
    free(enigma);
}

void enigma_state_reset(enigma_state_t *enigma, const uint8_t *r_positions)
{
    for (int i = 0; i < enigma->rotor_count; i++)
        enigma->rotors[i].pos = r_positions[i];
}

void enigma_state_transform(enigma_state_t* enigma, const void* src,
    void* dst, size_t length)
{
    for (size_t i = 0; i<length; i++)
    {
        uint8_t c = ((uint8_t*)src)[i];
        // from first to last rotor except reflector
        for (int r = 0; r < enigma->rotor_count-1; r++)
            c = enigma_rotor_transform(&enigma->rotors[r], c);
        // from reflector back to first rotor
        for (int r = enigma->rotor_count-1; r>=0; r--)
            c = enigma_rotor_transform(&enigma->rotors[r], c);
        ((uint8_t*)dst)[i] = c;
        enigma_state_rotate(enigma);
    }
}

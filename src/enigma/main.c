#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "enigma.h"
#include "rotor_config.h"

static void print_usage()
{ puts("usage: enigma <key file> <source file> <destination file>"); }

static int file_exists(const char *path)
{
    struct stat buffer;
    buffer.st_size;
    return !stat(path, &buffer);
}

typedef struct
{
    uint8_t count;
    uint8_t positions[];
} enigma_rotor_cfg_t;

typedef struct
{
    uint8_t count;
    byte_pair_t pairs[];
} enigma_plugboard_cfg_t;

#define MSG_INVALID_KEY_FILE "invalid key file."
#define ROTOR_COUNT 4

static enigma_rotor_t rotor_buf[ROTOR_COUNT];

static void load_key_file(const char *path, enigma_rotor_cfg_t **r_cfg,
    enigma_plugboard_cfg_t **pb_cfg)
{
    FILE *key = NULL;
    if (!file_exists(path))
    {
        puts("key file not found.");
        exit(1);
    }
    key = fopen(path, "rb");
    uint8_t count;
    if (fread(&count, 1, 1, key)!=1 || count!=ROTOR_COUNT)
    {
        puts(MSG_INVALID_KEY_FILE);
        exit(1);
    }
    *r_cfg = malloc(sizeof(enigma_rotor_cfg_t)+count);
    (*r_cfg)->count = count;
    if (fread((*r_cfg)->positions, 1, count, key)!=count)
    {
        puts(MSG_INVALID_KEY_FILE);
        exit(1);
    }
    if (fread(&count, 1, 1, key)!=1)
    {
        puts(MSG_INVALID_KEY_FILE);
        exit(1);
    }
    *pb_cfg = malloc(sizeof(enigma_plugboard_cfg_t)+count);
    (*pb_cfg)->count = count;
    if (count &&
        fread((*pb_cfg)->pairs, sizeof(byte_pair_t), count, key)!=count)
    {
        puts(MSG_INVALID_KEY_FILE);
        exit(1);
    }
    fclose(key);
}

static int get_rtype(int index)
{ return index ? ENIGMA_ROTOR_TYPE_CUSTOM : ENIGMA_ROTOR_TYPE_PLAIN; }

int main(int argc, char *argv[])
{
    if (argc!=4)
    {
        print_usage();
        return 1;
    }
    const char *key_path = argv[1], *src_path = argv[2], *dst_path = argv[3];
    enigma_rotor_cfg_t *r_cfg = NULL;
    enigma_plugboard_cfg_t *pb_cfg = NULL;
    load_key_file(key_path, &r_cfg, &pb_cfg);
    uint8_t *r_substs[ROTOR_COUNT] = {
        r_substs_0, r_substs_1, r_substs_2, r_substs_refl
    };
    uint8_t *r_notches[ROTOR_COUNT] = {
        NULL, r_notches_1, r_notches_1, r_notches_1
    };
    for (int i = 0; i<ROTOR_COUNT; i++)
    enigma_rotor_init(rotor_buf+i, r_cfg->positions[i],
        get_rtype(i), r_substs[i], r_notches[i]);
    enigma_state_t *enigma = enigma_state_init(
        rotor_buf, ROTOR_COUNT, pb_cfg->pairs, pb_cfg->count);
    FILE *src = fopen(src_path, "rb");
    if (!src)
    {
        puts("can't open source file.");
        return 1;
    }
    FILE *dst = fopen(dst_path, "wb+");
    if (!dst)
    {
        puts("can't open destination file.");
        return 1;
    }
    uint8_t buf[CONFIG_TRANSFORM_BUFFER_SIZE];
    size_t r_sz = 0;
    while ((r_sz = fread(buf, 1, CONFIG_TRANSFORM_BUFFER_SIZE, src)))
    {
        enigma_state_transform(enigma, buf, buf, r_sz);
        fwrite(buf, 1, r_sz, dst);
    }
    fclose(src);
    fclose(dst);
    enigma_state_deinit(enigma);
    return 0;
}

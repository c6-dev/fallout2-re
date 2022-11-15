#include "game/sfxcache.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"
#include "game/cache.h"
#include "game/gconfig.h"
#include "memory.h"
#include "sound_decoder.h"
#include "sound_effects_list.h"

#define SOUND_EFFECTS_CACHE_MIN_SIZE 0x40000

typedef struct SoundEffect {
    // NOTE: This field is only 1 byte, likely unsigned char. It always uses
    // cmp for checking implying it's not bitwise flags. Therefore it's better
    // to express it as boolean.
    bool used;
    CacheEntry* cacheHandle;
    int tag;
    int dataSize;
    int fileSize;
    // TODO: Make size_t.
    int position;
    int dataPosition;
    unsigned char* data;
} SoundEffect;

static_assert(sizeof(SoundEffect) == 32, "wrong size");

static int sfxc_effect_size(int tag, int* sizePtr);
static int sfxc_effect_load(int tag, int* sizePtr, unsigned char* data);
static void sfxc_effect_free(void* ptr);
static int sfxc_handle_list_create();
static void sfxc_handle_list_destroy();
static int sfxc_handle_create(int* handlePtr, int id, void* data, CacheEntry* cacheHandle);
static void sfxc_handle_destroy(int handle);
static bool sfxc_handle_is_legal(int a1);
static int sfxc_decode(int handle, void* buf, unsigned int size);
static int sfxc_ad_reader(int handle, void* buf, unsigned int size);

// 0x51C8F0
static bool sfxc_initialized = false;

// 0x51C8F4
static int sfxc_cmpr = 1;

// 0x51C8EC
static Cache* sfxc_pcache = NULL;

// 0x51C8DC
static int sfxc_dlevel = INT_MAX;

// 0x51C8E0
static char* sfxc_effect_path = NULL;

// 0x51C8E4
static SoundEffect* sfxc_handle_list = NULL;

// 0x51C8E8
static int sfxc_files_open = 0;

// 0x4A8FC0
int sfxc_init(int cacheSize, const char* effectsPath)
{
    if (!config_get_value(&game_config, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DEBUG_SFXC_KEY, &sfxc_dlevel)) {
        sfxc_dlevel = 1;
    }

    if (cacheSize <= SOUND_EFFECTS_CACHE_MIN_SIZE) {
        return -1;
    }

    if (effectsPath == NULL) {
        effectsPath = "";
    }

    sfxc_effect_path = internal_strdup(effectsPath);
    if (sfxc_effect_path == NULL) {
        return -1;
    }

    if (soundEffectsListInit(sfxc_effect_path, sfxc_cmpr, sfxc_dlevel) != SFXL_OK) {
        internal_free(sfxc_effect_path);
        return -1;
    }

    if (sfxc_handle_list_create() != 0) {
        soundEffectsListExit();
        internal_free(sfxc_effect_path);
        return -1;
    }

    sfxc_pcache = (Cache*)internal_malloc(sizeof(*sfxc_pcache));
    if (sfxc_pcache == NULL) {
        sfxc_handle_list_destroy();
        soundEffectsListExit();
        internal_free(sfxc_effect_path);
        return -1;
    }

    if (!cache_init(sfxc_pcache, sfxc_effect_size, sfxc_effect_load, sfxc_effect_free, cacheSize)) {
        internal_free(sfxc_pcache);
        sfxc_handle_list_destroy();
        soundEffectsListExit();
        internal_free(sfxc_effect_path);
        return -1;
    }

    sfxc_initialized = true;

    return 0;
}

// 0x4A90FC
void sfxc_exit()
{
    if (sfxc_initialized) {
        cache_exit(sfxc_pcache);
        internal_free(sfxc_pcache);
        sfxc_pcache = NULL;

        sfxc_handle_list_destroy();

        soundEffectsListExit();

        internal_free(sfxc_effect_path);

        sfxc_initialized = false;
    }
}

// 0x4A9140
int sfxc_is_initialized()
{
    return sfxc_initialized;
}

// 0x4A9148
void sfxc_flush()
{
    if (sfxc_initialized) {
        cache_flush(sfxc_pcache);
    }
}

// 0x4A915C
int sfxc_cached_open(const char* fname, int mode, ...)
{
    if (sfxc_files_open >= SOUND_EFFECTS_MAX_COUNT) {
        return -1;
    }

    char* copy = internal_strdup(fname);
    if (copy == NULL) {
        return -1;
    }

    int tag;
    int err = soundEffectsListGetTag(copy, &tag);

    internal_free(copy);

    if (err != SFXL_OK) {
        return -1;
    }

    void* data;
    CacheEntry* cacheHandle;
    if (!cache_lock(sfxc_pcache, tag, &data, &cacheHandle)) {
        return -1;
    }

    int handle;
    if (sfxc_handle_create(&handle, tag, data, cacheHandle) != 0) {
        cache_unlock(sfxc_pcache, cacheHandle);
        return -1;
    }

    return handle;
}

// 0x4A9220
int sfxc_cached_close(int handle)
{
    if (!sfxc_handle_is_legal(handle)) {
        return -1;
    }

    SoundEffect* soundEffect = &(sfxc_handle_list[handle]);
    if (!cache_unlock(sfxc_pcache, soundEffect->cacheHandle)) {
        return -1;
    }

    // NOTE: Uninline.
    sfxc_handle_destroy(handle);

    return 0;
}

// 0x4A9274
int sfxc_cached_read(int handle, void* buf, unsigned int size)
{
    if (!sfxc_handle_is_legal(handle)) {
        return -1;
    }

    if (size == 0) {
        return 0;
    }

    SoundEffect* soundEffect = &(sfxc_handle_list[handle]);
    if (soundEffect->dataSize - soundEffect->position <= 0) {
        return 0;
    }

    size_t bytesToRead;
    // NOTE: Original code uses signed comparison.
    if ((int)size < (soundEffect->dataSize - soundEffect->position)) {
        bytesToRead = size;
    } else {
        bytesToRead = soundEffect->dataSize - soundEffect->position;
    }

    switch (sfxc_cmpr) {
    case 0:
        memcpy(buf, soundEffect->data + soundEffect->position, bytesToRead);
        break;
    case 1:
        if (sfxc_decode(handle, buf, bytesToRead) != 0) {
            return -1;
        }
        break;
    default:
        return -1;
    }

    soundEffect->position += bytesToRead;

    return bytesToRead;
}

// 0x4A9350
int sfxc_cached_write(int handle, const void* buf, unsigned int size)
{
    return -1;
}

// 0x4A9358
long sfxc_cached_seek(int handle, long offset, int origin)
{
    if (!sfxc_handle_is_legal(handle)) {
        return -1;
    }

    SoundEffect* soundEffect = &(sfxc_handle_list[handle]);

    int position;
    switch (origin) {
    case SEEK_SET:
        position = 0;
        break;
    case SEEK_CUR:
        position = soundEffect->position;
        break;
    case SEEK_END:
        position = soundEffect->dataSize;
        break;
    default:
        assert(false && "Should be unreachable");
    }

    long normalizedOffset = abs(offset);

    if (offset >= 0) {
        long remainingSize = soundEffect->dataSize - soundEffect->position;
        if (normalizedOffset > remainingSize) {
            normalizedOffset = remainingSize;
        }
        offset = position + normalizedOffset;
    } else {
        if (normalizedOffset > position) {
            return -1;
        }

        offset = position - normalizedOffset;
    }

    soundEffect->position = offset;

    return offset;
}

// 0x4A93F4
long sfxc_cached_tell(int handle)
{
    if (!sfxc_handle_is_legal(handle)) {
        return -1;
    }

    SoundEffect* soundEffect = &(sfxc_handle_list[handle]);
    return soundEffect->position;
}

// 0x4A9418
long sfxc_cached_file_size(int handle)
{
    if (!sfxc_handle_is_legal(handle)) {
        return 0;
    }

    SoundEffect* soundEffect = &(sfxc_handle_list[handle]);
    return soundEffect->dataSize;
}

// 0x4A9434
static int sfxc_effect_size(int tag, int* sizePtr)
{
    int size;
    if (soundEffectsListGetFileSize(tag, &size) == -1) {
        return -1;
    }

    *sizePtr = size;

    return 0;
}

// 0x4A945C
static int sfxc_effect_load(int tag, int* sizePtr, unsigned char* data)
{
    if (!soundEffectsListIsValidTag(tag)) {
        return -1;
    }

    int size;
    soundEffectsListGetFileSize(tag, &size);

    char* name;
    soundEffectsListGetFilePath(tag, &name);

    if (dbGetFileContents(name, data)) {
        internal_free(name);
        return -1;
    }

    internal_free(name);

    *sizePtr = size;

    return 0;
}

// 0x4A94CC
static void sfxc_effect_free(void* ptr)
{
    internal_free(ptr);
}

// 0x4A94D4
static int sfxc_handle_list_create()
{
    sfxc_handle_list = (SoundEffect*)internal_malloc(sizeof(*sfxc_handle_list) * SOUND_EFFECTS_MAX_COUNT);
    if (sfxc_handle_list == NULL) {
        return -1;
    }

    for (int index = 0; index < SOUND_EFFECTS_MAX_COUNT; index++) {
        SoundEffect* soundEffect = &(sfxc_handle_list[index]);
        soundEffect->used = false;
    }

    sfxc_files_open = 0;

    return 0;
}

// 0x4A9518
static void sfxc_handle_list_destroy()
{
    if (sfxc_files_open) {
        for (int index = 0; index < SOUND_EFFECTS_MAX_COUNT; index++) {
            SoundEffect* soundEffect = &(sfxc_handle_list[index]);
            if (!soundEffect->used) {
                sfxc_cached_close(index);
            }
        }
    }

    internal_free(sfxc_handle_list);
}

// 0x4A9550
static int sfxc_handle_create(int* handlePtr, int tag, void* data, CacheEntry* cacheHandle)
{
    if (sfxc_files_open >= SOUND_EFFECTS_MAX_COUNT) {
        return -1;
    }

    SoundEffect* soundEffect;
    int index;
    for (index = 0; index < SOUND_EFFECTS_MAX_COUNT; index++) {
        soundEffect = &(sfxc_handle_list[index]);
        if (!soundEffect->used) {
            break;
        }
    }

    if (index == SOUND_EFFECTS_MAX_COUNT) {
        return -1;
    }

    soundEffect->used = true;
    soundEffect->cacheHandle = cacheHandle;
    soundEffect->tag = tag;

    soundEffectsListGetDataSize(tag, &(soundEffect->dataSize));
    soundEffectsListGetFileSize(tag, &(soundEffect->fileSize));

    soundEffect->position = 0;
    soundEffect->dataPosition = 0;

    soundEffect->data = (unsigned char*)data;

    *handlePtr = index;

    return 0;
}

// NOTE: Inlined.
//
// 0x4A9604
static void sfxc_handle_destroy(int handle)
{
    // NOTE: There is an overflow when handle == SOUND_EFFECTS_MAX_COUNT, but
    // thanks to [sfxc_handle_is_legal] handle will always be less than
    // [SOUND_EFFECTS_MAX_COUNT].
    if (handle <= SOUND_EFFECTS_MAX_COUNT) {
        sfxc_handle_list[handle].used = false;
    }
}

// 0x4A961C
static bool sfxc_handle_is_legal(int handle)
{
    if (handle >= SOUND_EFFECTS_MAX_COUNT) {
        return false;
    }

    SoundEffect* soundEffect = &sfxc_handle_list[handle];

    if (!soundEffect->used) {
        return false;
    }

    if (soundEffect->dataSize < soundEffect->position) {
        return false;
    }

    return soundEffectsListIsValidTag(soundEffect->tag);
}

// 0x4A967C
static int sfxc_decode(int handle, void* buf, unsigned int size)
{
    if (!sfxc_handle_is_legal(handle)) {
        return -1;
    }

    SoundEffect* soundEffect = &(sfxc_handle_list[handle]);
    soundEffect->dataPosition = 0;

    int v1;
    int v2;
    int v3;
    SoundDecoder* soundDecoder = soundDecoderInit(sfxc_ad_reader, handle, &v1, &v2, &v3);

    if (soundEffect->position != 0) {
        void* temp = internal_malloc(soundEffect->position);
        if (temp == NULL) {
            soundDecoderFree(soundDecoder);
            return -1;
        }

        size_t bytesRead = soundDecoderDecode(soundDecoder, temp, soundEffect->position);
        internal_free(temp);

        if (bytesRead != soundEffect->position) {
            soundDecoderFree(soundDecoder);
            return -1;
        }
    }

    size_t bytesRead = soundDecoderDecode(soundDecoder, buf, size);
    soundDecoderFree(soundDecoder);

    if (bytesRead != size) {
        return -1;
    }

    return 0;
}

// 0x4A9774
static int sfxc_ad_reader(int handle, void* buf, unsigned int size)
{
    if (size == 0) {
        return 0;
    }

    SoundEffect* soundEffect = &(sfxc_handle_list[handle]);

    unsigned int bytesToRead = soundEffect->fileSize - soundEffect->dataPosition;
    if (size <= bytesToRead) {
        bytesToRead = size;
    }

    memcpy(buf, soundEffect->data + soundEffect->dataPosition, bytesToRead);

    soundEffect->dataPosition += bytesToRead;

    return bytesToRead;
}

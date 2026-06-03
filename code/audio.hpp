#ifndef WASAPI_H
#define WASAPI_H

#pragma comment(lib, "ole32.lib")
#include <mmdeviceapi.h>
#include <audioclient.h>

// todo: consider using a linked list as a freelist
typedef struct Audio_Cursor{
    //WaveCursor* next;
    u16* base;
    u32 sample_count;
    bool active;
    u32 index;

    u32 at;
    f32 volume;
    bool loop;
} Audio_Cursor;

#define AUDIO_CURSORS_MAX 32
typedef struct Audio_State{
    IAudioClient*        audio_client;
    IAudioRenderClient*  render_client;
    WAVEFORMATEX         format;

    u32 buffer_samples;
    REFERENCE_TIME default_device_period;
    REFERENCE_TIME minimum_device_period;
    bool running;

    Audio_Cursor cursors[AUDIO_CURSORS_MAX];
    s32 cursor_count;
    s32 free_cursor_idx;
    u32 free_cursors[AUDIO_CURSORS_MAX];
} Audio_State;
global Audio_State audio;

static void audio_init(u16 channels, u32 samples_per_sec, u16 bits_per_sample);
static void audio_release(void);
static Audio_Cursor* audio_play(Wave* wave, f32 volume, bool loop);
static bool audio_stop(Audio_Cursor* cursor);
static void audio_update(void);

static void audio_stream_start(void);
static void audio_stream_stop(void);
static Audio_Cursor* audio_cursor_alloc(void);
static void audio_cursors_clear(void);
static void audio_cursor_remove(Audio_Cursor* cursor);

//static HRESULT audio_play_sine(f32 freq);

#endif


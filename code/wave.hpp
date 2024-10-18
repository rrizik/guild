#ifndef WAVE_H
#define WAVE_H

typedef enum WaveChunkId{
    WaveChunkId_RIFF,
    WaveChunkId_WAVE,
    WaveChunkId_FMT,
    WaveChunkId_DATA,
    WaveChunkId_COUNT,
} WaveChunkId;

String8 wave_chunk_ids[WaveChunkId_COUNT] = {
    str8_literal("RIFF"),
    str8_literal("WAVE"),
    str8_literal("fmt"), // IMPORTANT: TODO: This might have to be "fmt " with a space
    str8_literal("data"),
};

typedef struct WaveHeader{
    u8 chunk_id[4];
    u32 chunk_size;
    u8 format[4];
} WaveHeader;

typedef struct WaveChunkInfo{
    u8 chunk_id[4];
    u32 chunk_size;
} WaveChunkInfo;

typedef struct WaveFormat{
    u16 audio_format;
    u16 channels;
    u32 samples_per_sec;
    u32 bytes_per_sec;
    u16 block_align;
    u16 bits_per_sample;
    u16 ext_size;
    u16 valid_bits_per_sample;
    u32 channel_mask;
    u32 sub_format[4];
} WaveFormat;

typedef struct Wave{
    WaveFormat format;
    u16* base;
    u32 sample_count; // size in samples, not bytes
} Wave;

static Wave wave_file_read(Arena* arena, String8 dir, String8 filename);

#endif

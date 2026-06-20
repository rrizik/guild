#ifndef ASSET_H
#define ASSET_H

typedef enum FontAsset{
    FontAsset_None,
    FontAsset_Arial, // not monospace
    FontAsset_Arial1,
    FontAsset_Arial2,
    FontAsset_Arial3,
    FontAsset_Arial4,
    FontAsset_Arial5,
    FontAsset_Arial6,
    FontAsset_Golos,
    FontAsset_Consolas,

    FontAsset_Count,
} FontAsset;

typedef enum WaveAsset{
    WaveAsset_None,
    WaveAsset_Track1,
    WaveAsset_Track2,
    WaveAsset_Track3,
    WaveAsset_Track4,
    WaveAsset_Track5,
    WaveAsset_Rail1,
    WaveAsset_Rail2,
    WaveAsset_Rail3,
    WaveAsset_Rail4,
    WaveAsset_Rail5,
    WaveAsset_GameWon,
    WaveAsset_GameLost,
    WaveAsset_AsteroidBreak1,
    WaveAsset_AsteroidBreak2,
    WaveAsset_AsteroidBreak3,
    WaveAsset_ShipExplode,
    WaveAsset_Music,

    WaveAsset_Count,
} WaveAsset;

typedef enum TextureAsset{
    TextureAsset_None,


    TextureAsset_Grass1,
    //TextureAsset_Grass2,
    //TextureAsset_Grass3,
    //TextureAsset_Grass4,
    //TextureAsset_Grass5,
    //TextureAsset_Grass6,
    //TextureAsset_Grass7,
    //TextureAsset_Grass8,

    TextureAsset_Water1,
    TextureAsset_Lava1,
    TextureAsset_Wood1,
    TextureAsset_Skeleton1,
    TextureAsset_Char_Idle,
    TextureAsset_Char_Punch,
    TextureAsset_Char_Run,
    TextureAsset_Char_Walk,

    TextureAsset_Human_Idle,
    TextureAsset_Human_Dmg,
    TextureAsset_Human_Walk,
    TextureAsset_Human_Attack,
    TextureAsset_Human_Charged_Attack,
    TextureAsset_Human_Jump,
    TextureAsset_Human_Spin_Die,
    TextureAsset_Human_Soul_Die,

    TextureAsset_Orc_Idle,
    TextureAsset_Orc_Walk,
    TextureAsset_Orc_Attack,
    TextureAsset_Orc_Charged_Attack,
    TextureAsset_Orc_Jump,
    TextureAsset_Orc_Dmg,
    TextureAsset_Orc_Die,


    TextureAsset_Castle1,

    TextureAsset_White,

    TextureAsset_Count,
} TextureAsset;

typedef struct Assets{
    Wave    waves[WaveAsset_Count];
    Font    fonts[FontAsset_Count];
    Texture textures[TextureAsset_Count];
} Assets;
static void load_texture(Arena* arena, TextureAsset texture_id, String8 build_path, String8 path);
static void load_audio(Arena* arena, WaveAsset audio_id, String8 build_path, String8 path);
static void load_font(Arena* arena, FontAsset font_id, String8 build_path, String8 path);
static void assets_load(Arena* arena);

static Wave* a_get_texture(s32 texture_id);
static Texture* a_get_audio(s32 audio_id);
static Font* a_get_font(s32 font_id);

global Assets assets;

#endif

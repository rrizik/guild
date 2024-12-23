#ifndef ASSET_H
#define ASSET_H

typedef enum FontAsset{
    FontAsset_None,
    FontAsset_Arial,
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

    TextureAsset_Castle1,

    TextureAsset_White,

    TextureAsset_Count,
} TextureAsset;

typedef struct Assets{
    Wave    waves[WaveAsset_Count];
    Font    fonts[FontAsset_Count];
    Texture textures[TextureAsset_Count];
} Assets;
static void load_assets(Arena* arena, Assets* assets);

#endif

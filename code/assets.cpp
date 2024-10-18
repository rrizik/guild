#ifndef ASSET_C
#define ASSET_C


static void
load_assets(Arena* arena, Assets* assets){

    ScratchArena scratch = begin_scratch();
    String8 build_path = os_application_path(scratch.arena);

    Bitmap bm;
    bm = stb_load_image(scratch.arena, build_path, str8_literal("sprites/grass1.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Grass1].view, &bm);
    bm = stb_load_image(scratch.arena, build_path, str8_literal("sprites/grass2.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Grass2].view, &bm);
    bm = stb_load_image(scratch.arena, build_path, str8_literal("sprites/grass3.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Grass3].view, &bm);
    bm = stb_load_image(scratch.arena, build_path, str8_literal("sprites/grass4.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Grass4].view, &bm);
    bm = stb_load_image(scratch.arena, build_path, str8_literal("sprites/grass5.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Grass5].view, &bm);
    bm = stb_load_image(scratch.arena, build_path, str8_literal("sprites/grass6.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Grass6].view, &bm);
    bm = stb_load_image(scratch.arena, build_path, str8_literal("sprites/grass7.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Grass7].view, &bm);
    bm = stb_load_image(scratch.arena, build_path, str8_literal("sprites/grass8.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Grass8].view, &bm);
    bm = stb_load_image(scratch.arena, build_path, str8_literal("sprites/castle1.bmp"));
    d3d_init_texture_resource(&assets->textures[TextureAsset_Castle1].view, &bm);
    Texture texture = {white_shader_resource};
    assets->textures[TextureAsset_White] = texture;

    assets->waves[WaveAsset_Track1] = wave_file_read(arena, build_path, str8_literal("sounds/track1.wav"));
    assets->waves[WaveAsset_Track2] = wave_file_read(arena, build_path, str8_literal("sounds/track2.wav"));
    assets->waves[WaveAsset_Track3] = wave_file_read(arena, build_path, str8_literal("sounds/track3.wav"));
    assets->waves[WaveAsset_Track4] = wave_file_read(arena, build_path, str8_literal("sounds/track4.wav"));
    assets->waves[WaveAsset_Track5] = wave_file_read(arena, build_path, str8_literal("sounds/track5.wav"));
    assets->waves[WaveAsset_Rail1]  = wave_file_read(arena, build_path, str8_literal("sounds/rail1.wav"));
    assets->waves[WaveAsset_Rail2]  = wave_file_read(arena, build_path, str8_literal("sounds/rail2.wav"));
    assets->waves[WaveAsset_Rail3]  = wave_file_read(arena, build_path, str8_literal("sounds/rail3.wav"));
    assets->waves[WaveAsset_Rail4]  = wave_file_read(arena, build_path, str8_literal("sounds/rail4.wav"));
    assets->waves[WaveAsset_Rail5]  = wave_file_read(arena, build_path, str8_literal("sounds/rail5.wav"));

    assets->waves[WaveAsset_GameWon] = wave_file_read(arena, build_path, str8_literal("sounds/game_won.wav"));
    assets->waves[WaveAsset_GameLost] = wave_file_read(arena, build_path, str8_literal("sounds/game_lost.wav"));
    assets->waves[WaveAsset_AsteroidBreak1] = wave_file_read(arena, build_path, str8_literal("sounds/asteroid_break1.wav"));
    assets->waves[WaveAsset_AsteroidBreak2] = wave_file_read(arena, build_path, str8_literal("sounds/asteroid_break2.wav"));
    assets->waves[WaveAsset_AsteroidBreak3] = wave_file_read(arena, build_path, str8_literal("sounds/asteroid_break3.wav"));
    assets->waves[WaveAsset_ShipExplode] = wave_file_read(arena, build_path, str8_literal("sounds/ship_explode.wav"));
    assets->waves[WaveAsset_Music] = wave_file_read(arena, build_path, str8_literal("sounds/music.wav"));

    assets->fonts[FontAsset_Arial] = font_ttf_read(arena, build_path, str8_literal("fonts/arial.ttf"), 16); // not monospace
    assets->fonts[FontAsset_Golos] = font_ttf_read(arena, build_path, str8_literal("fonts/GolosText-Regular.ttf"), 16);
    assets->fonts[FontAsset_Consolas] = font_ttf_read(arena, build_path, str8_literal("fonts/consola.ttf"), 16); // monospace

    end_scratch(scratch);
}


#endif

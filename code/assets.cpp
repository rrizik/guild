#ifndef ASSET_C
#define ASSET_C

static void
load_font(Arena* arena, FontAsset font_id, String8 build_path, String8 path, s32 size){
    assets.fonts[font_id] = font_ttf_read(arena, build_path, path, size);
    assets.fonts[font_id].texture_id = font_id;
    s32 a = 1;
}

static void
load_audio(Arena* arena, WaveAsset audio_id, String8 build_path, String8 path){
    assets.waves[audio_id] = wave_file_read(arena, build_path, path);
}

static void
load_texture(Arena* arena, TextureAsset texture_id, String8 build_path, String8 path){
    Bitmap bm = stb_load_image(arena, build_path, path);
    d3d_init_texture_resource(&assets.textures[texture_id], &bm);
}

static Font*
a_get_font(s32 font_id){
    return(&assets.fonts[font_id]);
}

static void
assets_load(Arena* arena){
    ScratchArena scratch = begin_scratch();
    String8 build_path = os_application_path(scratch.arena);

    Texture white_texture = {white_shader_resource};
    white_texture.width, white_texture.height = 1;
    assets.textures[TextureAsset_White] = white_texture;

    Texture magenta_texture = {magenta_shader_resource};
    magenta_texture.width, magenta_texture.height = 1;
    assets.textures[TextureAsset_Magenta] = magenta_texture;

    load_texture(scratch.arena, TextureAsset_Human_Attack, build_path, str8_lit("sprites/base_humanoids/human/base_human/human_attack.png"));
    load_texture(scratch.arena, TextureAsset_Human_Charged_Attack, build_path, str8_lit("sprites/base_humanoids/human/base_human/human_charged_attack.png"));
    load_texture(scratch.arena, TextureAsset_Human_Dmg, build_path, str8_lit("sprites/base_humanoids/human/base_human/human_dmg.png"));
    load_texture(scratch.arena, TextureAsset_Human_Idle, build_path, str8_lit("sprites/base_humanoids/human/base_human/human_idle.png"));
    load_texture(scratch.arena, TextureAsset_Human_Jump, build_path, str8_lit("sprites/base_humanoids/human/base_human/human_jump.png"));
    load_texture(scratch.arena, TextureAsset_Human_Soul_Die, build_path, str8_lit("sprites/base_humanoids/human/base_human/human_soul_die.png"));
    load_texture(scratch.arena, TextureAsset_Human_Spin_Die, build_path, str8_lit("sprites/base_humanoids/human/base_human/human_spin_die.png"));
    load_texture(scratch.arena, TextureAsset_Human_Walk, build_path, str8_lit("sprites/base_humanoids/human/base_human/human_walk.png"));

    load_texture(scratch.arena, TextureAsset_Orc_Attack, build_path, str8_lit("sprites/base_humanoids/orc/base_orc/orc_base_attack.png"));
    load_texture(scratch.arena, TextureAsset_Orc_Charged_Attack, build_path, str8_lit("sprites/base_humanoids/orc/base_orc/orc_charged_attack.png"));
    load_texture(scratch.arena, TextureAsset_Orc_Dmg, build_path, str8_lit("sprites/base_humanoids/orc/base_orc/orc_dmg.png"));
    load_texture(scratch.arena, TextureAsset_Orc_Idle, build_path, str8_lit("sprites/base_humanoids/orc/base_orc/orc_idle.png"));
    load_texture(scratch.arena, TextureAsset_Orc_Jump, build_path, str8_lit("sprites/base_humanoids/orc/base_orc/orc_jump.png"));
    load_texture(scratch.arena, TextureAsset_Orc_Die, build_path, str8_lit("sprites/base_humanoids/orc/base_orc/orc_die.png"));
    load_texture(scratch.arena, TextureAsset_Orc_Walk, build_path, str8_lit("sprites/base_humanoids/orc/base_orc/orc_walk.png"));

    load_texture(scratch.arena, TextureAsset_Grass1, build_path, str8_lit("sprites/tiles/grass1.png"));
    load_texture(scratch.arena, TextureAsset_Water1, build_path, str8_lit("sprites/tiles/water1.png"));
    load_texture(scratch.arena, TextureAsset_Lava1, build_path, str8_lit("sprites/tiles/lava1.png"));
    load_texture(scratch.arena, TextureAsset_Wood1, build_path, str8_lit("sprites/tiles/wood1.png"));

    load_texture(scratch.arena, TextureAsset_Castle1, build_path, str8_lit("sprites/tiles/castle1.bmp"));

    load_audio(arena, WaveAsset_Track1, build_path, str8_lit("sounds/track1.wav"));
    load_audio(arena, WaveAsset_Track1, build_path, str8_lit("sounds/track2.wav"));
    load_audio(arena, WaveAsset_Track1, build_path, str8_lit("sounds/track3.wav"));
    load_audio(arena, WaveAsset_Track1, build_path, str8_lit("sounds/track4.wav"));
    load_audio(arena, WaveAsset_Track1, build_path, str8_lit("sounds/track5.wav"));
    load_audio(arena, WaveAsset_Track1, build_path, str8_lit("sounds/rail1.wav"));
    load_audio(arena, WaveAsset_Track1, build_path, str8_lit("sounds/rail2.wav"));
    load_audio(arena, WaveAsset_Track1, build_path, str8_lit("sounds/rail3.wav"));
    load_audio(arena, WaveAsset_Track1, build_path, str8_lit("sounds/rail4.wav"));
    load_audio(arena, WaveAsset_Track1, build_path, str8_lit("sounds/rail5.wav"));

    load_audio(arena, WaveAsset_GameWon, build_path, str8_lit("sounds/game_won.wav"));
    load_audio(arena, WaveAsset_GameLost, build_path, str8_lit("sounds/game_lost.wav"));
    load_audio(arena, WaveAsset_AsteroidBreak1, build_path, str8_lit("sounds/asteroid_break1.wav"));
    load_audio(arena, WaveAsset_AsteroidBreak2, build_path, str8_lit("sounds/asteroid_break2.wav"));
    load_audio(arena, WaveAsset_AsteroidBreak3, build_path, str8_lit("sounds/asteroid_break3.wav"));
    load_audio(arena, WaveAsset_ShipExplode, build_path, str8_lit("sounds/ship_explode.wav"));
    load_audio(arena, WaveAsset_Music, build_path, str8_lit("sounds/music.wav"));

    load_font(arena, FontAsset_Arial, build_path, str8_lit("fonts/arial.ttf"), 32);
    load_font(arena, FontAsset_Arial1, build_path, str8_lit("fonts/arial.ttf"), 24);
    load_font(arena, FontAsset_Arial2, build_path, str8_lit("fonts/arial.ttf"), 16);
    load_font(arena, FontAsset_Arial3, build_path, str8_lit("fonts/arial.ttf"), 8);
    load_font(arena, FontAsset_Arial4, build_path, str8_lit("fonts/arial.ttf"), 4);
    load_font(arena, FontAsset_Arial5, build_path, str8_lit("fonts/arial.ttf"), 2);
    load_font(arena, FontAsset_Arial6, build_path, str8_lit("fonts/arial.ttf"), 1);
    load_font(arena, FontAsset_Golos, build_path, str8_lit("fonts/GolosText-Regular.ttf"), 16);
    load_font(arena, FontAsset_Consolas, build_path, str8_lit("fonts/consola.ttf"), 16);

    end_scratch(scratch);
}


#endif

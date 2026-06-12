#ifndef ASSET_C
#define ASSET_C

static void
load_texture(Arena* arena, TextureAsset texture_id, String8 build_path, String8 path){
    Bitmap bm = stb_load_image(arena, build_path, path);
    d3d_init_texture_resource(&assets.textures[texture_id], &bm);
}

static void
assets_load(Arena* arena){

    ScratchArena scratch = begin_scratch();
    String8 build_path = os_application_path(scratch.arena);

    load_texture(scratch.arena, TextureAsset_Human_Attack, build_path, str8_lit("sprites/Base_Humanoids/Human/Base_Human/HumanAttack.png"));
    load_texture(scratch.arena, TextureAsset_Human_Charged_Attack, build_path, str8_lit("sprites/Base_Humanoids/Human/Base_Human/HumanChargedAttack.png"));
    load_texture(scratch.arena, TextureAsset_Human_Dmg, build_path, str8_lit("sprites/Base_Humanoids/Human/Base_Human/HumanDmg.png"));
    load_texture(scratch.arena, TextureAsset_Human_Idle, build_path, str8_lit("sprites/Base_Humanoids/Human/Base_Human/HumanIdle.png"));
    load_texture(scratch.arena, TextureAsset_Human_Jump, build_path, str8_lit("sprites/Base_Humanoids/Human/Base_Human/HumanJump.png"));
    load_texture(scratch.arena, TextureAsset_Human_Soul_Die, build_path, str8_lit("sprites/Base_Humanoids/Human/Base_Human/HumanSoulDie.png"));
    load_texture(scratch.arena, TextureAsset_Human_Spin_Die, build_path, str8_lit("sprites/Base_Humanoids/Human/Base_Human/HumanSpinDie.png"));
    load_texture(scratch.arena, TextureAsset_Human_Walk, build_path, str8_lit("sprites/Base_Humanoids/Human/Base_Human/HumanWalk.png"));

    load_texture(scratch.arena, TextureAsset_Orc_Attack, build_path, str8_lit("sprites/Base_Humanoids/Orc/Base_Orc/OrcBaseAttack.png"));
    load_texture(scratch.arena, TextureAsset_Orc_Charged_Attack, build_path, str8_lit("sprites/Base_Humanoids/Orc/Base_Orc/OrcChargedAttack.png"));
    load_texture(scratch.arena, TextureAsset_Orc_Dmg, build_path, str8_lit("sprites/Base_Humanoids/Orc/Base_Orc/OrcDmg.png"));
    load_texture(scratch.arena, TextureAsset_Orc_Idle, build_path, str8_lit("sprites/Base_Humanoids/Orc/Base_Orc/OrcIdle.png"));
    load_texture(scratch.arena, TextureAsset_Orc_Jump, build_path, str8_lit("sprites/Base_Humanoids/Orc/Base_Orc/OrcJump.png"));
    load_texture(scratch.arena, TextureAsset_Orc_Die, build_path, str8_lit("sprites/Base_Humanoids/Orc/Base_Orc/OrcDie.png"));
    load_texture(scratch.arena, TextureAsset_Orc_Walk, build_path, str8_lit("sprites/Base_Humanoids/Orc/Base_Orc/OrcWalk.png"));

    Bitmap bm;
    //bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/orc_idle.png"));
    //d3d_init_texture_resource(&assets.textures[TextureAsset_Orc_Idle], &bm);
    //bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/orc_walk.png"));
    //d3d_init_texture_resource(&assets.textures[TextureAsset_Orc_Walk], &bm);
    //bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/orc_attack.png"));
    //d3d_init_texture_resource(&assets.textures[TextureAsset_Orc_Attack], &bm);
    //bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/orc_jump.png"));
    //d3d_init_texture_resource(&assets.textures[TextureAsset_Orc_Jump], &bm);
    //bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/orc_die.png"));
    //(&assets.textures[TextureAsset_Orc_Die], &bm);

    bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/tiles/grass1.png"));
    d3d_init_texture_resource(&assets.textures[TextureAsset_Grass1], &bm);
    //bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/grass2.bmp"));
    //d3d_init_texture_resource(&assets.textures[TextureAsset_Grass2], &bm);
    //bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/grass3.bmp"));
    //d3d_init_texture_resource(&assets.textures[TextureAsset_Grass3], &bm);
    //bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/grass4.bmp"));
    //d3d_init_texture_resource(&assets.textures[TextureAsset_Grass4], &bm);
    //bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/grass5.bmp"));
    //d3d_init_texture_resource(&assets.textures[TextureAsset_Grass5], &bm);
    //bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/grass6.bmp"));
    //d3d_init_texture_resource(&assets.textures[TextureAsset_Grass6], &bm);
    //bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/grass7.bmp"));
    //d3d_init_texture_resource(&assets.textures[TextureAsset_Grass7], &bm);
    //bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/grass8.bmp"));
    //d3d_init_texture_resource(&assets.textures[TextureAsset_Grass8], &bm);
    bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/tiles/castle1.bmp"));
    d3d_init_texture_resource(&assets.textures[TextureAsset_Castle1], &bm);

    bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/tiles/water1.png"));
    d3d_init_texture_resource(&assets.textures[TextureAsset_Water1], &bm);
    bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/tiles/lava1.png"));
    d3d_init_texture_resource(&assets.textures[TextureAsset_Lava1], &bm);
    bm = stb_load_image(scratch.arena, build_path, str8_lit("sprites/tiles/wood1.png"));
    d3d_init_texture_resource(&assets.textures[TextureAsset_Wood1], &bm);

    Texture texture = {white_shader_resource};
    texture.width, texture.height = 1;
    assets.textures[TextureAsset_White] = texture;

    assets.waves[WaveAsset_Track1] = wave_file_read(arena, build_path, str8_lit("sounds/track1.wav"));
    assets.waves[WaveAsset_Track2] = wave_file_read(arena, build_path, str8_lit("sounds/track2.wav"));
    assets.waves[WaveAsset_Track3] = wave_file_read(arena, build_path, str8_lit("sounds/track3.wav"));
    assets.waves[WaveAsset_Track4] = wave_file_read(arena, build_path, str8_lit("sounds/track4.wav"));
    assets.waves[WaveAsset_Track5] = wave_file_read(arena, build_path, str8_lit("sounds/track5.wav"));
    assets.waves[WaveAsset_Rail1]  = wave_file_read(arena, build_path, str8_lit("sounds/rail1.wav"));
    assets.waves[WaveAsset_Rail2]  = wave_file_read(arena, build_path, str8_lit("sounds/rail2.wav"));
    assets.waves[WaveAsset_Rail3]  = wave_file_read(arena, build_path, str8_lit("sounds/rail3.wav"));
    assets.waves[WaveAsset_Rail4]  = wave_file_read(arena, build_path, str8_lit("sounds/rail4.wav"));
    assets.waves[WaveAsset_Rail5]  = wave_file_read(arena, build_path, str8_lit("sounds/rail5.wav"));

    assets.waves[WaveAsset_GameWon] = wave_file_read(arena, build_path, str8_lit("sounds/game_won.wav"));
    assets.waves[WaveAsset_GameLost] = wave_file_read(arena, build_path, str8_lit("sounds/game_lost.wav"));
    assets.waves[WaveAsset_AsteroidBreak1] = wave_file_read(arena, build_path, str8_lit("sounds/asteroid_break1.wav"));
    assets.waves[WaveAsset_AsteroidBreak2] = wave_file_read(arena, build_path, str8_lit("sounds/asteroid_break2.wav"));
    assets.waves[WaveAsset_AsteroidBreak3] = wave_file_read(arena, build_path, str8_lit("sounds/asteroid_break3.wav"));
    assets.waves[WaveAsset_ShipExplode] = wave_file_read(arena, build_path, str8_lit("sounds/ship_explode.wav"));
    assets.waves[WaveAsset_Music] = wave_file_read(arena, build_path, str8_lit("sounds/music.wav"));

    assets.fonts[FontAsset_Arial]  = font_ttf_read(arena, build_path, str8_lit("fonts/arial.ttf"), 16);
    assets.fonts[FontAsset_Arial1] = font_ttf_read(arena, build_path, str8_lit("fonts/arial.ttf"), 32);
    assets.fonts[FontAsset_Arial2] = font_ttf_read(arena, build_path, str8_lit("fonts/arial.ttf"), 16);
    assets.fonts[FontAsset_Arial3] = font_ttf_read(arena, build_path, str8_lit("fonts/arial.ttf"), 8);
    assets.fonts[FontAsset_Arial4] = font_ttf_read(arena, build_path, str8_lit("fonts/arial.ttf"), 4);
    assets.fonts[FontAsset_Arial5] = font_ttf_read(arena, build_path, str8_lit("fonts/arial.ttf"), 2);
    assets.fonts[FontAsset_Arial6] = font_ttf_read(arena, build_path, str8_lit("fonts/arial.ttf"), 1);

    assets.fonts[FontAsset_Golos] = font_ttf_read(arena, build_path, str8_lit("fonts/GolosText-Regular.ttf"), 16);
    assets.fonts[FontAsset_Consolas] = font_ttf_read(arena, build_path, str8_lit("fonts/consola.ttf"), 16); // monospace

    end_scratch(scratch);
}


#endif

#ifndef MAIN_H
#define MAIN_H

#if DEBUG
#define ENABLE_ASSERT 1
#endif

#pragma comment(lib, "user32")
#include "base_inc.h"
#include "win32_base_inc.h"

#define PROFILER 1
#include "profiler.h"

#include "input.hpp"
#include "clock.hpp"
#include "window.hpp"
#include "camera.hpp"
#include "bitmap.hpp"
#include "d3d11_init.hpp"
#include "font.hpp"
#include "wave.hpp"
#include "wasapi.hpp"
#include "assets.hpp"
#include "entity.hpp"
#include "console.hpp"
#include "command.hpp"
#include "draw.hpp"
#include "ui.hpp"
#include "game.hpp"

#include "input.cpp"
#include "clock.cpp"
#include "camera.cpp"
#include "bitmap.cpp"
#include "d3d11_init.cpp"
#include "font.cpp"
#include "wave.cpp"
#include "wasapi.cpp"
#include "assets.cpp"
#include "entity.cpp"
#include "console.cpp"
#include "command.cpp"
#include "draw.cpp"
#include "ui.cpp"


#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define WORLD_UNITS_WIDTH 1000
#define WORLD_UNITS_HEIGHT 1000
//#define SCREEN_WIDTH 1920
//#define SCREEN_HEIGHT 1080
s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type);
static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param);
static Window win32_window_create(const wchar* window_name, u32 width, u32 height);

typedef struct Memory{
    void* base;
    u64 size;

    void* permanent_base;
    u64 permanent_size;
    void* transient_base;
    u64 transient_size;

    bool initialized;
} Memory;
global Memory memory;
static void init_memory(u64 permanent, u64 transient);
static void show_cursor(bool show);
static void init_paths(Arena* arena);

global String8 build_path;
global String8 fonts_path;
global String8 shaders_path;
global String8 saves_path;
global String8 sprites_path;
global String8 sounds_path;

global u64 frame_count;
global bool pause;
global bool should_quit;
global Arena* global_arena = os_make_arena(MB(100));
global Assets assets;


static bool tracking_mouse = false;
#define MAX_LEVELS 3
#define MAX_LIVES 3
#define WIN_SCORE 3000
#define ENTITIES_MAX 4096
#define WORLD_SIZE 100
#define GRID_SIZE 10

typedef struct State{
    Arena arena;
    SceneState scene_state;
    GameState game_state;
    GameResult game_result;
    bool editor;

    Entity entities[ENTITIES_MAX];
    u32 entities_count;

    u32 generation[ENTITIES_MAX];
    u32 free_entities[ENTITIES_MAX];
    u32 free_entities_at;

    Font* font;

    u32 world_grid[WORLD_SIZE * WORLD_SIZE];
    Entity* castle;

} State, PermanentMemory;
global State* state;

typedef struct TransientMemory{
    Arena arena;
    Arena *frame_arena;
    Arena *asset_arena;
    Arena *ui_arena;
    Arena *hash_arena;
    Arena *batch_arena;
} TransientMemory, TState;
global TState* ts;

static void os_fullscreen_mode(Window* window);
static void os_windowed_mode(Window* window);
static void change_resolution(Window* window, f32 width, f32 height);

// todo(rr): this guy
#include "game.cpp"

// todo(rr): get rid of this
global f32 text_padding = 20;

static void draw_entities(State* state){
    // todo(rr): later change to screen space in shader with matrix multiplication (identify matrix)
    for(s32 index = 0; index < array_count(state->entities); ++index){
        Entity *e = state->entities + index;

        Quad quad = quad_from_entity(e);
        if(has_flags(e->flags, EntityFlag_Active)){

            switch(e->type){
                case EntityType_Quad:{
                    quad = rotate_quad(quad, e->deg, e->pos);
                    quad = quad_screen_from_world(quad);

                    draw_quad(quad, e->color);
                } break;
            }
        }
    }
    for(s32 index = 0; index < array_count(state->entities); ++index){
        Entity *e = state->entities + index;

        Quad quad = quad_from_entity(e);
        if(has_flags(e->flags, EntityFlag_Active)){

            switch(e->type){
                case EntityType_Texture:{
                    quad = rotate_quad(quad, e->deg, e->pos);
                    quad = quad_screen_from_world(quad);

                    set_texture(&r_assets->textures[e->texture]);
                    draw_texture(quad, e->color);
                } break;
            }
        }
    }
    for(s32 index = 0; index < array_count(state->entities); ++index){
        Entity *e = state->entities + index;

        Quad quad = quad_from_entity(e);
        if(has_flags(e->flags, EntityFlag_Active)){

            switch(e->type){
                case EntityType_Castle:{
                    //quad = rotate_quad(quad, e->deg, e->pos);
                    quad = quad_screen_from_world(quad);

                    set_texture(&r_assets->textures[e->texture]);
                    draw_texture(quad, e->color);
                } break;
            }
        }
    }
}

static void
debug_draw_render_batches(){

    ui_begin(ts->ui_arena);

    ui_push_pos_x(100);
    ui_push_pos_y(300);
    ui_push_size_w(ui_size_children(0));
    ui_push_size_h(ui_size_children(0));

    ui_push_border_thickness(10);
    ui_push_background_color(DEFAULT);
    UI_Box* box1 = ui_box(str8_literal("box1##4"),
                          UI_BoxFlag_DrawBackground|
                          UI_BoxFlag_Draggable|
                          UI_BoxFlag_Clickable);
    ui_push_parent(box1);
    ui_pop_pos_x();
    ui_pop_pos_y();

    ui_push_size_w(ui_size_text(0));
    ui_push_size_h(ui_size_text(0));

    ui_push_text_color(LIGHT_GRAY);


    String8 title = str8_format(ts->frame_arena, "Render Batches Count: %i", render_batches.count);
    ui_label(title);

    s32 count = 0;
    for(RenderBatch* batch = render_batches.first; batch != 0; batch = batch->next){
        if(count < 25){
            String8 batch_str = str8_format(ts->frame_arena, "%i - %i/%i ##%i", batch->id, batch->count, batch->cap, batch->id);
            ui_label(batch_str);
        }
        count++;
    }

    ui_layout();
    ui_draw(ui_root());
    ui_end();
}


#endif
        //ui_begin(ts->ui_arena);

        //ui_push_background_color(ORANGE);
        //ui_push_pos_x(50);
        //ui_push_pos_y(50);
        //ui_push_size_w(ui_size_children(0));
        //ui_push_size_h(ui_size_children(0));
        //ui_push_border_thickness(10);

        //UI_Box* box1 = ui_box(str8_literal("box1"), UI_BoxFlag_DrawBackground|UI_BoxFlag_Draggable|UI_BoxFlag_Clickable);
        //ui_push_parent(box1);
        //ui_pop_border_thickness();
        //ui_pop_pos_x();
        //ui_pop_pos_y();

        //ui_push_size_w(ui_size_pixel(100, 0));
        //ui_push_size_h(ui_size_pixel(50, 0));
        //ui_push_background_color(BLUE);
        //ui_label(str8_literal("MAH LAHBEL"));
        //if(ui_button(str8_literal("button 1")).pressed_left){
        //    print("button 1: PRESSED\n");
        //    wasapi_play(WaveAsset_Rail1, 0.1f, false);
        //}
        //ui_spacer(10);

        //ui_push_size_w(ui_size_pixel(50, 0));
        //ui_push_size_h(ui_size_pixel(50, 0));
        //ui_push_background_color(GREEN);
        //if(ui_button(str8_literal("button 2")).pressed_left){
        //    print("button 2: PRESSED\n");
        //}
        //ui_pop_background_color();
        //ui_pop_background_color();

        //ui_spacer(50);
        //ui_push_size_w(ui_size_children(0));
        //ui_push_size_h(ui_size_children(0));
        //ui_push_layout_axis(Axis_X);
        //ui_push_background_color(MAGENTA);
        //UI_Box* box2 = ui_box(str8_literal("box2"));
        //ui_push_parent(box2);
        //ui_pop_background_color();

        //ui_pop_size_w();
        //ui_pop_size_h();
        //ui_pop_size_w();
        //ui_pop_size_h();
        //ui_push_size_w(ui_size_pixel(100, 1));
        //ui_push_size_h(ui_size_pixel(50, 1));
        //ui_push_background_color(TEAL);
        //if(ui_button(str8_literal("button 3")).pressed_left){
        //    print("button 3: PRESSED\n");
        //}
        //ui_spacer(50);
        //ui_push_background_color(RED);
        //if(ui_button(str8_literal("button 4")).pressed_left){
        //    print("button 4: PRESSED\n");
        //}
        //ui_spacer(50);
        //ui_pop_background_color();
        //if(ui_button(str8_literal("button 5")).pressed_left){
        //    print("button 5: PRESSED\n");
        //}
        //ui_pop_parent();

        //ui_spacer(50);
        //ui_push_size_w(ui_size_children(0));
        //ui_push_size_h(ui_size_children(0));
        //ui_push_layout_axis(Axis_Y);
        //ui_push_background_color(MAGENTA);
        //UI_Box* box3 = ui_box(str8_literal("box3"));
        //ui_push_parent(box3);
        //ui_pop_background_color();

        //ui_push_size_w(ui_size_pixel(100, 0));
        //ui_push_size_h(ui_size_pixel(100, 0));
        //ui_push_background_color(YELLOW);
        //if(ui_button(str8_literal("button 6")).pressed_left){
        //    print("button 6: PRESSED\n");
        //}
        //ui_spacer(50);
        //ui_push_background_color(DARK_GRAY);
        //ui_push_size_w(ui_size_text(0));
        //ui_push_size_h(ui_size_text(0));
        //ui_push_text_padding(50);
        //if(ui_button(str8_literal("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz")).pressed_left){
        //    print("button 7: PRESSED\n");
        //}
        //ui_pop_text_padding();
        //ui_pop_parent();

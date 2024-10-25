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
#include "draw.hpp"
#include "ui.hpp"

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
#include "draw.cpp"
#include "ui.cpp"



#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type);
static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param);
static Window win32_window_create(const wchar* window_name, u32 width, u32 height);

global Arena* global_arena = os_make_arena(MB(100));
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

static void init_paths(Arena* arena);
global String8 build_path;
global String8 fonts_path;
global String8 shaders_path;
global String8 saves_path;
global String8 sprites_path;
global String8 sounds_path;

global u64 frame_count;
global bool should_quit;
global Assets assets;

global v2 world_mouse_record;
global Camera2D world_camera_record;

global bool game_in_focus = true;
global bool tracking_mouse = false;
#define ENTITIES_MAX 4096

#define WORLD_WIDTH_MAX 1000
#define WORLD_HEIGHT_MAX 1000
f32 grid_size = 10;
typedef struct State{
    Arena arena;
    bool editor;

    Entity entities[ENTITIES_MAX];
    u32 entities_count;

    u32 generation[ENTITIES_MAX];
    u32 free_entities[ENTITIES_MAX];
    u32 free_entities_at;

    Font* font;

    s32 world_grid[WORLD_WIDTH_MAX * WORLD_HEIGHT_MAX];
    Entity* castle;

    bool terrain_selected;
    s32 selected_terrain;

    f32 world_width;
    f32 world_height;

    String8 current_world;

} State, PermanentMemory;
global State* state;

typedef struct TransientMemory{
    Arena arena;
    Arena *frame_arena;
    Arena *asset_arena;
    Arena *ui_arena;
    Arena *hash_arena;
    Arena *batch_arena;
    Arena *data_arena;
} TransientMemory, TState;
global TState* ts;

static void show_cursor(bool show);
static void os_fullscreen_mode(Window* window);
static void os_windowed_mode(Window* window);
static void change_resolution(Window* window, f32 width, f32 height);

static void sim_game(void);
static v2 grid_pos_from_cell(f32 x, f32 y);
static v2 grid_cell_from_pos(f32 x, f32 y);
static v2 grid_cell_center(f32 x, f32 y);

// todo(rr): get rid of this
global f32 text_padding = 20;

static    void remove_entity(Entity* e);
static Entity* add_entity(EntityType type);
static Entity* add_quad(v2 pos, v2 dim, RGBA color);
static Entity* add_texture(u32 texture, v2 pos, v2 dim, RGBA color=WHITE, u32 flags = 0);
static Entity* add_castle(u32 texture, v2 pos, v2 dim, RGBA color=WHITE, u32 flags = 0);
static void entities_clear(void);

static bool handle_global_events(Event event);
static bool handle_camera_events(Event event);
static bool handle_controller_events(Event event);
static bool handle_game_events(Event event);

static void generate_new_world(f32 width, f32 height);

static void draw_world_grid(void);
static void draw_world_terrain(void);
static void draw_entities(State* state);
static void debug_draw_render_batches();
static void debug_draw_mouse_cell_pos();
static void draw_level_editor();

static void serialize_world(String8 world);
static void deserialize_world(String8 world);
static void save_state();
static void load_state();

// how can command know about any variables in the game if its imported above the entire game? Like show all entities of type or something like that
#include "console.hpp"
#include "command.hpp"
#include "console.cpp"
#include "command.cpp"

#endif

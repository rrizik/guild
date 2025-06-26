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



//#define SCREEN_WIDTH 1280
//#define SCREEN_HEIGHT 720
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
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

#define BIN_SIZE 8
typedef struct BinNode{
    BinNode* next;
    Entity* entities[BIN_SIZE];
    u32 at;
    u32 cap;
} BinNode;

typedef struct Cell{
    BinNode* bin;
    u32 bin_count;
} Cell;

typedef enum SceneState{
    SceneState_None,
    //SceneState_Menu,
    SceneState_Game,
    SceneState_Editor,
} SceneState;

//#define ENTITIES_MAX 4096
#define ENTITIES_MAX 10001
#define ENTITIES_SELECTED_MAX ENTITIES_MAX
#define WORLD_WIDTH_IN_TILES_MAX 1000
#define WORLD_HEIGHT_IN_TILES_MAX 1000
f32 grid_size = 10;
typedef struct State{
    Arena arena;
    SceneState scene_state;
    //bool editor;

    Entity entities[ENTITIES_MAX];
    u32 entities_count;

    u32 generation[ENTITIES_MAX];
    u32 free_entities[ENTITIES_MAX];
    u32 free_entities_at;
    Font* font;

    Cell cells[WORLD_WIDTH_IN_TILES_MAX * WORLD_HEIGHT_IN_TILES_MAX];
    s32 world_grid[WORLD_WIDTH_IN_TILES_MAX * WORLD_HEIGHT_IN_TILES_MAX];
    // todo(rr): remove this. We need to have some cell info that we can gather
    // from having our mouse over cells, and we can do things on those cells based
    // on user input
    Entity* castle;
    v2 castle_cell;

    //bool is_entity_selected;
    Entity* entity_selected;
    Entity* entity_hovered;
    Entity* entities_selected[ENTITIES_SELECTED_MAX];
    s32 entities_selected_count;
    v2 entities_selected_center;

    v2 selection_mouse_record;
    bool selecting;
    Rect selection_rect;

    bool terrain_selected;
    s32 terrain_selected_id;

    bool show_world_cells;
    bool show_flocking_cells;
    bool show_pathing_cells;
    bool show_entity_info;
    f32 world_cell_size;
    f32 flocking_cell_size;
    f32 pathing_cell_size;
    f32 world_width_in_cells;
    f32 world_height_in_cells;
    union{
        struct{
            f32 world_pos_x;
            f32 world_pos_y;
        };
        v2 world_pos;
    };
    f32 world_height;
    f32 world_width;
    bool dragging_world;

    String8 current_world;
    bool draw_terrain;

} State, PermanentMemory;
global State* state;
static bool move_randomly = false;

typedef struct TransientMemory{
    Arena arena;
    Arena *frame_arena;
    Arena *asset_arena;
    Arena *ui_arena;
    Arena *ui_state_arena;
    Arena *batch_arena;
    Arena *data_arena;
    Arena *bin_arena;
} TransientMemory, TState;
global TState* ts;

typedef struct Cell_Info{
    u32 tex_bottom;
    u32 tex_top;
} Cell_Info;

static void show_cursor(bool show);
static void os_fullscreen_mode(Window* window);
static void os_windowed_mode(Window* window);
static void change_resolution(Window* window, f32 width, f32 height);

static void sim_game(void);

// todo(rr): get rid of this
global f32 text_padding = 20;

static    void remove_entity(Entity* e);
static Entity* add_entity(EntityType type);
static Entity* add_quad(v2 pos, v2 dim, RGBA color);
static Entity* add_texture(TextureAsset texture, v2 pos, v2 dim, RGBA color=WHITE, u32 flags = 0);
static Entity* add_castle(TextureAsset texture, v2 pos, v2 dim, RGBA color=WHITE, u32 flags = 0);
static Entity* add_skeleton(TextureAsset texture, v2 pos, v2 dim, v2 dir, RGBA color=WHITE, u32 flags = 0);
static void entities_clear(void);

static bool handle_global_events(Event event);
static bool handle_camera_events(Event event);
static bool handle_controller_events(Event event);
static bool handle_game_events(Event event);

static void generate_new_world(f32 width, f32 height);

static void draw_grid(f32 size);
static void draw_world_terrain(void);
static void draw_entities(State* state);
static void debug_draw_mouse_cell_pos(void);
static void ui_editor(void);
static void ui_structure_castle(void);

static bool mouse_in_cell(v2 cell);
static bool mouse_in_boundingbox(Entity* e);
static v2 grid_pos_from_cell(v2 cell);
static v2 grid_pos_from_cell(v2 cell, f32 size);
static v2 grid_cell_from_pos(v2 pos);
static v2 grid_cell_from_pos(v2 pos, f32 size);
static v2 grid_cell_center(v2 pos);
static bool grid_cell_coords_in_bounds(v2 coords);
static bool v2_close_enough(v2 p1, v2 p2, f32 epsilon);

static void serialize_world(String8 world);
static void deserialize_world(String8 world);
static void save_state(void);
static void load_state(void);

static void partition_entities_in_bins(void);

static void clear_entities_selected(void);

static m4 m4_screen_from_world();
static m4 m4_world_from_screen();
static v2 m4_translate_v2(m4 mat, v2 value);

#include "console.hpp"
#include "command.hpp"
#include "console.cpp"
#include "command.cpp"
static bool do_motion = true;
static v2 min;
static v2 max;


#endif


// rendering
// simulation



        // step 1, everything thats not on the edges (separate thread, produces deltas maybe)
        // step 2, WAIT for step 1, do the deltas (main thread applies deltas)
        // step 3 apply motion
        //for(s32 i = 0; i < array_count(state->entities); ++i){
        //    // FLOCKING
        //    e->velocity.x     += (dir.x * 50 * (f32)clock.dt) / distance;
        //    e->velocity.y     += (dir.y * 50 * (f32)clock.dt) / distance;
        //    other->velocity.x -= (dir.x * 50 * (f32)clock.dt) / distance;
        //    other->velocity.y -= (dir.y * 50 * (f32)clock.dt) / distance;
        //}

        //// wait for step 1 (step 2)
        //for(s32 i = 0; i < array_count(state->entities); ++i){
        //{
        //    // NORMAL VELOCITYM MOTION
        //    e->velocity.x += (move_dir.x * e->speed) * (f32)clock.dt;
        //    e->velocity.y += (move_dir.y * e->speed) * (f32)clock.dt;

        //    e->velocity.x *= 0.75f;
        //    e->velocity.y *= 0.75f;
        //    // Apply motion.
        //    e->pos.x += e->velocity.x * (f32)clock.dt;
        //    e->pos.y += e->velocity.y * (f32)clock.dt;
        //}

        //for(
        //    [
        //        [{1, -50},{4, 20},{20, 4}],
        //        [{1, -50},{4, 20},{20, 4}],
        //        [{1, -50},{4, 20},{20, 4}],
        //        [{1, -50},{4, 20},{20, 4}],
        //        [{1, -50},{4, 20},{20, 4}],
        //    ]
        //)

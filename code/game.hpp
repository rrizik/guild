#ifndef GAME_H
#define GAME_H

typedef enum SceneState{
    SceneState_None,
    SceneState_Menu,
    SceneState_Game,
} SceneState;

typedef enum GameState{
    GameState_None,
    GameState_Running,
    GameState_Paused,
    GameState_Finished,
} GameMode;

typedef enum GameResult{
    GameResult_None,
    GameResult_Won,
    GameResult_Lost,
} GameResult;

typedef enum LevelState{
    LevelState_None,
    LevelState_Starting,
    LevelState_Started,
    LevelState_Running,
    LevelState_Ending,
    LevelState_Ended,
} LevelState;

typedef struct Level{
    LevelState state;
    s32 asteroid_spawned;
    s32 asteroid_destroyed;
    s32 asteroid_count_max;
} Level;

v2s32 static mouse_cell();

static void do_one_frame(void);
static void draw_world_grid(void);
static v2 grid_pos_from_idx(f32 x, f32 y);

//static Entity* entity_from_handle(EntityHandle handle);
//static EntityHandle handle_from_entity(Entity *e);

static    void remove_entity(Entity* e);
static Entity* add_entity(EntityType type);
static Entity* add_quad(v2 pos, v2 dim, RGBA color);
static Entity* add_texture(u32 texture, v2 pos, v2 dim, RGBA color=WHITE, u32 flags = 0);
static Entity* add_castle(u32 texture, v2 pos, v2 dim, RGBA color=WHITE, u32 flags = 0);

static void entities_clear(void);
static void serialize_data(String8 filename);
static void deserialize_data(String8 filename);
static bool handle_global_events(Event event);
static bool handle_camera_events(Event event);
static bool handle_controller_events(Event event);
static bool handle_game_events(Event event);

v2 world_mouse_record;
Camera2D world_camera_record;

static v2 grid_pos_from_cell(f32 x, f32 y);
static v2 grid_cell_from_pos(f32 x, f32 y);
static v2 grid_cell_center(f32 x, f32 y);

static void update_game(void);

#endif


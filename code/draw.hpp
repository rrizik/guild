#ifndef DRAW_H
#define DRAW_H

static RGBA TEST_COLOR =   {34/255.0f, 177/255.0f, 76/255.0f,  1.0f};
static RGBA CLEAR =   {1.0f, 1.0f, 1.0f,  0.0f};
static RGBA RED =     {1.0f, 0.0f, 0.0f,  1.0f};
static RGBA GREEN =   {0.0f, 1.0f, 0.0f,  1.0f};
static RGBA BLUE =    {0.0f, 0.0f, 1.0f,  1.0f};
static RGBA MAGENTA = {1.0f, 0.0f, 1.0f,  1.0f};
static RGBA TEAL =    {0.0f, 1.0f, 1.0f,  1.0f};
static RGBA PINK =    {0.92f, 0.62f, 0.96f, 1.0f};
static RGBA YELLOW =  {0.9f, 0.9f, 0.0f,  1.0f};
static RGBA ORANGE =  {1.0f, 0.5f, 0.15f,  1.0f};
static RGBA ORANGE_HALF = {1.0f, 0.5f, 0.15f,  0.1f};
static RGBA LIGHT_GRAY =  {0.85f, 0.85f, 0.85f,  1.0f};
static RGBA DARK_GRAY =   {0.5f, 0.5f, 0.5f,  1.0f};
static RGBA DARK_GRAY_LIGHT =  {0.5f, 0.5f, 0.5f,  0.1f};
static RGBA BACKGROUND_COLOR = {1.0f/255.0f, 1.0f/255.0f, 1.0f/255.0f};
static RGBA ARMY_GREEN = {0.25f, 0.25f, 0.23f, 1.0f};
static RGBA DEFAULT    = {0.31f, 0.36f, 0.41f, 1.0f};
static RGBA WHITE =      {1.0f, 1.0f, 1.0f,  1.0f};
static RGBA BLACK =      {0.0f, 0.0f, 0.0f,  1.0f};

//------------------------------------------------------------
// Node Definitions

//typedef struct R_TextureNode         { R_TextureNode*         next; Texture* v; } R_TextureNode;
//typedef struct R_PosXNode            { R_PosXNode*            next; f32 v;     } R_PosXNode;
//typedef struct R_PosYNode            { R_PosYNode*            next; f32 v;     } R_PosYNode;
//typedef struct R_SizeWNode           { R_SizeWNode*           next; R_Size v; } R_SizeWNode;
//typedef struct R_SizeHNode           { R_SizeHNode*           next; R_Size v; } R_SizeHNode;
//typedef struct R_LayoutAxisNode      { R_LayoutAxisNode*      next; Axis v;    } R_LayoutAxisNode;
//typedef struct R_TextPaddingNode     { R_TextPaddingNode*     next; f32 v;     } R_TextPaddingNode;
//typedef struct R_TextColorNode       { R_TextColorNode*       next; RGBA v;    } R_TextColorNode;
//typedef struct R_BackgroundColorNode { R_BackgroundColorNode* next; RGBA v;    } R_BackgroundColorNode;
//typedef struct R_BorderThicknessNode { R_BorderThicknessNode* next; f32 v;     } R_BorderThicknessNode;
//typedef struct R_FontNode            { R_FontNode*            next; s32 v;   } R_FontNode;
//
////------------------------------------------------------------
//// Stack Definitions
//
//typedef struct R_ParentStack          { R_ParentNode*          top; bool auto_pop; } R_ParentStack;
//typedef struct R_PosXStack            { R_PosXNode*            top; bool auto_pop; } R_PosXStack;
//typedef struct R_PosYStack            { R_PosYNode*            top; bool auto_pop; } R_PosYStack;
//typedef struct R_SizeWStack           { R_SizeWNode*           top; bool auto_pop; } R_SizeWStack;
//typedef struct R_SizeHStack           { R_SizeHNode*           top; bool auto_pop; } R_SizeHStack;
//typedef struct R_LayoutAxisStack      { R_LayoutAxisNode*      top; bool auto_pop; } R_LayoutAxisStack;
//typedef struct R_TextPaddingStack     { R_TextPaddingNode*     top; bool auto_pop; } R_TextPaddingStack;
//typedef struct R_TextColorStack       { R_TextColorNode*       top; bool auto_pop; } R_TextColorStack;
//typedef struct R_BackgroundColorStack { R_BackgroundColorNode* top; bool auto_pop; } R_BackgroundColorStack;
//typedef struct R_BorderThicknessStack { R_BorderThicknessNode* top; bool auto_pop; } R_BorderThicknessStack;
//typedef struct R_FontStack            { R_FontNode*            top; bool auto_pop; } R_FontStack;
typedef enum Draw_Command_Kind{
    Draw_Command_Quad,
    Draw_Command_Bounding_Box,
    Draw_Command_Line,

    Draw_Command_Texture,
    Draw_Command_Sprite,

    Draw_Command_Text,

    Draw_Command_Count,
} Draw_Command_Kind;

typedef struct Draw_Command{
    Draw_Command_Kind kind;

    Quad quad;
    Spritesheet sprite;

    String8 text;
    v2 pos;

    v2 p0;
    v2 p1;

    f32 width; 

    RGBA color;

    s32 texture_id;
    s32 font_id;
    m4 transform;

    s32 layer;
    f32 z;
} Draw_Command;

#define DRAW_COMMANDS_COUNT 409600
global Draw_Command  draw_screen_commands[DRAW_COMMANDS_COUNT];
global Draw_Command  draw_world_commands[DRAW_COMMANDS_COUNT];
global Draw_Command  draw_world_sorted_commands[DRAW_COMMANDS_COUNT];

global s32 draw_screen_commands_at = 0;
global s32 draw_world_commands_at = 0;
global s32 draw_world_sorted_commands_at = 0;

global Draw_Command* draw_commands;
global s32* draw_commands_at;

typedef enum Render_Space{
    Render_Space_Screen,
    Render_Space_World,
    Render_Space_World_Sorted,

    Render_Space_Count,
} Render_Space;

typedef struct Render_State{
    Arena*       arena;
    Assets*      assets;
    Texture*     texture;
    s32          texture_id;
    Font*        font;
    s32          font_id;
    m4           transform;
    u32          transform_gen;
    s32          layer;
    f32          z;
    Render_Space space;
} Render_State;
Render_State* render_state;

//#define DEFAULT_BATCH_SIZE MB(8)
//#define DEFAULT_BATCH_SIZE KB(200)
#define DEFAULT_BATCH_SIZE KB(100)
typedef struct RenderBatch{
    RenderBatch* next;
    Vertex2* buffer;
    s32 id;
    s32 idx_in_vertex_buffer;

    s32 vertex_count;
    s32 vertex_cap;
    Texture* texture;

    m4 transform;
    u32 transform_gen;
} RenderBatch;

typedef struct RenderBatchNode{
    RenderBatch* first;
    RenderBatch* last;
    s32 count;
} RenderBatchNode;
global RenderBatchNode render_batches = {0};

static Spritesheet*
push_spritesheet(s32 texture_id, f32 col, f32 row, f32 anim_speed);

static void r_set_texture(s32 texture_id);
static void r_set_font(s32 font_id);
static void r_set_transform(m4 transform);
static void r_set_generation(m4 transform);
static void r_set_layer(s32 layer);
static void r_set_z(f32 z);
static void r_set_render_space(Render_Space space);

static RGBA brighten_color(RGBA color, float factor);
static RGBA darken_color(RGBA color, float factor);

static v2 v2_world_from_screen(v2 screen_pos, Camera2D* camera);
static v2 v2_world_from_screen(v2 screen_pos);
//static v2 v2_screen_from_world(v2 world_pos);
//static Rect rect_screen_from_world(Rect rect);
//static Quad quad_screen_from_world(Quad rect);
static m4 m4_screen_from_world(void);
static m4 m4_world_from_screen(void);
static v2 m4_translate_v2(m4 mat, v2 value);

static RGBA srgb_to_linear_approx(RGBA value);
static RGBA linear_to_srgb_approx(RGBA value);
static RGBA linear_from_srgb(RGBA value);
static RGBA srgb_from_linear(RGBA value);

static void draw_init(Arena* arena, Arena* batch_arena, Arena* sprite_arena, Assets* assets);

static void push_texture_quad();

static void imm_draw_quad(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color=WHITE);
static void imm_draw_quad(v2 pos, v2 dim, RGBA color=WHITE);
static void imm_draw_quad(Rect rect, RGBA color=WHITE);
static void imm_draw_quad(Quad quad, RGBA color=WHITE);

static void imm_draw_texture(v2 p0, v2 p1, v2 p2, v2 p3, v2 u0, v2 u1, v2 u2, v2 u3, RGBA color=WHITE);
static void imm_draw_texture(v2 pos, v2 dim, v2 u0, v2 u1, v2 u2, v2 u3, RGBA color=WHITE);
static void imm_draw_texture(Rect rect, v2 u0, v2 u1, v2 u2, v2 u3, RGBA color=WHITE);
static void imm_draw_texture(Quad quad, v2 u0, v2 u1, v2 u2, v2 u3, RGBA color=WHITE);

static void imm_draw_texture(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color=WHITE);
static void imm_draw_texture(v2 pos, v2 dim, RGBA color=WHITE);
static void imm_draw_texture(Rect rect, RGBA color=WHITE);
static void imm_draw_texture(Quad quad, RGBA color=WHITE);

static void imm_draw_sprite(Spritesheet sprite, Quad quad, RGBA color=WHITE);

static void imm_draw_bounding_box(v2 p0, v2 p1, v2 p2, v2 p3, f32 width, RGBA color=WHITE);
static void imm_draw_bounding_box(v2 pos, v2 dim, f32 width, RGBA color=WHITE);
static void imm_draw_bounding_box(Rect rect, f32 width, RGBA color=WHITE);
static void imm_draw_bounding_box(Quad quad, f32 width, RGBA color=WHITE);

static void imm_draw_line(v2 p0, v2 p1, f32 width, RGBA color=WHITE);

static void imm_draw_text(String8 text, v2 pos, RGBA color=WHITE);

static void draw_quad(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color=WHITE);
static void draw_quad(v2 pos, v2 dim, RGBA color=WHITE);
static void draw_quad(Rect rect, RGBA color=WHITE);
static void draw_quad(Quad quad, RGBA color=WHITE);

static void draw_texture(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color=WHITE);
static void draw_texture(v2 pos, v2 dim, RGBA color=WHITE);
static void draw_texture(Rect rect, RGBA color=WHITE);
static void draw_texture(Quad quad, RGBA color=WHITE);

static void draw_sprite(Spritesheet sprite, Quad quad, RGBA color=WHITE);

static void draw_bounding_box(v2 p0, v2 p1, v2 p2, v2 p3, f32 width, RGBA color=WHITE);
static void draw_bounding_box(v2 pos, v2 dim, f32 width, RGBA color=WHITE);
static void draw_bounding_box(Rect rect, f32 width, RGBA color=WHITE);
static void draw_bounding_box(Quad quad, f32 width, RGBA color=WHITE);
static void draw_line(v2 p0, v2 p1, f32 width, RGBA color=WHITE);

static void draw_text(String8 text, v2 pos, RGBA color=WHITE);

static void draw_render_commands(void);
static void draw_commands_clear(void);

static RenderBatch* get_render_batch(u64 vertex_count);
static void draw_render_batches(void);
static void render_batches_reset(void);

#endif


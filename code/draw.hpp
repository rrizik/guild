#ifndef DRAW_H
#define DRAW_H

static RGBA TEST_COLOR  = {34/255.0f, 177/255.0f, 76/255.0f,  1.0f};
static RGBA CLEAR       = {1.0f, 1.0f, 1.0f,  0.0f};
static RGBA RED         = {1.0f, 0.0f, 0.0f,  1.0f};
static RGBA GREEN       = {0.0f, 1.0f, 0.0f,  1.0f};
static RGBA BLUE        = {0.0f, 0.0f, 1.0f,  1.0f};
static RGBA MAGENTA     = {1.0f, 0.0f, 1.0f,  1.0f};
static RGBA TEAL        = {0.0f, 1.0f, 1.0f,  1.0f};
static RGBA PINK        = {0.92f, 0.62f, 0.96f, 1.0f};
static RGBA YELLOW      = {0.9f, 0.9f, 0.0f,  1.0f};
static RGBA ORANGE      = {1.0f, 0.5f, 0.15f,  1.0f};
static RGBA ORANGE_HALF = {1.0f, 0.5f, 0.15f,  0.1f};
static RGBA ARMY_GREEN  = {0.25f, 0.25f, 0.23f, 1.0f};
static RGBA DEFAULT     = {0.31f, 0.36f, 0.41f, 1.0f};
static RGBA WHITE       = {1.0f, 1.0f, 1.0f,  1.0f};
static RGBA BLACK       = {0.0f, 0.0f, 0.0f,  1.0f};
static RGBA LIGHT_GRAY  = {0.85f, 0.85f, 0.85f,  1.0f};
static RGBA DARK_GRAY   = {0.5f, 0.5f, 0.5f,  1.0f};
static RGBA DARK_GRAY_LIGHT = {0.5f, 0.5f, 0.5f,  0.1f};
static RGBA BACKGROUND_COLOR = {1.0f/255.0f, 1.0f/255.0f, 1.0f/255.0f};

typedef enum Render_Space{
    Render_Space_Screen,
    Render_Space_World,
    Render_Space_World_Sorted,

    Render_Space_Count,
} Render_Space;

//------------------------------------------------------------
// Node Definitions

typedef struct R_RenderSpaceNode { R_RenderSpaceNode* next; Render_Space v; } R_RenderSpaceNode;
typedef struct R_TextureNode     { R_TextureNode*     next; s32 v;          } R_TextureNode;
typedef struct R_FontNode        { R_FontNode*        next; s32 v;          } R_FontNode;
typedef struct R_TransformNode   { R_TransformNode*   next;  m4 v;          } R_TransformNode;
typedef struct R_LayerNode       { R_LayerNode*       next; s32 v;          } R_LayerNode;
typedef struct R_ZNode           { R_ZNode*           next; f32 v;          } R_ZNode;

////------------------------------------------------------------
//// Stack Definitions

typedef struct R_RenderSpaceStack { R_RenderSpaceNode* top; bool auto_pop; } R_RenderSpaceStack;
typedef struct R_TextureStack     { R_TextureNode*     top; bool auto_pop; } R_TextureStack;
typedef struct R_FontStack        { R_FontNode*        top; bool auto_pop; } R_FontStack;
typedef struct R_TransformStack   { R_TransformNode*   top; bool auto_pop; } R_TransformStack;
typedef struct R_LayerStack       { R_LayerNode*       top; bool auto_pop; } R_LayerStack;
typedef struct R_ZStack           { R_ZNode*           top; bool auto_pop; } R_ZStack;

R_RenderSpaceNode r_render_space_null = {0};
R_TextureNode     r_texture_null = {0};
R_FontNode        r_font_null = {0};
R_TransformNode   r_transform_null = {0};
R_LayerNode       r_layer_null = {0};
R_ZNode           r_z_null = {0};

typedef struct Render_State{
    Arena*       batch_arena;
    Arena*       stack_arena;
    Assets*      assets;

    Render_Space space;
    Texture*     texture;
    //s32          texture_id;
    Font*        font;
    //s32          font_id;
    m4           transform;
    u32          transform_gen;
    //s32          layer;
    //f32          z;

    R_RenderSpaceStack render_space_stack;
    R_TextureStack     texture_stack;
    R_FontStack        font_stack;
    R_TransformStack   transform_stack;
    R_LayerStack       layer_stack;
    R_ZStack           z_stack;
} Render_State;
Render_State* render_state;

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


static RGBA brighten_color(RGBA color, float factor);
static RGBA darken_color(RGBA color, float factor);

static v2 v2_world_from_screen(v2 screen_pos, Camera2D* camera);
static v2 v2_world_from_screen(v2 screen_pos);
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

static void draw_stack_clear(void);
static void draw_end(void);

//------------------------------------------------------------
// Stack Macros

#define r_stack_push_impl(arena, type, name, value) \
    R_##type##Node* node = push_struct(arena, R_##type##Node); \
    node->v = value; \
    node->next = render_state->##name##_stack.top; \
    R_##type##Node* old_node = render_state->##name##_stack.top; \
    render_state->##name##_stack.top = node; \
    render_state->##name##_stack.auto_pop = false; \

#define r_stack_pop_impl(type, name) \
    R_##type##Node* node = render_state->##name##_stack.top; \
    render_state->##name##_stack.top = render_state->##name##_stack.top->next; \

#define r_stack_top_impl(name) \
    return(render_state->##name##_stack.top->v); \

//------------------------------------------------------------
// Push/Pop/Top/Set

static Render_Space r_top_render_space(void) { r_stack_top_impl(render_space) }
static s32 r_top_texture(void)        { r_stack_top_impl(texture) }
static s32 r_top_font(void)           { r_stack_top_impl(font) }
static m4  r_top_transform(void)      { r_stack_top_impl(transform) }
static s32 r_top_layer(void)          { r_stack_top_impl(layer) }
static f32 r_top_z(void)              { r_stack_top_impl(z) }

static void r_push_render_space(Render_Space v) { 
    r_stack_push_impl(render_state->stack_arena, RenderSpace, render_space, v) 
    switch(v){
        case Render_Space_Screen:{
            r_stack_push_impl(render_state->stack_arena, Transform, transform, m4_make_ident())
            draw_commands = draw_screen_commands;
            draw_commands_at = &draw_screen_commands_at;
        } break;
        case Render_Space_World:{
            r_stack_push_impl(render_state->stack_arena, Transform, transform, m4_screen_from_world())
            draw_commands = draw_world_commands;
            draw_commands_at = &draw_world_commands_at;
        } break;
        case Render_Space_World_Sorted:{
            r_stack_push_impl(render_state->stack_arena, Transform, transform, m4_screen_from_world())
            draw_commands = draw_world_sorted_commands;
            draw_commands_at = &draw_world_sorted_commands_at;
        } break;
    }
}
static void r_push_texture(s32 v)  { r_stack_push_impl(render_state->stack_arena, Texture, texture, v) }
static void r_push_font(s32 v)     { r_stack_push_impl(render_state->stack_arena, Font, font, v) }
static void r_push_transform(m4 v) { r_stack_push_impl(render_state->stack_arena, Transform, transform, v) }
static void r_push_layer(s32 v)    { r_stack_push_impl(render_state->stack_arena, Layer, layer, v) }
static void r_push_z(f32 v)        { r_stack_push_impl(render_state->stack_arena, Z, z, v) }

static void r_pop_render_space(void) { 
    { r_stack_pop_impl(RenderSpace, render_space) }
    { r_stack_pop_impl(Transform, transform) }

    switch(r_top_render_space()){
        case Render_Space_Screen:{
            draw_commands = draw_screen_commands;
            draw_commands_at = &draw_screen_commands_at;
        } break;
        case Render_Space_World:{
            draw_commands = draw_world_commands;
            draw_commands_at = &draw_world_commands_at;
        } break;
        case Render_Space_World_Sorted:{
            draw_commands = draw_world_sorted_commands;
            draw_commands_at = &draw_world_sorted_commands_at;
        } break;
    }
}
static void r_pop_texture(void)       { r_stack_pop_impl(Texture, texture) }
static void r_pop_font(void)          { r_stack_pop_impl(Font, font) }
static void r_pop_transform(void)     { r_stack_pop_impl(Transform, transform) }
static void r_pop_layer(void)         { r_stack_pop_impl(Layer, layer) }
static void r_pop_z(void)             { r_stack_pop_impl(Z, z) }

////------------------------------------------------------------
//// DeferLoops

#define r_render_space(v) defer_loop(r_push_render_space(v), r_pop_render_space())
#define r_texture(v)      defer_loop(r_push_texture(v), r_pop_texture())
#define r_font(v) 		  defer_loop(r_push_font(v), r_pop_font())
#define r_transform(v)    defer_loop(r_push_transform(v), r_pop_transform())
#define r_layer(v)        defer_loop(r_push_layer(v), r_pop_layer())
#define r_z(v)            defer_loop(r_push_z(v), r_pop_z())

#endif

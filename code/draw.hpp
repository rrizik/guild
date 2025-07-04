#ifndef DRAW_H
#define DRAW_H

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

global Arena*   r_arena = 0;
global Assets*  r_assets = 0;
global Texture* r_texture;
global Font*    r_font;
global m4       r_transform;
global u32      r_transform_gen = 0;

//#define DEFAULT_BATCH_SIZE MB(8)
//#define DEFAULT_BATCH_SIZE KB(200)
#define DEFAULT_BATCH_SIZE KB(100)
typedef struct RenderBatch{
    RenderBatch* next;
    Vertex3* buffer;
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

static void set_texture(Texture* texture);
static Texture* get_texture(void);
static void set_font(Font* font);
static Font* get_font(void);
static void set_transform(m4 transform);
static m4 get_transform(void);
static RenderBatch* get_render_batch(u64 vertex_count);

typedef enum RenderCommandType{
    RenderCommandType_ClearColor,
    RenderCommandType_Quad,
    RenderCommandType_Line,
    RenderCommandType_Texture,
    RenderCommandType_Text,
} RenderCommandType;

typedef struct RenderCommand{
    RenderCommandType type;

    union {
        v2 pos;
        v2 p0;
    };
    v2 p1;
    v2 p2;
    v2 p3;

    RGBA color;
    Font* font;
	u32 texture_id;

    String8 text;
} RenderCommand;

static RGBA brighten_color(RGBA color);
static RGBA darken_color(RGBA color);
static v2 v2_world_from_screen(v2 screen_pos, Camera2D* camera);
static v2 v2_world_from_screen(v2 screen_pos);

static v2 v2_screen_from_world(v2 world_pos);
static Rect rect_screen_from_world(Rect rect);
static Quad quad_screen_from_world(Quad rect);
static m4 m4_screen_from_world(void);

static RGBA srgb_to_linear_approx(RGBA value);
static RGBA linear_to_srgb_approx(RGBA value);
static RGBA linear_from_srgb(RGBA value);
static RGBA srgb_from_linear(RGBA value);

static void init_draw(Arena* arena);

static void draw_quad(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color);
static void draw_quad(v2 pos, v2 dim, RGBA color);
static void draw_quad(Rect rect, RGBA color);
static void draw_quad(Quad quad, RGBA color);

// todo: not implemented yet
static void draw_quad(v2 p0, v2 p1, v2 p2, v2 p3, v2 uv0, v2 uv1, v2 uv2, v2 uv3, RGBA color);
static void draw_quad(v2 pos, v2 dim, v2 uv0, v2 uv1, v2 uv2, v2 uv3, RGBA color);
static void draw_quad(Rect rect, v2 uv0, v2 uv1, v2 uv2, v2 uv3, RGBA color);
static void draw_quad(Quad quad, v2 uv0, v2 uv1, v2 uv2, v2 uv3, RGBA color);

static void draw_texture(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color=WHITE);
static void draw_texture(v2 pos, v2 dim, RGBA color=WHITE);
static void draw_texture(Rect rect, RGBA color=WHITE);
static void draw_texture(Quad quad, RGBA color=WHITE);

static void draw_bounding_box(v2 p0, v2 p1, v2 p2, v2 p3, f32 width, RGBA color);
static void draw_bounding_box(Quad quad, f32 width, RGBA color);
static void draw_bounding_box(Rect rect, f32 width, RGBA color);
static void draw_line(v2 p0, v2 p1, f32 width, RGBA color);
static void draw_text(String8 text, v2 pos, RGBA color);

static void draw_render_batches(void);
static void render_batches_reset(void);


#endif


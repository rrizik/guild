#ifndef UI_H
#define UI_H

/*
INFLUENCE & INSPIRATION:
    A large part of this UI library is heavily influenced and inspired by Ryan Fleury's work, UI Series, code base and mentorship.
    If you are interested in learning more about Ryan's work, you can view his website and UI series here:
    - Website: https://www.rfleury.com
    - UI Series: https://www.rfleury.com/p/ui-part-1-the-interaction-medium
    - Code Base (subscription required): https://git.rfleury.com/
*/

typedef struct UI_Signal{
    bool pressed_left;
    bool pressed_middle;
    bool pressed_right;
    bool mouse_hot;
    bool mouse_over;
} UI_Signal;

typedef u32 Axis;
enum{
    Axis_X,
    Axis_Y,
    Axis_Count,
};

typedef u32 UI_SizeType;
enum {
    UI_SizeType_None,
    UI_SizeType_Pixel,
    UI_SizeType_TextContent,
    UI_SizeType_PercentOfParent,
    UI_SizeType_ChildrenSum,
    UI_SizeType_Count,
};

typedef struct UI_Size{
  UI_SizeType type;
  f32 value;
  f32 strictness;
} UI_Size;

static UI_Size ui_make_size(UI_SizeType type, f32 value, f32 strictness);
#define ui_size_pixel(v, s)   ui_make_size(UI_SizeType_Pixel, (v), (s))
#define ui_size_text(s)       ui_make_size(UI_SizeType_TextContent, (0), (s))
#define ui_size_percent(v, s) ui_make_size(UI_SizeType_PercentOfParent, (v), (s))
#define ui_size_children(s)   ui_make_size(UI_SizeType_ChildrenSum, (0), (s))

// note consider(rr): Clickable/Draggable are pretty much the same thing. Its about whether you want to use the dragging information or not.
typedef u32 UI_BoxFlags;
enum {
  UI_BoxFlag_None            = (1<<0),
  UI_BoxFlag_Clickable       = (1<<1),
  UI_BoxFlag_ViewScroll      = (1<<2),
  UI_BoxFlag_DrawText        = (1<<3),
  UI_BoxFlag_DrawBorder      = (1<<4),
  UI_BoxFlag_DrawBackground  = (1<<5),
  UI_BoxFlag_DrawDropShadow  = (1<<6),
  UI_BoxFlag_Clip            = (1<<7),
  UI_BoxFlag_HotAnimation    = (1<<8),
  UI_BoxFlag_ActiveAnimation = (1<<9),
  UI_BoxFlag_Draggable       = (1<<10),
  UI_BoxFlag_NoSiblings      = (1<<11),
};
UI_BoxFlags ui_floating_panel = UI_BoxFlag_DrawBackground |
                                UI_BoxFlag_Clickable |
                                UI_BoxFlag_Draggable |
                                UI_BoxFlag_NoSiblings;
UI_BoxFlags ui_fixed_panel    = UI_BoxFlag_DrawBackground |
                                UI_BoxFlag_Clickable |
                                UI_BoxFlag_NoSiblings;

// todo: not even using this
typedef struct TextInfo{
    s32 text_width;
    s32 vertical_offset;
} TextInfo;

typedef struct UI_Box{
    UI_Box *first;
    UI_Box *last;
    UI_Box *next;
    UI_Box *prev;
    UI_Box *parent;

    // hash links
    //UI_Widget *hash_next;
    //UI_Widget *hash_prev;

    //u64 generation_touched;
    u64 key;

    UI_BoxFlags flags;
    String8 string;

    // cached data
    f32 size[Axis_Count];
    f32 pos[Axis_Count];
    Rect rect;
    //bool hot_dt;
    //bool active_dt;

    // stack parameters
    f32 rel_pos[Axis_Count];
    UI_Size semantic_size[Axis_Count];
    Axis layout_axis;
    f32 text_padding;
    RGBA text_color;
    RGBA background_color;
    f32 border_thickness;

    Font* font;

} UI_Box;

typedef struct BoxCache{
    Rect rect;
    f32 size[Axis_Count];
    f32 pos[Axis_Count];
    f32 rel_pos[Axis_Count];
} BoxCache;

typedef struct UI_State{
    Arena* arena;
    Window* window;
    Controller* controller;
    Font* default_font;

    UI_Box* root;

    HashTable table;

    u64 hot;
    u64 active;

    v2 mouse_pos_record;

    bool closed;

    //u64 generation;

} UI_State;
global UI_State* ui_state;

static void ui_init(Arena* arena, Window* window, Controller* controller, Assets* assets);
static void ui_begin(void);
static void ui_end(void);
static void ui_layout(void);
static void ui_draw(UI_Box* box);

static Arena*    ui_arena(void);
static Window*   ui_window(void);
static UI_Box*   ui_root(void);
static HashTable ui_table(void);
static v2        ui_mouse_pos(void);
static Mouse     ui_mouse(void);
static bool      ui_closed(void);
static void      ui_close(void);
static String8   ui_text_part_from_key(String8 string);

static UI_Box*   ui_make_box(String8 str, UI_BoxFlags flags);
static UI_Box*   ui_box(String8 str, UI_BoxFlags flags=0);
static UI_Signal ui_button(String8 string, UI_BoxFlags flags_in=0);
static void      ui_label(String8 string);
static void      ui_spacer(f32 size);
static void      ui_begin_panel(String8 string, UI_BoxFlags flags=0);
static void      ui_end_panel(void);

static BoxCache cache_from_box(UI_Box* box);
static UI_Signal ui_signal_from_box(UI_Box* box);

static void ui_traverse_independent(UI_Box* box, Axis axis);
static void ui_traverse_children(UI_Box* box, Axis axis);
static void ui_traverse_positions(UI_Box* box, Axis axis);
static void ui_traverse_rects(UI_Box* box);

//------------------------------------------------------------

// Stack Macros
#define ui_stack_push_impl(arena, type, name, v) \
    UI_##type##Node* node = push_struct(arena, UI_##type##Node); \
    node->v = v; \
    node->next = ui_##name##_top; \
    ui_##name##_top = node; \
    return(node->v); \

#define ui_stack_pop_impl(type, name) \
    UI_##type##Node* node = ui_##name##_top; \
    ui_##name##_top = ui_##name##_top->next; \
    return(node->v); \

#define ui_stack_top_impl(name) return(ui_##name##_top->v);

//#define UI_StackSetNextImpl(name_upper, name, type, v) \
//	UI_##name_upper##Node *node = ui_##name##_top.free;\
//	if(node != 0) {
//		StackPop(ui_##name##_top.free);
//    }\
//	else {
//		node = PushArray(UI_BuildArena(), UI_##name_upper##Node, 1);
//	}\
//	type old_value = ui_##name##_top.top->v;\
//	node->v = v;\
//	StackPush(ui_##name##_top.top, node);\
//	ui_##name##_top.auto_pop = 1;\
//	return old_value;

//------------------------------------------------------------

// Stack Definitions
typedef struct UI_ParentNode          { UI_ParentNode*          next; UI_Box* v; } UI_ParentNode;
typedef struct UI_PosXNode            { UI_PosXNode*            next; f32 v;     } UI_PosXNode;
typedef struct UI_PosYNode            { UI_PosYNode*            next; f32 v;     } UI_PosYNode;
typedef struct UI_SizeWNode           { UI_SizeWNode*           next; UI_Size v; } UI_SizeWNode;
typedef struct UI_SizeHNode           { UI_SizeHNode*           next; UI_Size v; } UI_SizeHNode;
typedef struct UI_LayoutAxisNode      { UI_LayoutAxisNode*      next; Axis v;    } UI_LayoutAxisNode;
typedef struct UI_TextPaddingNode     { UI_TextPaddingNode*     next; f32 v;     } UI_TextPaddingNode;
typedef struct UI_TextColorNode       { UI_TextColorNode*       next; RGBA v;    } UI_TextColorNode;
typedef struct UI_BackgroundColorNode { UI_BackgroundColorNode* next; RGBA v;    } UI_BackgroundColorNode;
typedef struct UI_BorderThicknessNode { UI_BorderThicknessNode* next; f32 v;     } UI_BorderThicknessNode;
typedef struct UI_FontNode            { UI_FontNode*            next; Font* v;   } UI_FontNode;

// Stacks
UI_ParentNode*          ui_parent_top = 0;
UI_PosXNode*            ui_pos_x_top = 0;
UI_PosYNode*            ui_pos_y_top = 0;
UI_SizeWNode*           ui_size_w_top = 0;
UI_SizeHNode*           ui_size_h_top = 0;
UI_LayoutAxisNode*      ui_layout_axis_top = 0;
UI_TextPaddingNode*     ui_text_padding_top = 0;
UI_TextColorNode*       ui_text_color_top = 0;
UI_BackgroundColorNode* ui_background_color_top = 0;
UI_BorderThicknessNode* ui_border_thickness_top = 0;
UI_FontNode*            ui_font_top = 0;

// Push/Pop/Top/Set
static UI_Box* ui_push_parent(UI_Box* v)        { ui_stack_push_impl(ui_arena(), Parent, parent, v) }
static f32     ui_push_pos_x(f32 v)             { ui_stack_push_impl(ui_arena(), PosX, pos_x, v) }
static f32     ui_push_pos_y(f32 v)             { ui_stack_push_impl(ui_arena(), PosY, pos_y, v) }
static UI_Size ui_push_size_w(UI_Size v)        { ui_stack_push_impl(ui_arena(), SizeW, size_w, v) }
static UI_Size ui_push_size_h(UI_Size v)        { ui_stack_push_impl(ui_arena(), SizeH, size_h, v) }
static Axis    ui_push_layout_axis(Axis v)      { ui_stack_push_impl(ui_arena(), LayoutAxis, layout_axis, v) }
static f32     ui_push_text_padding(f32 v)      { ui_stack_push_impl(ui_arena(), TextPadding, text_padding, v) }
static RGBA    ui_push_text_color(RGBA v)       { ui_stack_push_impl(ui_arena(), TextColor, text_color, v) }
static RGBA    ui_push_background_color(RGBA v) { ui_stack_push_impl(ui_arena(), BackgroundColor, background_color, v) }
static f32     ui_push_border_thickness(f32 v)  { ui_stack_push_impl(ui_arena(), BorderThickness, border_thickness, v) }
static Font*   ui_push_font(Font* v)            { ui_stack_push_impl(ui_arena(), Font, font, v) }

static UI_Box* ui_pop_parent(void)              { ui_stack_pop_impl(Parent, parent) }
static f32     ui_pop_pos_x(void)               { ui_stack_pop_impl(PosX, pos_x) }
static f32     ui_pop_pos_y(void)               { ui_stack_pop_impl(PosY, pos_y) }
static UI_Size ui_pop_size_w(void)              { ui_stack_pop_impl(SizeW, size_w) }
static UI_Size ui_pop_size_h(void)              { ui_stack_pop_impl(SizeH, size_h) }
static Axis    ui_pop_layout_axis(void)         { ui_stack_pop_impl(LayoutAxis, layout_axis) }
static f32     ui_pop_text_padding(void)        { ui_stack_pop_impl(TextPadding, text_padding) }
static RGBA    ui_pop_text_color(void)          { ui_stack_pop_impl(TextColor, text_color) }
static RGBA    ui_pop_background_color(void)    { ui_stack_pop_impl(BackgroundColor, background_color) }
static f32     ui_pop_border_thickness(void)    { ui_stack_pop_impl(BorderThickness, border_thickness) }
static Font*   ui_pop_font(void)                { ui_stack_pop_impl(Font, font) }

static UI_Box* ui_top_parent(void)              { ui_stack_top_impl(parent) }
static f32     ui_top_pos_x(void)               { ui_stack_top_impl(pos_x) }
static f32     ui_top_pos_y(void)               { ui_stack_top_impl(pos_y) }
static UI_Size ui_top_size_w(void)              { ui_stack_top_impl(size_w) }
static UI_Size ui_top_size_h(void)              { ui_stack_top_impl(size_h) }
static Axis    ui_top_layout_axis(void)         { ui_stack_top_impl(layout_axis) }
static f32     ui_top_text_padding(void)        { ui_stack_top_impl(text_padding) }
static RGBA    ui_top_text_color(void)          { ui_stack_top_impl(text_color) }
static RGBA    ui_top_background_color(void)    { ui_stack_top_impl(background_color) }
static f32     ui_top_border_thickness(void)    { ui_stack_top_impl(border_thickness) }
static Font*   ui_top_font(void)                { ui_stack_top_impl(font) }
// set sets only for the next item and none after

// Push/Pop/Top/Set Compositions
static void ui_push_pos(v2 pos)                 { ui_push_pos_x(pos.x); ui_push_pos_y(pos.y); }
static void ui_pop_pos()                        { ui_pop_pos_x(); ui_pop_pos_y(); }
static void ui_push_size(v2 pos);
static void ui_pop_size();

//------------------------------------------------------------

// DeferLoops
#define ui_parent(v)           defer_loop(ui_push_parent(v), ui_pop_parent())
#define ui_pos_x(v) 		   defer_loop(ui_push_pos_x(v), ui_pop_pos_x())
#define ui_pos_y(v) 		   defer_loop(ui_push_pos_y(v), ui_pop_pos_y())
#define ui_size_w(v)           defer_loop(ui_push_size_w(v), ui_pop_size_w())
#define ui_size_h(v)           defer_loop(ui_push_size_h(v), ui_pop_size_h())
#define ui_layout_axis(v)      defer_loop(ui_push_layout_axis(v), ui_pop_layout_axis())
#define ui_text_padding(v)     defer_loop(ui_push_text_padding(v), ui_pop_text_padding())
#define ui_text_color(v)       defer_loop(ui_push_text_color(v), ui_pop_text_color())
#define ui_background_color(v) defer_loop(ui_push_background_color(v), ui_pop_background_color())
#define ui_border_thickness(v) defer_loop(ui_push_border_thickness(v), ui_pop_border_thickness())
#define ui_font(v)             defer_loop(ui_push_font(v), ui_pop_font())

// DeferLoops Compositions
#define ui_pos(v) defer_loop(ui_push_pos(v), ui_pop_pos())

#endif

/*
root_function UI_BoxFlags           UI_TopFlags(void);
root_function F32                   UI_TopOpacity(void);
root_function Vec4F32               UI_TopBorderColor(void);
root_function Vec4F32               UI_TopOverlayColor(void);
root_function Vec4F32               UI_TopFillColor(void);
root_function Vec4F32               UI_TopCursorColor(void);
root_function F32                   UI_TopCornerRadius00(void);
root_function F32                   UI_TopCornerRadius01(void);
root_function F32                   UI_TopCornerRadius10(void);
root_function F32                   UI_TopCornerRadius11(void);
root_function F32                   UI_TopFontSize(void);
root_function OS_CursorKind         UI_TopHoverCursor(void);
root_function UI_TextAlignment      UI_TopTextAlign(void);
root_function UI_Key                UI_TopSeedKey(void);
root_function UI_FocusKind          UI_TopFocusHot(void);
root_function UI_FocusKind          UI_TopFocusActive(void);
*/

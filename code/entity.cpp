#ifndef ENTITY_C
#define ENTITY_C

static bool
has_flag(u32 lflags, u32 rflags){
    bool result = (lflags & rflags) == rflags;
    return(result);
}

static void
set_flag(u32* lflags, u32 rflags){
    *lflags |= rflags;
}

static void
clear_flag(u32* lflags, u32 rflags){
    *lflags &= ~rflags;
}

static EntityHandle
zero_entity_handle(void){
    EntityHandle result = {0};
    return(result);
}

static Rect
rect_from_entity(Entity* e){
    Rect result = make_rect(make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2),
                            make_v2(e->pos.x + e->dim.x/2, e->pos.y + e->dim.h/2));
    return(result);
}

static Rect
collision_box_from_entity(Entity* e){
    Rect result = make_rect(
        make_v2(e->pos.x - e->dim.w/2 + e->collision_padding, e->pos.y - e->dim.h/2 + e->collision_padding),
        make_v2(e->pos.x + e->dim.x/2 - e->collision_padding, e->pos.y + e->dim.h/2 - e->collision_padding)
    );
    return(result);
}

static Quad
quad_from_entity(Entity* e){
    Quad result = {0};
    result.p0 = make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2);
    result.p1 = make_v2(e->pos.x + e->dim.w/2, e->pos.y - e->dim.h/2);
    result.p2 = make_v2(e->pos.x + e->dim.w/2, e->pos.y + e->dim.h/2);
    result.p3 = make_v2(e->pos.x - e->dim.w/2, e->pos.y + e->dim.h/2);
    return(result);
}




//static Vec4F32
//UI_PushBackgroundColor(Vec4F32 v){
//    UI_StackPushImpl(ui_state, BackgroundColor, background_color, Vec4F32, v)
//}
//
//
//
//UI_PushBackgroundColor(BLACK);
//
//#define UI_StackPushImpl(ui_state, BackgroundColor, background_color, Vec4F32, BLACK)
//
//UI_BackgroundColorNode *node = state->background_color_stack.free;
//if(node != 0) {
//    StackPop(state->background_color_stack.free);
//}
//else {
//    node = PushArray(UI_FrameArena(), UI_BackgroundColorNode, 1);
//}
//
//Vec4F32 old_value = state->background_color_stack.top->v;
//node->v = v;
//StackPush(state->background_color_stack.top, node);
//state->background_color_stack.auto_pop = 0;

//root_function UI_Size UI_PushPrefWidth(UI_Size v) { UI_StackPushImpl(ui_state, PrefWidth, pref_width, UI_Size, v)
//UI_PrefWidthNode *node = state->pref_width_stack.free;
//if(node != 0) {
//    StackPop(state->pref_width_stack.free);
//}
//else {
//    node = PushArray(UI_FrameArena(), UI_PrefWidthNode, 1);
//}
//
//UI_Size old_value = state->pref_width_stack.top->v;
//node->v = v;
//StackPush(state->pref_width_stack.top, node);
//state->background_color_stack.auto_pop = 0;

#endif

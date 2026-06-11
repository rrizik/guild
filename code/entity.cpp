#ifndef ENTITY_C
#define ENTITY_C

static bool
has_flags(u32 lflags, u32 rflags){
    bool result = (lflags & rflags) == rflags;
    return(result);
}

static void
set_flags(u32* lflags, u32 rflags){
    *lflags |= rflags;
}

static void
clear_flags(u32* lflags, u32 rflags){
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
quad_from_entity_world(Entity* e){
    Quad result = {0};
    result.p0 = make_v2(e->pos.x - e->dim.w/2, e->pos.y + e->dim.h/2);
    result.p1 = make_v2(e->pos.x + e->dim.w/2, e->pos.y + e->dim.h/2);
    result.p2 = make_v2(e->pos.x + e->dim.w/2, e->pos.y - e->dim.h/2);
    result.p3 = make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2);
    return(result);
}

static Quad
quad_from_entity_screen(Entity* e){
    Quad result = {0};
    result.p0 = make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2);
    result.p1 = make_v2(e->pos.x + e->dim.w/2, e->pos.y - e->dim.h/2);
    result.p2 = make_v2(e->pos.x + e->dim.w/2, e->pos.y + e->dim.h/2);
    result.p3 = make_v2(e->pos.x - e->dim.w/2, e->pos.y + e->dim.h/1);
    return(result);
}

static u32
entity_commands_count(Entity* e){
    u32 result = e->commands_write - e->commands_read;
    return(result);
}

static bool
entity_commands_empty(Entity* e){
    bool result = e->commands_read == e->commands_write;
    return(result);
}

static bool
entity_commands_full(Entity* e){
    bool result = (entity_commands_count(e) == ENTITY_COMMANDS_MAX);
    return(result);
}

static void
entity_commands_clear(Entity* e){
    e->active_command = 0;
    e->commands_read = 0;
    e->commands_write = 0;
}

static void
entity_commands_add(Entity* e, EntityCommand c){
    assert(!entity_commands_full(e));

    u32 write_idx = e->commands_write & (ENTITY_COMMANDS_MAX - 1);
    e->commands[write_idx] = c;
    e->commands_write++;
}

static EntityCommand*
entity_commands_read(Entity* e, u32 read){
    assert(!entity_commands_empty(e));

    u32 read_idx = read & (ENTITY_COMMANDS_MAX - 1);
    EntityCommand* c = e->commands + read_idx;

    return(c);
}

static EntityCommand*
entity_commands_next(Entity* e){
    assert(!entity_commands_empty(e));

    u32 read_idx = e->commands_read & (ENTITY_COMMANDS_MAX - 1);
    EntityCommand* c = e->commands + read_idx;
    e->commands_read++;

    return(c);
}

static Sprite_Direction 
entity_direction_from_velocity(Entity* e){
    Sprite_Direction result = e->sprite.direction;

    f32 deadzone = 0.1f;
    bool moving_right = e->velocity.x > deadzone;
    bool moving_left =  e->velocity.x < -deadzone;
    bool moving_up =    e->velocity.y > deadzone;
    bool moving_down =  e->velocity.y < -deadzone;

    if(moving_right && moving_up){
        result = RIGHT_BACK;
    }
    else if(moving_right && moving_down){
        result = RIGHT_FRONT;
    }
    else if(moving_left && moving_up){
        result = LEFT_BACK;
    }
    else if(moving_left && moving_down){
        result = LEFT_FRONT;
    }

    else if(moving_right){
        result = RIGHT_FRONT;
    }
    else if(moving_left){
        result = LEFT_FRONT;
    }

    else if(moving_up){
        if(result == LEFT_FRONT || result == LEFT_BACK){
            result = LEFT_BACK;
        }
        else{
            result = RIGHT_BACK;
        }
    }
    else if(moving_down){
        if(result == LEFT_BACK || result == LEFT_FRONT){
            result = LEFT_FRONT;
        }
        else{
            result = RIGHT_FRONT;
        }
    }

    return(result);
}

static void
entity_commands_move(Entity* e, v2 move_to, v2 clicked_at){
    EntityCommand c = {0};
    c.type = EntityCommandType_Move;
    c.move_to = move_to;
    c.clicked_at = clicked_at;

    entity_commands_add(e, c);
}

static bool
entity_is_moving(Entity* e){
    f32 deadzone = 0.1f;

    bool result = (abs_f32(e->velocity.x) > deadzone || 
                   abs_f32(e->velocity.y) > deadzone);

    return(result);
}

static void
update_sprite(Spritesheet* sprite, f32 dt){
    Sprite_Animation* anim = sprite->animations + sprite->kind;
    Sprite_Sequence* sequence = anim->directions + sprite->direction;
    anim->time += anim->speed * dt;
    if(anim->time >= anim->anim_time){
        anim->time = 0;
        anim->col = (s32)(anim->col + 1) % sequence->frame_count;
    }

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

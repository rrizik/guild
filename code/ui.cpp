#ifndef UI_C
#define UI_C

// TODO IMPORTANT(rr): do more loops rather than recursive calls in layout building
// TODO IMPORTANT(rr): do more loops rather than recursive calls in layout building
// TODO IMPORTANT(rr): do more loops rather than recursive calls in layout building

static void
ui_init(Arena* arena, Window* window, Controller* controller, Assets* assets){
    ui_state = push_struct(arena, UI_State);
    ui_state->window = window;
    ui_state->controller = controller;
    ui_state->default_font = &assets->fonts[FontAsset_Arial];
    //ui_state->generation = 0;
    table_init(arena, &ui_state->table);
    ui_state->arena = make_arena(MB(100));

    ui_state->parent_stack.top = &ui_parent_null;
    ui_state->pos_x_stack.top = &ui_pos_x_null;
    ui_state->pos_y_stack.top = &ui_pos_y_null;
    ui_state->size_w_stack.top = &ui_size_w_null;
    ui_state->size_h_stack.top = &ui_size_h_null;
    ui_state->layout_axis_stack.top = &ui_layout_axis_null;
    ui_state->text_padding_stack.top = &ui_text_padding_null;
    ui_state->text_color_stack.top = &ui_text_color_null;
    ui_state->background_color_stack.top = &ui_background_color_null;
    ui_state->border_thickness_stack.top = &ui_border_thickness_null;
    ui_state->font_stack.top = &ui_font_null;
}

static void
ui_begin(void){
    //ui_state->generation += 1;
    ui_state->hot = 0;

    ui_push_pos_x(0);
    ui_push_pos_y(0);
    ui_push_size_w(ui_size_pixel(ui_window()->width, 0));
    ui_push_size_h(ui_size_pixel(ui_window()->height, 0));
    ui_push_layout_axis(Axis_Y);

    ui_push_text_padding(0);
    ui_push_text_color(BLACK);

    ui_push_border_thickness(10);

    ui_push_font(ui_state->default_font);

    ui_push_background_color(CLEAR);

    ui_state->root = ui_make_box(str8_literal(""), 0);
    ui_push_parent(ui_state->root);
}

static void
ui_end(void){
    ui_layout();
    ui_draw(ui_root());

    ui_state->parent_stack.top = &ui_parent_null;
    ui_state->pos_x_stack.top = &ui_pos_x_null;
    ui_state->pos_y_stack.top = &ui_pos_y_null;
    ui_state->size_w_stack.top = &ui_size_w_null;
    ui_state->size_h_stack.top = &ui_size_h_null;
    ui_state->layout_axis_stack.top = &ui_layout_axis_null;
    ui_state->text_padding_stack.top = &ui_text_padding_null;
    ui_state->text_color_stack.top = &ui_text_color_null;
    ui_state->background_color_stack.top = &ui_background_color_null;
    ui_state->border_thickness_stack.top = &ui_border_thickness_null;
    ui_state->font_stack.top = &ui_font_null;

    // todo(rr): go back to this
    //ui_state->parent_stack.top = 0;
    //ui_state->pos_x_stack.top = 0;
    //ui_state->pos_y_stack.top = 0;
    //ui_state->size_w_stack.top = 0;
    //ui_state->size_h_stack.top = 0;
    //ui_state->layout_axis_stack.top = 0;
    //ui_state->text_padding_stack.top = 0;
    //ui_state->text_color_stack.top = 0;
    //ui_state->background_color_stack.top = 0;
    //ui_state->border_thickness_stack.top = 0;
    //ui_state->font_stack.top = 0;

    arena_free(ui_arena());
}

static void
ui_layout(void){
    for(Axis axis=(Axis)0; axis < Axis_Count; axis = (Axis)(axis + 1)){
        ui_traverse_independent(ui_root(), axis);
        ui_traverse_children(ui_root(), axis);
        ui_traverse_positions(ui_root(), axis);
    }
    ui_traverse_rects(ui_root());
}

static void
ui_auto_pop(void){
    if(ui_state->parent_stack.auto_pop)           { ui_state->parent_stack.auto_pop = false; ui_pop_parent(); }
    if(ui_state->pos_x_stack.auto_pop)            { ui_state->pos_x_stack.auto_pop  = false; ui_pop_pos_x(); }
    if(ui_state->pos_y_stack.auto_pop)            { ui_state->pos_y_stack.auto_pop  = false; ui_pop_pos_y(); }
    if(ui_state->size_w_stack.auto_pop)           { ui_state->size_w_stack.auto_pop = false; ui_pop_size_w(); }
    if(ui_state->size_h_stack.auto_pop)           { ui_state->size_h_stack.auto_pop = false; ui_pop_size_h(); }
    if(ui_state->layout_axis_stack.auto_pop)      { ui_state->layout_axis_stack.auto_pop  = false; ui_pop_layout_axis(); }
    if(ui_state->text_padding_stack.auto_pop)     { ui_state->text_padding_stack.auto_pop = false; ui_pop_text_padding(); }
    if(ui_state->text_color_stack.auto_pop)       { ui_state->text_color_stack.auto_pop   = false; ui_pop_text_color(); }
    if(ui_state->background_color_stack.auto_pop) { ui_state->background_color_stack.auto_pop = false; ui_pop_background_color(); }
    if(ui_state->border_thickness_stack.auto_pop) { ui_state->border_thickness_stack.auto_pop = false; ui_pop_border_thickness(); }
    if(ui_state->font_stack.auto_pop)             { ui_state->font_stack.auto_pop = false; ui_pop_font(); }
}

// todo: ui_root() on first function call so that you don't have to pass it in.
static void
ui_draw(UI_Box* box){
    if(box == 0){
        return;
    }

    if(has_flag(box->flags, UI_BoxFlag_DrawBackground)){
        if(ui_state->hot == box->key && ui_state->active == box->key){
            if(has_flag(box->flags, UI_BoxFlag_ActiveAnimation)){
                box->background_color = darken_color(box->background_color, 0.2f);
            }
        }
        if(ui_state->hot == box->key && ui_state->active == 0){
            if(has_flag(box->flags, UI_BoxFlag_HotAnimation)){
                box->background_color = brighten_color(box->background_color, 0.2f);
            }
        }
        draw_quad(box->rect, box->background_color);
    }
    if(has_flag(box->flags, UI_BoxFlag_DrawText)){
        set_font(box->font);

        String8 text = ui_text_part_from_key(box->string);

        f32 width = font_string_width(box->font, text);
        f32 vertical_offset = font_vertical_offset(box->font);
        f32 ascent = font_ascent(box->font);
        f32 descent = font_descent(box->font);

        f32 center = ascent - (ascent - descent)/2;
        v2 pos = make_v2(box->rect.min.x + box->size[Axis_X]/2 - width/2,
                         box->rect.min.y + box->size[Axis_Y]/2 + center);
        draw_text(text, pos, box->text_color);
    }

    if(box->first){
        ui_draw(box->first);
    }
    ui_draw(box->next);
}

static Arena*
ui_arena(void){
    return(ui_state->arena);
}

static Window*
ui_window(void){
    return(ui_state->window);
}

static UI_Box*
ui_root(void){
    return(ui_state->root);
}

static HashTable
ui_table(void){
    return(ui_state->table);
}

static v2
ui_mouse_pos(void){
    return(ui_state->controller->mouse.pos);
}

static Mouse
ui_mouse(void){
    return(ui_state->controller->mouse);
}

static String8
ui_text_part_from_key(String8 string){
    String8 result = {0};
    s64 idx = byte_index_from_left(string, '#');
    result = str8_split_left(string, (u64)idx);
    return(result);
}

static UI_Size
ui_make_size(UI_SizeType type, f32 value, f32 strictness){
    UI_Size result = {0};
    result.type = type;
    result.value = value;
    result.strictness = strictness;
    return(result);
}

static BoxCache cache_from_box(UI_Box* box) {
    BoxCache result = {0};
    result.rect = box->rect;
    result.size[Axis_X] = box->size[Axis_X];
    result.size[Axis_Y] = box->size[Axis_Y];
    result.pos[Axis_X] = box->pos[Axis_X];
    result.pos[Axis_Y] = box->pos[Axis_Y];
    result.rel_pos[Axis_X] = box->rel_pos[Axis_X];
    result.rel_pos[Axis_Y] = box->rel_pos[Axis_Y];
    return(result);
}

static UI_Box*
ui_make_box(String8 string, UI_BoxFlags flags){
    UI_Box* result = push_struct_zero(ui_arena(), UI_Box);

    if(ui_state->parent_stack.top != &ui_parent_null){
        UI_Box* top_parent = ui_top_parent();
        result->parent = top_parent;
        if(top_parent->first == 0){
            top_parent->first = result;
            top_parent->last = result;
        }
        else{
            top_parent->last->next = result;
            result->prev = top_parent->last;
            top_parent->last = result;
        }
    }

    result->flags = flags;
    result->string = string;
    result->key = hash_from_string(string);

    result->rel_pos[Axis_X] = ui_top_pos_x();
    result->rel_pos[Axis_Y] = ui_top_pos_y();
    result->semantic_size[Axis_X] = ui_top_size_w();
    result->semantic_size[Axis_Y] = ui_top_size_h();
    result->layout_axis = ui_top_layout_axis();

    result->text_padding = ui_top_text_padding();
    result->text_color = ui_top_text_color();

    result->background_color = ui_top_background_color();
    result->border_thickness = ui_top_border_thickness();
    result->font = ui_top_font();

    BoxCache* cache = table_lookup(BoxCache, &ui_state->table, string);
    if(cache){
        // todo(rr): I think I need a per cache item flag check, almost like auto_pop
        if(!has_flag(flags, UI_BoxFlag_NoCache)){
            result->rect = cache->rect;
            result->size[Axis_X] = cache->size[Axis_X];
            result->size[Axis_Y] = cache->size[Axis_Y];
            result->pos[Axis_X] = cache->pos[Axis_X];
            result->pos[Axis_Y] = cache->pos[Axis_Y];
            result->rel_pos[Axis_X] = cache->rel_pos[Axis_X];
            result->rel_pos[Axis_Y] = cache->rel_pos[Axis_Y];
        }
    }

    ui_auto_pop();

    return(result);
}

// Basic Widgets
static UI_Box*
ui_begin_panel(String8 string, UI_BoxFlags flags){
    UI_Box* box = ui_make_box(string, flags);
    ui_push_parent(box);
    return(box);
}

static void
ui_end_panel(void){
    UI_Box* box = ui_pop_parent();
    UI_Signal signal = ui_signal_from_box(box);
}

static UI_Signal
ui_button(String8 string, UI_BoxFlags flags_in){
    u32 flags = UI_BoxFlag_Clickable |
                UI_BoxFlag_DrawBorder |
                UI_BoxFlag_DrawBackground |
                UI_BoxFlag_HotAnimation |
                UI_BoxFlag_ActiveAnimation |
                UI_BoxFlag_DrawText |
                flags_in;
    UI_Box* box = ui_make_box(string, flags);

    UI_Signal signal = ui_signal_from_box(box);

    return(signal);
}

// todo(rr): make string unique
static void
ui_label(String8 string){
    u32 flags = UI_BoxFlag_DrawText;
    UI_Box* box = ui_make_box(string, flags);
}

static void
ui_spacer(f32 size){
    u32 flags = 0;
    ui_size(ui_size_pixel(size, 0), ui_size_pixel(size, 0))
    {
        UI_Box* box = ui_make_box(str8_literal(""), flags);
    }
}

static UI_Signal
ui_signal_from_box(UI_Box* box){
    UI_Signal signal = {0};
    if(box == ui_state->root){
        return(signal);
    }

    v2 mouse_pos = ui_mouse_pos();
    if(has_flag(box->flags, UI_BoxFlag_Clickable)){

        if(rect_contains_point(box->rect, mouse_pos)){
            if(ui_state->hot == 0){
                ui_state->hot = box->key;
            }
        }

        if(ui_state->hot == box->key && ui_state->active == 0){
            if(controller_button_pressed(MOUSE_BUTTON_LEFT) && controller_button_held(MOUSE_BUTTON_LEFT)){
                ui_state->active = box->key;
                ui_state->mouse_pos_record = mouse_pos;
                ui_state->mouse_pos_record.x -= box->rel_pos[Axis_X];
                ui_state->mouse_pos_record.y -= box->rel_pos[Axis_Y];
            }
        }
        else if(ui_state->active == box->key){
            if(!controller_button_held(MOUSE_BUTTON_LEFT)){
                if(rect_contains_point(box->rect, mouse_pos)){
                    signal.pressed_left = true;
                }
                ui_state->active = 0;
                ui_state->hot = 0;
                ui_state->mouse_pos_record = {0};
            }
        }
    }

    if(has_flag(box->flags, UI_BoxFlag_Draggable)){
        if(ui_state->active == box->key && controller_button_held(MOUSE_BUTTON_LEFT)){
            box->rel_pos[Axis_X] = (f32)(mouse_pos.x - ui_state->mouse_pos_record.x);
            box->rel_pos[Axis_Y] = (f32)(mouse_pos.y - ui_state->mouse_pos_record.y);
        }
    }

    return(signal);
}

static void
ui_traverse_independent(UI_Box* box, Axis axis){
    //if(box == 0){
    //    return;
    //}

    for(UI_Box* child = box->first; child != 0; child = child->next){
        ui_traverse_independent(child, axis);
    }

    switch(box->semantic_size[axis].type){
        case UI_SizeType_Pixel:{
            box->size[axis] = box->semantic_size[axis].value;
        } break;
        case UI_SizeType_TextContent:{
            if(axis == Axis_X){
                f32 width = font_string_width(box->font, box->string);
                box->size[axis] = (f32)width + box->text_padding;
            }
            if(axis == Axis_Y){
                f32 height = font_vertical_offset(box->font);
                box->size[axis] = height + box->text_padding;
            }
        } break;
    }

    //if(box->first != 0){
    //    ui_traverse_independent(box->first, axis);
    //}

    //ui_traverse_independent(box->next, axis);
}

static void
ui_traverse_children(UI_Box* box, Axis axis){
    //if(box == 0){
    //    return;
    //}

    //if(box->last){
    //    ui_traverse_children(box->last, axis);
    //}

    for(UI_Box* child = box->first; child != 0; child = child->next){
        ui_traverse_children(child, axis);
    }

    if(box->semantic_size[axis].type == UI_SizeType_ChildrenSum){
        f32 total = 0;
        if(box->layout_axis == axis){
            for(UI_Box* child=box->first; child !=0; child = child->next){
                total += child->size[axis];
            }
            box->size[axis] = total;
        }
        else{
            for(UI_Box* child=box->first; child !=0; child = child->next){
                total = MAX(total, child->size[axis]);
            }
            box->size[axis] = total;
        }
        // todo: when we introduce border color, this needs to change as we will be drawing 2 rects for the border
        box->size[axis] += box->border_thickness * 2;
    }

    //ui_traverse_children(box->prev, axis);
}

// There is a root box that is essentially the window, simply the (0, 0) pos and w/h.
static void
ui_traverse_positions(UI_Box* box, Axis axis){
    //if(box == 0){
    //    return;
    //}

    for(UI_Box* child = box->first; child != 0; child = child->next){
        ui_traverse_positions(child, axis);
    }

    // TODO(rr): why do I have this here?
    //if(!box->prev){
    if(box->layout_axis == axis){
        if(has_flag(box->flags, UI_BoxFlag_NoSiblings)){
            if(box->parent){
                box->pos[axis] = box->parent->pos[axis] + box->rel_pos[axis] + box->border_thickness;
            }
        }
        else{
            f32 position = box->rel_pos[axis];
            for(UI_Box* sibling = box; sibling != 0; sibling = sibling->next){
                sibling->rel_pos[axis] = position;
                if(sibling->parent){
                    sibling->pos[axis] = sibling->parent->pos[axis] + sibling->rel_pos[axis] + sibling->border_thickness;
                }
                position += sibling->size[axis];
            }
        }
    }
    else{
        if(has_flag(box->flags, UI_BoxFlag_NoSiblings)){
            if(box->parent){
                box->pos[axis] = box->parent->pos[axis] + box->rel_pos[axis] + box->border_thickness;
            }
        }
        else{
            for(UI_Box* sibling = box; sibling != 0; sibling = sibling->next){
                if(sibling->parent){
                    sibling->pos[axis] = sibling->parent->pos[axis] + sibling->rel_pos[axis] + sibling->border_thickness;
                }
            }
        }
    }
    //}

    //if(box->first){
    //    ui_traverse_positions(box->first, axis);
    //}
    //ui_traverse_positions(box->next, axis);
}

static void
ui_traverse_rects(UI_Box* box){
    if(box == 0){
        return;
    }

    if(box->parent){
        box->rect.min = make_v2(box->parent->pos[Axis_X] + box->rel_pos[Axis_X],
                                box->parent->pos[Axis_Y] + box->rel_pos[Axis_Y]);
        box->rect.max = make_v2(box->parent->pos[Axis_X] + box->rel_pos[Axis_X] + box->size[Axis_X],
                                box->parent->pos[Axis_Y] + box->rel_pos[Axis_Y] + box->size[Axis_Y]);
    }
    else{
        box->rect.min = make_v2(box->pos[Axis_X], box->pos[Axis_Y]);
        box->rect.max = make_v2(box->pos[Axis_X] + box->size[Axis_X], box->pos[Axis_Y] + box->size[Axis_Y]);
    }

    // cache rect
    if(!str8_compare(box->string, str8_literal(""))){
        BoxCache cache = cache_from_box(box);
        table_insert(&ui_state->table, box->string, cache);
    }

    if(box->first){
        ui_traverse_rects(box->first);
    }
    ui_traverse_rects(box->next);
}

#endif

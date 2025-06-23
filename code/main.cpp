#include "main.hpp"

static void
sim_game(void){

    // camera
    {
        // movement
        if(controller_button_held(KeyCode_UP)){
            camera.y += ((camera.size) + 50) * (f32)clock.dt;
        }
        if(controller_button_held(KeyCode_DOWN)){
            camera.y -= ((camera.size) + 50) * (f32)clock.dt;
        }
        if(controller_button_held(KeyCode_LEFT)){
            camera.x -= ((camera.size) + 50) * (f32)clock.dt;
        }
        if(controller_button_held(KeyCode_RIGHT)){
            camera.x += ((camera.size) + 50) * (f32)clock.dt;
        }
    }

    // draw terrain
    if(state->scene_state == SceneState_Editor){
        begin_timed_scope("draw terrain");
        if(controller_button_held(MOUSE_BUTTON_LEFT) &&
           controller_button_pressed(MOUSE_BUTTON_LEFT, false)){
            state->draw_terrain = true;
        }
        if(!controller_button_held(MOUSE_BUTTON_LEFT)){
            state->draw_terrain = false;
        }

        if(state->draw_terrain){
            if(state->terrain_selected){
                v2 pos = controller.mouse.world_pos;
                v2 cell = make_v2(floor_f32(pos.x/state->world_cell_size), floor_f32(pos.y/state->world_cell_size));
                if((cell.x < state->world_width_in_cells && cell.x >= 0)){
                    s32 idx = (s32)((cell.y * state->world_width_in_cells) + cell.x);
                    state->world_grid[idx] = state->terrain_selected_id;
                }
            }
        }
    }

    if(do_motion){
    // Resolve motion.
        begin_timed_scope("flocking");
        for(s32 i = 0; i < array_count(state->entities); ++i){
            Entity *e = state->entities + i;
            if(!has_flag(e->flags, EntityFlag_Active)) continue;
            if(!has_flag(e->flags, EntityFlag_MoveWithPhys)) continue;

            // All 9 surrounding cells.
            v2 cell_coords = grid_cell_from_pos(e->pos, state->flocking_cell_size);
            v2 all_coords[9] = {
                cell_coords,
                make_v2(cell_coords.x - 1, cell_coords.y - 1),
                make_v2(cell_coords.x - 1, cell_coords.y),
                make_v2(cell_coords.x - 1, cell_coords.y + 1),
                make_v2(cell_coords.x, cell_coords.y - 1),
                make_v2(cell_coords.x, cell_coords.y + 1),
                make_v2(cell_coords.x + 1, cell_coords.y - 1),
                make_v2(cell_coords.x + 1, cell_coords.y),
                make_v2(cell_coords.x + 1, cell_coords.y + 1),
            };

            if(controller_button_pressed(KeyCode_1)){
                do_motion = !do_motion;
            }
            if(controller_button_pressed(KeyCode_2)){
                for(s32 i=0; i < array_count(state->entities_selected); ++i){
                    Entity* e = state->entities_selected[i];
                    entity_commands_clear(e);
                }
            }

            // Flocking, cumulative velocity.
            for(s32 j=0; j < array_count(all_coords); ++j){
                v2 coords = all_coords[j];
                if(!grid_cell_coords_in_bounds(coords)) continue;

                Cell* cell = state->cells + ((s32)coords.x + (WORLD_WIDTH_IN_TILES_MAX * (s32)coords.y));
                for(BinNode* bin = cell->bin; bin != 0; bin = bin->next){
                    for(s32 k = 0; k < bin->at; ++k){
                        Entity* other = bin->entities[k];
                        if(!has_flag(other->flags, EntityFlag_MoveWithPhys)) continue;
                        if(e == other) continue;

                        f32 distance_squared = distance_squared_v2(e->pos, other->pos);
                        if(distance_squared > 0 && distance_squared < 1){
                            f32 distance = sqrtf(distance_squared);
                            v2 dir = (e->pos - other->pos);
                            dir.x /= distance;
                            dir.y /= distance;
                            e->velocity.x     += (dir.x * 50 * (f32)clock.dt) / distance;
                            e->velocity.y     += (dir.y * 50 * (f32)clock.dt) / distance;
                            other->velocity.x -= (dir.x * 50 * (f32)clock.dt) / distance;
                            other->velocity.y -= (dir.y * 50 * (f32)clock.dt) / distance;
                        }
                    }
                }
            }

            // Look for command.
            if(!entity_commands_empty(e) && !e->active_command){
                EntityCommand* c = entity_commands_next(e);
                e->active_command = c;
            }

            // Resolve command.
            if(e->active_command){
                EntityCommand* c = e->active_command;

                switch(c->type){
                    case EntityCommandType_Move:{
                        v2 move_dir = direction_v2(e->pos, c->move_to);
                        e->dir = move_dir;
                        if(!v2_close_enough(e->pos, c->move_to, 0.1f)){
                            e->velocity.x += (move_dir.x * e->speed) * (f32)clock.dt;
                            e->velocity.y += (move_dir.y * e->speed) * (f32)clock.dt;
                        }
                        else{
                            e->active_command = 0;
                        }
                    }
                }
            }

            // Apply friction.
            e->velocity.x *= 0.75f;
            e->velocity.y *= 0.75f;
            // Apply motion.
            e->pos.x += e->velocity.x * (f32)clock.dt;
            e->pos.y += e->velocity.y * (f32)clock.dt;
        }
    }

    // resolve death todo(rr): this will be different
    //for(s32 i = 0; i < array_count(state->entities); ++i){
    //    Entity *e = state->entities + i;
    //    if(!has_flag(e->flags, EntityFlag_Active)){
    //        continue;
    //    }
    //}

    // type loop
    //for(s32 i = 0; i < array_count(state->entities); ++i){
    //    Entity *e = state->entities + i;
    //    Rect e_rect = rect_from_entity(e);

    //    //switch(e->type){
    //    //}
    //}
}

static m4
m4_screen_from_world(){
    f32 sx =  window.width  / (2.0f * camera.size * window.aspect_ratio);
    f32 sy = -window.height / (2.0f * camera.size);
    f32 tx = (-camera.x / (2.0f * camera.size * window.aspect_ratio) + 0.5f) * window.width;
    f32 ty = ( camera.y / (2.0f * camera.size) + 0.5f) * window.height;

    m4 world_to_screen = {
         sx, 0,  0, tx,
         0,  sy, 0, ty,
         0,  0,  1, 0,
         0,  0,  0, 1
    };

    return(world_to_screen);
}

static m4
m4_world_from_screen(){
    // recompute the same parameters:
    f32 sx =  window.width  / (2.0f * camera.size * window.aspect_ratio);
    f32 sy = -window.height / (2.0f * camera.size);
    f32 tx = (-camera.x / (2.0f * camera.size * window.aspect_ratio) + 0.5f) * window.width;
    f32 ty = ( camera.y / (2.0f * camera.size)                + 0.5f) * window.height;

    // invert them:
    f32 isx = 1.0f / sx;
    f32 isy = 1.0f / sy;
    f32 itx = -tx * isx;
    f32 ity = -ty * isy;

    m4 screen_to_world = {
        isx, 0,   0,  itx,
         0,  isy, 0,  ity,
         0,  0,   1,   0,
         0,  0,   0,   1
    };

    return(screen_to_world);
}

static v2
m4_translate_v2(m4 mat, v2 value){
    f32 x = value.x;
    f32 y = value.y;
    f32 z = 0.0f;
    f32 w = 1.0f;

    v2 result;
    result.x = x * mat._11 + y * mat._12 + z * mat._13 + w * mat._14;
    result.y = x * mat._21 + y * mat._22 + z * mat._23 + w * mat._24;
    return(result);
}

static bool
v2_close_enough(v2 p1, v2 p2, f32 epsilon){
    f32 x = abs_f32(p1.x - p2.x);
    f32 y = abs_f32(p1.y - p2.y);
    return(x <= epsilon && y <= epsilon);
}

// todo(rr): Move these to entity once you move PermanentMemory further up in the tool chain
static Entity*
entity_from_handle(EntityHandle handle){
    Entity *result = 0;
    if(handle.index < (s32)array_count(state->entities)){
        Entity *e = state->entities + handle.index;
        if(e->generation == handle.generation){
            result = e;
        }
    }
    return(result);
}

static EntityHandle
handle_from_entity(Entity *e){
    assert(e != 0);
    EntityHandle result = {0};
    if((e >= state->entities) && (e < (state->entities + array_count(state->entities)))){
        result.index = e->index;
        result.generation = e->generation;
    }
    return(result);
}

static void
remove_entity(Entity* e){
    e->type = EntityType_None;
    clear_flag(&e->flags, EntityFlag_Active);
    state->free_entities[++state->free_entities_at] = e->index;
    state->entities_count--;
    *e = {0};
}

static Entity*
add_entity(EntityType type){
    if(state->free_entities_at < ENTITIES_MAX){
        u32 free_entity_index = state->free_entities[state->free_entities_at--];
        Entity *e = state->entities + free_entity_index;
        e->index = free_entity_index;
        set_flag(&e->flags, EntityFlag_Active);
        state->generation[e->index]++;
        state->entities_count++;
        e->generation = state->generation[e->index]; // CONSIDER: this might not be necessary
        e->type = type;

        return(e);
    }
    return(0);
}

static Entity*
add_quad(v2 pos, v2 dim, RGBA color){
    Entity* e = add_entity(EntityType_Quad);
    if(e){
        e->rot = make_v2(1, 1);
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->rot = make_v2(0, 1);
        e->deg = 90;
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static Entity*
add_texture(TextureAsset texture, v2 pos, v2 dim, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Texture);
    if(e){
        e->rot = make_v2(1, 1);
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->rot = make_v2(0, 1);
        e->deg = 90;
        e->texture = texture;
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static Entity*
add_castle(TextureAsset texture, v2 cell, v2 dim, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Structure);
    if(e){
        e->color = color;
        e->cell = cell;
        e->pos = grid_cell_center(e->cell);
        e->rallypoint_cell = make_v2(e->cell.x, e->cell.y - 1);
        e->rallypoint = grid_cell_center(e->rallypoint_cell);
        e->dim = dim;
        e->texture = texture;
        e->deg = 0;
        e->rot = dir_from_deg(e->deg);
        e->structure_type = StructureType_Castle;
    }
    else{
        print("Failed to add entity: Castle\n");
    }
    return(e);
}

static Entity*
add_skeleton(TextureAsset texture, v2 cell, v2 dim, v2 dir, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Skeleton1);
    if(e){
        e->color = color;
        e->pos = grid_pos_from_cell(cell, state->world_cell_size);
        e->dim = dim;
        e->texture = texture;
        e->velocity = {0};
        e->speed = 1000.0f;
        e->dir = dir;
        e->rot = make_v2(1, 0);
        e->deg = deg_from_dir(e->rot);
        set_flag(&e->flags, EntityFlag_MoveWithPhys);
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static void
entities_clear(void){
    state->free_entities_at = ENTITIES_MAX - 1;
    for(u32 i = state->free_entities_at; i <= state->free_entities_at; --i){
        Entity* e = state->entities + i;
        e->type = EntityType_None;
        clear_flag(&e->flags, EntityFlag_Active);
        state->free_entities[i] = state->free_entities_at - i;
        state->generation[i] = 0;
    }
    state->entities_count = 0;
}


static bool
handle_global_events(Event event){
    if(event.keycode == KeyCode_ESCAPE){
        should_quit = true;
    }
    if(event.type == EventType_QUIT){
        should_quit = true;
        return(true);
    }
    if(event.type == EventType_KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == KeyCode_TILDE && !event.repeat){
                if(event.shift_pressed){
                    if(console.state == OPEN_BIG){
                        console_set_state(CLOSED);
                    }
                    else{
                        console_set_state(OPEN_BIG);
                    }
                }
                else{
                    if(console.state == OPEN || console.state == OPEN_BIG){
                        console_set_state(CLOSED);
                    }
                    else{
                        console_set_state(OPEN);
                    }

                }
                return(true);
            }
        }
    }
    return(false);
}

static bool
handle_camera_events(Event event){
    if(event.type == EventType_KEYBOARD){
        if(event.key_pressed){
        }
    }
    return(false);
}

static bool
handle_controller_events(Event event){
    if(event.type == EventType_MOUSE){
        controller.mouse.x  = event.mouse_x;
        controller.mouse.y  = event.mouse_y;
        controller.mouse.dx = event.mouse_dx;
        controller.mouse.dy = event.mouse_dy;
        controller.mouse.world_x  = event.world_mouse_x;
        controller.mouse.world_y  = event.world_mouse_y;
        //controller.mouse.edge_left   = event.mouse_edge_left;
        //controller.mouse.edge_right  = event.mouse_edge_right;
        //controller.mouse.edge_top    = event.mouse_edge_top;
        //controller.mouse.edge_bottom = event.mouse_edge_bottom;
    }
    if(event.type == EventType_KEYBOARD){
        controller.mouse.wheel_dir = event.mouse_wheel_dir;
        // todo(rr): change this to
        // controller.button[event.keycode].pressed = event.key_pressed;
        // and check for repeat for held.
        if(event.key_pressed){
            controller.button[event.keycode].pressed = true;
            controller.button[event.keycode].held = true;
        }
        if(event.key_released){
            controller.button[event.keycode].released = true;
            controller.button[event.keycode].held = false;
        }
        controller.shift_pressed = event.shift_pressed;
        controller.ctrl_pressed = event.ctrl_pressed;
        controller.alt_pressed = event.alt_pressed;
    }
    return(false);
}

static bool
handle_game_events(Event event){
    if(event.type == EventType_KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == KeyCode_ESCAPE){
                //if(state->scene_state == SceneState_Game){
                //    if(state->game_state == GameState_Running){
                //        state->game_state = GameState_Paused;
                //        return(true);
                //    }
                //    if(state->game_state == GameState_Paused){
                //        state->game_state = GameState_Running;
                //        return(true);
                //    }
                //    return(false);
                //}
            }
        }
    }
    return(false);
}

static void
generate_new_world(f32 width, f32 height){
    f32 y = 0;
    while(y < width){
        f32 x = 0;
        while(x < height){
            s32 idx = (s32)((y * height) + x);
            state->world_grid[idx] = 1;

            x += 1;
        }
        y += 1;
    }
}

static void
draw_entities(State* state){
    for(s32 idx = 0; idx < array_count(state->entities); ++idx){
        Entity *e = state->entities + idx;

        Quad quad = quad_from_entity(e);
        if(has_flag(e->flags, EntityFlag_Active)){

            switch(e->type){
                case EntityType_Quad:{
                    quad = rotate_quad(quad, e->deg, e->pos);

                    draw_quad(quad, e->color);
                } break;
            }
        }
    }
    for(s32 idx = 0; idx < array_count(state->entities); ++idx){
        Entity *e = state->entities + idx;

        Quad quad = quad_from_entity(e);
        if(has_flag(e->flags, EntityFlag_Active)){

            switch(e->type){
                case EntityType_Structure:{
                    quad = rotate_quad(quad, e->deg, e->pos);

                    switch(e->structure_type){
                        case StructureType_Castle:{
                            set_texture(&r_assets->textures[e->texture]);
                            draw_texture(quad, e->color);

                            if(e->selected){
                                draw_line(e->pos, e->rallypoint, 0.1f, RED);
                            }

                            set_font(state->font);
                            String8 fmt_str = str8_format(ts->frame_arena, "(%f, %f)", e->rallypoint.x, e->rallypoint.y);
                            // no
                            //draw_text(fmt_str, v2_screen_from_world(e->rallypoint), GREEN);
                        }
                    }
                } break;
            }
        }
    }
    for(s32 idx = 0; idx < array_count(state->entities); ++idx){
        Entity *e = state->entities + idx;

        Quad quad = quad_from_entity(e);
        if(has_flag(e->flags, EntityFlag_Active)){

            switch(e->type){
                case EntityType_Texture:{
                    quad = rotate_quad(quad, e->deg, e->pos);

                    set_texture(&r_assets->textures[e->texture]);
                    draw_texture(quad, e->color);
                } break;
                case EntityType_Skeleton1:{
                    quad = rotate_quad(quad, e->deg, e->pos);

                    set_texture(&r_assets->textures[e->texture]);
                    draw_texture(quad, e->color);



                    if(e->selected){
                        if(e->active_command){
                            draw_quad(e->active_command->clicked_at, make_v2(0.25f, 0.25f), RED);
                        }

                        u32 read_idx = e->commands_read;
                        while(read_idx != e->commands_write){
                            EntityCommand* c = entity_commands_read(e, read_idx);
                            read_idx++;

                            draw_quad(c->clicked_at, make_v2(0.1f, 0.1f), RED);
                            // no
                            //draw_line(e->pos, screen_space, 0.1f, ORANGE);
                        }
                    }

                    if(state->scene_state == SceneState_Editor){
                        if(state->show_entity_info){
                            v2 pos = e->pos;
                            ui_set_pos(pos.x + 20, pos.y);
                            ui_set_size(ui_size_children(0), ui_size_children(0));
                            ui_set_border_thickness(10);
                            ui_set_background_color(DEFAULT);

                            String8 box_name = str8_formatted(ts->frame_arena, "skelebox##%i", idx);
                            ui_begin_panel(box_name, ui_floating_panel_world);

                            String8 fmt_str;
                            ui_size(ui_size_text(0), ui_size_text(0))
                            ui_text_color(LIGHT_GRAY)
                            {
                                fmt_str = str8_formatted(ts->frame_arena, "idx: %i", e->index);
                                ui_label(fmt_str);
                            }
                            ui_end_panel();
                        }
                    }
                } break;
            }
        }
    }
}

static void
debug_draw_mouse_cell_pos(void){
    set_font(state->font);
    v2 pos = controller.mouse.world_pos;
    v2 cell = make_v2(floor_f32(pos.x/state->world_cell_size), floor_f32(pos.y/state->world_cell_size));
    String8 cell_str = str8_format(ts->frame_arena, "(%i, %i)", (s32)cell.x, (s32)cell.y);
    draw_text(cell_str, controller.mouse.pos, RED);
}

static void
ui_editor(void){

    ui_set_pos(20, 20);
    ui_set_size(ui_size_children(0), ui_size_children(0));
    ui_set_border_thickness(10);
    ui_set_background_color(DEFAULT);
    ui_begin_panel(str8_literal("box1##2"), ui_floating_panel);

    ui_size(ui_size_pixel(100, 0), ui_size_pixel(50, 0))
    ui_background_color(DARK_GRAY)
    {
        if(ui_button(str8_literal("none")).pressed_left){
            state->terrain_selected_id = 0;
            state->terrain_selected = false;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("erase")).pressed_left){
            state->terrain_selected_id = 0;
            state->terrain_selected = true;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("grass")).pressed_left){
            state->terrain_selected_id = TextureAsset_Grass1;
            state->terrain_selected = true;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("water")).pressed_left){
            state->terrain_selected_id = TextureAsset_Water1;
            state->terrain_selected = true;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("wood")).pressed_left){
            state->terrain_selected_id = TextureAsset_Wood1;
            state->terrain_selected = true;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("lava")).pressed_left){
            state->terrain_selected_id = TextureAsset_Lava1;
            state->terrain_selected = true;
        }
    }

    ui_end_panel();

    ui_set_pos(SCREEN_WIDTH - 200, 10);
    ui_set_size(ui_size_children(0), ui_size_children(0));
    ui_set_border_thickness(10);
    ui_set_background_color(DEFAULT);
    ui_begin_panel(str8_literal("box1##render_batches"), ui_floating_panel);

    ui_size(ui_size_text(0), ui_size_text(0))
    ui_text_color(LIGHT_GRAY)
    {

        String8 mouse_pos = str8_format(ts->frame_arena, "mouse pos: %f, %f", controller.mouse.x, controller.mouse.y);
        ui_label(mouse_pos);

        mouse_pos = str8_format(ts->frame_arena, "mouse pos(world): %f, %f", controller.mouse.world_x, controller.mouse.world_y);
        ui_label(mouse_pos);

        String8 fmt;
        v2 world_mouse = v2_world_from_screen(controller.mouse.pos);
        v2 cell = grid_cell_from_pos(world_mouse, state->world_cell_size);
        fmt = str8_format(ts->frame_arena, "mouse cell: %i, %i", (s32)cell.x, (s32)cell.y);
        ui_label(fmt);

        fmt = str8_format(ts->frame_arena, "mouse cell: %f, %f", world_mouse.x, world_mouse.y);
        ui_label(fmt);

        fmt = str8_format(ts->frame_arena, "cam zoom: %f", camera.size);
        ui_label(fmt);

        fmt = str8_format(ts->frame_arena, "cam pos: (%.2f, %.2f)", camera.x, camera.y);
        ui_label(fmt);
        ui_spacer(10);

        fmt = str8_format(ts->frame_arena, "Render Batches Count: %i", render_batches.count);
        ui_label(fmt);



        //fmt = str8_format(ts->frame_arena, "Render Batches Count: %i", render_batches.count);
        //ui_label(fmt);
        //s32 count = 0;
        //for(RenderBatch* batch = render_batches.first; batch != 0; batch = batch->next){
        //    if(count < 50){
        //        fmt = str8_format(ts->frame_arena, "%i - %i/%i ##%i", batch->id, batch->vertex_count, batch->vertex_cap, batch->id);
        //        ui_label(fmt);
        //    }
        //    count++;
        //}
    }

    ui_end_panel();

    ui_set_pos(200, 20);
    ui_set_size(ui_size_children(0), ui_size_children(0));
    ui_set_border_thickness(5);
    ui_set_background_color(DEFAULT);
    ui_begin_panel(str8_literal("box1##3"), ui_floating_panel);

    ui_size(ui_size_pixel(100, 0), ui_size_pixel(50, 0))
    ui_background_color(DARK_GRAY)
    {
        ui_label(str8_literal("Show Grid"));
        if(ui_button(str8_literal("World")).pressed_left){
            state->show_world_cells = !state->show_world_cells;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("Flocking")).pressed_left){
            state->show_flocking_cells = !state->show_flocking_cells;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("Pathing")).pressed_left){
            state->show_pathing_cells = !state->show_pathing_cells;
        }

        ui_spacer(10);
        ui_label(str8_literal("Other"));
        if(ui_button(str8_literal("Entity Info")).pressed_left){
            state->show_entity_info = !state->show_entity_info;
        }
    }

    ui_end_panel();

    ui_set_pos(200, 20);
    ui_set_size(ui_size_children(0), ui_size_children(0));
    ui_set_border_thickness(5);
    ui_set_background_color(DEFAULT);
    ui_begin_panel(str8_literal("box1##5"), ui_floating_panel);

    ui_size(ui_size_pixel(100, 0), ui_size_pixel(50, 0))
    ui_background_color(DARK_GRAY)
    {
        String8 str_fmt = str8_formatted(ts->frame_arena, "Cell Size (%i)", state->flocking_cell_size);
        ui_label(str_fmt);
        if(ui_button(str8_literal("^")).pressed_left){
            state->flocking_cell_size++;
        }
        ui_spacer(2);
        if(ui_button(str8_literal("V")).pressed_left){
            if(state->flocking_cell_size > 1){
                state->flocking_cell_size--;
            }
        }
    }

    ui_end_panel();
}

static void
ui_structure_castle(void){
    ui_layout_axis(Axis_X)
    {
        ui_set_border_thickness(10);
        ui_set_background_color(DEFAULT);
        ui_set_pos(20, window.height - 100);
        ui_set_size(ui_size_children(0), ui_size_children(0));
        ui_begin_panel(str8_literal("box1##4"), ui_floating_panel);

        ui_size(ui_size_pixel(100, 0), ui_size_pixel(50, 0))
        ui_background_color(DARK_GRAY)
        {
            if(ui_button(str8_literal("skeleton")).pressed_left){
                Entity* castle = state->entities_selected[0];

                v2 dir = direction_v2(castle->pos, castle->rallypoint);
                Entity* e = add_skeleton(TextureAsset_Skeleton1, grid_cell_from_pos(castle->pos, state->world_cell_size), make_v2(1, 1), dir);
                e->origin = castle;
                e->rallypoint = castle->rallypoint;
                e->rallypoint_cell = castle->rallypoint_cell;
                entity_commands_move(e, e->rallypoint, e->rallypoint);
            }

            ui_spacer(10);
            if(ui_button(str8_literal("skeleton - 15")).pressed_left){
                Entity* castle = state->entities_selected[0];
                v2 cell_coords = grid_cell_from_pos(castle->pos, state->world_cell_size);
                v2 a = grid_cell_from_pos(castle->pos, 1);
                v2 b = grid_pos_from_cell(castle->pos, state->world_cell_size);
                v2 c = grid_pos_from_cell(castle->pos, 1);
                f32 projected_distance  = distance_v2(castle->pos, castle->rallypoint);
                v2  projected_direction = direction_v2(castle->pos, castle->rallypoint);
                f32 projected_rad = rad_from_dir(projected_direction);

                f32 y = -1;
                for(s32 i=0; i<3; ++i){
                    f32 x = -2;
                    for(s32 j=0; j<5; ++j){
                        v2 new_coords = make_v2(cell_coords.x + x, cell_coords.y + y);

                        v2 dir = direction_v2(castle->pos, castle->rallypoint);
                        Entity* e = add_skeleton(TextureAsset_Skeleton1, new_coords, make_v2(1, 1), dir);
                        e->origin = castle;

                        v2 target_direction = direction_v2(e->pos, castle->rallypoint);
                        f32 target_rad = rad_from_dir(target_direction);

                        projected_rad = slerp_f32(projected_rad, target_rad, 0.5);
                        v2 projected_offset = dir_from_rad(projected_rad) * projected_distance;
                        v2 target_pos = e->pos + projected_offset;

                        entity_commands_move(e, target_pos, target_pos);
                        x += 1;
                    }
                    y += 1;
                }
            }
            ui_spacer(10);
            if(ui_button(str8_literal("skeleton - 50")).pressed_left){
                Entity* castle = state->entities_selected[0];
                v2 cell_coords = grid_cell_from_pos(castle->pos, state->world_cell_size);
                f32 projected_distance  = distance_v2(castle->pos, castle->rallypoint);
                v2  projected_direction = direction_v2(castle->pos, castle->rallypoint);
                f32 projected_rad = rad_from_dir(projected_direction);
                f32 y = -2;
                for(s32 i=0; i<5; ++i){
                    f32 x = -4;
                    for(s32 j=0; j<10; ++j){
                        v2 new_coords = make_v2(cell_coords.x + x, cell_coords.y + y);

                        v2 dir = direction_v2(castle->pos, castle->rallypoint);
                        Entity* e = add_skeleton(TextureAsset_Skeleton1, new_coords, make_v2(1, 1), dir);
                        e->origin = castle;

                        v2 target_direction = direction_v2(e->pos, castle->rallypoint);
                        f32 target_rad = rad_from_dir(target_direction);

                        projected_rad = slerp_f32(projected_rad, target_rad, 0.5);
                        v2 projected_offset = dir_from_rad(projected_rad) * projected_distance;
                        v2 target_pos = e->pos + projected_offset;

                        entity_commands_move(e, target_pos, target_pos);
                        x += 1;
                    }
                    y += 1;
                }
            }
        }
    }

    ui_end_panel();
}

static void
draw_grid(f32 size, RGBA color){
    v2 low  = make_v2(floor_f32(camera.p3.x/size) * size,
                      floor_f32(camera.p3.y/size) * size);
    v2 high = make_v2(ceil_f32(camera.p1.x/size) * size,
                      ceil_f32(camera.p1.y/size) * size);

    f32 x = low.x;
    while(x < high.x){
        v2 p0 = make_v2(x, low.y);
        v2 p1 = make_v2(x, high.y);

        draw_line(p0, p1, 0.1f, color);
        x += size;
    }

    f32 y = low.y;
    while(y < high.y){
        v2 p0 = make_v2(low.x, y);
        v2 p1 = make_v2(high.x, y);

        draw_line(p0, p1, 0.1f, color);
        y += size;
    }

    // draw coordinates
#if 0
    y = low.y;
    while(y < high.y){

        f32 x = low.x;
        while(x < high.x){

            if(x >= 0 && (x/state->world_cell_size) < state->world_width_in_cells){
                if(y >= 0 && (y/state->world_cell_size) < state->world_height_in_cells){
                    v2 cell = make_v2(x, y);

                    set_font(state->font);
                    String8 coord = str8_formatted(ts->frame_arena, "(%i, %i)", (s32)x/(s32)state->world_cell_size, (s32)y/(s32)state->world_cell_size);
                    draw_text(coord, screen_cell, YELLOW);
                }
            }

            x += state->world_cell_size;
        }

        y += state->world_cell_size;
    }
#endif
}

static void
draw_world_terrain(void){
    v2 low  = make_v2(floor_f32(camera.p3.x/state->world_cell_size) * state->world_cell_size, floor_f32(camera.p3.y/state->world_cell_size) * state->world_cell_size);
    v2 high = make_v2( ceil_f32(camera.p1.x/state->world_cell_size) * state->world_cell_size,  ceil_f32(camera.p1.y/state->world_cell_size) * state->world_cell_size);

    for(s32 i=1; i < TextureAsset_Count; ++i){
        f32 y = low.y;
        while(y < high.y){

            f32 x = low.x;
            while(x < high.x){

                if(x >= 0 && (x/state->world_cell_size) < state->world_width_in_cells){
                    if(y >= 0 && (y/state->world_cell_size) < state->world_height_in_cells){
                        v2 cell = make_v2(x, y);

                        s32 idx = (s32)(((y/state->world_cell_size) * state->world_width_in_cells) + (x/state->world_cell_size));
                        s32 cell_tex = state->world_grid[idx];
                        if(cell_tex == i){
                            set_texture(&r_assets->textures[cell_tex]);
                            Rect tex_rect = make_rect_size(cell, make_v2(state->world_cell_size, state->world_cell_size));
                            draw_texture(tex_rect);
                        }
                    }
                }

                x += state->world_cell_size;
            }

            y += state->world_cell_size;
        }
    }
}

static bool
mouse_in_boundingbox(Entity* e){
    Rect rect = rect_from_entity(e);

    if(rect_contains_point(rect, controller.mouse.world_pos)){
        return(true);
    }

    return(false);
}

static bool
mouse_in_cell(v2 cell){
    v2 mouse_cell = grid_cell_from_pos(controller.mouse.world_pos, state->world_cell_size);
    if(mouse_cell == cell){
        return(true);
    }
    return(false);
}

static v2
grid_pos_from_cell(v2 cell){
    v2 result = {0};
    result.x = cell.x * state->world_cell_size;
    result.y = cell.y * state->world_cell_size;
    return(result);
}

static v2
grid_pos_from_cell(v2 cell, f32 size){
    v2 result = {0};
    result.x = cell.x * size;
    result.y = cell.y * size;
    return(result);
}

global v2 tp;
global v2 wm;

static v2
grid_cell_from_pos(v2 pos){
    v2 result = {0};
    result.x = floor_f32(pos.x / state->world_cell_size);
    result.y = floor_f32(pos.y / state->world_cell_size);
    return(result);
}

static v2
grid_cell_from_pos(v2 pos, f32 size){
    v2 result = {0};
    result.x = floor_f32(pos.x / size);
    result.y = floor_f32(pos.y / size);
    return(result);
}

static bool
grid_cell_coords_in_bounds(v2 coords){
    if(coords.x >= 0 && coords.x < WORLD_WIDTH_IN_TILES_MAX &&
       coords.y >= 0 && coords.y < WORLD_HEIGHT_IN_TILES_MAX){
        return(true);
    }
    return(false);
}

// todo(rr): maybe I don't need this and I can calculate in place
static v2
grid_cell_center(v2 cell){
    v2 result = {0};

    v2 pos = grid_pos_from_cell(cell, state->world_cell_size);
    result.x = pos.x + state->world_cell_size/2;
    result.y = pos.y + state->world_cell_size/2;
    return(result);
}

WINDOWPLACEMENT window_info = { sizeof(WINDOWPLACEMENT) };
static void
os_fullscreen_mode(Window* window){
    window->type = WindowType_Fullscreen;
    s32 style = GetWindowLong(window->handle, GWL_STYLE);

    if(style & WS_OVERLAPPEDWINDOW){ // is windows mode?
        MONITORINFO monitor_info = { sizeof(MONITORINFO) };

        u32 flags = SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
        if(GetWindowPlacement(window->handle, &window_info) &&
           GetMonitorInfo(MonitorFromWindow(window->handle, MONITOR_DEFAULTTOPRIMARY), &monitor_info)){
            SetWindowLong(window->handle, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window->handle, HWND_TOP,
                         monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                         flags);
        }
    }
}

static void
os_windowed_mode(Window* window){
    window->type = WindowType_Windowed;
    s32 style = GetWindowLong(window->handle, GWL_STYLE);

    u32 flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
    SetWindowLong(window->handle, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
    SetWindowPlacement(window->handle, &window_info);
    SetWindowPos(window->handle, 0,
                 0, 0, 0, 0, flags);
}

static void
change_resolution(Window* window, f32 width, f32 height) {
    window->width = width;
    window->height = height;
    window->aspect_ratio = window->width/window->height;

    s32 style = GetWindowLong(window->handle, GWL_STYLE);
    RECT rect = {0, 0, (s32)width, (s32)height};
    AdjustWindowRect(&rect, (DWORD)style, FALSE);

    u32 flags = 0;
    if(window->type == WindowType_Fullscreen){
        flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
    }
    if(window->type == WindowType_Windowed){
        flags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
    }
    SetWindowPos(window->handle, 0,
                 0, 0, rect.right - rect.left, rect.bottom - rect.top, flags);

}

static void
init_paths(Arena* arena){
    build_path = os_application_path(global_arena);
    fonts_path = str8_path_append(global_arena, build_path, str8_literal("fonts"));
    shaders_path = str8_path_append(global_arena, build_path, str8_literal("shaders"));
    saves_path = str8_path_append(global_arena, build_path, str8_literal("saves"));
    sprites_path = str8_path_append(global_arena, build_path, str8_literal("sprites"));
    sounds_path = str8_path_append(global_arena, build_path, str8_literal("sounds"));
}

static void
init_memory(u64 permanent, u64 transient){
    memory.permanent_size = permanent;
    memory.transient_size = transient;
    memory.size = memory.permanent_size + memory.transient_size;

    memory.base = os_alloc(memory.size);
    memory.permanent_base = memory.base;
    memory.transient_base = (u8*)memory.base + memory.permanent_size;
}

static Window
win32_window_create(s32 width, s32 height, const wchar* window_name){
    Window result = {0};
    result.type = WindowType_Windowed;

    WNDCLASSW window_class = {
        .style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC,
        .lpfnWndProc = win_message_handler_callback,
        .hInstance = GetModuleHandle(0),
        .hIcon = LoadIcon(0, IDI_APPLICATION),
        .hCursor = LoadCursor(0, IDC_ARROW),
        .lpszClassName = L"window class",
    };

    if(!RegisterClassW(&window_class)){
        return(result);
    }

    result.width = (f32)width;
    result.height = (f32)height;
    result.aspect_ratio = result.width/result.height;

    // adjust window size to exclude client area
    DWORD style = WS_OVERLAPPEDWINDOW|WS_VISIBLE;
    style = style & ~WS_MAXIMIZEBOX; // disable maximize button
    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, style, FALSE);
    s32 adjusted_w = rect.right - rect.left;
    s32 adjusted_h = rect.bottom - rect.top;

    result.handle = CreateWindowW(L"window class", window_name, style, CW_USEDEFAULT, CW_USEDEFAULT, adjusted_w, adjusted_h, 0, 0, GetModuleHandle(0), 0);
    if(!IsWindow(result.handle) || !result.handle){
        print("Error: Could not create window\n");
        // todo(rr): log error
    }
    assert(IsWindow(result.handle));

    return(result);
}

static void
show_cursor(bool show){
    if(show){
        while(ShowCursor(1) < 0);
    }
    else{
        while(ShowCursor(0) >= 0);
    }
}

static void
serialize_world(String8 world){
    Arena* arena = ts->data_arena;

    for(s32 y=0; y<(s32)state->world_height_in_cells; ++y){
        for(s32 x=0; x<(s32)state->world_width_in_cells; ++x){
            s32 cell = (y * (s32)state->world_width_in_cells) + x;
            s32 cell_tex = state->world_grid[cell];

            arena->at += snprintf((char*)arena->base + arena->at,
                                  arena->size - arena->at,
                                  "%i", cell_tex);
        }
        arena->at += snprintf((char*)arena->base + arena->at,
                              arena->size - arena->at,
                              "\n");
    }

    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, saves_path, world);
    File file = os_file_open(full_path, GENERIC_WRITE, CREATE_ALWAYS);
    os_file_write(file, arena->base, arena->at);

    os_file_close(file);
    end_scratch(scratch);
    arena_free(ts->data_arena);
}

static void
deserialize_world(String8 world){
    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, saves_path, world);
    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    if(!file.size){
        os_file_close(file);
        return;
    }

    String8 data = os_file_read(ts->data_arena, file);
    String8* ptr = &data;

    f32 y = 0;
    s32 count = 0;
    while(ptr->count){
        char c = *(char*)ptr->str;
        str8_advance(ptr, 1);
        if(c != '\n'){
            s32 cell_tex = atoi(&c);
            state->world_grid[count] = cell_tex;
            count++;
        }

        if(c == '\n'){
            ++y;
        }
    }
    state->world_width_in_cells = (f32)count / y;
    state->world_height_in_cells = y;
    state->world_width = state->world_width_in_cells * state->world_cell_size;
    state->world_height = state->world_height_in_cells * state->world_cell_size;

    arena_free(ts->data_arena);
    os_file_close(file);
    end_scratch(scratch);
}

static void
deserialize_state(void){
    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, build_path, str8_literal("config.g"));
    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    if(!file.size){
        os_file_close(file);
        return;
    }

    String8 data = os_file_read(ts->data_arena, file);
    String8* ptr = &data;

    s32 count = 0;
    while(ptr->count){
        String8 line = str8_eat_line(ptr);
        while(line.size){
            String8 word = str8_eat_word(&line);
            if(str8_starts_with(word, str8_literal("current_world"))){
                String8Node* str8_node = {0};
                str8_node = str8_split(scratch.arena, word, ':');
                if(str8_ends_with(str8_node->prev->str, str8_literal(".g"))){
                    memcpy(state->current_world.str, str8_node->prev->str.str, str8_node->prev->str.count);
                    state->current_world.count = str8_node->prev->str.count;
                }
            }
        }
    }

    arena_free(ts->data_arena);
    os_file_close(file);
    end_scratch(scratch);
}

static void
serialize_state(void){
    serialize_world(state->current_world);

    Arena* arena = ts->data_arena;

    arena->at += snprintf((char*)arena->base + arena->at,
                          arena->size - arena->at,
                          "current_world:%s\n", state->current_world.str);

    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, build_path, str8_literal("config.g"));
    File file = os_file_open(full_path, GENERIC_WRITE, CREATE_ALWAYS);
    os_file_write(file, arena->base, arena->at);

    arena_free(ts->data_arena);
    os_file_close(file);
    end_scratch(scratch);
}

static void
partition_entities_in_bins(){
    memset(state->cells, 0, sizeof(state->cells));
    for(s32 i=0; i < array_count(state->entities); ++i){
        Entity* e = state->entities + i;
        if(!has_flag(e->flags, EntityFlag_Active)) continue;

        v2 cell_coords = grid_cell_from_pos(e->pos, state->flocking_cell_size);
        if(!grid_cell_coords_in_bounds(cell_coords)) continue;

        Cell* cell = state->cells + ((s32)cell_coords.x + (WORLD_WIDTH_IN_TILES_MAX * (s32)cell_coords.y));
        if(cell->bin_count == 0){ // Is it empty?
            cell->bin_count = 1;

            BinNode* bin = push_struct_zero(ts->bin_arena, BinNode); // Will change to default zero.
            cell->bin = bin;

            bin->cap = BIN_SIZE;
        }

        if(cell->bin->at == cell->bin->cap){
            BinNode* bin = push_struct_zero(ts->bin_arena, BinNode); // Will change to default zero.
            bin->cap = BIN_SIZE;
            bin->next = cell->bin;

            cell->bin_count++;
            cell->bin = bin;
        }

        BinNode* bin = cell->bin;
        bin->entities[bin->at++] = e;
    }
}

static void
clear_entities_selected(){
    for(s32 i=0; i < state->entities_selected_count; ++i){
        Entity* selected_entity = state->entities_selected[i];
        selected_entity->selected = false;
        state->entities_selected[0] = 0;
    }
    state->entities_selected_count = 0;
}

static LRESULT
win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param){
    LRESULT result = 0;

    switch(message){
        case WM_CLOSE:
        case WM_QUIT:
        case WM_DESTROY:{
            Event event = {0};
            event.type = EventType_QUIT;
            events_add(&events, event);
        } break;

        // note: prevent resizing on edges
        case WM_NCHITTEST:{
            LRESULT hit = DefWindowProcW(hwnd, message, w_param, l_param);
            if (hit == HTLEFT       || hit == HTRIGHT || // edges of window
                hit == HTTOP        || hit == HTBOTTOM ||
                hit == HTTOPLEFT    || hit == HTTOPRIGHT || // corners of window
                hit == HTBOTTOMLEFT || hit == HTBOTTOMRIGHT){
                return HTCLIENT;
            }
            return hit;
        } break;

        // note(rr): is the window the currently active window or not.
        case WM_ACTIVATE:{
            if(w_param == WA_ACTIVE){
                game_in_focus = true;
            }
            if(w_param == WA_INACTIVE){
                game_in_focus = false;
            }
        } break;

        // note(rr): currently not happening because we clip mouse to client region.
        case WM_MOUSELEAVE:{
            Event event = {0};
            event.type = EventType_NO_CLIENT;

            events_add(&events, event);

            tracking_mouse = false;
        } break;

        case WM_MOUSEMOVE:{
            // post mouse event WM_MOUSELEAVE message when mouse leaves client area.
            {
                if(!tracking_mouse){
                    TRACKMOUSEEVENT tme;
                    tme.cbSize = sizeof(TRACKMOUSEEVENT);
                    tme.dwFlags = TME_LEAVE;
                    tme.hwndTrack = hwnd; // The window to track
                    TrackMouseEvent(&tme);

                    tracking_mouse = true;
                }
            }

            // clip mouse to client region
            //RECT client_rect;
            //GetClientRect(hwnd, &client_rect);
            //{
            //    if(game_in_focus){
            //        POINT top_left     = { client_rect.left, client_rect.top };
            //        POINT bottom_right = { client_rect.right, client_rect.bottom };
            //        ClientToScreen(hwnd, &top_left);
            //        ClientToScreen(hwnd, &bottom_right);

            //        RECT screen_rect = { top_left.x, top_left.y, bottom_right.x, bottom_right.y };
            //        ClipCursor(&screen_rect);
            //    }
            //}

            Event event = {0};
            event.type = EventType_MOUSE;
            event.mouse_x = (f32)((s16)(l_param & 0xFFFF));
            event.mouse_y = (f32)((s16)(l_param >> 16));
            m4 mat = m4_world_from_screen();
            v2 pos = m4_translate_v2(mat, make_v2(event.mouse_x, event.mouse_y));
            event.world_mouse_x = pos.x;
            event.world_mouse_y = pos.y;

            // calc dx/dy and normalize from -1:1
            f32 dx = event.mouse_x - controller.mouse.x;
            f32 dy = event.mouse_y - controller.mouse.y;
            v2 delta_normalized = normalize_v2(make_v2(dx, dy));
            event.mouse_dx = delta_normalized.x;
            event.mouse_dy = delta_normalized.y;

            // check if mouse is at edge of client region
            //if(game_in_focus){
            //    if((s32)event.mouse_x <= client_rect.left){
            //        event.mouse_edge_left = true;
            //    }
            //    if((s32)event.mouse_x >= client_rect.right - 1){
            //        event.mouse_edge_right = true;
            //    }
            //    if((s32)event.mouse_y <= client_rect.top){
            //        event.mouse_edge_top = true;
            //    }
            //    if((s32)event.mouse_y >= client_rect.bottom - 1){
            //        event.mouse_edge_bottom = true;
            //    }
            //}

            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;
            event.alt_pressed   = alt_pressed;

            events_add(&events, event);

            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
            if(w_param == VK_MENU)    { alt_pressed   = true; }
        } break;

        case WM_MOUSEWHEEL:{
            Event event = {0};
            event.type = EventType_KEYBOARD;
            event.mouse_wheel_dir = GET_WHEEL_DELTA_WPARAM(w_param) > 0? 1 : -1;
            if(event.mouse_wheel_dir > 0){
                event.keycode = MOUSE_WHEEL_UP;
            }
            else{
                event.keycode = MOUSE_WHEEL_DOWN;
            }

            event.key_pressed = true;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;
            event.alt_pressed   = alt_pressed;

            events_add(&events, event);

            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
            if(w_param == VK_MENU)    { alt_pressed   = true; }
        } break;

        // TODO: IMPORTANT: These events likely pass in mouse positions, need to store them as part of the event
        // TODO: IMPORTANT: These events likely pass in mouse positions, need to store them as part of the event
        // TODO: IMPORTANT: These events likely pass in mouse positions, need to store them as part of the event
        // TODO: IMPORTANT: These events likely pass in mouse positions, need to store them as part of the event
        // TODO: IMPORTANT: Make sure the alt/shit/ctrl stuff is correct and the UP/DOWN ordering is correct
        // note(rr): mouse buttons are keyboard because it makes it easier to set pressed/held with everything else
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:{
            //if(message == WM_LBUTTONUP){
                //u32 a = 1;
            //}
            Event event = {0};
            event.type = EventType_KEYBOARD;
            event.keycode = MOUSE_BUTTON_LEFT;
            event.repeat = ((s32)l_param) & 0x40000000;

            event.key_pressed  = message == WM_LBUTTONDOWN ? true : false;
            event.key_released = message == WM_LBUTTONUP   ? true : false;

            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;
            event.alt_pressed   = alt_pressed;

            events_add(&events, event);

            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
            if(w_param == VK_MENU)    { alt_pressed   = true; }
        } break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:{
            Event event = {0};
            event.type = EventType_KEYBOARD;
            event.keycode = MOUSE_BUTTON_RIGHT;
            event.repeat = ((s32)l_param) & 0x40000000;

            event.key_pressed  = message == WM_RBUTTONDOWN ? true : false;
            event.key_released = message == WM_RBUTTONUP   ? true : false;

            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;
            event.alt_pressed   = alt_pressed;

            events_add(&events, event);

            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
            if(w_param == VK_MENU)    { alt_pressed   = true; }
        } break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:{
            Event event = {0};
            event.type = EventType_KEYBOARD;
            event.keycode = MOUSE_BUTTON_MIDDLE;
            event.repeat = ((s32)l_param) & 0x40000000;

            event.key_pressed  = message == WM_MBUTTONDOWN ? true : false;
            event.key_released = message == WM_MBUTTONUP   ? true : false;

            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;
            event.alt_pressed   = alt_pressed;

            events_add(&events, event);

            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
            if(w_param == VK_MENU)    { alt_pressed   = true; }
        } break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:{
            Event event = {0};
            event.type = EventType_KEYBOARD;
            event.keycode = w_param;
            event.repeat = ((s32)l_param) & 0x40000000;

            event.key_pressed = true;
            event.key_released = false;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;
            event.alt_pressed   = alt_pressed;

            events_add(&events, event);

            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
            if(w_param == VK_MENU)    { alt_pressed   = true; }
        } break;
        case WM_SYSKEYUP:
        case WM_KEYUP:{
            Event event = {0};
            event.type = EventType_KEYBOARD;
            event.keycode = w_param;

            event.key_pressed = false;
            event.key_released = true;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;
            event.alt_pressed   = alt_pressed;

            events_add(&events, event);

            if(w_param == VK_SHIFT)   { shift_pressed = false; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = false; }
            if(w_param == VK_MENU)    { alt_pressed   = false; }
        } break;

        case WM_CHAR:{
            u64 keycode = w_param;

            if(keycode > 31){
                Event event = {0};
                event.type = EventType_TEXT_INPUT;
                event.keycode = keycode;
                event.repeat = ((s32)l_param) & 0x40000000;

                event.shift_pressed = shift_pressed;

                events_add(&events, event);

                if(w_param == VK_SHIFT)   { shift_pressed = true; }
            }

        } break;

        default:{
            result = DefWindowProcW(hwnd, message, w_param, l_param);
        } break;
    }
    return(result);
}

s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type){
    begin_profiler();

    window = win32_window_create(SCREEN_WIDTH, SCREEN_HEIGHT, L"Guild");
    if(!window.handle){
        print("Error: Could not create window\n");
        return(0);
    }

    init_d3d(window.handle, (u32)window.width, (u32)window.height);
#if DEBUG
    d3d_init_debug_stuff();
#endif

    random_seed(0, 1);

    init_paths(global_arena);
    init_memory(MB(500), GB(4));
    init_clock(&clock);
    wasapi_init(2, 48000, 32);
    init_events(&events);

    // note: sim measurements
	u32 simulations = 0;
    f64 time_elapsed = 0;
    f64 accumulator = 0.0;

    clock.dt =  1.0/240.0;
    u64 last_ticks = clock.get_os_timer();

    // note: fps measurement
    f64 FPS = 0;
    f64 MSPF = 0;
    u64 frame_inc = 0;
    u64 frame_tick_start = clock.get_os_timer();

    assert(sizeof(PermanentMemory) < memory.permanent_size);
    assert(sizeof(TransientMemory) < memory.transient_size);
    state = (PermanentMemory*)memory.permanent_base;
    ts    = (TransientMemory*)memory.transient_base;

    if(!memory.initialized){
        // consider: maybe move this memory stuff to init_memory()
        init_arena(&state->arena, (u8*)memory.permanent_base + sizeof(PermanentMemory), memory.permanent_size - sizeof(PermanentMemory));
        init_arena(&ts->arena, (u8*)memory.transient_base + sizeof(TransientMemory), memory.transient_size - sizeof(TransientMemory));

        ts->frame_arena    = push_arena(&ts->arena, MB(100));
        ts->asset_arena    = push_arena(&ts->arena, MB(100));
        //ts->ui_arena       = push_arena(&ts->arena, MB(100));
        ts->ui_state_arena = push_arena(&ts->arena, MB(100));
        ts->batch_arena    = push_arena(&ts->arena, GB(1));
        ts->data_arena     = push_arena(&ts->arena, KB(1024));
        ts->bin_arena      = push_arena(&ts->arena, MB(512));

        show_cursor(true);
        assets_load(ts->asset_arena, &assets);

        state->world_width_in_cells = 10;
        state->world_height_in_cells = 10;
        state->world_cell_size = 1;
        state->flocking_cell_size = 1;
        state->pathing_cell_size = 25;
        state->show_world_cells = true;
        state->show_flocking_cells = true;
        state->show_pathing_cells = true;

        ui_init(ts->ui_state_arena, &window, &controller, &assets);
        init_draw(ts->batch_arena, &assets);

        state->font = &assets.fonts[FontAsset_Arial];

        // setup free entities array in reverse order
        entities_clear();

        // load default level
        state->current_world.str = push_array(global_arena, u8, 1024);
        deserialize_state();
        deserialize_world(state->current_world);

        // load castle
        //state->castle_cell = make_v2(10, 5);
        state->castle_cell = make_v2(0, 0);
        state->castle = add_castle(TextureAsset_Castle1, state->castle_cell, make_v2(2, 2));
        //add_skeleton(TextureAsset_Skeleton1, make_v2(51, 51), make_v2(1, 1), make_v2(1, 0));
        //add_skeleton(TextureAsset_Skeleton1, make_v2(51, 51), make_v2(1, 1), make_v2(1, 0));
        //add_skeleton(TextureAsset_Skeleton1, make_v2(51, 51), make_v2(1, 1), make_v2(1, 0));
        //add_skeleton(TextureAsset_Skeleton1, make_v2(51, 51), make_v2(1, 1), make_v2(1, 0));
        //add_skeleton(TextureAsset_Skeleton1, make_v2(51, 51), make_v2(1, 1), make_v2(1, 0));
        //add_skeleton(TextureAsset_Skeleton1, make_v2(51, 51), make_v2(1, 1), make_v2(1, 0));
        //add_skeleton(TextureAsset_Skeleton1, make_v2(51, 51), make_v2(1, 1), make_v2(1, 0));
        //add_skeleton(TextureAsset_Skeleton1, make_v2(51, 51), make_v2(1, 1), make_v2(1, 0));
        //add_skeleton(TextureAsset_Skeleton1, make_v2(51, 51), make_v2(1, 1), make_v2(1, 0));
        //add_skeleton(TextureAsset_Skeleton1, make_v2(51, 51), make_v2(1, 1), make_v2(1, 0));
        //add_skeleton(TextureAsset_Skeleton1, make_v2(51, 51), make_v2(1, 1), make_v2(1, 0));
        //add_skeleton(TextureAsset_Skeleton1, make_v2(51, 51), make_v2(1, 1), make_v2(1, 0));

        //state->scene_state = SceneState_Game;
        state->scene_state = SceneState_Editor;
        //init_camera_2d(&camera, make_v2((state->world_width_in_cells/2) * state->world_cell_size, (state->world_height_in_cells/2) * state->world_cell_size), 30);
        init_camera_2d(&camera, make_v2(10, 5), 15);
        init_console(global_arena, &camera, &window, &assets);

        memory.initialized = true;
    }

    should_quit = false;
    while(!should_quit){
        begin_timed_scope("while(!should_quit)");

        ui_begin();

        u64 now_ticks = clock.get_os_timer();
        f64 frame_time = clock.get_seconds_elapsed(now_ticks, last_ticks);
        MSPF = 1000/1000/((f64)clock.frequency / (f64)(now_ticks - last_ticks));
        last_ticks = now_ticks;

        MSG message;
        while(PeekMessageW(&message, window.handle, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        // handle events
        bool handled;
        while(!events_empty(&events)){
            Event event = events_next(&events);

            // clear held buttons if mouse leaves client area
            if(event.type == EventType_NO_CLIENT){
                clear_controller_held();
            }

            handled = handle_global_events(event);

            if(console_is_open()){
                handled = handle_console_events(event);
                continue;
            }
            handled = handle_camera_events(event);
            handled = handle_controller_events(event);
            handled = handle_game_events(event);
        }

        partition_entities_in_bins();

        //print("53 | %i, %i, %i, %i, %i, %i\n", state->cells[48 + (1000 * 53)].bin_count, state->cells[49 + (1000 * 53)].bin_count, state->cells[50 + (1000 * 53)].bin_count, state->cells[51 + (1000 * 53)].bin_count, state->cells[52 + (1000 * 53)].bin_count, state->cells[53 + (1000 * 53)].bin_count);
        //print("52 | %i, %i, %i, %i, %i, %i\n", state->cells[48 + (1000 * 52)].bin_count, state->cells[49 + (1000 * 52)].bin_count, state->cells[50 + (1000 * 52)].bin_count, state->cells[51 + (1000 * 52)].bin_count, state->cells[52 + (1000 * 52)].bin_count, state->cells[53 + (!v!5<Mouse>C!w!5
        //print("51 | %i, %i, %i, %i, %i, %i\n", state->cells[48 + (1000 * 51)].bin_count, state->cells[49 + (1000 * 51)].bin_count, state->cells[50 + (1000 * 51)].bin_count, state->cells[51 + (1000 * 51)].bin_count, state->cells[52 + (1000 * 51)].bin_count, state->cells[53 + (1000 * 51)].bin_count);
        //print("50 | %i, %i, %i, %i, %i, %i\n", state->cells[48 + (1000 * 50)].bin_count, state->cells[49 + (1000 * 50)].bin_count, state->cells[50 + (1000 * 50)].bin_count, state->cells[51 + (1000 * 50)].bin_count, state->cells[52 + (1000 * 50)].bin_count, state->cells[53 + (1000 * 50)].bin_count);
        //print("49 | %i, %i, %i, %i, %i, %i\n", state->cells[48 + (1000 * 49)].bin_count, state->cells[49 + (1000 * 49)].bin_count, state->cells[50 + (1000 * 49)].bin_count, state->cells[51 + (1000 * 49)].bin_count, state->cells[52 + (1000 * 49)].bin_count, state->cells[53 + (1000 * 49)].bin_count);
        //print("48 | %i, %i, %i, %i, %i, %i\n", state->cells[48 + (1000 * 48)].bin_count, state->cells[49 + (1000 * 48)].bin_count, state->cells[50 + (1000 * 48)].bin_count, state->cells[51 + (1000 * 48)].bin_count, state->cells[52 + (1000 * 48)].bin_count, state->cells[53 + (1000 * 48)].bin_count);
        //print("     --------------\n");
        //print("     48 49 50 51 52 53\n");
        //print("-------------------------------------------------------\n");

        // note: consumes input so needs to be here
        if(state->scene_state == SceneState_Editor){
            ui_editor();
        }

        simulations = 0;
        accumulator += frame_time;
        while(accumulator >= clock.dt){
            begin_timed_scope("sim loop");
            sim_game();

            accumulator -= clock.dt;
            time_elapsed += clock.dt;
            simulations++;

        }

        if(controller_button_pressed(KeyCode_8)){
            if(state->scene_state == SceneState_Editor){
                state->scene_state = SceneState_Game;
                state->terrain_selected = false;
                state->terrain_selected_id = 0;
            }
            else if(state->scene_state == SceneState_Game){
                state->scene_state = SceneState_Editor;
            }
        }

        // camera drag
        if(controller.ctrl_pressed && controller_button_pressed(MOUSE_BUTTON_RIGHT, false)){
            world_camera_record = camera;
            world_mouse_record = controller.mouse.world_pos;
            state->dragging_world = true;
        }
        if(controller.ctrl_pressed && controller_button_held(MOUSE_BUTTON_RIGHT)){
            v2 world_mouse_current = v2_world_from_screen(controller.mouse.pos, &world_camera_record);
            v2 world_rel_pos = world_mouse_record - world_mouse_current;
            state->selecting = false;
            camera.x = world_camera_record.x + world_rel_pos.x;
            camera.y = world_camera_record.y + world_rel_pos.y;

        }
        if(state->dragging_world && controller_button_released(MOUSE_BUTTON_RIGHT)){
            world_camera_record = {0};
            world_mouse_record = {0};
            state->dragging_world = false;
        }

        // Entity Selection.
        if(!state->dragging_world){
            if(controller_button_pressed(MOUSE_BUTTON_LEFT, false)){
                state->selection_mouse_record = controller.mouse.world_pos;
                state->selecting = true;
            }
            if(state->selecting && controller_button_held(MOUSE_BUTTON_LEFT)){
                state->selection_rect = make_rect(state->selection_mouse_record, controller.mouse.world_pos);
            }
            if(controller_button_released(MOUSE_BUTTON_LEFT)){
                state->selection_mouse_record = {0};
                state->selecting = false;

                s32 count = 0;
                bool selected_new_units = false;
                for(s32 i=0; i < array_count(state->entities); ++i){
                    Entity* e = state->entities + i;
                    if(e->type == EntityType_Structure) continue;

                    if(rect_contains_point(state->selection_rect, e->pos)){
                        selected_new_units = true;
                        if(controller.ctrl_pressed){
                            state->entities_selected[state->entities_selected_count++] = e;
                        }
                        else{
                            state->entities_selected[count] = e;
                        }
                        e->selected = true;
                        count++;
                    }
                }

                if(!controller.ctrl_pressed && selected_new_units == true){
                    state->entities_selected_count = count;
                }

                state->selection_rect = {0};
            }

            // mouse hover
            bool found = false;
            for(s32 idx = 0; idx < array_count(state->entities); ++idx){
                Entity *e = state->entities + idx;
                if(mouse_in_boundingbox(e)){
                    state->entity_hovered = e;
                    found = true;
                }
            }
            if(!found){
                state->entity_hovered = 0;
            }

            // single select
            if(state->entity_hovered && !controller.ctrl_pressed && controller_button_pressed(MOUSE_BUTTON_LEFT)){
                //memset(state->entities_selected, 0, sizeof(state->entities_selected));
                clear_entities_selected();
                state->entities_selected[0] = state->entity_hovered;
                state->entities_selected[0]->selected = true;
                state->entities_selected_count = 1;
            }
            if(state->entity_hovered && controller.ctrl_pressed && controller_button_pressed(MOUSE_BUTTON_LEFT)){
                state->entities_selected[state->entities_selected_count] = state->entity_hovered;
                state->entities_selected[state->entities_selected_count]->selected = true;
                state->entities_selected_count++;
            }

        }

        // Calc center position of selection.
        v2 average_position = make_v2(0, 0);
        for(s32 i=0; i < state->entities_selected_count; ++i){
            Entity* e = state->entities_selected[i];
            average_position.x += e->pos.x;
            average_position.y += e->pos.y;
        }
        state->entities_selected_center.x = average_position.x/(f32)state->entities_selected_count;
        state->entities_selected_center.y = average_position.y/(f32)state->entities_selected_count;

        v2 world_mouse = v2_world_from_screen(controller.mouse.pos);
        for(s32 i=0; i < state->entities_selected_count; ++i){
            Entity* e = state->entities_selected[i];

            switch(e->structure_type){
                case StructureType_Castle:{
                    if(!controller.ctrl_pressed && controller_button_pressed(MOUSE_BUTTON_RIGHT)){
                        e->rallypoint = world_mouse;
                        e->rallypoint_cell = grid_cell_from_pos(world_mouse, state->world_cell_size);
                    }
                    ui_structure_castle();
                }
            }

            switch(e->type){
                case EntityType_Skeleton1:{
                    if(!state->dragging_world && controller_button_released(MOUSE_BUTTON_RIGHT, false)){
                        f32 projected_distance  = distance_v2(state->entities_selected_center, world_mouse);
                        v2  projected_direction = direction_v2(state->entities_selected_center, world_mouse);
                        f32 projected_rad = rad_from_dir(projected_direction);

                        v2 target_direction = direction_v2(e->pos, world_mouse);
                        f32 target_rad = rad_from_dir(target_direction);

                        projected_rad = slerp_f32(projected_rad, target_rad, 0.5);
                        v2 projected_offset = dir_from_rad(projected_rad) * projected_distance;
                        v2 target_pos = e->pos + projected_offset;

                        if(!controller.shift_pressed){
                            entity_commands_clear(e);
                        }
                        tp = e->pos;
                        wm = world_mouse;
                        entity_commands_move(e, target_pos, world_mouse);
                    }
                }
            }
        }

        // CLEAR SELECTION
        if(!controller.ctrl_pressed && controller_button_pressed(MOUSE_BUTTON_LEFT)){
            clear_entities_selected();
        }

        console_update();

        // zoom
        if(camera.size > 30){
            camera.size -= (f32)controller.mouse.wheel_dir * 10;
        }
        if(camera.size <= 30){
            camera.size -= (f32)controller.mouse.wheel_dir;
            if(camera.size < 3){
                camera.size = 3;
            }
        }

        camera_2d_update(&camera, window.aspect_ratio);
        wasapi_play_cursors();

        // rendering
        {
            begin_timed_scope("rendering");
            //----constant buffer----
            D3D11_MAPPED_SUBRESOURCE mapped_subresource;
            d3d_context->Map(d3d_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
            ConstantBuffer2D* constants = (ConstantBuffer2D*)mapped_subresource.pData;
            constants->screen_res = make_v2s32((s32)window.width, (s32)window.height);
            d3d_context->Unmap(d3d_constant_buffer, 0);

            render_batches_reset();
            //arena_free(ts->batch_arena);
            set_transform(m4_screen_from_world());
            draw_world_terrain();
            if(state->scene_state == SceneState_Editor){
                if(state->show_world_cells){
                    draw_grid(state->world_cell_size, RED);
                }
                if(state->show_flocking_cells){
                    draw_grid(state->flocking_cell_size, BLUE);
                }
                if(state->show_pathing_cells){
                    draw_grid(state->pathing_cell_size, GREEN);
                }
            }
            draw_entities(state);

            // draw selection rects
            f32 max_x = 0;
            f32 max_y = 0;
            f32 min_x = 1000;
            f32 min_y = 1000;
            if(state->entities_selected_count){

                if(state->entities_selected_count == 1){
                    Entity* e = state->entities_selected[0];
                    Rect rect = rect_from_entity(e);
                    draw_bounding_box(rect, 0.1f, RED);
                }
                else{
                    for(s32 i=0; i < state->entities_selected_count; ++i){
                        Entity* e = state->entities_selected[i];
                        if(e->selected){
                            if(e->pos.x > max_x){
                                max_x = e->pos.x;
                            }
                            if(e->pos.y > max_y){
                                max_y = e->pos.y;
                            }
                            if(e->pos.x < min_x){
                                min_x = e->pos.x;
                            }
                            if(e->pos.y < min_y){
                                min_y = e->pos.y;
                            }
                        }
                    }
                    min_x -= 0.5f;
                    min_y -= 0.5f;
                    max_x += 0.5f;
                    max_y += 0.5f;
                    Rect rect = make_rect(make_v2(min_x, min_y), make_v2(max_x, max_y));
                    draw_bounding_box(rect, 0.1f, RED);
                }
            }

            // no
            //if(state->entities_selected_count){
            //    for(s32 i=0; i < state->entities_selected_count; ++i){
            //        u32 read_idx = e->commands_read;
            //        while(read_idx != e->commands_write){
            //            EntityCommand* c = entity_commands_read(e, read_idx);
            //            read_idx++;

            //            draw_quad(c->move_to, make_v2(10, 10), RED);
            //            //draw_line(e->pos, screen_space, 2, ORANGE);
            //        }
            //    }
            //}

            // no
            //debug_draw_mouse_cell_pos();

            draw_line(tp, wm, 0.1f, RED);

            if(state->selecting && !state->dragging_world){
                draw_bounding_box(state->selection_rect, 0.1f, RED);
            }

            set_transform(make_m4_ident());
            // draw selected texture
            if(state->terrain_selected){
                set_texture(&r_assets->textures[state->terrain_selected_id]);
                draw_texture(controller.mouse.pos, make_v2(50, 50));
                draw_bounding_box(make_rect_size(controller.mouse.pos, make_v2(50, 50)), 0.1f, RED);
            }


            ui_end();
            set_transform(m4_screen_from_world());
            set_font(state->font);
            String8 fps = str8_formatted(ts->frame_arena, "fps: %.0f", FPS);
            //draw_text(fps, make_v2(text_padding, 20), GREEN);

            set_font(state->font);
            String8 str_fmt = str8_formatted(ts->frame_arena, "entities_count: %i\n", state->entities_count);

            console_draw();
            // draw border
            //draw_line(camera.p0, camera.p1, 5, RED);
            //draw_line(camera.p1, camera.p2, 5, RED);
            //draw_line(camera.p2, camera.p3, 5, RED);
            //draw_line(camera.p3, camera.p0, 5, RED);

            {
                d3d_clear_color(BACKGROUND_COLOR);
                draw_render_batches();
                d3d_present();

                String8 title = str8_fmt(ts->frame_arena, "Entity Count: %i - FPS: %.2f", state->entities_count - 1, FPS);
                //SetWindowText(window.handle, (char*)title.str);

                arena_free(ts->frame_arena);
            }
        }
        arena_free(ts->bin_arena);
        clear_controller_pressed();

        frame_inc++;
        f64 second_elapsed = clock.get_seconds_elapsed(clock.get_os_timer(), frame_tick_start);
        if(second_elapsed > 1){
            FPS = ((f64)frame_inc / second_elapsed);
            frame_tick_start = clock.get_os_timer();
            frame_inc = 0;
        }
        ++frame_count;

        // todo(rr): why is this here?
        //end_profiler();
    }

    serialize_state();
    d3d_release();
    end_profiler();
    wasapi_release();

    return(0);
}

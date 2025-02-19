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
    {
        if(controller_button_held(MOUSE_BUTTON_LEFT) &&
           controller_button_pressed(MOUSE_BUTTON_LEFT, true)){
            state->draw_terrain = true;
        }
        if(!controller_button_held(MOUSE_BUTTON_LEFT)){
            state->draw_terrain = false;
        }

        if(state->draw_terrain){
            if(state->terrain_selected){
                v2 pos = v2_world_from_screen(controller.mouse.pos);
                v2 cell = make_v2(floor_f32(pos.x/state->tile_size), floor_f32(pos.y/state->tile_size));
                if((cell.x < state->world_width_in_tiles && cell.x >= 0)){
                    s32 idx = (s32)((cell.y * state->world_width_in_tiles) + cell.x);
                    state->world_grid[idx] = state->terrain_selected_id;
                }
            }
        }
    }

    // resolve motion
    for(s32 i = 0; i < array_count(state->entities); ++i){
        Entity *e = state->entities + i;
        if(!has_flag(e->flags, EntityFlag_Active)){
            continue;
        }

        if(has_flag(e->flags, EntityFlag_MoveWithPhys)){
            //if(!v2_close_enough(e->pos, e->waypoint, 0.1f)){
            //    e->pos.x += (e->dir.x * e->velocity * e->speed) * (f32)clock.dt;
            //    e->pos.y += (e->dir.y * e->velocity * e->speed) * (f32)clock.dt;
            //}
            e->pos.x += (e->dir.x * e->velocity * e->speed) * (f32)clock.dt;
            e->pos.y += (e->dir.y * e->velocity * e->speed) * (f32)clock.dt;

            v2 p0 = v2_screen_from_world(camera.p0);
            v2 p1 = v2_screen_from_world(camera.p1);
            v2 p2 = v2_screen_from_world(camera.p2);
            v2 p3 = v2_screen_from_world(camera.p3);

            v2 e_pos = v2_screen_from_world(e->pos);

            if(e_pos.x < 0){
                e->dir.x = -e->dir.x;
            }
            if(e_pos.x > window.width){
                e->dir.x = -e->dir.x;
            }
            if(e_pos.y < 0){
                e->dir.y = -e->dir.y;
            }
            if(e_pos.y > window.height){
                e->dir.y = -e->dir.y;
            }
        }
    }

    // resolve death todo(rr): this will be different
    for(s32 i = 0; i < array_count(state->entities); ++i){
        Entity *e = state->entities + i;
        if(!has_flag(e->flags, EntityFlag_Active)){
            continue;
        }
    }

    // type loop
    for(s32 i = 0; i < array_count(state->entities); ++i){
        Entity *e = state->entities + i;
        Rect e_rect = rect_from_entity(e);

        //switch(e->type){
        //}
    }
}

static bool
v2_close_enough(v2 p1, v2 p2, f32 epsilon){
    f32 x = abs_f32(p1.x - p2.x);
    f32 y = abs_f32(p1.y - p2.y);
    bool tx = x <= epsilon;
    bool ty = y <= epsilon;
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
        e->waypoint_cell = make_v2(e->cell.x, e->cell.y - 1);
        e->waypoint = grid_cell_center(e->waypoint_cell);
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
add_skeleton(TextureAsset texture, v2 pos, v2 dim, v2 dir, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Skeleton1);
    if(e){
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->texture = texture;
        e->velocity = 1;
        e->speed = 5.0f;
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
    // todo(rr): later change to screen space in shader with matrix multiplication (identify matrix)
    for(s32 idx = 0; idx < array_count(state->entities); ++idx){
        Entity *e = state->entities + idx;

        Quad quad = quad_from_entity(e);
        if(has_flag(e->flags, EntityFlag_Active)){

            switch(e->type){
                case EntityType_Quad:{
                    quad = rotate_quad(quad, e->deg, e->pos);
                    quad = quad_screen_from_world(quad);

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
                    quad = quad_screen_from_world(quad);
                    switch(e->structure_type){
                        case StructureType_Castle:{
                            set_texture(&r_assets->textures[e->texture]);
                            draw_texture(quad, e->color);

                            v2 p1 = v2_screen_from_world(e->pos);
                            v2 p2 = v2_screen_from_world(e->waypoint);
                            draw_line(p1, p2, 2, RED);

                            set_font(state->font);
                            String8 fmt_str = str8_format(ts->frame_arena, "(%f, %f)", e->waypoint.x, e->waypoint.y);
                            draw_text(fmt_str, v2_screen_from_world(e->waypoint), GREEN);
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
                    quad = quad_screen_from_world(quad);

                    set_texture(&r_assets->textures[e->texture]);
                    draw_texture(quad, e->color);
                } break;
                case EntityType_Skeleton1:{
                    quad = rotate_quad(quad, e->deg, e->pos);
                    quad = quad_screen_from_world(quad);

                    set_texture(&r_assets->textures[e->texture]);
                    draw_texture(quad, e->color);

                    if(state->scene_state == SceneState_Editor){
                        v2 pos = v2_screen_from_world(e->pos);
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
                            fmt_str = str8_formatted(ts->frame_arena, "pos: %f, %f##pos%i", e->pos.x, e->pos.y);
                            ui_label(fmt_str);
                            v2 spos = v2_screen_from_world(e->pos);
                            fmt_str = str8_formatted(ts->frame_arena, "screen_pos: %f, %f##spos%i", spos.x, spos.y);
                            ui_label(fmt_str);
                            fmt_str = str8_formatted(ts->frame_arena, "dir: %f, %f##dir%i", e->dir.x, e->dir.y);
                            ui_label(fmt_str);
                            fmt_str = str8_formatted(ts->frame_arena, "velocity: %f##velocity%i", e->velocity, e->velocity);
                            ui_label(fmt_str);
                            fmt_str = str8_formatted(ts->frame_arena, "speed: %f##speed%i", e->speed, e->speed);
                            ui_label(fmt_str);
                        }
                        ui_end_panel();
                    }
                } break;
            }
        }
    }
}

static void
debug_draw_mouse_cell_pos(void){
    set_font(state->font);
    v2 pos = v2_world_from_screen(controller.mouse.pos);
    v2 cell = make_v2(floor_f32(pos.x/state->tile_size), floor_f32(pos.y/state->tile_size));
    String8 cell_str = str8_format(ts->frame_arena, "(%i, %i)", (s32)cell.x, (s32)cell.y);
    draw_text(cell_str, controller.mouse.pos, RED);
}

static void
debug_ui_render_batches(void){

    ui_set_pos(SCREEN_WIDTH - 200, 10);
    ui_set_size(ui_size_children(0), ui_size_children(0));
    ui_set_border_thickness(10);
    ui_set_background_color(DEFAULT);
    ui_begin_panel(str8_literal("box1##1"), ui_floating_panel);

    ui_size(ui_size_text(0), ui_size_text(0))
    ui_text_color(LIGHT_GRAY)
    {

        String8 mouse_pos = str8_format(ts->frame_arena, "mouse pos: %f, %f", controller.mouse.x, controller.mouse.y);
        ui_label(mouse_pos);

        v2 world_mouse = v2_world_from_screen(controller.mouse.pos);
        v2 cell = grid_cell_from_pos(world_mouse);
        String8 mouse_cell = str8_format(ts->frame_arena, "mouse cell: %i, %i", (s32)cell.x, (s32)cell.y);
        ui_label(mouse_cell);

        String8 mouse_world_pos = str8_format(ts->frame_arena, "mouse cell: %f, %f", world_mouse.x, world_mouse.y);
        ui_label(mouse_world_pos);

        String8 zoom = str8_format(ts->frame_arena, "cam zoom: %f", camera.size);
        ui_label(zoom);
        String8 pos = str8_format(ts->frame_arena, "cam pos: (%.2f, %.2f)", camera.x, camera.y);
        ui_label(pos);
        ui_spacer(10);

        String8 title = str8_format(ts->frame_arena, "Render Batches Count: %i", render_batches.count);
        ui_label(title);

        s32 count = 0;
        for(RenderBatch* batch = render_batches.first; batch != 0; batch = batch->next){
            if(count < 50){
                String8 batch_str = str8_format(ts->frame_arena, "%i - %i/%i ##%i", batch->id, batch->vertex_count, batch->vertex_cap, batch->id);
                ui_label(batch_str);
            }
            count++;
        }
    }

    ui_end_panel();
}

static void
ui_level_editor(void){

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
}

static void
ui_structure_castle(void){
    ui_layout_axis(Axis_X)
    {
        ui_set_border_thickness(10);
        ui_set_background_color(DEFAULT);
        ui_set_pos(20, window.height - 100);
        ui_set_size(ui_size_children(0), ui_size_children(0));
        ui_begin_panel(str8_literal("box1##3"), ui_fixed_panel);

        ui_size(ui_size_pixel(100, 0), ui_size_pixel(50, 0))
        ui_background_color(DARK_GRAY)
        {
            if(ui_button(str8_literal("skeleton1")).pressed_left){
                for(s32 i=0; i < 50; ++i){
                    //v2 dir = direction_v2(state->entity_selected->pos, state->entity_selected->waypoint);
                    //Entity* e = add_skeleton(TextureAsset_Skeleton1, state->entity_selected->pos, make_v2(3, 3), dir);
                    //e->origin = state->entity_selected;
                    //e->waypoint = state->entity_selected->waypoint;
                    //e->waypoint_cell = state->entity_selected->waypoint_cell;

                    f32 x = random_range_f32(2.0f) - 1;
                    f32 y = random_range_f32(2.0f) - 1;

                    v2 dir = make_v2(x, y);
                    Entity* e = add_skeleton(TextureAsset_Skeleton1, state->entity_selected->pos, make_v2(3, 3), dir);
                    e->origin = state->entity_selected;
                    e->waypoint = state->entity_selected->waypoint;
                    e->waypoint_cell = state->entity_selected->waypoint_cell;
                }
            }
            ui_spacer(10);
            if(ui_button(str8_literal("unit 2")).pressed_left){
            }
            ui_spacer(10);
            if(ui_button(str8_literal("unit 3")).pressed_left){
            }
        }
    }

    ui_end_panel();
}

static void
draw_world_grid(void){
    v2 low  = make_v2(floor_f32(camera.p3.x/state->tile_size) * state->tile_size, floor_f32(camera.p3.y/state->tile_size) * state->tile_size);
    v2 high = make_v2( ceil_f32(camera.p1.x/state->tile_size) * state->tile_size,  ceil_f32(camera.p1.y/state->tile_size) * state->tile_size);

    f32 x = low.x;
    while(x < high.x){
        v2 p0 = make_v2(x, low.y);
        v2 p1 = make_v2(x, high.y);

        p0 = v2_screen_from_world(p0);
        p1 = v2_screen_from_world(p1);
        draw_line(p0, p1, 1, RED);
        x += state->tile_size;
    }

    f32 y = low.y;
    while(y < high.y){
        v2 p0 = make_v2(low.x, y);
        v2 p1 = make_v2(high.x, y);

        p0 = v2_screen_from_world(p0);
        p1 = v2_screen_from_world(p1);
        draw_line(p0, p1, 1, RED);
        y += state->tile_size;
    }

    // draw coordinates
#if 0
    f32 y = low.y;
    while(y < high.y){

        f32 x = low.x;
        while(x < high.x){

            if(x >= 0 && (x/state->tile_size) < state->world_width_in_tiles){
                if(y >= 0 && (y/state->tile_size) < state->world_height_in_tiles){
                    v2 cell = make_v2(x, y);
                    v2 screen_cell = v2_screen_from_world(cell);

                    set_font(state->font);
                    String8 coord = str8_formatted(ts->frame_arena, "(%i, %i)", (s32)x/(s32)state->tile_size, (s32)y/(s32)state->tile_size);
                    draw_text(coord, screen_cell, YELLOW);
                }
            }

            x += state->tile_size;
        }

        y += state->tile_size;
    }
#endif
}

static void
draw_world_terrain(void){
    v2 low  = make_v2(floor_f32(camera.p3.x/state->tile_size) * state->tile_size, floor_f32(camera.p3.y/state->tile_size) * state->tile_size);
    v2 high = make_v2( ceil_f32(camera.p1.x/state->tile_size) * state->tile_size,  ceil_f32(camera.p1.y/state->tile_size) * state->tile_size);

    for(s32 i=1; i < TextureAsset_Count; ++i){
        f32 y = low.y;
        while(y < high.y){

            f32 x = low.x;
            while(x < high.x){

                if(x >= 0 && (x/state->tile_size) < state->world_width_in_tiles){
                    if(y >= 0 && (y/state->tile_size) < state->world_height_in_tiles){
                        v2 cell = make_v2(x, y);
                        v2 screen_cell = v2_screen_from_world(cell);

                        s32 idx = (s32)(((y/state->tile_size) * state->world_width_in_tiles) + (x/state->tile_size));
                        s32 cell_tex = state->world_grid[idx];
                        if(cell_tex == i){
                            set_texture(&r_assets->textures[cell_tex]);
                            Rect tex_rect = make_rect_size(cell, make_v2(state->tile_size, state->tile_size));
                            tex_rect = rect_screen_from_world(tex_rect);
                            draw_texture(tex_rect);
                        }
                    }
                }

                x += state->tile_size;
            }

            y += state->tile_size;
        }
    }
}

static bool
mouse_in_cell(v2 cell){
    v2 world_mouse = v2_world_from_screen(controller.mouse.pos);
    v2 mouse_cell = grid_cell_from_pos(world_mouse);
    if(controller_button_pressed(KeyCode_U)){
        debug_break();
    }
    if(mouse_cell == cell){
        return(true);
    }
    return(false);
}

static v2
grid_pos_from_cell(v2 cell){
    v2 result = {0};
    result.x = cell.x * state->tile_size;
    result.y = cell.y * state->tile_size;
    return(result);
}

static v2
grid_cell_from_pos(v2 pos){
    v2 result = {0};
    result.x = floor_f32(pos.x / state->tile_size);
    result.y = floor_f32(pos.y / state->tile_size);
    return(result);
}

static s32
world_grid_idx_from_cell(v2 cell){
    s32 idx = (s32)((cell.y * state->world_width_in_tiles) + cell.x);
    return(idx);
}

// todo(rr): maybe I don't need this and I can calculate in place
static v2
grid_cell_center(v2 cell){
    v2 result = {0};

    v2 pos = grid_pos_from_cell(cell);
    result.x = pos.x + state->tile_size/2;
    result.y = pos.y + state->tile_size/2;
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

    for(s32 y=0; y<(s32)state->world_height_in_tiles; ++y){
        for(s32 x=0; x<(s32)state->world_width_in_tiles; ++x){
            s32 cell = (y * (s32)state->world_width_in_tiles) + x;
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
    state->world_width_in_tiles = (f32)count / y;
    state->world_height_in_tiles = y;
    state->world_width = state->world_width_in_tiles * state->tile_size;
    state->world_height = state->world_height_in_tiles * state->tile_size;

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
            if(str8_contains(word, str8_literal("current_world"))){
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

        show_cursor(true);
        assets_load(ts->asset_arena, &assets);

        state->world_width_in_tiles = 10;
        state->world_height_in_tiles = 10;
        state->tile_size = 10;

        init_console(global_arena, &camera, &window, &assets);
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
        state->castle_cell = make_v2(1, 2);
        state->castle = add_castle(TextureAsset_Castle1, make_v2(1, 2), make_v2(10, 10));

        state->scene_state = SceneState_Game;
        //init_camera_2d(&camera, make_v2((state->world_width_in_tiles/2) * state->tile_size, (state->world_height_in_tiles/2) * state->tile_size), 30);
        init_camera_2d(&camera, make_v2(0, 0), 30);

        memory.initialized = true;
    }


    should_quit = false;
    while(!should_quit){
        ui_begin();

        begin_timed_scope("while(!should_quit)");

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

        if(controller_button_pressed(KeyCode_F8, true)){
            if(state->scene_state == SceneState_Editor){
                state->scene_state = SceneState_Game;
                state->terrain_selected = false;
                state->terrain_selected_id = 0;
            }
            else if(state->scene_state == SceneState_Game){
                state->scene_state = SceneState_Editor;
            }
        }

        // DRAW UI
        if(state->scene_state == SceneState_Editor){
            print("HERE1\n");
            ui_level_editor();
            debug_ui_render_batches();
        }

        if(state->is_entity_selected){
            switch(state->entity_selected->structure_type){
                case StructureType_Castle:{
                    if(controller.ctrl_pressed && controller_button_pressed(MOUSE_BUTTON_LEFT, true)){
                        v2 world_mouse = v2_world_from_screen(controller.mouse.pos);
                        state->entity_selected->waypoint = world_mouse;
                        state->entity_selected->waypoint_cell = grid_cell_from_pos(world_mouse);
                    }
                    ui_structure_castle();
                }
            }
        }

        if(mouse_in_cell(state->castle_cell) && controller_button_pressed(MOUSE_BUTTON_LEFT, true)){
            state->entity_selected = state->castle;
            state->is_entity_selected = true;
        }

        console_update();

        // camera drag
        if(controller_button_held(MOUSE_BUTTON_RIGHT) &&
           controller_button_pressed(MOUSE_BUTTON_RIGHT)){
            world_camera_record = camera;
            world_mouse_record = v2_world_from_screen(controller.mouse.pos);
            state->dragging_world = true;
        }
        if(controller_button_held(MOUSE_BUTTON_RIGHT)){
            v2 world_mouse_current = v2_world_from_screen(controller.mouse.pos, &world_camera_record);
            v2 world_rel_pos = world_mouse_record - world_mouse_current;

            camera.x = world_camera_record.x + world_rel_pos.x;
            camera.y = world_camera_record.y + world_rel_pos.y;
        }
        else{
            world_camera_record = {0};
            world_mouse_record = {0};
            state->dragging_world = false;
        }

        // zoom
        if(camera.size > 10){
            camera.size -= (f32)controller.mouse.wheel_dir * 20;
        }
        if(camera.size <= 10){
            camera.size -= (f32)controller.mouse.wheel_dir;
            if(camera.size < 3){
                camera.size = 3;
            }
        }

        simulations = 0;
        accumulator += frame_time;
        while(accumulator >= clock.dt){
            sim_game();

            accumulator -= clock.dt;
            time_elapsed += clock.dt;
            simulations++;

        }

        camera_2d_update(&camera, window.aspect_ratio);
        wasapi_play_cursors();

        // rendering
        {
            //----constant buffer----
            D3D11_MAPPED_SUBRESOURCE mapped_subresource;
            d3d_context->Map(d3d_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
            ConstantBuffer2D* constants = (ConstantBuffer2D*)mapped_subresource.pData;
            constants->screen_res = make_v2s32((s32)window.width, (s32)window.height);
            d3d_context->Unmap(d3d_constant_buffer, 0);

            render_batches_reset();
            //arena_free(ts->batch_arena);
            draw_world_terrain();
            if(state->scene_state == SceneState_Editor){
                draw_world_grid();
            }
            draw_entities(state);

            //debug_draw_mouse_cell_pos();

            // draw selected texture
            if(state->terrain_selected){
                set_texture(&r_assets->textures[state->terrain_selected_id]);
                draw_texture(controller.mouse.pos, make_v2(50, 50));
                draw_bounding_box(make_rect_size(controller.mouse.pos, make_v2(50, 50)), 2, RED);
            }


            ui_end();
            set_font(state->font);
            String8 fps = str8_formatted(ts->frame_arena, "FPS: %.2f", FPS);
            draw_text(fps, make_v2(window.width - text_padding - font_string_width(state->font, fps), window.height - text_padding), ORANGE);

            Quad quad = make_quad(make_v2(0, 0), make_v2(3, 3));
            quad = quad_screen_from_world(quad);
            set_texture(&assets.textures[TextureAsset_Skeleton1]);
            //draw_texture(quad);
            //draw_bounding_box(quad, 2, RED);

            set_font(state->font);
            String8 str_fmt = str8_formatted(ts->frame_arena, "entities_count: %i\n", state->entities_count);
            draw_text(str_fmt, make_v2(200, 100), GREEN);


            console_draw();
            v2 p0 = v2_screen_from_world(camera.p0);
            v2 p1 = v2_screen_from_world(camera.p1);
            v2 p2 = v2_screen_from_world(camera.p2);
            v2 p3 = v2_screen_from_world(camera.p3);
            draw_line(p0, p1, 5, RED);
            draw_line(p1, p2, 5, RED);
            draw_line(p2, p3, 5, RED);
            draw_line(p3, p0, 5, RED);
            {
                d3d_clear_color(BACKGROUND_COLOR);
                //draw_render_batches();
                draw_render_batches_new();
                d3d_present();

                arena_free(ts->frame_arena);
            }
        }
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


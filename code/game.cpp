#ifndef GAME_C
#define GAME_C

static void
do_one_frame(void){
    if(!memory.initialized){
        // consider: maybe move this memory stuff to init_memory()
        init_arena(&state->arena, (u8*)memory.permanent_base + sizeof(PermanentMemory), memory.permanent_size - sizeof(PermanentMemory));
        init_arena(&ts->arena, (u8*)memory.transient_base + sizeof(TransientMemory), memory.transient_size - sizeof(TransientMemory));

        ts->frame_arena = push_arena(&ts->arena, MB(100));
        ts->asset_arena = push_arena(&ts->arena, MB(100));
        ts->ui_arena = push_arena(&ts->arena, MB(100));
        ts->hash_arena = push_arena(&ts->arena, MB(100));
        ts->batch_arena = push_arena(&ts->arena, GB(1));

        state->scene_state = SceneState_Menu;
        state->game_state = GameState_None;

        show_cursor(true);
        load_assets(ts->asset_arena, &assets);

        init_camera_2d(&camera, make_v2((WORLD_SIZE/2) * GRID_SIZE, (WORLD_SIZE/2) * GRID_SIZE), 30);
        init_console(global_arena, &camera, &window, &assets);
        init_ui(ts->hash_arena, &window, &controller, &assets);
        init_draw(ts->batch_arena, &assets);

        state->font = &assets.fonts[FontAsset_Arial];

        // setup free entities array in reverse order
        entities_clear();

        // populate world cell textures
        f32 y = 0;
        while(y < WORLD_SIZE){
            f32 x = 0;
            while(x < WORLD_SIZE){
                s32 idx = (s32)((y * 100) + x);
                u32 cell_tex = state->world_grid[idx];
                if(cell_tex == 0){
                    cell_tex = random_range_u32(8) + 1;
                    state->world_grid[idx] = cell_tex;
                }

                x += 1;
            }
            y += 1;
        }

        v2 pos = grid_pos_from_cell(50, 50);
        pos = grid_cell_center(pos.x, pos.y);
        state->castle = add_castle(TextureAsset_Castle1, pos, make_v2(10, 10));

        memory.initialized = true;
    }
}

static void
draw_world_grid(void){
    v2 low  = make_v2(floor_f32(camera.p3.x/GRID_SIZE) * GRID_SIZE, floor_f32(camera.p3.y/GRID_SIZE) * GRID_SIZE);
    v2 high = make_v2( ceil_f32(camera.p1.x/GRID_SIZE) * GRID_SIZE,  ceil_f32(camera.p1.y/GRID_SIZE) * GRID_SIZE);

    f32 x = low.x;
    while(x < high.x){
        v2 p0 = make_v2(x, low.y);
        v2 p1 = make_v2(x, high.y);

        p0 = v2_screen_from_world(p0);
        p1 = v2_screen_from_world(p1);
        draw_line(p0, p1, 1, RED);
        x += GRID_SIZE;
    }

    f32 y = low.y;
    while(y < high.y){
        v2 p0 = make_v2(low.x, y);
        v2 p1 = make_v2(high.x, y);

        p0 = v2_screen_from_world(p0);
        p1 = v2_screen_from_world(p1);
        draw_line(p0, p1, 1, RED);
        y += GRID_SIZE;
    }
}

static void
draw_world_terrain(void){
    v2 low  = make_v2(floor_f32(camera.p3.x/GRID_SIZE) * GRID_SIZE, floor_f32(camera.p3.y/GRID_SIZE) * GRID_SIZE);
    v2 high = make_v2( ceil_f32(camera.p1.x/GRID_SIZE) * GRID_SIZE,  ceil_f32(camera.p1.y/GRID_SIZE) * GRID_SIZE);
    Rect rect = make_rect(low, high);

    for(u32 i=1; i < 9; ++i){
        f32 y = 0;
        while(y < WORLD_SIZE){

            f32 x = 0;
            while(x < WORLD_SIZE){
                v2 cell = make_v2(x * GRID_SIZE, y * GRID_SIZE);
                if(rect_contains_point(rect, cell)){
                    s32 idx = (s32)((y * 100) + x);
                    u32 cell_tex = state->world_grid[idx];
                    if(cell_tex == i){
                        set_texture(&r_assets->textures[cell_tex]);
                        Rect tex_rect = make_rect_size(cell, make_v2(10, 10));
                        tex_rect = rect_screen_from_world(tex_rect);
                        draw_texture(tex_rect);
                    }
                }

                x += 1;
            }
            y += 1;
        }
    }

    f32 y = 0;
    while(y < WORLD_SIZE){

        f32 x = 0;
        while(x < WORLD_SIZE){
            v2 cell = make_v2(x * GRID_SIZE, y * GRID_SIZE);
            if(rect_contains_point(rect, cell)){
                v2 screen_cell = v2_screen_from_world(cell);
                set_font(state->font);
                String8 coord = str8_formatted(ts->frame_arena, "(%i, %i)", (s32)x, (s32)y);
                draw_text(coord, screen_cell, RED);
            }

            x += 1;
        }

        y += 1;
    }

    //f32 y = low.y;
    //while(y < high.y){

    //    f32 x = low.x;
    //    while(x < high.x){

    //        v2 cell = make_v2(x, y);


    //        //s32 offset = abs_s32((-WORLD_SIZE/2 * GRID_SIZE));
    //        //s32 idx = (((s32)y + offset) * (s32)high.y) + ((s32)x + offset);
    //        //u32 cell_tex = state->world_grid[idx];
    //        //if(cell_tex == 0){
    //        //    cell_tex = random_range_u32(2) + 1; // todo: hard coded for now, 5 rails 6 offset
    //        //    state->world_grid[idx] = cell_tex;
    //        //}

    //        v2 screen_cell = v2_screen_from_world(cell);


    //        //set_texture(&r_assets->textures[1]);
    //        //Rect rect = make_rect_size(make_v2(x, y), make_v2(10, 10));
    //        //rect = rect_screen_from_world(rect);
    //        //draw_texture(rect);
    //        if(x >= 0 && x < WORLD_SIZE){
    //            set_font(state->font);
    //            String8 coord = str8_formatted(ts->frame_arena, "(%i, %i)", (s32)x, (s32)y);
    //            draw_text(coord, screen_cell, YELLOW);
    //        }


    //        x += GRID_SIZE;
    //    }

    //    y += GRID_SIZE;
    //}
}

static v2
grid_pos_from_cell(f32 x, f32 y){
    v2 result = {0};
    result.x = x * GRID_SIZE;
    result.y = y * GRID_SIZE;
    return(result);
}

static v2
grid_cell_from_pos(f32 x, f32 y){
    v2 result = {0};
    result.x = floor_f32(x / GRID_SIZE);
    result.y = floor_f32(y / GRID_SIZE);
    return(result);
}

// todo(rr): maybe I don't need this and I can calculate in place
static v2
grid_cell_center(f32 x, f32 y){
    v2 result = {0};
    result.x = x + GRID_SIZE/2;
    result.y = x + GRID_SIZE/2;
    return(result);
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
    clear_flags(&e->flags, EntityFlag_Active);
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
        set_flags(&e->flags, EntityFlag_Active);
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
        e->dir = make_v2(1, 1);
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->dir = make_v2(0, 1);
        e->deg = 90;
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static Entity*
add_texture(u32 texture, v2 pos, v2 dim, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Texture);
    if(e){
        e->dir = make_v2(1, 1);
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->dir = make_v2(0, 1);
        e->deg = 90;
        e->texture = texture;
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static Entity*
add_castle(u32 texture, v2 pos, v2 dim, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Texture);
    if(e){
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->texture = texture;
        e->deg = 180;
        e->dir = dir_from_deg(e->deg);

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
        clear_flags(&e->flags, EntityFlag_Active);
        state->free_entities[i] = state->free_entities_at - i;
        state->generation[i] = 0;
    }
    state->entities_count = 0;
}

// todo(rr): redo this entire thing so that its text based
static void
serialize_data(String8 filename){
    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, saves_path, filename);

    File file = os_file_open(full_path, GENERIC_WRITE, CREATE_NEW);
    if(file.handle != INVALID_HANDLE_VALUE){
        os_file_write(file, state->entities, sizeof(Entity) * ENTITIES_MAX);
    }
    else{
        // log error
        print("Save file \"%s\" already exists: Could not serialize data\n", filename.str);
    }

    os_file_close(file);
    end_scratch(scratch);
}

// todo(rr): redo this entire thing so that its text based
static void
deserialize_data(String8 filename){
    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, saves_path, filename);

    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    if(!file.size){
        //todo(rr): log error
        print("Error: failed to open file <%s>\n", full_path.str);
        os_file_close(file);
        end_scratch(scratch);
        return;
    }

    String8 data = os_file_read(&state->arena, file);
    entities_clear();

    u32 offset = 0;
    while(offset < data.size){
        Entity* e = (Entity*)(data.str + offset);
        //switch(e->type){
        //    case EntityType_Ship:{
        //        Entity* ship = add_entity(EntityType_Ship);
        //        *ship = *e;
        //        //ship->texture = TextureAsset_Ship;

        //        //state->ship = ship;
        //        //state->ship_loaded = true;
        //    } break;
        //    case EntityType_Bullet:{
        //        Entity* bullet = add_entity(EntityType_Bullet);
        //        *bullet = *e;
        //        //bullet->texture = TextureAsset_Bullet;
        //    } break;
        //    case EntityType_Asteroid:{
        //        Entity* ast = add_entity(EntityType_Asteroid);
        //        *ast = *e;
        //        //ast->texture = TextureAsset_Asteroid;
        //    } break;
        //}
        offset += sizeof(Entity);
    }
    os_file_close(file);
    end_scratch(scratch);
}

static bool
handle_global_events(Event event){
    if(event.keycode == KeyCode_ESCAPE){
        should_quit = true;
    }
    if(event.type == QUIT){
        should_quit = true;
        return(true);
    }
    if(event.type == KEYBOARD){
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
    if(event.type == KEYBOARD){
        if(event.key_pressed){
        }
    }
    return(false);
}

static bool
handle_controller_events(Event event){
    if(event.type == MOUSE){
        controller.mouse.x  = event.mouse_x;
        controller.mouse.y  = event.mouse_y;
        controller.mouse.dx = event.mouse_dx;
        controller.mouse.dy = event.mouse_dy;
        controller.mouse.edge_left   = event.mouse_edge_left;
        controller.mouse.edge_right  = event.mouse_edge_right;
        controller.mouse.edge_top    = event.mouse_edge_top;
        controller.mouse.edge_bottom = event.mouse_edge_bottom;
    }
    if(event.type == KEYBOARD){
        controller.mouse.wheel_dir = event.mouse_wheel_dir;
        if(event.key_pressed){
            if(!event.repeat){
                controller.button[event.keycode].pressed = true;
            }
            controller.button[event.keycode].held = true;
        }
        else{
            controller.button[event.keycode].held = false;
        }
    }
    return(false);
}

static bool
handle_game_events(Event event){
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == KeyCode_ESCAPE){
                if(state->scene_state == SceneState_Game){
                    if(state->game_state == GameState_Running){
                        state->game_state = GameState_Paused;
                        return(true);
                    }
                    if(state->game_state == GameState_Paused){
                        state->game_state = GameState_Running;
                        return(true);
                    }
                    return(false);
                }
            }
        }
    }
    return(false);
}

v2s32 static
mouse_cell(){
    v2 pos = v2_world_from_screen(controller.mouse.pos);
    pos.x = pos.x / GRID_SIZE;
    pos.y = pos.y / GRID_SIZE;
    v2s32 result = { (s32)pos.x, (s32)pos.y };
    return(result);
}

static void update_game(void){

    // camera
    {
        // movement
        if(controller.button[KeyCode_UP].held || controller.mouse.edge_top){
            camera.y += ((camera.size) + 50) * (f32)clock.dt;
        }
        if(controller.button[KeyCode_DOWN].held || controller.mouse.edge_bottom){
            camera.y -= ((camera.size) + 50) * (f32)clock.dt;
        }
        if(controller.button[KeyCode_LEFT].held || controller.mouse.edge_left){
            camera.x -= ((camera.size) + 50) * (f32)clock.dt;
        }
        if(controller.button[KeyCode_RIGHT].held || controller.mouse.edge_right){
            camera.x += ((camera.size) + 50) * (f32)clock.dt;
        }
    }

    if(controller.button[MOUSE_BUTTON_LEFT].pressed || controller.button[MOUSE_BUTTON_LEFT].held){
        v2s32 cell = mouse_cell();
        s32 idx = (s32)((cell.y * 100) + cell.x);
        state->world_grid[idx] = state->selected_texture;
    }

    // resolve entity motion
    for(s32 i = 0; i < array_count(state->entities); ++i){
        Entity *e = state->entities + i;
        if(!has_flags(e->flags, EntityFlag_Active)){
            continue;
        }

        // todo(rr): this will be different
        if(has_flags(e->flags, EntityFlag_MoveWithPhys)){
            //e->pos.x += (e->dir.x * e->velocity * e->speed) * (f32)clock.dt;
            //e->pos.y += (e->dir.y * e->velocity * e->speed) * (f32)clock.dt;
        }
    }

    // resolve death todo(rr): this will be different
    for(s32 i = 0; i < array_count(state->entities); ++i){
        Entity *e = state->entities + i;
        if(!has_flags(e->flags, EntityFlag_Active)){
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

#endif


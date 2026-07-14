#include "main.hpp"

static void
init_spawner(){
    state->spawner.pos_offset = 5.0f;
    state->spawner.render_size = 0.5f;
    state->spawner.timer = clock.get_os_timer();
}

static void
update_spawners(){
    state->spawner.spawn_point[0].pos = player->pos - make_v2(state->spawner.pos_offset * -1.5f, state->spawner.pos_offset * 1);
    state->spawner.spawn_point[1].pos = player->pos - make_v2(state->spawner.pos_offset *  0,    state->spawner.pos_offset * 1.5f);
    state->spawner.spawn_point[2].pos = player->pos - make_v2(state->spawner.pos_offset *  1.5f, state->spawner.pos_offset * 1);

    state->spawner.spawn_point[3].pos = player->pos - make_v2(state->spawner.pos_offset * -1.5f, state->spawner.pos_offset * -1);
    state->spawner.spawn_point[4].pos = player->pos - make_v2(state->spawner.pos_offset *  0,    state->spawner.pos_offset * -1.5f);
    state->spawner.spawn_point[5].pos = player->pos - make_v2(state->spawner.pos_offset *  1.5f, state->spawner.pos_offset * -1);
}

static void
sim_game(void){
    // DUMB DUMB ADDED THIS
    begin_timed_function();

    if(controller_button_pressed(KeyCode_Q)){ 
        player->dead = !player->dead;
    }

    f64 elapsed_time = clock.get_seconds_elapsed(clock.get_os_timer(), state->spawner.timer);
    print("%f\n", elapsed_time);
    if(elapsed_time > 2.5f){
        state->spawner.timer = clock.get_os_timer();
        v2 cell;
        cell = grid_cell_from_pos(state->spawner.spawn_point[0].pos);
        add_monster(cell, make_v2(2, 2));
        cell = grid_cell_from_pos(state->spawner.spawn_point[1].pos);
        add_monster(cell, make_v2(2, 2));
        cell = grid_cell_from_pos(state->spawner.spawn_point[2].pos);
        add_monster(cell, make_v2(2, 2));
        cell = grid_cell_from_pos(state->spawner.spawn_point[3].pos);
        add_monster(cell, make_v2(2, 2));
        cell = grid_cell_from_pos(state->spawner.spawn_point[4].pos);
        add_monster(cell, make_v2(2, 2));
        cell = grid_cell_from_pos(state->spawner.spawn_point[5].pos);
        add_monster(cell, make_v2(2, 2));
    }

    if(state->scene_state == SceneState_Game){
        if(!player->dead){
            if(controller_button_held(KeyCode_D)){ 
                player->velocity.x = player->speed;
                player->left_right = 1;
            }
            if(controller_button_held(KeyCode_A)){ 
                player->velocity.x = -player->speed;
                player->left_right = -1;
            }
            if(controller_button_held(KeyCode_W)){ 
                player->velocity.y = player->speed;
                player->up_down = 1;
            }
            if(controller_button_held(KeyCode_S)){ 
                player->velocity.y = -player->speed;
                player->up_down = -1;
            }

            if(controller_button_pressed(KeyCode_SPACEBAR)){ 
                player->jumping = true;
            }
            //if(controller_button_pressed(MOUSE_BUTTON_LEFT, false)){ 
            if(controller_button_pressed(KeyCode_E)){ 
                player->attacking = true;
            }


            // note(rr): Digonal movement adjustment.
            if(player->left_right != 0 && player->up_down != 0){
                player->velocity.x *= 0.707f;
                player->velocity.y *= 0.707f;
            }
            player->left_right = 0;
            player->up_down = 0;


            // update attack_box
            if(player->sprite.direction == RIGHT_FRONT){
                player->attack_box.min = make_v2(player->pos.x, player->pos.y - 0.42f);
                player->attack_box.max = make_v2(player->pos.x + 0.7f, player->pos.y + 0.25f);
            }
            if(player->sprite.direction == RIGHT_BACK){
                player->attack_box.min = make_v2(player->pos.x, player->pos.y - 0.25f);
                player->attack_box.max = make_v2(player->pos.x + 0.7f, player->pos.y + 0.4f);
            }
            if(player->sprite.direction == LEFT_FRONT){
            player->attack_box.min = make_v2(player->pos.x, player->pos.y - 0.36f);
            player->attack_box.max = make_v2(player->pos.x - 0.65f, player->pos.y + 0.2);
            }
            if(player->sprite.direction == LEFT_BACK){
                player->attack_box.min = make_v2(player->pos.x, player->pos.y - 0.2f);
                player->attack_box.max = make_v2(player->pos.x - 0.65f, player->pos.y + 0.35f);
            }


        }
    }

    entity_sprite_update();
    update_spawners();

    // camera
    {
        if(state->scene_state == SceneState_Editor){
            if(controller_button_held(KeyCode_W)){
                camera.y += ((camera.size) + 50) * (f32)clock.dt;
            }
            if(controller_button_held(KeyCode_S)){
                camera.y -= ((camera.size) + 50) * (f32)clock.dt;
            }
            if(controller_button_held(KeyCode_A)){
                camera.x -= ((camera.size) + 50) * (f32)clock.dt;
            }
            if(controller_button_held(KeyCode_D)){
                camera.x += ((camera.size) + 50) * (f32)clock.dt;
            }
        }
        else if(state->scene_state == SceneState_Game){
            camera.x = player->pos.x;
            camera.y = player->pos.y;
        }
    }

    // draw terrain
    if(state->scene_state == SceneState_Editor){
        // DUMB DUMB ADDED THIS
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
        // DUMB DUMB ADDED THIS
        begin_timed_scope("flocking");
        {
            // DUMB DUMB ADDED THIS
            begin_timed_scope("entity movement and flocking");

            for(s32 i = 0; i < state->active_entities_count; ++i){
                Entity *e = state->active_entities[i];
                if(!has_flags(e->flags, EntityFlag_Active)) continue;
                if(!has_flags(e->flags, EntityFlag_MoveWithPhys)) continue;

                // All 9 surrounding cells.
                v2 cell_coords = grid_cell_from_pos(e->pos, state->flocking_cell_size);
                v2 all_coords[9] = {
                    cell_coords,
                    make_v2(cell_coords.x - 1, cell_coords.y - 1),
                    make_v2(cell_coords.x - 1, cell_coords.y),
                    make_v2(cell_coords.x - 1, cell_coords.y + 1),
                    make_v2(cell_coords.x,     cell_coords.y - 1),
                    make_v2(cell_coords.x,     cell_coords.y + 1),
                    make_v2(cell_coords.x + 1, cell_coords.y - 1),
                    make_v2(cell_coords.x + 1, cell_coords.y),
                    make_v2(cell_coords.x + 1, cell_coords.y + 1),
                };

                // Flocking, cumulative velocity.
                for(s32 j=0; j < array_count(all_coords); ++j){
                    v2 coords = all_coords[j];
                    if(!grid_cell_coords_in_bounds(coords)) continue;

                    Cell* cell = state->cells + ((s32)coords.x + (WORLD_WIDTH_IN_TILES_MAX * (s32)coords.y));
                    if(cell->generation != cell_generation) continue;

                    for(BinNode* bin = cell->bin; bin != 0; bin = bin->next){
                        for(s32 k = 0; k < bin->at; ++k){
                            Entity* other = bin->entities[k];
                            if(!has_flags(other->flags, EntityFlag_MoveWithPhys)) continue;
                            if(e == other) continue;
                            if(e->index > other->index) continue;

                            f32 separation_radius = 0.45f;
                            f32 separation_radius_squared = separation_radius * separation_radius;
                            f32 distance_squared = distance_squared_v2(e->pos, other->pos);

                            if(distance_squared < separation_radius_squared){
                                f32 distance;
                                v2 dir;

                                if(distance_squared < 0.0001f){
                                    if(e->index < other->index){
                                        // todo: can randomize this more
                                        dir = make_v2(1.0f, -1.0f);
                                    }
                                    else{
                                        dir = make_v2(-1.0f, 1.0f);
                                    }
                                    distance = 0.01f;
                                }
                                else{
                                    distance = sqrtf(distance_squared);
                                    dir = (e->pos - other->pos);
                                    dir.x /= distance;
                                    dir.y /= distance;
                                }


                                f32 push_strength = 1.0f;
                                if(e == player){
                                    push_strength = 10.0f;
                                }
                                if(e != player){
                                    e->velocity.x += (dir.x * push_strength * (f32)clock.dt) / distance;
                                    e->velocity.y += (dir.y * push_strength * (f32)clock.dt) / distance;
                                }
                                if(other != player){
                                    other->velocity.x -= (dir.x * push_strength * (f32)clock.dt) / distance;
                                    other->velocity.y -= (dir.y * push_strength * (f32)clock.dt) / distance;
                                }
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
                            if(!v2_close_enough(e->pos, c->move_to, 0.01f)){
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

                e->pos.x += e->velocity.x * (f32)clock.dt;
                e->pos.y += e->velocity.y * (f32)clock.dt;
            }
        }

        {
            // DUMB DUMB ADDED THIS
            begin_timed_scope("player attack query");

            v2 cell_coords = grid_cell_from_pos(player->pos, state->flocking_cell_size);
            v2 all_coords[9] = {
                cell_coords,
                make_v2(cell_coords.x - 1, cell_coords.y - 1),
                make_v2(cell_coords.x - 1, cell_coords.y),
                make_v2(cell_coords.x - 1, cell_coords.y + 1),
                make_v2(cell_coords.x,     cell_coords.y - 1),
                make_v2(cell_coords.x,     cell_coords.y + 1),
                make_v2(cell_coords.x + 1, cell_coords.y - 1),
                make_v2(cell_coords.x + 1, cell_coords.y),
                make_v2(cell_coords.x + 1, cell_coords.y + 1),
            };
            for(s32 j=0; j < array_count(all_coords); ++j){
                v2 coords = all_coords[j];
                if(!grid_cell_coords_in_bounds(coords)) continue;

                Cell* cell = state->cells + ((s32)coords.x + (WORLD_WIDTH_IN_TILES_MAX * (s32)coords.y));
                if(cell->generation != cell_generation) continue;

                for(BinNode* bin = cell->bin; bin != 0; bin = bin->next){
                    for(s32 k = 0; k < bin->at; ++k){
                        Entity* other = bin->entities[k];
                        if(other == player) continue;
                        if(!has_flags(other->flags, EntityFlag_Active)) continue;

                        if(player->attacking){
                            if(has_flags(other->flags, EntityFlag_CanDie)){
                                Rect other_rect = rect_from_center(other);
                                if(rect_collides_rect(player->attack_box, other_rect)){
                                    other->dead = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
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

static void 
activate_entity(Entity* e){
    assert(!has_flags(e->flags, EntityFlag_Active));

    e->active_list_idx = state->active_entities_count;
    state->active_entities[state->active_entities_count++] = e;

    set_flags(&e->flags, EntityFlag_Active);
}

static void 
deactive_entity(Entity* e){
    assert(has_flags(e->flags, EntityFlag_Active));

    u32 remove_idx = e->active_list_idx;
    u32 move_idx = --state->active_entities_count;

    Entity* move_e = state->active_entities[move_idx];
    state->active_entities[remove_idx] = move_e;
    move_e->active_list_idx = remove_idx;

    e->active_list_idx = u32_max;
    clear_flags(&e->flags, EntityFlag_Active);
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
        Entity* e = state->entities + free_entity_index;
        e->index = free_entity_index;
        state->generation[e->index]++;
        state->entities_count++;
        e->generation = state->generation[e->index]; // CONSIDER: this might not be necessary
        e->type = type;
        activate_entity(e);

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
        e->texture_id = texture;
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
        e->texture_id = texture;
        e->deg = 0;
        e->rot = dir_from_deg(e->deg);
        e->structure_type = StructureType_Castle;
        e->bounding_box_scale = make_v2(0.85f, 0.85f);
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
        e->texture_id = texture;
        e->velocity = {0};
        e->speed = 1000.0f;
        e->dir = dir;
        e->rot = make_v2(1, 0);
        e->deg = deg_from_dir(e->rot);
        set_flags(&e->flags, EntityFlag_MoveWithPhys);
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static Entity*
add_monster(v2 cell, v2 dim, v2 dir, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Monster);
    if(e){
        e->color = color;
        e->pos = grid_pos_from_cell(cell, state->world_cell_size);
        e->dim = dim;
        e->velocity = {0};
        e->speed = 250.0f;
        e->bounding_box_scale = make_v2(0.25f, 0.25f);
        set_flags(&e->flags, EntityFlag_MoveWithPhys|EntityFlag_HasSprite|EntityFlag_CanDie);

        e->sprite.kind = SPRITE_ANIM_IDLE;
        e->sprite.direction = (Sprite_Direction)random_range_u32(SPRITE_DIRECTION_COUNT);

        Texture* tex = {0};
        tex = &assets.textures[TextureAsset_Orc_Idle];
        e->sprite.animations[SPRITE_ANIM_IDLE].texture_id = TextureAsset_Orc_Idle;
        e->sprite.animations[SPRITE_ANIM_IDLE].speed = 2.0f;
        e->sprite.animations[SPRITE_ANIM_IDLE].time_max = 1.0f;
        e->sprite.animations[SPRITE_ANIM_IDLE].width = tex->width;
        e->sprite.animations[SPRITE_ANIM_IDLE].height = tex->height;
        e->sprite.animations[SPRITE_ANIM_IDLE].inc = tex->width / 16;
        e->sprite.animations[SPRITE_ANIM_IDLE].col = (s32)random_range_u32(16 + 1);
        e->sprite.animations[SPRITE_ANIM_IDLE].directions[RIGHT_FRONT] = {0, 0, 16};
        e->sprite.animations[SPRITE_ANIM_IDLE].directions[LEFT_FRONT]  = {0, 1, 16};
        e->sprite.animations[SPRITE_ANIM_IDLE].directions[RIGHT_BACK]  = {0, 2, 16};
        e->sprite.animations[SPRITE_ANIM_IDLE].directions[LEFT_BACK]   = {0, 3, 16};

        tex = &assets.textures[TextureAsset_Orc_Idle_Shadow];
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].texture_id = TextureAsset_Orc_Idle_Shadow;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].speed = 2.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].time_max = 1.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].width = tex->width;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].height = tex->height;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].inc = tex->width / 16;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].col = e->sprite.animations[SPRITE_ANIM_IDLE].col;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].directions[RIGHT_FRONT] = {0, 0, 16};
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].directions[LEFT_FRONT]  = {0, 1, 16};
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].directions[RIGHT_BACK]  = {0, 2, 16};
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].directions[LEFT_BACK]   = {0, 3, 16};

        tex = &assets.textures[TextureAsset_Orc_Walk];
        e->sprite.animations[SPRITE_ANIM_WALK].texture_id = TextureAsset_Orc_Walk;
        e->sprite.animations[SPRITE_ANIM_WALK].speed = 18.0f;
        e->sprite.animations[SPRITE_ANIM_WALK].time_max = 1.0f;
        e->sprite.animations[SPRITE_ANIM_WALK].width = tex->width;
        e->sprite.animations[SPRITE_ANIM_WALK].height = tex->height;
        e->sprite.animations[SPRITE_ANIM_WALK].inc = tex->width / 4;
        e->sprite.animations[SPRITE_ANIM_WALK].directions[RIGHT_FRONT] = {0, 0, 4};
        e->sprite.animations[SPRITE_ANIM_WALK].directions[LEFT_FRONT]  = {0, 1, 4};
        e->sprite.animations[SPRITE_ANIM_WALK].directions[RIGHT_BACK]  = {0, 2, 4};
        e->sprite.animations[SPRITE_ANIM_WALK].directions[LEFT_BACK]   = {0, 3, 4};

        tex = &assets.textures[TextureAsset_Orc_Walk_Shadow];
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].texture_id = TextureAsset_Orc_Walk_Shadow;
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].speed = 18.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].time_max = 1.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].width = tex->width;
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].height = tex->height;
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].inc = tex->width / 4;
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].directions[RIGHT_FRONT] = {0, 0, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].directions[LEFT_FRONT]  = {0, 1, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].directions[RIGHT_BACK]  = {0, 2, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].directions[LEFT_BACK]   = {0, 3, 4};

        tex = &assets.textures[TextureAsset_Orc_Attack];
        e->sprite.animations[SPRITE_ANIM_ATTACK].texture_id = TextureAsset_Orc_Attack;
        e->sprite.animations[SPRITE_ANIM_ATTACK].speed = 20.0f;
        e->sprite.animations[SPRITE_ANIM_ATTACK].time_max = 1.0f;
        e->sprite.animations[SPRITE_ANIM_ATTACK].width = tex->width;
        e->sprite.animations[SPRITE_ANIM_ATTACK].height = tex->height;
        e->sprite.animations[SPRITE_ANIM_ATTACK].inc = tex->width / 4;
        e->sprite.animations[SPRITE_ANIM_ATTACK].do_once = true;
        e->sprite.animations[SPRITE_ANIM_ATTACK].directions[RIGHT_FRONT] = {1, 0, 4};
        e->sprite.animations[SPRITE_ANIM_ATTACK].directions[LEFT_FRONT]  = {1, 1, 4};
        e->sprite.animations[SPRITE_ANIM_ATTACK].directions[RIGHT_BACK]  = {1, 2, 4};
        e->sprite.animations[SPRITE_ANIM_ATTACK].directions[LEFT_BACK]   = {1, 3, 4};

        tex = &assets.textures[TextureAsset_Orc_Attack_Shadow];
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].texture_id = TextureAsset_Orc_Attack_Shadow;
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].speed = 20.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].time_max = 1.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].width = tex->width;
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].height = tex->height;
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].inc = tex->width / 4;
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].do_once = true;
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].directions[RIGHT_FRONT] = {1, 0, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].directions[LEFT_FRONT]  = {1, 1, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].directions[RIGHT_BACK]  = {1, 2, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].directions[LEFT_BACK]   = {1, 3, 4};

        tex = &assets.textures[TextureAsset_Orc_Jump];
        e->sprite.animations[SPRITE_ANIM_JUMP].texture_id = TextureAsset_Orc_Jump;
        e->sprite.animations[SPRITE_ANIM_JUMP].speed = 20.0f;
        e->sprite.animations[SPRITE_ANIM_JUMP].time_max = 1.0f;
        e->sprite.animations[SPRITE_ANIM_JUMP].width = tex->width;
        e->sprite.animations[SPRITE_ANIM_JUMP].height = tex->height;
        e->sprite.animations[SPRITE_ANIM_JUMP].inc = tex->width / 4;
        e->sprite.animations[SPRITE_ANIM_JUMP].do_once = true;
        e->sprite.animations[SPRITE_ANIM_JUMP].directions[RIGHT_FRONT] = {0, 0, 4};
        e->sprite.animations[SPRITE_ANIM_JUMP].directions[LEFT_FRONT]  = {0, 0, 4};
        e->sprite.animations[SPRITE_ANIM_JUMP].directions[RIGHT_BACK]  = {0, 1, 4};
        e->sprite.animations[SPRITE_ANIM_JUMP].directions[LEFT_BACK]   = {0, 2, 4};

        tex = &assets.textures[TextureAsset_Orc_Jump_Shadow];
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].texture_id = TextureAsset_Orc_Jump_Shadow;
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].speed = 20.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].time_max = 1.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].width = tex->width;
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].height = tex->height;
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].inc = tex->width / 4;
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].do_once = true;
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].directions[RIGHT_FRONT] = {0, 0, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].directions[LEFT_FRONT]  = {0, 0, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].directions[RIGHT_BACK]  = {0, 1, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].directions[LEFT_BACK]   = {0, 2, 4};

        tex = &assets.textures[TextureAsset_Orc_Die];
        e->sprite.animations[SPRITE_ANIM_DIE].texture_id = TextureAsset_Orc_Die;
        e->sprite.animations[SPRITE_ANIM_DIE].speed = 12.0f;
        e->sprite.animations[SPRITE_ANIM_DIE].time_max = 1.0f;
        e->sprite.animations[SPRITE_ANIM_DIE].width = tex->width;
        e->sprite.animations[SPRITE_ANIM_DIE].height = tex->height;
        e->sprite.animations[SPRITE_ANIM_DIE].inc = tex->width / 12;
        e->sprite.animations[SPRITE_ANIM_DIE].do_once = true;
        e->sprite.animations[SPRITE_ANIM_DIE].directions[RIGHT_FRONT] = {0, 0, 8};
        e->sprite.animations[SPRITE_ANIM_DIE].directions[LEFT_FRONT]  = {0, 0, 0};
        e->sprite.animations[SPRITE_ANIM_DIE].directions[RIGHT_BACK]  = {0, 0, 0};
        e->sprite.animations[SPRITE_ANIM_DIE].directions[LEFT_BACK]   = {0, 0, 0};

        tex = &assets.textures[TextureAsset_Orc_Die_Shadow];
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].texture_id = TextureAsset_Orc_Die_Shadow;
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].speed = 12.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].time_max = 1.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].width = tex->width;
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].height = tex->height;
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].inc = tex->width / 12;
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].do_once = true;
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].directions[RIGHT_FRONT] = {0, 0, 8};
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].directions[LEFT_FRONT]  = {0, 0, 0};
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].directions[RIGHT_BACK]  = {0, 0, 0};
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].directions[LEFT_BACK]   = {0, 0, 0};

        // note: we infer the bounding box from the sprite pixel increment
        
        f32 size = e->shadow_sprite.animations[SPRITE_ANIM_DIE].inc;
        e->bounding_box = make_rect_size(e->pos, make_v2(size, size));
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static Entity*
add_fire(v2 cell, v2 dim, v2 dir, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Fire);
    if(e){
        e->color = color;
        e->pos = grid_pos_from_cell(cell, state->world_cell_size);
        e->dim = dim;
        e->bounding_box_scale = make_v2(0.5f, 0.5f);
        set_flags(&e->flags, EntityFlag_HasSprite);

        e->sprite.kind = SPRITE_ANIM_IDLE;
        e->sprite.direction = RIGHT_FRONT;

        Texture* tex = {0};
        tex = &assets.textures[TextureAsset_Fire];
        e->sprite.animations[SPRITE_ANIM_IDLE].texture_id = TextureAsset_Fire;
        e->sprite.animations[SPRITE_ANIM_IDLE].speed = 6.0f;
        e->sprite.animations[SPRITE_ANIM_IDLE].time_max = 1.0f;
        e->sprite.animations[SPRITE_ANIM_IDLE].width = tex->width;
        e->sprite.animations[SPRITE_ANIM_IDLE].height = tex->height;
        e->sprite.animations[SPRITE_ANIM_IDLE].inc = tex->width / 8;
        e->sprite.animations[SPRITE_ANIM_IDLE].directions[RIGHT_FRONT] = {0, 0, 8};

        tex = &assets.textures[TextureAsset_Fire_Shadow];
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].texture_id = TextureAsset_Fire_Shadow;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].speed = 6.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].time_max = 1.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].width = tex->width;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].height = tex->height;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].inc = tex->width / 8;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].directions[RIGHT_FRONT] = {0, 0, 8};
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}


static Entity*
add_human(v2 cell, v2 dim, v2 dir, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Player);
    if(e){
        e->color = color;
        e->pos = grid_pos_from_cell(cell, state->world_cell_size);
        e->dim = dim;
        e->velocity = {0};
        e->speed = 3.5f;
        e->bounding_box_scale = make_v2(0.25f, 0.25f);
        set_flags(&e->flags, EntityFlag_MoveWithPhys|EntityFlag_HasSprite|flags);

        e->sprite.kind = SPRITE_ANIM_IDLE;
        e->sprite.direction = RIGHT_FRONT;

        e->attack_box_max = make_v2(0.6f, 0.3f);
        e->attack_box = make_rect_size(e->pos, e->attack_box_max);

        Texture* tex = {0};
        tex = &assets.textures[TextureAsset_Human_Idle];
        e->sprite.animations[SPRITE_ANIM_IDLE].texture_id = TextureAsset_Human_Idle;
        e->sprite.animations[SPRITE_ANIM_IDLE].speed = 6.0f;
        e->sprite.animations[SPRITE_ANIM_IDLE].time_max = 1.0f;
        e->sprite.animations[SPRITE_ANIM_IDLE].width = tex->width;
        e->sprite.animations[SPRITE_ANIM_IDLE].height = tex->height;
        e->sprite.animations[SPRITE_ANIM_IDLE].inc = tex->width / 16;
        e->sprite.animations[SPRITE_ANIM_IDLE].directions[RIGHT_FRONT] = {0, 0, 16};
        e->sprite.animations[SPRITE_ANIM_IDLE].directions[LEFT_FRONT]  = {0, 1, 16};
        e->sprite.animations[SPRITE_ANIM_IDLE].directions[RIGHT_BACK]  = {0, 2, 16};
        e->sprite.animations[SPRITE_ANIM_IDLE].directions[LEFT_BACK]   = {0, 3, 16};

        tex = &assets.textures[TextureAsset_Human_Idle_Shadow];
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].texture_id = TextureAsset_Human_Idle_Shadow;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].speed = 6.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].time_max = 1.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].width = tex->width;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].height = tex->height;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].inc = tex->width / 16;
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].directions[RIGHT_FRONT] = {0, 0, 16};
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].directions[LEFT_FRONT]  = {0, 1, 16};
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].directions[RIGHT_BACK]  = {0, 2, 16};
        e->shadow_sprite.animations[SPRITE_ANIM_IDLE].directions[LEFT_BACK]   = {0, 3, 16};

        tex = &assets.textures[TextureAsset_Human_Walk];
        e->sprite.animations[SPRITE_ANIM_WALK].texture_id = TextureAsset_Human_Walk;
        e->sprite.animations[SPRITE_ANIM_WALK].speed = 18.0f;
        e->sprite.animations[SPRITE_ANIM_WALK].time_max = 1.0f;
        e->sprite.animations[SPRITE_ANIM_WALK].width = tex->width;
        e->sprite.animations[SPRITE_ANIM_WALK].height = tex->height;
        e->sprite.animations[SPRITE_ANIM_WALK].inc = tex->width / 4;
        e->sprite.animations[SPRITE_ANIM_WALK].directions[RIGHT_FRONT] = {0, 0, 4};
        e->sprite.animations[SPRITE_ANIM_WALK].directions[LEFT_FRONT]  = {0, 1, 4};
        e->sprite.animations[SPRITE_ANIM_WALK].directions[RIGHT_BACK]  = {0, 2, 4};
        e->sprite.animations[SPRITE_ANIM_WALK].directions[LEFT_BACK]   = {0, 3, 4};

        tex = &assets.textures[TextureAsset_Human_Walk_Shadow];
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].texture_id = TextureAsset_Human_Walk_Shadow;
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].speed = 18.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].time_max = 1.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].width = tex->width;
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].height = tex->height;
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].inc = tex->width / 4;
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].directions[RIGHT_FRONT] = {0, 0, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].directions[LEFT_FRONT]  = {0, 1, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].directions[RIGHT_BACK]  = {0, 2, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_WALK].directions[LEFT_BACK]   = {0, 3, 4};

        tex = &assets.textures[TextureAsset_Human_Attack];
        e->sprite.animations[SPRITE_ANIM_ATTACK].texture_id = TextureAsset_Human_Attack;
        e->sprite.animations[SPRITE_ANIM_ATTACK].speed = 20.0f;
        e->sprite.animations[SPRITE_ANIM_ATTACK].time_max = 1.0f;
        e->sprite.animations[SPRITE_ANIM_ATTACK].width = tex->width;
        e->sprite.animations[SPRITE_ANIM_ATTACK].height = tex->height;
        e->sprite.animations[SPRITE_ANIM_ATTACK].inc = tex->width / 4;
        e->sprite.animations[SPRITE_ANIM_ATTACK].do_once = true;
        e->sprite.animations[SPRITE_ANIM_ATTACK].directions[RIGHT_FRONT] = {1, 0, 4};
        e->sprite.animations[SPRITE_ANIM_ATTACK].directions[LEFT_FRONT]  = {1, 1, 4};
        e->sprite.animations[SPRITE_ANIM_ATTACK].directions[RIGHT_BACK]  = {1, 2, 4};
        e->sprite.animations[SPRITE_ANIM_ATTACK].directions[LEFT_BACK]   = {1, 3, 4};

        tex = &assets.textures[TextureAsset_Human_Attack_Shadow];
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].texture_id = TextureAsset_Human_Attack_Shadow;
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].speed = 20.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].time_max = 1.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].width = tex->width;
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].height = tex->height;
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].inc = tex->width / 4;
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].do_once = true;
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].directions[RIGHT_FRONT] = {1, 0, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].directions[LEFT_FRONT]  = {1, 1, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].directions[RIGHT_BACK]  = {1, 2, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_ATTACK].directions[LEFT_BACK]   = {1, 3, 4};

        tex = &assets.textures[TextureAsset_Human_Jump];
        e->sprite.animations[SPRITE_ANIM_JUMP].texture_id = TextureAsset_Human_Jump;
        e->sprite.animations[SPRITE_ANIM_JUMP].speed = 16.0f;
        e->sprite.animations[SPRITE_ANIM_JUMP].time_max = 1.0f;
        e->sprite.animations[SPRITE_ANIM_JUMP].width = tex->width;
        e->sprite.animations[SPRITE_ANIM_JUMP].height = tex->height;
        e->sprite.animations[SPRITE_ANIM_JUMP].inc = tex->width / 4;
        e->sprite.animations[SPRITE_ANIM_JUMP].do_once = true;
        e->sprite.animations[SPRITE_ANIM_JUMP].directions[RIGHT_FRONT] = {0, 0, 4};
        e->sprite.animations[SPRITE_ANIM_JUMP].directions[LEFT_FRONT]  = {0, 1, 4};
        e->sprite.animations[SPRITE_ANIM_JUMP].directions[RIGHT_BACK]  = {0, 2, 4};
        e->sprite.animations[SPRITE_ANIM_JUMP].directions[LEFT_BACK]   = {0, 3, 4};

        tex = &assets.textures[TextureAsset_Human_Jump_Shadow];
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].texture_id = TextureAsset_Human_Jump_Shadow;
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].speed = 1.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].time_max = 1.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].width = tex->width;
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].height = tex->height;
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].inc = tex->width / 4;
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].do_once = true;
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].directions[RIGHT_FRONT] = {0, 0, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].directions[LEFT_FRONT]  = {0, 1, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].directions[RIGHT_BACK]  = {0, 2, 4};
        e->shadow_sprite.animations[SPRITE_ANIM_JUMP].directions[LEFT_BACK]   = {0, 3, 4};

        tex = &assets.textures[TextureAsset_Human_Spin_Die];
        e->sprite.animations[SPRITE_ANIM_DIE].texture_id = TextureAsset_Human_Spin_Die;
        e->sprite.animations[SPRITE_ANIM_DIE].speed = 12.0f;
        e->sprite.animations[SPRITE_ANIM_DIE].time_max = 1.0f;
        e->sprite.animations[SPRITE_ANIM_DIE].width = tex->width;
        e->sprite.animations[SPRITE_ANIM_DIE].height = tex->height;
        e->sprite.animations[SPRITE_ANIM_DIE].inc = tex->width / 12;
        e->sprite.animations[SPRITE_ANIM_DIE].do_once = true;
        e->sprite.animations[SPRITE_ANIM_DIE].directions[RIGHT_FRONT] = {0, 0, 8};
        e->sprite.animations[SPRITE_ANIM_DIE].directions[LEFT_FRONT]  = {0, 0, 0};
        e->sprite.animations[SPRITE_ANIM_DIE].directions[RIGHT_BACK]  = {0, 0, 0};
        e->sprite.animations[SPRITE_ANIM_DIE].directions[LEFT_BACK]   = {0, 0, 0};

        tex = &assets.textures[TextureAsset_Human_Spin_Die_Shadow];
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].texture_id = TextureAsset_Human_Spin_Die_Shadow;
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].speed = 12.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].time_max = 1.0f;
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].width = tex->width;
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].height = tex->height;
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].inc = tex->width / 12;
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].do_once = true;
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].directions[RIGHT_FRONT] = {0, 0, 8};
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].directions[LEFT_FRONT]  = {0, 0, 0};
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].directions[RIGHT_BACK]  = {0, 0, 0};
        e->shadow_sprite.animations[SPRITE_ANIM_DIE].directions[LEFT_BACK]   = {0, 0, 0};
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
    m4 mat = m4_world_from_screen();
    v2 pos = m4_translate_v2(mat, make_v2(controller.mouse.x, controller.mouse.y));
    controller.mouse.world_x = pos.x;
    controller.mouse.world_y = pos.y;

    if(event.type == EventType_KEYBOARD){
        controller.mouse.wheel_dir = event.mouse_wheel_dir;
        // todo(rr): change this to
        // controller.button[event.keycode].pressed = event.key_pressed;
        // and check for repeat for held.
        if(event.key_pressed && !controller.button[event.keycode].held){
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
    // DUMB DUMB ADDED THIS
    begin_timed_function();

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
entity_sprite_update(){
    // DUMB DUMB ADDED THIS
    begin_timed_function();

    //for(s32 idx = 0; idx < array_count(state->entities); ++idx){
    //    Entity *e = state->entities + idx;
    for(s32 i = 0; i < state->active_entities_count; ++i){
        Entity *e = state->active_entities[i];

        if(has_flags(e->flags, EntityFlag_Active|EntityFlag_HasSprite)){
            if(!e->dead){
                e->sprite.direction = entity_direction_from_velocity(e); 
                e->shadow_sprite.direction = entity_direction_from_velocity(e); 
            }

            if(e->dead){
                if(e->sprite.kind != SPRITE_ANIM_DIE){
                    e->sprite.kind = SPRITE_ANIM_DIE;
                    e->sprite.direction = RIGHT_FRONT;
                    e->shadow_sprite.kind = SPRITE_ANIM_DIE;
                    e->shadow_sprite.direction = RIGHT_FRONT;

                    sprite_anim_reset(&e->sprite, e->sprite.kind);
                    sprite_anim_reset(&e->shadow_sprite, e->shadow_sprite.kind);
                }
            }
            else if(e->attacking){
                if(e->sprite.kind != SPRITE_ANIM_ATTACK){
                    e->sprite.kind = SPRITE_ANIM_ATTACK;
                    e->shadow_sprite.kind = SPRITE_ANIM_ATTACK;
                    sprite_anim_reset(&e->sprite, e->sprite.kind);
                    sprite_anim_reset(&e->shadow_sprite, e->shadow_sprite.kind);
                }
            }
            else if(e->jumping){
                if(e->sprite.kind != SPRITE_ANIM_JUMP){
                    e->sprite.kind = SPRITE_ANIM_JUMP;
                    e->shadow_sprite.kind = SPRITE_ANIM_JUMP;
                    sprite_anim_reset(&e->sprite, e->sprite.kind);
                    sprite_anim_reset(&e->shadow_sprite, e->shadow_sprite.kind);
                }
            }
            else if(entity_is_moving(e)){
                if(e->sprite.kind != SPRITE_ANIM_WALK){
                    e->sprite.kind = SPRITE_ANIM_WALK;
                    e->shadow_sprite.kind = SPRITE_ANIM_WALK;
                    sprite_anim_reset(&e->sprite, e->sprite.kind);
                    sprite_anim_reset(&e->shadow_sprite, e->shadow_sprite.kind);
                }
            }
            else{
                if(e->sprite.kind != SPRITE_ANIM_IDLE){
                    e->sprite.kind = SPRITE_ANIM_IDLE;
                    e->shadow_sprite.kind = SPRITE_ANIM_IDLE;
                    sprite_anim_reset(&e->sprite, e->sprite.kind);
                    sprite_anim_reset(&e->shadow_sprite, e->shadow_sprite.kind);
                }
            }

            bool anim_done = sprite_update(&e->sprite, (f32)clock.dt);
            sprite_update(&e->shadow_sprite, (f32)clock.dt);
            if(anim_done){
                e->attacking = false;
                e->jumping = false;
                e->moving = false; // todo: probably shouldn't be here
            }
        }
    }
}

static void
draw_entities(State* state){
    // DUMB DUMB ADDED THIS
    begin_timed_function();

    r_layer(1)
    r_render_space(Render_Space_World_Sorted)
    {
        //for(s32 idx = 0; idx < array_count(state->entities); ++idx){
        //    Entity *e = state->entities + idx;
        for(s32 i = 0; i < state->active_entities_count; ++i){
            Entity *e = state->active_entities[i];

            if(has_flags(e->flags, EntityFlag_Active)){
                Quad quad = quad_from_center(e);

                switch(e->type){
                    case EntityType_Texture:{
                        quad = rotate_quad(quad, e->deg, e->pos);

                        r_texture(e->texture_id)
                        r_z(e->pos.y)
                        {
                            draw_texture(quad, e->color);
                        }
                    } break;
                    case EntityType_Monster:{

                        Sprite_Animation_Kind kind = e->sprite.kind;
                        Sprite_Animation anim = e->sprite.animations[kind];
                        //Quad quad = quad_from_center(e);
                        r_texture(anim.texture_id)
                        r_z(e->pos.y)
                        {
                            draw_sprite(e->sprite, quad);

                            Sprite_Animation_Kind kind = e->shadow_sprite.kind;
                            Sprite_Animation anim = e->shadow_sprite.animations[kind];
                            r_texture(anim.texture_id)
                            {
                                draw_sprite(e->shadow_sprite, quad);
                            }
                        }

                        if(e->selected){
                            if(e->active_command){
                                r_render_space(Render_Space_World){
                                    draw_quad(e->active_command->clicked_at, make_v2(0.25f, 0.25f), RED);
                                }
                            }

                            u32 read_idx = e->commands_read;
                            while(read_idx != e->commands_write){
                                EntityCommand* c = entity_commands_read(e, read_idx);
                                read_idx++;

                                r_render_space(Render_Space_World){
                                    draw_quad(c->clicked_at, make_v2(0.1f, 0.1f), RED);
                                }
                            }
                        }
                    } break;
                    case EntityType_Fire:{
                        Sprite_Animation_Kind kind = e->sprite.kind;
                        Sprite_Animation anim = e->sprite.animations[kind];
                        //Quad quad = quad_from_center(e);

                        r_texture(anim.texture_id)
                        r_z(e->pos.y)
                        {
                            draw_sprite(e->sprite, quad);

                            Sprite_Animation_Kind kind = e->shadow_sprite.kind;
                            Sprite_Animation anim = e->shadow_sprite.animations[kind];
                            r_texture(anim.texture_id)
                            r_z(e->pos.y + 0.1f)
                            {
                                draw_sprite(e->shadow_sprite, quad);
                            }
                        }
                    } break;
                    case EntityType_Skeleton1:{
                        quad = rotate_quad(quad, e->deg, e->pos);

                        r_texture(e->texture_id)
                        r_z(e->pos.y)
                        {
                            draw_texture(quad, e->color);
                        }



                        if(e->selected){
                            if(e->active_command){
                                //r_set_z(e->active_command->clicked_at.y);
                                draw_quad(e->active_command->clicked_at, make_v2(0.25f, 0.25f), RED);
                            }

                            u32 read_idx = e->commands_read;
                            while(read_idx != e->commands_write){
                                EntityCommand* c = entity_commands_read(e, read_idx);
                                read_idx++;

                                //r_set_z(c->clicked_at.y);
                                draw_quad(c->clicked_at, make_v2(0.1f, 0.1f), RED);
                                // no
                                //draw_line(e->pos, screen_space, 0.1f, ORANGE);
                            }
                        }

                        //if(state->scene_state == SceneState_Editor){
                        //    if(state->show_entity_info){
                        //        v2 pos = e->pos;
                        //        ui_set_pos(pos.x + 20, pos.y);
                        //        ui_set_size(ui_size_children(0), ui_size_children(0));
                        //        ui_set_border_thickness(10);
                        //        ui_set_background_color(DEFAULT);

                        //        String8 box_name = str8_formatted(ts->frame_arena, "skelebox##%i", idx);
                        //        ui_begin_panel(box_name, ui_floating_panel_world);

                        //        String8 fmt_str;
                        //        //ui_font(font);
                        //        ui_size(ui_size_text(0), ui_size_text(0))
                        //        ui_text_color(LIGHT_GRAY)
                        //        {
                        //            fmt_str = str8_formatted(ts->frame_arena, "idx: %i", e->index);
                        //            ui_label(fmt_str);
                        //            fmt_str = str8_formatted(ts->frame_arena, "pos: %f, %f", e->pos.x, e->pos.y);
                        //            ui_label(fmt_str);
                        //        }
                        //        ui_end_panel();
                        //    }
                        //}
                    } break;
                    case EntityType_Quad:{
                        quad = rotate_quad(quad, e->deg, e->pos);
                        r_z(e->pos.y)
                        {
                            draw_quad(quad, e->color);
                        }
                    } break;

                    case EntityType_Structure:{
                        if(e->structure_type == StructureType_Castle){
                            r_texture(e->texture_id)
                            r_z(e->pos.y)
                            {
                                draw_texture(quad, e->color);
                            }

                            if(e->selected){
                                //r_set_z(e->pos.y);
                                r_render_space(Render_Space_World){
                                    draw_line(e->pos, e->rallypoint, 0.1f, RED);
                                }
                            }

                            // no
                            //r_set_font(state->font_id);
                            //String8 fmt_str = str8_format(ts->frame_arena, "(%f, %f)", e->rallypoint.x, e->rallypoint.y);
                            //draw_text(fmt_str, v2_screen_from_world(e->rallypoint), GREEN);
                        }
                    } break;
                }
            }
        }

        // draw player
        Sprite_Animation_Kind kind = player->sprite.kind;
        Sprite_Animation anim = player->sprite.animations[kind];
        Quad quad = quad_from_center(player);
        r_texture(anim.texture_id)
        r_z(player->pos.y)
        {
            draw_sprite(player->sprite, quad);

            Sprite_Animation_Kind kind = player->shadow_sprite.kind;
            Sprite_Animation anim = player->shadow_sprite.animations[kind];
            r_texture(anim.texture_id)
            {
                draw_sprite(player->shadow_sprite, quad);
            }
        }

        if(state->scene_state == SceneState_Editor){
            draw_bounding_box(player->attack_box, 0.05f, RED);
            draw_bounding_box(player->bounding_box, 0.05f, RED);
        }
    }
}

static void
ui_editor(void){
    // DUMB DUMB ADDED THIS
    begin_timed_function();

    r_render_space(Render_Space_Screen)
    {

        ui_set_pos(20, 200);
        ui_set_size(ui_size_children(0), ui_size_children(0));
        ui_set_border_thickness(10);
        ui_set_background_color(DEFAULT);
        ui_begin_panel(str8_literal("tile_panel##2"), ui_floating_panel);

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

        ui_set_pos(SCREEN_WIDTH - 450, 10);
        ui_set_size(ui_size_children(0), ui_size_children(0));
        ui_set_border_thickness(10);
        ui_set_background_color(DEFAULT);
        ui_begin_panel(str8_literal("info_panel##info_panel"), ui_floating_panel);

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

            fmt = str8_format(ts->frame_arena, "mouse cell: %.2f, %.2f", world_mouse.x, world_mouse.y);
            ui_label(fmt);
            ui_spacer(10);

            fmt = str8_format(ts->frame_arena, "cam zoom: %f", camera.size);
            ui_label(fmt);

            fmt = str8_format(ts->frame_arena, "cam pos: (%.2f, %.2f)", camera.x, camera.y);
            ui_label(fmt);
            ui_spacer(10);

            fmt = str8_format(ts->frame_arena, "Render Batches Count: %i", render_batches_count);
            if(controller_button_pressed(KeyCode_M)){
                debug_break();
            }
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

        ui_set_pos(200, 200);
        ui_set_size(ui_size_children(0), ui_size_children(0));
        ui_set_border_thickness(5);
        ui_set_background_color(DEFAULT);
        ui_begin_panel(str8_literal("grid_panel##3"), ui_floating_panel);

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
        ui_begin_panel(str8_literal("cell_size##5"), ui_floating_panel);

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
}

static void
ui_castle(void){
    // DUMB DUMB ADDED THIS
    begin_timed_function();

    ui_layout_axis(Axis_X)
    {
        ui_set_border_thickness(10);
        ui_set_background_color(DEFAULT);
        ui_set_pos(20, window.height - 100);
        ui_set_size(ui_size_children(0), ui_size_children(0));
        ui_begin_panel(str8_literal("castle_panel##4"), ui_floating_panel);

        ui_size(ui_size_pixel(100, 0), ui_size_pixel(50, 0))
        ui_background_color(DARK_GRAY)
        {
            if(ui_button(str8_literal("monster")).pressed_left){
                Entity* castle = state->entities_selected[0];

                v2 dir = direction_v2(castle->pos, castle->rallypoint);
                Entity* e = add_monster(grid_cell_from_pos(castle->pos, state->world_cell_size), make_v2(2, 2), dir);
                e->origin = castle;
                e->rallypoint = castle->rallypoint;
                e->rallypoint_cell = castle->rallypoint_cell;
                entity_commands_move(e, e->rallypoint, e->rallypoint);
            }

            ui_spacer(10);
            if(ui_button(str8_literal("monster - 15")).pressed_left){
                for(s32 i=0; i < 15; ++i){
                    Entity* castle = state->entities_selected[0];

                    v2 dir = direction_v2(castle->pos, castle->rallypoint);
                    Entity* e = add_monster(grid_cell_from_pos(castle->pos, state->world_cell_size), make_v2(2, 2), dir);
                    e->origin = castle;
                    e->rallypoint = castle->rallypoint;
                    e->rallypoint_cell = castle->rallypoint_cell;
                    entity_commands_move(e, e->rallypoint, e->rallypoint);
                }
            }
            ui_spacer(10);
            if(ui_button(str8_literal("monster - 50")).pressed_left){
                for(s32 i=0; i < 50; ++i){
                    Entity* castle = state->entities_selected[0];

                    v2 dir = direction_v2(castle->pos, castle->rallypoint);
                    Entity* e = add_monster(grid_cell_from_pos(castle->pos, state->world_cell_size), make_v2(2, 2), dir);
                    e->origin = castle;
                    e->rallypoint = castle->rallypoint;
                    e->rallypoint_cell = castle->rallypoint_cell;
                    entity_commands_move(e, e->rallypoint, e->rallypoint);
                }
            }
        }
    }

    ui_end_panel();
}

static void
draw_grid(f32 size, RGBA color){
    // DUMB DUMB ADDED THIS
    begin_timed_function();

    r_render_space(Render_Space_World){
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
}

static void
draw_world_terrain(void){
    // DUMB DUMB ADDED THIS
    begin_timed_function();
    // note: only draws terrain that is within that camera space.

    r_render_space(Render_Space_World_Terrain)
    r_layer(0)
    r_z(0)
    {

        v2 low;
        v2 high;
        {
            // DUMB DUMB ADDED THIS
            begin_timed_scope("terrain visible bounds");

            low  = make_v2(floor_f32(camera.p3.x/state->world_cell_size) * state->world_cell_size,
                           floor_f32(camera.p3.y/state->world_cell_size) * state->world_cell_size);
            high = make_v2(ceil_f32(camera.p1.x/state->world_cell_size) * state->world_cell_size,
                           ceil_f32(camera.p1.y/state->world_cell_size) * state->world_cell_size);
        }


        //for(s32 i=1; i < TextureAsset_Count; ++i){
        {
            // DUMB DUMB ADDED THIS
            begin_timed_scope("terrain texture pass");

            f32 y = high.y;
            while(y >= low.y){

                f32 x = low.x;
                while(x < high.x){

                    if(x >= 0 && (x/state->world_cell_size) < state->world_width_in_cells){
                        if(y >= 0 && (y/state->world_cell_size) < state->world_height_in_cells){
                            v2 cell = make_v2(x, y);

                            s32 idx = (s32)(((y/state->world_cell_size) * state->world_width_in_cells) + (x/state->world_cell_size));
                            s32 cell_tex = state->world_grid[idx];
                            if(cell_tex > 0){
                                r_texture(cell_tex)
                                {
                                    Rect tex_rect = make_rect_size(cell, make_v2(state->world_cell_size, state->world_cell_size));
                                    draw_texture(tex_rect);
                                }
                            }
                        }
                    }

                    x += state->world_cell_size;
                }

                y -= state->world_cell_size;
            }
        }
    }
}

static bool
mouse_in_bounding_box(Entity* e){
    //Rect rect = rect_from_entity(e);

    Rect rect = rect_from_center(e);
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
    // DUMB DUMB ADDED THIS
    begin_timed_function();

    build_path = os_application_path(global_arena);
    fonts_path = str8_path_append(global_arena, build_path, str8_literal("fonts"));
    shaders_path = str8_path_append(global_arena, build_path, str8_literal("shaders"));
    saves_path = str8_path_append(global_arena, build_path, str8_literal("saves"));
    sprites_path = str8_path_append(global_arena, build_path, str8_literal("sprites"));
    sounds_path = str8_path_append(global_arena, build_path, str8_literal("sounds"));
}

static void
init_memory(u64 permanent, u64 transient){
    // DUMB DUMB ADDED THIS
    begin_timed_function();

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
        .lpfnWndProc = win_message_callback,
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
    // DUMB DUMB ADDED THIS
    begin_timed_function();

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
    os_file_write(&file, arena->base, arena->at);

    os_file_close(&file);
    end_scratch(scratch);
    arena_free(ts->data_arena);
}

static void
deserialize_world(String8 world){
    // DUMB DUMB ADDED THIS
    begin_timed_function();

    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, saves_path, world);
    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    if(!file.size){
        os_file_close(&file);
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
    os_file_close(&file);
    end_scratch(scratch);
}

static void
deserialize_state(void){
    // DUMB DUMB ADDED THIS
    begin_timed_function();

    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, build_path, str8_literal("config.g"));
    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    String8 data = os_file_read(ts->data_arena, file);

    String8* ptr = &data;
    while(ptr->count){
        String8 line = str8_next_line(ptr);
        String8List list = str8_split(scratch.arena, line, ':');

        String8 left = list.first->string;
        String8 right = list.last->string;
        if(str8_compare(left, str8_lit("current_world"))){
            memcpy(state->current_world.str, right.str, right.count);
            state->current_world.count = right.count;
        }
    }

    arena_free(ts->data_arena);
    os_file_close(&file);
    end_scratch(scratch);
}

static void
serialize_state(void){
    // DUMB DUMB ADDED THIS
    begin_timed_function();

    serialize_world(state->current_world);

    Arena* arena = ts->data_arena;

    arena->at += snprintf((char*)arena->base + arena->at,
                          arena->size - arena->at,
                          "current_world:%s\n", state->current_world.str);

    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, build_path, str8_literal("config.g"));
    File file = os_file_open(full_path, GENERIC_WRITE, CREATE_ALWAYS);
    os_file_write(&file, arena->base, arena->at);

    arena_free(ts->data_arena);
    os_file_close(&file);
    end_scratch(scratch);
}

// incomplete
static void
partition_entities_in_bins(){
    // DUMB DUMB ADDED THIS
    begin_timed_function();

    cell_generation += 1;
    if(cell_generation == 0){ // if we overflow, reset generations.
        memset(state->cells, 0, sizeof(state->cells));
        cell_generation = 1;
    }

    //for(s32 i=0; i < array_count(state->entities); ++i){
    //    Entity* e = state->entities + i;
    for(s32 i = 0; i < state->active_entities_count; ++i){
        Entity *e = state->active_entities[i];
        if(!has_flags(e->flags, EntityFlag_Active)) continue;
        if(!has_flags(e->flags, EntityFlag_MoveWithPhys)) continue;

        v2 cell_coords = grid_cell_from_pos(e->pos, state->flocking_cell_size);
        if(!grid_cell_coords_in_bounds(cell_coords)) continue;

        Cell* cell = state->cells + ((s32)cell_coords.x + (WORLD_WIDTH_IN_TILES_MAX * (s32)cell_coords.y));
        if(cell->generation != cell_generation){
            cell->generation = cell_generation;

            BinNode* bin = push_struct_zero(ts->bin_arena, BinNode); // Will change to default zero.
            bin->cap = BIN_SIZE;
            cell->bin = bin;
            cell->bin_count = 0;
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
win_message_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param){
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
            //m4 mat = m4_world_from_screen();
            //v2 pos = m4_translate_v2(mat, make_v2(event.mouse_x, event.mouse_y));
            //event.mouse_world_x = pos.x;
            //event.mouse_world_y = pos.y;

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


    init_paths(global_arena);
    init_memory(MB(500), GB(8));
    init_clock(&clock);

    //random_seed(0, 1);
    random_seed(clock.get_os_timer(), clock.get_cpu_timer());

    // todo: define a wasapi struct here and pass it in for clarity
    audio_init(2, 48000, 32);
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
        ts->batch_arena    = push_arena(&ts->arena, GB(4));
        ts->data_arena     = push_arena(&ts->arena, KB(1024));
        ts->bin_arena      = push_arena(&ts->arena, MB(512));

        show_cursor(true);
        assets_load(ts->asset_arena);

        state->world_width_in_cells = 10;
        state->world_height_in_cells = 10;
        state->world_cell_size = 1;
        state->flocking_cell_size = 1;
        state->pathing_cell_size = 25;
        state->show_world_cells = true;
        state->show_flocking_cells = true;
        state->show_pathing_cells = true;

        ui_init(&state->arena, &window, &controller, &assets);
        draw_init(&state->arena, ts->batch_arena, &assets);

        //state->font = &assets.fonts[FontAsset_Arial];
        state->font_id = FontAsset_Arial;
        font1 = &assets.fonts[FontAsset_Arial1];
        font2 = &assets.fonts[FontAsset_Arial2];
        font3 = &assets.fonts[FontAsset_Arial3];
        font4 = &assets.fonts[FontAsset_Arial4];
        font5 = &assets.fonts[FontAsset_Arial5];
        font6 = &assets.fonts[FontAsset_Arial6];

        // setup free entities array in reverse order
        entities_clear();

        // load default level
        state->current_world.str = push_array(global_arena, u8, 1024);
        deserialize_state();
        deserialize_world(state->current_world);

        // load castle
        state->castle_cell = make_v2(198, 98);
        state->castle = add_castle(TextureAsset_Castle1, state->castle_cell, make_v2(2, 2));
        state->player = add_human(make_v2(200, 100), make_v2(2, 2));
        player = state->player;

        add_monster(make_v2(201, 100), make_v2(2, 2));
        add_monster(make_v2(201, 100), make_v2(2, 2));
        add_monster(make_v2(201, 100), make_v2(2, 2));
        add_monster(make_v2(201, 100), make_v2(2, 2));
        add_monster(make_v2(201, 100), make_v2(2, 2));
        add_monster(make_v2(201, 100), make_v2(2, 2));
        add_monster(make_v2(201, 100), make_v2(2, 2));
        add_monster(make_v2(201, 100), make_v2(2, 2));
        add_monster(make_v2(201, 100), make_v2(2, 2));
        add_monster(make_v2(201, 100), make_v2(2, 2));
        add_monster(make_v2(201, 100), make_v2(2, 2));
        add_monster(make_v2(201, 100), make_v2(2, 2));
        add_monster(make_v2(201, 100), make_v2(2, 2));
        add_monster(make_v2(201, 100), make_v2(2, 2));

        add_fire(make_v2(195, 110), make_v2(1.25, 1.25));
        add_monster(make_v2(195, 111), make_v2(2, 2));
        add_monster(make_v2(196, 110), make_v2(2, 2));
        add_monster(make_v2(194, 110), make_v2(2, 2));
        add_monster(make_v2(195, 109), make_v2(2, 2));

        init_spawner();

        state->scene_state = SceneState_Game;
        init_camera_2d(&camera, make_v2(200, 100), 10);

        Arena* arena = push_arena(&state->arena, MB(8));
        init_console(arena, &camera, &window, &assets);

        memory.initialized = true;
    }

    should_quit = false;
    while(!should_quit){
        // DUMB DUMB ADDED THIS
        begin_timed_scope("while(!should_quit)");

        ui_begin();

        u64 now_ticks = clock.get_os_timer();
        f64 frame_time = clock.get_seconds_elapsed(now_ticks, last_ticks);
        //MSPF = 1000 / 1000 / ((f64)clock.frequency / (f64)(now_ticks - last_ticks));
        MSPF = clock.get_ms_elapsed(now_ticks, last_ticks);
        last_ticks = now_ticks;

        {
            // DUMB DUMB ADDED THIS
            begin_timed_scope("windows message pump");

            MSG message;
            while(PeekMessageW(&message, window.handle, 0, 0, PM_REMOVE)){
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }

        {
            // DUMB DUMB ADDED THIS
            begin_timed_scope("dispatch input events");

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
        }

        //state->active_entities_count = 0;
        //for(s32 i=0; i < array_count(state->entities); ++i){
        //    Entity* e = state->entities + i;
        //    if(has_flags(e->flags, EntityFlag_Active)){
        //        state->active_entities[state->active_entities_count] = e;
        //        state->active_entities_count++;
        //    }
        //}

        {
            // DUMB DUMB ADDED THIS
            begin_timed_scope("partition phase");

            partition_entities_in_bins();
        }

        // note todo fixme: consumes input so needs to be here, input needs to be 1 frame later
        // so that ui drawing doesn't have to happen before simulation and stuff like that
        if(state->scene_state == SceneState_Editor){
            // DUMB DUMB ADDED THIS
            begin_timed_scope("editor ui phase");

            ui_editor();
        }

        // SIM GAME HERE
        simulations = 0;
        accumulator += frame_time;
        while(accumulator >= clock.dt){
            // DUMB DUMB ADDED THIS
            begin_timed_scope("sim loop");
            sim_game();

            accumulator -= clock.dt;
            time_elapsed += clock.dt;
            simulations++;

        }

        if(controller_button_pressed(KeyCode_F1)){
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
            // DUMB DUMB ADDED THIS
            begin_timed_scope("entity hover and selection");

            if(controller_button_pressed(MOUSE_BUTTON_LEFT, false)){
                state->selection_mouse_record = controller.mouse.world_pos;
                state->selecting = true;
            }
            if(state->selecting && controller_button_held(MOUSE_BUTTON_LEFT)){
                min.x = state->selection_mouse_record.x <= controller.mouse.world_x ? state->selection_mouse_record.x : controller.mouse.world_x;
                min.y = state->selection_mouse_record.y <= controller.mouse.world_y ? state->selection_mouse_record.y : controller.mouse.world_y;
                max.x = state->selection_mouse_record.x >  controller.mouse.world_x ? state->selection_mouse_record.x : controller.mouse.world_x;
                max.y = state->selection_mouse_record.y >  controller.mouse.world_y ? state->selection_mouse_record.y : controller.mouse.world_y;
                state->selection_rect = make_rect(min, max);
            }
            if(controller_button_released(MOUSE_BUTTON_LEFT)){
                    state->selection_mouse_record = {0};
                    state->selecting = false;

                    s32 count = 0;
                    bool selected_new_units = false;
                    //for(s32 i=0; i < array_count(state->entities); ++i){
                    //    Entity* e = state->entities + i;
                    for(s32 i = 0; i < state->active_entities_count; ++i){
                        Entity *e = state->active_entities[i];
                        if(e == player) continue;
                        if(!has_flags(e->flags, EntityFlag_Active)) continue;

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
                //for(s32 idx = 0; idx < array_count(state->entities); ++idx){
                //    Entity *e = state->entities + idx;
                for(s32 i = 0; i < state->active_entities_count; ++i){
                    Entity *e = state->active_entities[i];
                    if(!has_flags(e->flags, EntityFlag_Active)) continue;

                    if(mouse_in_bounding_box(e)){
                        state->entity_hovered = e;
                        found = true;
                    }
                }
                if(!found){
                    state->entity_hovered = 0;
                }

                // single select
                if(state->entity_hovered != player){
                    if(state->entity_hovered && !controller.ctrl_pressed && controller_button_pressed(MOUSE_BUTTON_LEFT)){
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

            }

            {
                // DUMB DUMB ADDED THIS
                begin_timed_scope("selected entity commands");

                // Calc center position of selection.
                v2 average_position = make_v2(0, 0);
                for(s32 i=0; i < state->entities_selected_count; ++i){
                    Entity* e = state->entities_selected[i];
                    average_position.x += e->pos.x;
                    average_position.y += e->pos.y;
                }
                state->entities_selected_center.x = average_position.x/(f32)state->entities_selected_count;
                state->entities_selected_center.y = average_position.y/(f32)state->entities_selected_count;

                v2 world_mouse = controller.mouse.world_pos;
                for(s32 i=0; i < state->entities_selected_count; ++i){
                    Entity* e = state->entities_selected[i];

                    switch(e->structure_type){
                        case StructureType_Castle:{
                            if(!controller.ctrl_pressed && controller_button_pressed(MOUSE_BUTTON_RIGHT)){
                                e->rallypoint = world_mouse;
                                e->rallypoint_cell = grid_cell_from_pos(world_mouse, state->world_cell_size);
                            }
                            ui_castle();
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
                        case EntityType_Monster:{
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

            {
                // DUMB DUMB ADDED THIS
                begin_timed_scope("camera update");

                camera_2d_update(&camera, window.aspect_ratio);
            }
            {
                // DUMB DUMB ADDED THIS
                begin_timed_scope("audio phase");

                audio_update();
            }

        // rendering
        {
            // DUMB DUMB ADDED THIS
            begin_timed_scope("rendering");
            //render_batches_reset();

            //arena_free(ts->batch_arena);
            {
                // DUMB DUMB ADDED THIS
                begin_timed_scope("    draw_world_terrain");
                draw_world_terrain();
            }

            r_render_space(Render_Space_World){
                //static void draw_quad(Quad quad, RGBA color = WHITE);

                Rect rect1 = make_rect_size(state->spawner.spawn_point[0].pos, make_v2(state->spawner.render_size, state->spawner.render_size));
                Rect rect2 = make_rect_size(state->spawner.spawn_point[1].pos, make_v2(state->spawner.render_size, state->spawner.render_size));
                Rect rect3 = make_rect_size(state->spawner.spawn_point[2].pos, make_v2(state->spawner.render_size, state->spawner.render_size));

                Rect rect4 = make_rect_size(state->spawner.spawn_point[3].pos, make_v2(state->spawner.render_size, state->spawner.render_size));
                Rect rect5 = make_rect_size(state->spawner.spawn_point[4].pos, make_v2(state->spawner.render_size, state->spawner.render_size));
                Rect rect6 = make_rect_size(state->spawner.spawn_point[5].pos, make_v2(state->spawner.render_size, state->spawner.render_size));
                draw_quad(rect1);
                draw_quad(rect2);
                draw_quad(rect3);
                draw_quad(rect4);
                draw_quad(rect5);
                draw_quad(rect6);
            }

            {
                // DUMB DUMB ADDED THIS
                begin_timed_scope("    draw_grid");
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
            }

            {
                // DUMB DUMB ADDED THIS
                begin_timed_scope("    draw_entities");
                draw_entities(state);
            }

            {
                // DUMB DUMB ADDED THIS
                begin_timed_scope("    draw_selection_recs");
                // draw selection rects
                f32 max_x = 0;
                f32 max_y = 0;
                f32 min_x = 1000;
                f32 min_y = 1000;
                if(state->entities_selected_count){

                    r_render_space(Render_Space_World)
                    {
                        if(state->entities_selected_count == 1){
                            Entity* e = state->entities_selected[0];
                            //Rect rect = rect_from_entity(e);
                            Rect rect = rect_from_center(e);
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

            if(state->selecting && !state->dragging_world){
                r_render_space(Render_Space_World)
                {
                    draw_bounding_box(state->selection_rect, 0.1f, RED);
                }
            }
            if(state->terrain_selected){
                r_render_space(Render_Space_World)
                r_texture(state->terrain_selected_id)
                {
                    draw_texture(controller.mouse.pos, make_v2(50, 50));
                    draw_bounding_box(make_rect_size(controller.mouse.pos, make_v2(50, 50)), 0.1f, RED);
                }
            }

            ui_end();

            r_render_space(Render_Space_Screen)
            r_font(FontAsset_Arial1)
            {
                String8 text = str8_formatted(ts->frame_arena, "FPS: %.2f", FPS);
                draw_text(text, make_v2(window.width-130, 20), RED);
                text = str8_formatted(ts->frame_arena, "MSPF: %.2f", MSPF);
                draw_text(text, make_v2(window.width-130, 40), RED);
            }

            if(state->scene_state == SceneState_Editor){
                r_render_space(Render_Space_Screen)
                r_texture(TextureAsset_Castle1)
                {
                    Rect rr = make_rect(make_v2(0, 0), make_v2(100, 100));
                    draw_texture(rr);
                }

                r_render_space(Render_Space_Screen){
                    Rect rr = make_rect_size(make_v2(110, 0), make_v2(100, 100));
                    draw_quad(rr, RED);
                }
            }

            console_draw();

            // draw border
            r_render_space(Render_Space_World)
            {
                v2 p00 = make_v2(camera.p0.x + 0.1, camera.p0.y - 0.1);
                v2 p11 = make_v2(camera.p1.x - 0.1, camera.p1.y - 0.1);
                v2 p22 = make_v2(camera.p2.x - 0.1, camera.p2.y + 0.1);
                v2 p33 = make_v2(camera.p3.x + 0.1, camera.p3.y + 0.1);
                draw_line(p00, p11, 0.2f, RED);
                draw_line(p11, p22, 0.2f, RED);
                draw_line(p22, p33, 0.2f, RED);
                draw_line(p33, p00, 0.2f, RED);
            }

            {
                // DUMB DUMB ADDED THIS
                begin_timed_scope("    draw_commands");
                d3d_clear_color(BACKGROUND_COLOR);

                draw_render_commands();
                //draw_render_batches();

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
    assets_release();
    audio_release();
    d3d_release();
    end_profiler();

    return(0);
}


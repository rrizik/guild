#ifndef COMMAND_C
#define COMMAND_C

static void
init_console_commands(void){
    //add_command(str8_literal("saves"), 1, 1, command_save);
    add_command(str8_literal("add"),  str8_literal("add: mathematicall add"), 2, 2, command_add);
    //add_command(str8_literal("save"), str8_literal("save: save level"), 1, 1, command_save);
    //add_command(str8_literal("load"), str8_literal("load: load level"), 1, 1, command_load);
    add_command(str8_literal("exit"), str8_literal("exit: quit/exit game"), 0, 0, command_exit);
    add_command(str8_literal("quit"), str8_literal("quit: quit/exit game"), 0, 0, command_exit);
    add_command(str8_literal("help"), str8_literal("help: lists all commands"), 0, 1, command_help);
    add_command(str8_literal("goto"), str8_literal("goto: set camera position"), 2, 2, command_go_to);
    add_command(str8_literal("new_world"), str8_literal("new_world: generate new world"), 3, 3, command_new_world);
    add_command(str8_literal("save_world"), str8_literal("save_world: saves the world to a file"), 0, 1, command_save_world);
    add_command(str8_literal("load_world"), str8_literal("new_world: generate new world"), 1, 1, command_load_world);
}

static void
add_command(String8 name, String8 desc, u32 min, u32 max, Proc* proc){
    CommandInfo* command = commands + commands_count;
    ++commands_count;

    command->name = name;
    command->description = desc;
    command->min_args = min;
    command->max_args = max;
    command->proc = proc;
}

static void
command_help(String8* args){
    if(command_args_count){
        for(s32 i=0; i < commands_count; ++i){
            CommandInfo command = commands[i];
            if(str8_compare(*args, command.name)){
                console.output_history[console.output_history_count++] = command.description;
            }
        }
    }
    else{
        for(s32 i=0; i < commands_count; ++i){
            CommandInfo command = commands[i];
            console.output_history[console.output_history_count++] = command.name;
        }
    }
}

static void
command_exit(String8* args){
    console.output_history[console.output_history_count++] = str8_literal("Exiting!");
    Event event = {0};
    event.type = QUIT;
    events_add(&events, event);
}

static void
command_new_world(String8* args){
    s32 width  = atoi((char const*)(args->str));
    s32 height = atoi((char const*)(args + 1)->str);
    String8 name = *(args + 2);
    bool bad_input = false;
    if(width > WORLD_WIDTH_MAX){
        String8 str = str8_formatted(console.arena, "width(%i) must be <= WORLD_WIDTH_MAX(%i)", width, WORLD_WIDTH_MAX);
        console.output_history[console.output_history_count++] = str;
        bad_input = true;
    }
    if(height > WORLD_HEIGHT_MAX){
        String8 str = str8_formatted(console.arena, "height(%i) must be <= WORLD_HEIGHT_MAX(%i)", height, WORLD_WIDTH_MAX);
        console.output_history[console.output_history_count++] = str;
        bad_input = true;
    }
    if(bad_input){
        return;
    }

    state->world_width = (f32)width;
    state->world_height = (f32)height;
    state->current_world = name;
    generate_new_world(state->world_width, state->world_height);
}

static void
command_save_world(String8* args){
    if(command_args_count == 0){
        if(state->current_world.size){
            serialize_world(state->current_world);
            console.output_history[console.output_history_count++] = str8_formatted(console.arena, "saving to file: %s", state->current_world.str);
        }
        else{
            console.output_history[console.output_history_count++] = str8_formatted(console.arena, "no state->current_world set: %s", state->current_world.str);
            return;
        }
    }
    else{
        state->current_world = *args;
        serialize_world(state->current_world);
        console.output_history[console.output_history_count++] = str8_formatted(console.arena, "saving to file: %s", args->str);
    }
}

static void
command_load_world(String8* args){
    state->current_world = *args;
    deserialize_world(state->current_world);
    console.output_history[console.output_history_count++] = str8_formatted(console.arena, "loading from file: %s", args->str);
}

static void
command_go_to(String8* args){
    s32 x = atoi((char const*)(args->str));
    s32 y = atoi((char const*)(args + 1)->str);
    console.camera->pos.x = (f32)x;
    console.camera->pos.y = (f32)y;

    String8 str = str8_formatted(console.arena, "move camera to position (%i, %i)", x, y);
    console.output_history[console.output_history_count++] = str;
}

//static void
//command_load(String8* args){
//    console.output_history[console.output_history_count++] = str8_formatted(console.arena, "loading from file: %s", args->str);
//    //deserialize_data(*args);
//}
//
//static void
//command_save(String8* args){
//    console.output_history[console.output_history_count++] = str8_formatted(console.arena, "saving to file: %s", args->str);
//    //serialize_data(*args);
//    //console_store_output(str8_formatted(console.arena, "saving to file: %s", args->str));
//}

static void
command_add(String8* args){
    s32 left = atoi((char const*)(args->str));
    s32 right = atoi((char const*)(args + 1)->str);
    s32 value = left + right;
    String8 result = str8_formatted(console.arena, "Result: %i", value);
    console.output_history[console.output_history_count++] = result;
}

// todo: redo this entire function and reconsider how you are passing scratch arena
static void
command_saves(String8* args){
    ScratchArena scratch = begin_scratch();

    String8Node files = {0};
    files.next = &files;
    files.prev = &files;
    os_dir_files(scratch.arena, &files, str8_literal("saves"));
    dll_pop_front(&files);
    dll_pop_front(&files);

    for(String8Node* file = files.next; file != &files; file = file->next){
        console.output_history[console.output_history_count++] = file->str;
    }
    end_scratch(scratch);
}

static String8
push_str8(Arena* arena, String8 value){
    u8* str = push_array_zero(arena, u8, value.count + 1);
    memcpy(str, value.data, value.count);
    String8 result = {str, value.count};
    return(result);
}

static s32
command_parse_args(String8 line){
    command_args_count = 0;

    s32 args_count = 0;
    String8 remaining = line;
    while(remaining.size){
        remaining = str8_eat_spaces(remaining);
        if(!remaining.size){ break; }

        s64 index = byte_index_from_left(remaining, ' ');
        if(index != -1){
            String8 left_arg = str8_split_left(remaining, (u64)index);
            String8 arg = push_str8(console.arena, left_arg);
            command_args[command_args_count++] = arg;
            remaining = str8_advance(remaining, (u64)index);
        }
        else{
            String8 arg = push_str8(console.arena, remaining);
            command_args[command_args_count++] = arg;
            remaining = str8_advance(remaining, remaining.count);
        }

        args_count++;
    }
    return(args_count);
}

static void
run_command(String8 command){
    // run command proc
    bool found = false;
    for(s32 i=0; i < commands_count; ++i){
        CommandInfo c = commands[i];
        if(str8_compare(command, c.name)){
            found = true;
            if(c.min_args > command_args_count){
                console.output_history[console.output_history_count++] = str8_formatted(console.arena, "Argument count less than min - Expected %i - Got: %i", c.min_args, command_args_count);
                break;
            }
            if(c.max_args < command_args_count){
                console.output_history[console.output_history_count++] = str8_formatted(console.arena, "Argument count greater than max - Expected %i - Got: %i", c.max_args, command_args_count);
                break;
            }

            c.proc(command_args);
            break;
        }
    }

    // output unkown command
    if(!found){
        char buffer[1024];
        snprintf(buffer, command.size + 1, "%s", command.str);
        console.output_history[console.output_history_count++] = str8_formatted(console.arena, "Unkown command: %s", buffer);
    }
}

#endif

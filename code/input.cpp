#ifndef INPUT_C
#define INPUT_C

static void
clear_controller_held(void){
    for(s32 i=0; i < KeyCode_Count; ++i){
        controller.button[i].held = false;
    }
}

static void
clear_controller_pressed(void){
    for(s32 i=0; i < KeyCode_Count; ++i){
        controller.button[i].pressed = false;
    }
    controller.mouse.dx = 0;
    controller.mouse.dy = 0;
    controller.mouse.wheel_dir = 0;

    controller.mouse.edge_left   = false;
    controller.mouse.edge_right  = false;
    controller.mouse.edge_top    = false;
    controller.mouse.edge_bottom = false;
}

static bool
controller_button_pressed(KeyCode key, bool consume){
    bool result = controller.button[key].pressed;
    if(consume){
        controller.button[key].pressed = false;
    }

    return(result);
}

static bool
controller_button_held(KeyCode key, bool consume){
    bool result = controller.button[key].held;
    if(consume){
        controller.button[key].held = false;
    }

    return(result);
}

static void
init_events(Events* events){
    events->size = array_count(events->e);
}

static u32
events_count(Events* events){
    u32 result = events->write - events->read;
    return(result);
}

static bool
events_full(Events* events){
    bool result = (events_count(events) == events->size);
    return(result);
}

static bool
events_empty(Events* events){
    bool result = (events->write == events->read);
    return(result);
}

static u32
mask(Events* events, u32 idx){
    u32 result = idx & (events->size - 1);
    return(result);
}

static void
events_add(Events* events, Event event){
    assert(!events_full(events));

    u32 masked_idx = mask(events, events->write++);
    events->e[masked_idx] = event;
}

static Event
events_next(Events* events){
    assert(!events_empty(events));

    u32 masked_idx = mask(events, events->read++);
    Event event = events->e[masked_idx];
    return(event);
}

static void
events_quit(Events* events){
    Event event = {0};
    event.type = QUIT;
    events_add(events, event);
}

#endif

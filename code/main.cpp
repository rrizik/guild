#include "main.hpp"

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
win32_window_create(const wchar* window_name, u32 width, u32 height){
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
    RECT rect = { 0, 0, (s32)width, (s32)height };
    AdjustWindowRect(&rect, style, FALSE);
    s32 adjusted_w = rect.right - rect.left;
    s32 adjusted_h = rect.bottom - rect.top;

    result.handle = CreateWindowW(L"window class", window_name, style, CW_USEDEFAULT, CW_USEDEFAULT, adjusted_w, adjusted_h, 0, 0, GetModuleHandle(0), 0);
    if(!IsWindow(result.handle)){
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

static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param){
    LRESULT result = 0;

    switch(message){
        case WM_CLOSE:
        case WM_QUIT:
        case WM_DESTROY:{
            Event event = {0};
            event.type = QUIT;
            events_add(&events, event);
        } break;

        case WM_NCHITTEST:{ // note: prevent resizing on edges
            LRESULT hit = DefWindowProcW(hwnd, message, w_param, l_param);
            if (hit == HTLEFT       || hit == HTRIGHT || // edges of window
                hit == HTTOP        || hit == HTBOTTOM ||
                hit == HTTOPLEFT    || hit == HTTOPRIGHT || // corners of window
                hit == HTBOTTOMLEFT || hit == HTBOTTOMRIGHT){
                return HTCLIENT;
            }
            return hit;
        } break;

        case WM_ACTIVATE:{
            if(w_param == WA_ACTIVE){
                game_in_focus = true;
            }
            if(w_param == WA_INACTIVE){
                game_in_focus = false;
            }
        } break;

        case WM_MOUSELEAVE:{
            // note(rr): currently not happening because we clip mouse to client region
            Event event = {0};
            event.type = NO_CLIENT;

            events_add(&events, event);

            tracking_mouse = false;
        } break;

        case WM_MOUSEMOVE:{
            // post mouse event WM_MOUSELEAVE message when mouse leaves client area
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
            RECT client_rect;
            GetClientRect(hwnd, &client_rect);
            {
                if(game_in_focus){
                    POINT top_left     = { client_rect.left, client_rect.top };
                    POINT bottom_right = { client_rect.right, client_rect.bottom };
                    ClientToScreen(hwnd, &top_left);
                    ClientToScreen(hwnd, &bottom_right);

                    RECT screen_rect = { top_left.x, top_left.y, bottom_right.x, bottom_right.y };
                    ClipCursor(&screen_rect);
                }
            }

            Event event = {0};
            event.type = MOUSE;
            event.mouse_x = (f32)((s16)(l_param & 0xFFFF));
            event.mouse_y = (f32)((s16)(l_param >> 16));

            // calc dx/dy and normalize from -1:1
            f32 dx = event.mouse_x - controller.mouse.x;
            f32 dy = event.mouse_y - controller.mouse.y;
            v2 delta_normalized = normalize_v2(make_v2(dx, dy));
            event.mouse_dx = delta_normalized.x;
            event.mouse_dy = delta_normalized.y;

            // check if mouse is at edge of client region
            if(game_in_focus){
                if((s32)event.mouse_x <= client_rect.left){
                    event.mouse_edge_left = true;
                }
                if((s32)event.mouse_x >= client_rect.right - 1){
                    event.mouse_edge_right = true;
                }
                if((s32)event.mouse_y <= client_rect.top){
                    event.mouse_edge_top = true;
                }
                if((s32)event.mouse_y >= client_rect.bottom - 1){
                    event.mouse_edge_bottom = true;
                }
            }

            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
        } break;

        case WM_MOUSEWHEEL:{
            Event event = {0};
            event.type = KEYBOARD;
            event.mouse_wheel_dir = GET_WHEEL_DELTA_WPARAM(w_param) > 0? 1 : -1;
            if(event.mouse_wheel_dir > 0){
                event.keycode = MOUSE_WHEEL_UP;
            }
            else{
                event.keycode = MOUSE_WHEEL_DOWN;
            }

            event.key_pressed = true;
            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
        } break;

        // TODO: IMPORTANT: These events likely pass in mouse positions, need to store them as part of the event
        // TODO: IMPORTANT: These events likely pass in mouse positions, need to store them as part of the event
        // TODO: IMPORTANT: These events likely pass in mouse positions, need to store them as part of the event
        // TODO: IMPORTANT: These events likely pass in mouse positions, need to store them as part of the event
        // TODO: IMPORTANT: Make sure the alt/shit/ctrl stuff is correct and the UP/DOWN ordering is correct
        // note(rr): mouse buttons are keyboard because it makes it easier to set pressed/held with everything else
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:{
            Event event = {0};
            event.type = KEYBOARD;
            event.keycode = MOUSE_BUTTON_LEFT;

            bool pressed = false;
            if(message == WM_LBUTTONDOWN){ pressed = true; }
            event.key_pressed = pressed;

            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
        } break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:{
            Event event = {0};
            event.type = KEYBOARD;
            event.keycode = MOUSE_BUTTON_RIGHT;

            bool pressed = false;
            if(message == WM_RBUTTONDOWN){ pressed = true; }
            event.key_pressed = pressed;

            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
        } break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:{
            Event event = {0};
            event.type = KEYBOARD;
            event.keycode = MOUSE_BUTTON_MIDDLE;
            event.repeat = ((s32)l_param) & 0x40000000;

            bool pressed = false;
            if(message == WM_MBUTTONDOWN){ pressed = true; }
            event.key_pressed = pressed;

            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
        } break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:{
            Event event = {0};
            event.type = KEYBOARD;
            event.keycode = w_param;
            event.repeat = ((s32)l_param) & 0x40000000;

            event.key_pressed = true;
            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
        } break;
        case WM_SYSKEYUP:
        case WM_KEYUP:{
            Event event = {0};
            event.type = KEYBOARD;
            event.keycode = w_param;

            event.key_pressed = false;
            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = false; }
            if(w_param == VK_SHIFT)   { shift_pressed = false; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = false; }
        } break;

        case WM_CHAR:{
            u64 keycode = w_param;

            if(keycode > 31){
                Event event = {0};
                event.type = TEXT_INPUT;
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

    window = win32_window_create(L"Roids", SCREEN_WIDTH, SCREEN_HEIGHT);
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
    init_wasapi(2, 48000, 32);
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

    should_quit = false;
    while(!should_quit){
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

        do_one_frame();

        // handle events
        bool handled;
        while(!events_empty(&events)){
            Event event = events_next(&events);

            // clear held buttons if mouse leaves client area
            if(event.type == NO_CLIENT){
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

        //----constant buffer----
        D3D11_MAPPED_SUBRESOURCE mapped_subresource;
        d3d_context->Map(d3d_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
        ConstantBuffer2D* constants = (ConstantBuffer2D*)mapped_subresource.pData;
        constants->screen_res = make_v2s32((s32)window.width, (s32)window.height);
        d3d_context->Unmap(d3d_constant_buffer, 0);

        console_update();

        if(controller.button[MOUSE_BUTTON_RIGHT].held &&
           controller.button[MOUSE_BUTTON_RIGHT].pressed){
            world_camera_record = camera;
            world_mouse_record = v2_world_from_screen(controller.mouse.pos);
        }
        if(controller.button[MOUSE_BUTTON_RIGHT].held){
            v2 world_mouse_current = v2_world_from_screen(controller.mouse.pos, &world_camera_record);
            v2 world_rel_pos = world_mouse_record - world_mouse_current;

            camera.x = world_camera_record.x + world_rel_pos.x;
            camera.y = world_camera_record.y + world_rel_pos.y;
        }
        else{
            world_camera_record = {0};
            world_mouse_record = {0};
        }

        // zoom
        if(camera.size > 10){
            camera.size -= (f32)controller.mouse.wheel_dir * 20;
        }
        if(camera.size <= 10){
            camera.size -= (f32)controller.mouse.wheel_dir;
            if(camera.size < 5){
                camera.size = 5;
            }
        }

        simulations = 0;
        accumulator += frame_time;
        while(accumulator >= clock.dt){
            update_game();

            accumulator -= clock.dt;
            time_elapsed += clock.dt;
            simulations++;

        }

        camera_2d_update(&camera, window.aspect_ratio);


        //Rect rect = make_rect_size(make_v2(0, 0), make_v2(10, 10));
        //rect = rect_screen_from_world(rect);
        //draw_quad(rect, BLACK);
        draw_world_terrain();
        draw_world_grid();
        draw_entities(state);

        //v2 cell = {0};
        //String8 cell_str = {0};
        //set_font(state->font);

        //cell = grid_pos_from_cell(0, 0);
        //cell = v2_screen_from_world(cell);
        //cell_str = str8_formatted(ts->frame_arena, "(%i, %i)", (s32)0, (s32)0);
        //draw_text(cell_str, cell, YELLOW);

        set_font(state->font);
        String8 fps = str8_formatted(ts->frame_arena, "FPS: %.2f", FPS);
        draw_text(fps, make_v2(window.width - text_padding - font_string_width(state->font, fps), window.height - text_padding), ORANGE);

        wasapi_play_cursors();
        console_draw();

        debug_draw_render_batches();
        debug_draw_mouse_cell_pos();

        // draw selected texture
        if(state->selected_texture){
            set_texture(&r_assets->textures[state->selected_texture]);
            draw_texture(controller.mouse.pos, make_v2(50, 50));
            draw_bounding_box(make_rect_size(controller.mouse.pos, make_v2(50, 50)), 2, RED);
        }

        ui_begin(ts->ui_arena);

        ui_push_pos_x(20);
        ui_push_pos_y(20);
        ui_push_size_w(ui_size_children(0));
        ui_push_size_h(ui_size_children(0));

        ui_push_border_thickness(10);
        ui_push_background_color(DEFAULT);
        UI_Box* box1 = ui_box(str8_literal("box1##2"),
                              UI_BoxFlag_DrawBackground|
                              UI_BoxFlag_Draggable|
                              UI_BoxFlag_Clickable);
        ui_push_parent(box1);
        ui_pop_pos_x();
        ui_pop_pos_y();

        ui_push_size_w(ui_size_pixel(100, 0));
        ui_push_size_h(ui_size_pixel(50, 0));
        ui_push_background_color(DARK_GRAY);
        if(ui_button(str8_literal("none")).pressed_left){
            state->selected_texture = 0;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("grass1")).pressed_left){
            state->selected_texture = 1;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("grass2")).pressed_left){
            state->selected_texture = 2;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("grass3")).pressed_left){
            state->selected_texture = 3;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("grass4")).pressed_left){
            state->selected_texture = 4;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("grass5")).pressed_left){
            state->selected_texture = 5;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("grass6")).pressed_left){
            state->selected_texture = 6;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("grass7")).pressed_left){
            state->selected_texture = 7;
        }
        ui_spacer(10);
        if(ui_button(str8_literal("grass8")).pressed_left){
            state->selected_texture = 8;
        }

        //ui_push_text_color(LIGHT_GRAY);


        //String8 zoom = str8_format(ts->frame_arena, "cam zoom: %f", camera.size);
        //ui_label(zoom);
        //String8 pos = str8_format(ts->frame_arena, "cam pos: (%.2f, %.2f)", camera.x, camera.y);
        //ui_label(pos);

        //String8 title = str8_format(ts->frame_arena, "Render Batches Count: %i", render_batches.count);
        //ui_label(title);

        //s32 count = 0;
        //for(RenderBatch* batch = render_batches.first; batch != 0; batch = batch->next){
        //    if(count < 25){
        //        String8 batch_str = str8_format(ts->frame_arena, "%i - %i/%i ##%i", batch->id, batch->count, batch->cap, batch->id);
        //        ui_label(batch_str);
        //    }
        //    count++;
        //}

        ui_layout();
        ui_draw(ui_root());
        ui_end();


        {
            d3d_clear_color(BACKGROUND_COLOR);
            draw_render_batches();
            d3d_present();

            render_batches_reset();
            arena_free(ts->batch_arena);
            arena_free(ts->frame_arena);
            arena_free(ts->ui_arena);

            frame_inc++;
            f64 second_elapsed = clock.get_seconds_elapsed(clock.get_os_timer(), frame_tick_start);
            if(second_elapsed > 1){
                FPS = ((f64)frame_inc / second_elapsed);
                frame_tick_start = clock.get_os_timer();
                frame_inc = 0;
            }
        }
        clear_controller_pressed();
        // todo(rr): why is this here?
        //end_profiler();
    }

    d3d_release();
    end_profiler();
    wasapi_release();

    return(0);
}


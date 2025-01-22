#ifndef DRAW_C
#define DRAW_C

static RGBA
brighten_color(RGBA color, float factor){
    RGBA result;
    result.r = MIN(color.r + factor, 1);
    result.g = MIN(color.g + factor, 1);
    result.b = MIN(color.b + factor, 1);
    result.a = color.a;
    return(result);
}

static RGBA
darken_color(RGBA color, float factor){
    RGBA result;
    result.r = MAX(color.r - factor, 0);
    result.g = MAX(color.g - factor, 0);
    result.b = MAX(color.b - factor, 0);
    result.a = color.a;
    return(result);
}

static v2
v2_world_from_screen(v2 screen_pos, Camera2D* camera){
    v2 result = {0};
    result.x =  (2.0f * ((screen_pos.x / window.width)  - 0.5f)) * (camera->size * window.aspect_ratio) + camera->x;
    result.y = (-2.0f * ((screen_pos.y / window.height) - 0.5f)) *  camera->size                        + camera->y;
    return(result);
}

static v2
v2_world_from_screen(v2 screen_pos){
    v2 result = {0};
    result.x =  (2.0f * ((screen_pos.x / window.width)  - 0.5f)) * (camera.size * window.aspect_ratio) + camera.x;
    result.y = (-2.0f * ((screen_pos.y / window.height) - 0.5f)) *  camera.size                        + camera.y;
    return(result);
}

static v2
v2_screen_from_world(v2 world_pos){
    v2 result = {0};
    result.x =  ((((world_pos.x - camera.x) / (camera.size * window.aspect_ratio)) * 0.5f) + 0.5f) * window.width;
    result.y = (((-(world_pos.y - camera.y) /  camera.size)                        * 0.5f) + 0.5f) * window.height;
    return(result);
}

static Rect
rect_screen_from_world(Rect rect){
    Rect result = {0};
    result.min = v2_screen_from_world(rect.min);
    result.max = v2_screen_from_world(rect.max);
    return(result);
}

static Quad
quad_screen_from_world(Quad quad){
    Quad result = {0};
    result.p0 = v2_screen_from_world(quad.p0);
    result.p1 = v2_screen_from_world(quad.p1);
    result.p2 = v2_screen_from_world(quad.p2);
    result.p3 = v2_screen_from_world(quad.p3);
    return(result);
}

static RGBA
srgb_to_linear_approx(RGBA color){
    RGBA result = {
        .r = square_f32(color.r),
        .g = square_f32(color.g),
        .b = square_f32(color.b),
        .a = color.a,
    };
    return(result);
}

static RGBA
linear_to_srgb_approx(RGBA color){
    RGBA result = {
        .r = sqrt_f32(color.r),
        .g = sqrt_f32(color.g),
        .b = sqrt_f32(color.b),
        .a = color.a,
    };
    return(result);
}

static RGBA
linear_from_srgb(RGBA color){
    RGBA result = {0};
    result.a = color.a;

    if(color.r < 0.04045f){
        result.r = color.r / 12.92f;
    }
    else{
        result.r = powf(((color.r + 0.055f) / (1.055f)), 2.4f);
    }

    if(color.g < 0.04045f){
        result.g = color.g / 12.92f;
    }
    else{
        result.g = powf(((color.g + 0.055f) / (1.055f)), 2.4f);
    }

    if(color.b < 0.04045f){
        result.b = color.b / 12.92f;
    }
    else{
        result.b = powf(((color.b + 0.055f) / (1.055f)), 2.4f);
    }
    return(result);
}

static RGBA
srgb_from_linear(RGBA color){
    RGBA result = {0};
    result.a = color.a;

    if(color.r < 0.0031308f){
        result.r = color.r * 12.92f;
    }
    else{
        result.r = 1.055f * powf(color.r, 1.0f / 2.4f) - 0.055f;
    }

    if(color.g < 0.0031308f){
        result.g = color.g * 12.92f;
    }
    else{
        result.g = 1.055f * powf(color.g, 1.0f / 2.4f) - 0.055f;
    }

    if(color.b < 0.0031308f){
        result.b = color.b * 12.92f;
    }
    else{
        result.b = 1.055f * powf(color.b, 1.0f / 2.4f) - 0.055f;
    }

    return(result);
}

static void
init_draw(Arena* arena_b, Assets* assets){
    r_arena = arena_b;
    r_assets = assets;
}

static void
set_texture(Texture* texture){
    r_texture = texture;
}

static Texture*
get_texture(void){
    return(r_texture);
}

static void
set_font(Font* font){
    r_font = font;
}

static Font*
get_font(void){
    return(r_font);
}

// note: quads are always top-left, top-right, bottom-right, bottom-left order
static void
draw_quad(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color){

    set_texture(&r_assets->textures[TextureAsset_White]);
    RenderBatch *batch = get_render_batch(6);

    RGBA linear_color = linear_from_srgb(color); // gamma correction
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_quad(v2 pos, v2 dim, RGBA color){

    set_texture(&r_assets->textures[TextureAsset_White]);
    RenderBatch *batch = get_render_batch(6);

    v2 p0 = pos;
    v2 p1 = make_v2(pos.x + dim.w, pos.y);
    v2 p2 = make_v2(pos.x + dim.w, pos.y + dim.h);
    v2 p3 = make_v2(pos.x, pos.y + dim.h);

    RGBA linear_color = linear_from_srgb(color); // gamma correction
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_quad(Rect rect, RGBA color){

    set_texture(&r_assets->textures[TextureAsset_White]);
    RenderBatch *batch = get_render_batch(6);

    v2 p0 = make_v2(rect.x0, rect.y0);
    v2 p1 = make_v2(rect.x1, rect.y0);
    v2 p2 = make_v2(rect.x1, rect.y1);
    v2 p3 = make_v2(rect.x0, rect.y1);

    RGBA linear_color = linear_from_srgb(color); // gamma correction
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_quad(Quad quad, RGBA color){

    set_texture(&r_assets->textures[TextureAsset_White]);
    RenderBatch *batch = get_render_batch(6);

    RGBA linear_color = linear_from_srgb(color); // gamma correction
    batch->buffer[batch->count++] = { quad.p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { quad.p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { quad.p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { quad.p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { quad.p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { quad.p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_bounding_box(v2 p0, v2 p1, v2 p2, v2 p3, f32 width, RGBA color){
    draw_line(p0, p1, width, color);
    draw_line(p1, p2, width, color);
    draw_line(p2, p3, width, color);
    draw_line(p3, p0, width, color);
}

static void
draw_bounding_box(Quad quad, f32 width, RGBA color){
    draw_line(quad.p0, quad.p1, width, color);
    draw_line(quad.p1, quad.p2, width, color);
    draw_line(quad.p2, quad.p3, width, color);
    draw_line(quad.p3, quad.p0, width, color);
}

static void
draw_bounding_box(Rect rect, f32 width, RGBA color){
    draw_line(make_v2(rect.x0, rect.y0), make_v2(rect.x1, rect.y0), width, color);
    draw_line(make_v2(rect.x1, rect.y0), make_v2(rect.x1, rect.y1), width, color);
    draw_line(make_v2(rect.x1, rect.y1), make_v2(rect.x0, rect.y1), width, color);
    draw_line(make_v2(rect.x0, rect.y1), make_v2(rect.x0, rect.y0), width, color);
}

static void
draw_line(v2 p0, v2 p1, f32 width, RGBA color){

    set_texture(&r_assets->textures[TextureAsset_White]);
    RenderBatch *batch = get_render_batch(6);

    v2 dir = direction_v2(p0, p1);
    v2 perp = perpendicular(dir);
    v2 p2 = p1 + (perp * width);
    v2 p3 = p0 + (perp * width);

    RGBA linear_color = linear_from_srgb(color); // gamma correction
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_texture(v2 p0, v2 p1, v2 p2, v2 p3, RGBA color){

    RenderBatch *batch = get_render_batch(6);

    RGBA linear_color = linear_from_srgb(color); // gamma correction
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_texture(v2 pos, v2 dim, RGBA color){

    RenderBatch *batch = get_render_batch(6);

    v2 p0 = pos;
    v2 p1 = make_v2(pos.x + dim.w, pos.y);
    v2 p2 = make_v2(pos.x + dim.w, pos.y + dim.h);
    v2 p3 = make_v2(pos.x, pos.y + dim.h);

    RGBA linear_color = linear_from_srgb(color); // gamma correction
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_texture(Rect rect, RGBA color){

    RenderBatch *batch = get_render_batch(6);

    v2 p0 = make_v2(rect.x0, rect.y0);
    v2 p1 = make_v2(rect.x1, rect.y0);
    v2 p2 = make_v2(rect.x1, rect.y1);
    v2 p3 = make_v2(rect.x0, rect.y1);

    RGBA linear_color = linear_from_srgb(color); // gamma correction
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_texture(Quad quad, RGBA color){

    RenderBatch *batch = get_render_batch(6);

    RGBA linear_color = linear_from_srgb(color); // gamma correction
    batch->buffer[batch->count++] = { quad.p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { quad.p1, linear_color, make_v2(1.0f, 0.0f) };
    batch->buffer[batch->count++] = { quad.p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { quad.p0, linear_color, make_v2(0.0f, 0.0f) };
    batch->buffer[batch->count++] = { quad.p2, linear_color, make_v2(1.0f, 1.0f) };
    batch->buffer[batch->count++] = { quad.p3, linear_color, make_v2(0.0f, 1.0f) };
}

static void
draw_text(String8 text, v2 pos, RGBA color){
    Font* font = get_font();
    set_texture(&font->texture);

    u64 count = text.size * 6;
    RenderBatch* batch = get_render_batch(count);
    RGBA linear_color = linear_from_srgb(color);

    f32 start_x = pos.x;
    f32 y_offset = 0;
    stbtt_aligned_quad quad;
    u32 vertex_count = 0;
    for(s32 i=0; i < text.size; ++i){
        u8* character = text.str + i;
        if(*character == '\n'){
            y_offset += (f32)font->vertical_offset;
            pos.x = start_x;
        }
        else{
            stbtt_GetPackedQuad(font->packed_chars, font->texture_w, font->texture_h, (*character) - font->first_char, &pos.x, &pos.y, &quad, 1);
            v2 p0 = make_v2(quad.x0, quad.y0 + y_offset);
            v2 p1 = make_v2(quad.x1, quad.y0 + y_offset);
            v2 p2 = make_v2(quad.x1, quad.y1 + y_offset);
            v2 p3 = make_v2(quad.x0, quad.y1 + y_offset);

            // todo: remove this
            //g_angle += 1 * (f32)clock.dt;
            //v2 origin = make_v2((p0.x + p2.x)/2, (p0.y + p2.y)/2);
            //p0 = rotate_point_deg(p0, g_angle, origin);
            //p1 = rotate_point_deg(p1, g_angle, origin);
            //p2 = rotate_point_deg(p2, g_angle, origin);
            //p3 = rotate_point_deg(p3, g_angle, origin);

            batch->buffer[batch->count++] = { p0, linear_color, make_v2(quad.s0, quad.t0) };
            batch->buffer[batch->count++] = { p1, linear_color, make_v2(quad.s1, quad.t0) };
            batch->buffer[batch->count++] = { p2, linear_color, make_v2(quad.s1, quad.t1) };
            batch->buffer[batch->count++] = { p0, linear_color, make_v2(quad.s0, quad.t0) };
            batch->buffer[batch->count++] = { p2, linear_color, make_v2(quad.s1, quad.t1) };
            batch->buffer[batch->count++] = { p3, linear_color, make_v2(quad.s0, quad.t1) };
        }
    }
}

static RenderBatch*
get_render_batch(u64 vertex_count){
    Texture* texture = get_texture();

    RenderBatch *batch = render_batches.last;
    if(batch == 0 || batch->count + vertex_count >= batch->cap || batch->texture != texture){
        batch = push_array(r_arena, RenderBatch, 1);
        batch->buffer = push_array(r_arena, Vertex3, DEFAULT_BATCH_SIZE / sizeof(Vertex3));
        batch->cap = DEFAULT_BATCH_SIZE / sizeof(Vertex3);
        batch->count = 0;
        batch->texture = texture;
        batch->id = render_batches.count;
        if(render_batches.last == 0){
            render_batches.last = batch;
            render_batches.first = batch;
        }
        else{
            render_batches.last->next = batch;
            render_batches.last = batch;
        }
        ++render_batches.count;
    }
    return(batch);
}

static void draw_render_batches(){
    // todo: Its ok for d3d to understand the concept of a Renderbatch, you can move this straight to d3d and pass in the batches all together.
    //d3d_draw(render_batches);
    for(RenderBatch* batch = render_batches.first; batch != 0; batch = batch->next){
        d3d_draw(batch->buffer, batch->count, batch->texture);
    }
}

static void draw_render_batches_new(){
    s32 required_size = 0;
    for(RenderBatch* batch = render_batches.first; batch != 0; batch = batch->next){
        required_size += batch->count * sizeof(Vertex3);
    }

    if(required_size > d3d_vertex_buffer_size){
        d3d_release_vertex_buffer(d3d_vertex_buffer);
        d3d_vertex_buffer = d3d_make_vertex_buffer(required_size);
    }

    D3D11_MAPPED_SUBRESOURCE resource;
    hr = d3d_context->Map(d3d_vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
    assert_hr(hr);

    s32 at = 0;
    for(RenderBatch* batch = render_batches.first; batch != 0; batch = batch->next){
        batch->start_index_in_vertex_buffer = at;
        memcpy((u8*)resource.pData + at, batch->buffer, batch->count * sizeof(Vertex3));
        at += batch->count;
    }
    d3d_context->Unmap(d3d_vertex_buffer, 0);


    ID3D11Buffer* buffers[] = {d3d_vertex_buffer};
    u32 strides[] = {sizeof(Vertex3)};
    u32 offset[] = {0};

    d3d_context->IASetVertexBuffers(0, 1, buffers, strides, offset);

    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context->PSSetSamplers(0, 1, &d3d_sampler_state);

    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, 0);
    d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xFFFFFFFF);
    d3d_context->RSSetState(d3d_rasterizer_state);

    d3d_context->VSSetConstantBuffers(0, 1, &d3d_constant_buffer);

    d3d_context->RSSetViewports(1, &d3d_viewport);
    d3d_context->IASetInputLayout(d3d_2d_textured_il);
    d3d_context->VSSetShader(d3d_2d_textured_vs, 0, 0);
    d3d_context->PSSetShader(d3d_2d_textured_ps, 0, 0);

    for(RenderBatch* batch = render_batches.first; batch != 0; batch = batch->next){
        d3d_context->PSSetShaderResources(0, 1, &batch->texture->view);
        d3d_context->Draw((u32)batch->count, (u32)batch->start_index_in_vertex_buffer);
    }
}

static void
render_batches_reset(void){
    render_batches.first = 0;
    render_batches.last = 0;
    render_batches.count = 0;
    arena_free(r_arena);
}
#endif

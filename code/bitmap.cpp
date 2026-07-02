#ifndef BITMAP_C
#define BITMAP_C

static Bitmap
stb_load_image(Arena* arena, String8 dir, String8 file, bool vertical_flip){
    // DUMB DUMB ADDED THIS
    begin_timed_function();

    Bitmap result = {0};
    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, dir, file);

    if(vertical_flip){
        stbi_set_flip_vertically_on_load(true);
    }

    int x = 0, y = 0, n = 0;
    u8* base = 0;
    {
        // DUMB DUMB ADDED THIS
        begin_timed_scope("decode image");

        base = (u8*)stbi_load((char const*)full_path.str, &x, &y, &n, 4); // specify 4 channels
    }

    if(base){
        // DUMB DUMB ADDED THIS
        begin_timed_scope("copy image pixels");

        result.base = push_array(arena, u8, (x * y * 4));
        memcpy(result.base, base, (u32)(x * y * 4));

        result.width = x;
        result.height = y;
        result.stride = x * 4;
        result.channels = 4;

        stbi_image_free(base);
    }
    end_scratch(scratch);
    return(result);
}

#endif

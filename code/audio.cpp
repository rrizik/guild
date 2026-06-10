#ifndef WASAPI_C
#define WASAPI_C

// NOTE:
// Discord Convo: https://discord.com/channels/239737791225790464/1216549745015656459
// Improve this entire API by running it through a thread.

// todo: put audio in base file as it can be standalone
// todo: change all asserts to error logs

static void
audio_init(u16 channels, u32 samples_per_sec, u16 bits_per_sample){
    audio = {0};
    audio_cursors_clear();

    HRESULT hr = S_OK;
    IMMDeviceEnumerator* device_enumerator = 0;
    IMMDevice* audio_device = 0;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), (void**)&device_enumerator);
    if(FAILED(hr)){
        assert_hr(hr);
        return;
    }

    // get defualt audio device
    hr = device_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &audio_device);
    device_enumerator->Release();
    if(FAILED(hr)){
        assert_hr(hr);
        return;
    }

    // activate the audio client interface
    hr = audio_device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (void**)&audio.audio_client);
    audio_device->Release();
    if(FAILED(hr)){
        assert_hr(hr);
        return;
    }

    hr = audio.audio_client->GetDevicePeriod(&audio.default_device_period,
                                             &audio.minimum_device_period);
    if(FAILED(hr)){
        assert_hr(hr);
        return;
    }

    audio.format = {0};
    audio.format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    audio.format.nChannels = channels;
    audio.format.nSamplesPerSec = samples_per_sec;
    audio.format.wBitsPerSample = bits_per_sample;
    audio.format.nBlockAlign = (u16)(channels * bits_per_sample / 8);
    audio.format.nAvgBytesPerSec = audio.format.nSamplesPerSec * audio.format.nBlockAlign;
    audio.format.cbSize = 0;

    hr = audio.audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0,
                                        audio.default_device_period, 0,
                                        &audio.format, 0);
    if(FAILED(hr)){
        assert_hr(hr);
        return;
    }

    // get the size of the buffer
    hr = audio.audio_client->GetBufferSize(&audio.buffer_samples);
    if(FAILED(hr)){
        assert_hr(hr);
        return;
    }

    // get the render client
    hr = audio.audio_client->GetService(__uuidof(IAudioRenderClient),
                                        (void**)&audio.render_client);
    if(FAILED(hr)){
        assert_hr(hr);
        return;
    }

    //audio_stream_start();
}

static void
audio_release(void){
    audio_stream_stop();

    if(audio.render_client){
        audio.render_client->Release();
        audio.render_client = 0;
    }

    if(audio.audio_client){
        audio.audio_client->Release();
        audio.audio_client = 0;
    }

    audio.running = false;
}

static Audio_Cursor*
audio_play(Wave* wave, f32 volume, bool loop){
    Audio_Cursor* cursor = audio_cursor_alloc();
    if(cursor){
        cursor->base = wave->base;
        cursor->sample_count = wave->sample_count;
        cursor->at = 0;
        cursor->volume = volume;
        cursor->loop = loop;

        if(audio.cursor_count == 0){
            audio_stream_start();
        }
        audio.cursor_count++;
    }
    return(cursor);
}

static bool
audio_stop(Audio_Cursor* cursor){
    if(cursor && cursor->active){
        audio_cursor_remove(cursor);
        return(true);
    }
    return(false);
}

// todo: create a wave drawing function to draw the audio waves. Will be cool to do
static void 
audio_update(void){
    HRESULT hr = S_OK;

    u32 padding;
    hr = audio.audio_client->GetCurrentPadding(&padding);
    if (FAILED(hr)) {
        assert_hr(hr);
        return;
    }

    u32 available_samples = audio.buffer_samples - padding;
    if(available_samples == 0){
        return;
    }

    u8* buffer;
    hr = audio.render_client->GetBuffer(available_samples, &buffer);
    if (FAILED(hr)) {
        assert_hr(hr);
        return;
    }

    if(audio.cursor_count <= 0){
        hr = audio.render_client->ReleaseBuffer(available_samples, AUDCLNT_BUFFERFLAGS_SILENT); 
        if (FAILED(hr)) {
            assert_hr(hr);
        }
        audio_stream_stop();
        return;
    }

    memset(buffer, 0, available_samples * audio.format.nBlockAlign); // clear buffer from previously written data
    f32* buffer_f32 = (f32*)buffer;
    for(s32 cursor_i=0; cursor_i < AUDIO_CURSORS_MAX; ++cursor_i){
        Audio_Cursor* cursor = audio.cursors + cursor_i;
        if(cursor->active){

            u32 wave_remainder = cursor->sample_count - cursor->at;
            u32 buffer_size = wave_remainder > available_samples ? available_samples : wave_remainder;

            if(cursor->at < cursor->sample_count){
                for(s32 i=0; i < buffer_size; ++i){
                    f32 sample = ((s16)(cursor->base[(cursor->at + i)])) * (1.0f / 32767.0f); // note: normalize to a range of -1.0f to 1.0f by multiplying by 1/(max s16)

                    // todo: iterate over number of channels, this is a bit hardcoded.
                    buffer_f32[(i * audio.format.nChannels) + 0] += sample * cursor->volume; // channel 1
                    buffer_f32[(i * audio.format.nChannels) + 1] += sample * cursor->volume; // channel 2
                }

            }

            cursor->at += buffer_size;
            if(cursor->at >= cursor->sample_count){
                if(cursor->loop){
                    cursor->at = 0;
                }
                else{
                    audio_cursor_remove(cursor);
                }
            }

        }
    }

    hr = audio.render_client->ReleaseBuffer(available_samples, 0); // Audio handoff
    if(FAILED(hr)){
        assert_hr(hr);
    }
}

static void 
audio_stream_start(void){
    if(!audio.running){
        HRESULT hr = audio.audio_client->Start();
        if(FAILED(hr)){
            assert_hr(hr);
            return;
        }
        audio.running = true;
    }
}

static void 
audio_stream_stop(void){
    if(audio.running){
        HRESULT hr = audio.audio_client->Stop();
        if(FAILED(hr)){
            assert_hr(hr);
            return;
        }
        audio.running = false;
    }
}

static Audio_Cursor* 
audio_cursor_alloc(){
    if(audio.free_cursor_idx < 0){
        return(0);
    }

    u32 cursor_index = audio.free_cursors[audio.free_cursor_idx--];
    Audio_Cursor* cursor = audio.cursors + cursor_index;

    *cursor = {0};
    cursor->index = cursor_index;
    cursor->active = true;
    return(cursor);
}

static void
audio_cursors_clear(void){
    audio.cursor_count = 0;
    audio.free_cursor_idx = AUDIO_CURSORS_MAX - 1;

    for(u32 i=0; i < AUDIO_CURSORS_MAX; ++i){
        audio.cursors[i] = {0};
        audio.cursors[i].index = i;
        audio.free_cursors[i] = (AUDIO_CURSORS_MAX - 1) - i;
    }
}

static void 
audio_cursor_remove(Audio_Cursor* cursor){
    if(cursor && cursor->active){
        assert(audio.free_cursor_idx < (s32)(AUDIO_CURSORS_MAX - 1));

        audio.free_cursors[++audio.free_cursor_idx] = cursor->index;
        assert(audio.cursor_count > 0);
        audio.cursor_count--;
        *cursor = {0};

        if(audio.cursor_count <= 0){
            audio_stream_stop();
        }
    }
}

//static HRESULT audio_play_sine(f32 freq){
//    HRESULT hr = S_OK;
//
//    local u32 numerator = 0;
//    u32 padding;
//    hr = audio_client->GetCurrentPadding(&padding);
//    if(FAILED(hr)){
//        assert_hr(hr);
//        return(hr);
//    }
//    u32 available_size = buffer_size - padding;
//
//    u8* buffer;
//    hr = render_client->GetBuffer(available_size, &buffer);
//    if(FAILED(hr)){
//        assert_hr(hr);
//        return(hr);
//    }
//
//    static f32 time;
//    for(u32 i=0; i < available_size; ++i){
//        time = (f32)((f32)(numerator++ % buffer_size) / (f32)buffer_size);
//
//        f32 sine_value = sin_f32((2.0f * PI_f32 * freq * time));
//
//        // scale the sine value to the range -0.nf to 0.nf
//        sine_value *= 0.03f;
//
//        f32* buffer_f32 = (f32*)buffer;
//        if(wave_format.nChannels == 2){
//            buffer_f32[i * wave_format.nChannels] = sine_value;
//            buffer_f32[(i * wave_format.nChannels) + 1] = sine_value;
//        }
//        else{
//            buffer_f32[i * wave_format.nChannels] = sine_value;
//        }
//    }
//
//    hr = render_client->ReleaseBuffer(available_size, 0);
//    if (FAILED(hr)) {
//        assert_hr(hr);
//        return(hr);
//    }
//
//    return(hr);
//}


#endif

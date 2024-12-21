Texture2D my_texture;
SamplerState my_sampler;

struct Resolution{
    uint w;
    uint h;
};

cbuffer ConstantBuffer{
    Resolution screen_res;
}

struct VertexInput{
    float2 pos: POS;
    float4 col: COL;
    float2 tex: TEX;
};

struct VertexOutput{
    float4 pos: SV_POSITION;
    float4 col: COLOR;
    float2 tex: TEXCOORD;
};

VertexOutput vs_main(VertexInput vertex){
    VertexOutput result;

    // convert to clip space
    float x =   ((vertex.pos.x / screen_res.w) * 2.0f) - 1.0f;
    float y = -(((vertex.pos.y / screen_res.h) * 2.0f) - 1.0f);

    result.pos = float4(x, y, 0.0, 1.0);
    result.col = vertex.col;
    result.tex = vertex.tex;
    return result;
}

float4 ps_main(VertexOutput vertex) : SV_TARGET{
    float4 tex_color = my_texture.Sample(my_sampler, vertex.tex);
    float4 result = tex_color * vertex.col;
    return result;
}

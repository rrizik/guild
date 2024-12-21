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
};

struct VertexOutput{
    float4 pos: SV_POSITION;
    float4 col: COLOR;
};

VertexOutput vs_main(VertexInput vertex){
    VertexOutput result;

    // convert to clip space
    float x =   ((vertex.pos.x / screen_res.w) * 2.0f) - 1.0f;
    float y = -(((vertex.pos.y / screen_res.h) * 2.0f) - 1.0f);

    // -y to invert clip space in the y
    result.pos = float4(x, y, 0.0f, 1.0f);
    result.col = vertex.col;
    return result;
}

float4 ps_main(VertexOutput vertex) : SV_TARGET{
    float4 result = vertex.col;
    return result;
}

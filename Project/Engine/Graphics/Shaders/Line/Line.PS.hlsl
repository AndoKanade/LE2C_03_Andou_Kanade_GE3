#include "Line.hlsli"

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    // 頂点カラーをそのまま出力
    output.color = input.color;
    return output;
}

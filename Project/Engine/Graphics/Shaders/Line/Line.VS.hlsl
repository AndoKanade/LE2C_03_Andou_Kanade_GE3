#include "Line.hlsli"

struct TransformationMatrix
{
    float32_t4x4 WVP;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t4 color : COLOR0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    // WVP行列でジョイント座標をスクリーン空間へ変換
    output.position = mul(input.position, gTransformationMatrix.WVP);
    // 頂点ごとの色をそのままピクセルシェーダーへ渡す
    output.color = input.color;
    return output;
}

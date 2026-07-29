#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "MyMath.h"

class DXCommon;

// 線分描画（骨のデバッグ表示等）の共通パイプラインを管理するクラス
class LineCommon{
public:
	struct Vertex{
		Vector4 position;
		Vector4 color;
	};

	void Initialize(DXCommon* dxCommon);

	// ゲッター
	ID3D12RootSignature* GetRootSignature() const{ return rootSignature_.Get(); }
	ID3D12PipelineState* GetPipelineState() const{ return graphicsPipelineState_.Get(); }
	DXCommon* GetDxCommon() const{ return dxCommon_; }

private:
	void CreateRootSignature();
	void CreateGraphicsPipelineState();

private:
	DXCommon* dxCommon_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
};

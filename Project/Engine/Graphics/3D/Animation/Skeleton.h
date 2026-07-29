#pragma once

#include "MyMath.h"
#include "Animation.h"
#include <vector>
#include <string>
#include <optional>
#include <map>
// ↓骨デバッグ表示 追加
#include <d3d12.h>
#include <wrl.h>
#include "LineCommon.h"
// ↑骨デバッグ表示 追加

// ノード構造体
struct Node{
	QuaternionTransform transform;
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

// ジョイント構造体
struct Joint{
	QuaternionTransform transform;
	QuaternionTransform bindPoseTransform;
	Matrix4x4 localMatrix;
	Matrix4x4 skeletonSpaceMatrix;
	Matrix4x4 inverseBindPoseMatrix;
	std::string name;
	std::vector<int32_t> children;
	int32_t index;
	std::optional<int32_t> parent;
};

// スケルトンクラス
class Skeleton{
public:
	// 初期化・更新
	void Create(const Node& rootNode);
	void Update();
	void UpdateJointRecursive(int32_t jointIdx,const Matrix4x4& parentMatrix);

	// 描画・アニメーション適用
	void DrawDebug(const Matrix4x4& worldMatrix,const Matrix4x4& viewProjectionMatrix,LineCommon* lineCommon,bool drawBoneLines,bool drawLocalAxes);
	void ApplyAnimation(const Animation& animation,float animationTime);

	// メンバ変数
	int32_t root;
	std::map<std::string,int32_t> jointMap;
	std::vector<Joint> joints;

private:
	// 内部処理
	int32_t CreateJoint(const Node& node,const std::optional<int32_t>& parent,std::vector<Joint>& joints);

	// ↓骨デバッグ表示 追加
	// 骨のデバッグ線描画用リソース（初回のDrawDebug呼び出し時に生成）
	Microsoft::WRL::ComPtr<ID3D12Resource> lineVertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW lineVertexBufferView_{};
	LineCommon::Vertex* lineVertexData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> lineWvpResource_;
	Matrix4x4* lineWvpData_ = nullptr;
	bool isLineResourceCreated_ = false;
	// ↑骨デバッグ表示 追加
};
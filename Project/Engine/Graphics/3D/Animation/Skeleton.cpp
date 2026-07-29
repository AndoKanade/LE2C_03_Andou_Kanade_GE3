#include "Skeleton.h"
#include "Logger.h"
// ↓骨デバッグ表示 追加
#include "DXCommon.h"
// ↑骨デバッグ表示 追加

// 初期化・構築処理

void Skeleton::Create(const Node& rootNode){
	joints.clear();
	jointMap.clear();

	// ルートから順にジョイントを構築
	root = CreateJoint(rootNode,std::nullopt,joints);

	// 構築されたジョイントを使って辞書を作成
	for(const Joint& joint : joints){
		jointMap.emplace(joint.name,joint.index);
	}

	// アニメーション前の初期姿勢行列を計算
	Update();
}

int32_t Skeleton::CreateJoint(const Node& node,const std::optional<int32_t>& parent,std::vector<Joint>& joints){
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = MakeIdentity4x4();
	joint.transform = node.transform;
	joint.parent = parent;
	joint.index = static_cast<int32_t>(joints.size());
	joint.bindPoseTransform = joint.transform;

	joints.push_back(joint);

	// 子ノードがあれば再帰的にジョイントを作成
	for(const Node& childNode : node.children){
		int32_t childIndex = CreateJoint(childNode,joint.index,joints);
		joints[joint.index].children.push_back(childIndex);
	}

	return joint.index;
}

// 更新処理

void Skeleton::Update(){
	// 全てのジョイントをループする
	for(size_t i = 0; i < joints.size(); ++i){
		Joint& joint = joints[i];

		// ローカル行列の計算
		Matrix4x4 matScale = MakeScaleMatrix(joint.transform.scale);
		Matrix4x4 matRotate = MakeRotateQuaternionMatrix(joint.transform.rotate);
		Matrix4x4 matTranslate = MakeTranslateMatrix(joint.transform.translate);

		joint.localMatrix = Multiply(Multiply(matScale,matRotate),matTranslate);

		// スケルトン空間行列（ワールド行列）の計算
		if(joint.parent.has_value()){
			// 親がいる場合: 自分のローカル × 親のスケルトン空間行列
			joint.skeletonSpaceMatrix = Multiply(joint.localMatrix,joints[*joint.parent].skeletonSpaceMatrix);
		} else{
			// 親がいなければローカルがそのまま
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

// ↓骨デバッグ表示 追加
// デバッグ描画用の定数
namespace{
	constexpr uint32_t kVerticesPerLine = 2; // 線分1本あたりの頂点数
	constexpr uint32_t kAxisCount = 3; // ローカル軸の本数(X, Y, Z)
	constexpr float kLocalAxisLength = 0.1f; // ローカル軸の表示長さ
	constexpr Vector4 kBoneDebugLineColor = {0.0f, 1.0f, 0.0f, 1.0f}; // 骨のデバッグ線の色(緑)
	constexpr Vector4 kAxisColorX = {1.0f, 0.0f, 0.0f, 1.0f}; // X軸の色(赤)
	constexpr Vector4 kAxisColorY = {0.0f, 1.0f, 0.0f, 1.0f}; // Y軸の色(緑)
	constexpr Vector4 kAxisColorZ = {0.0f, 0.0f, 1.0f, 1.0f}; // Z軸の色(青)
}

void Skeleton::DrawDebug(const Matrix4x4& worldMatrix,const Matrix4x4& viewProjectionMatrix,LineCommon* lineCommon,bool drawBoneLines,bool drawLocalAxes){
	if(joints.empty() || lineCommon == nullptr || (!drawBoneLines && !drawLocalAxes)){
		return;
	}

	DXCommon* dxCommon = lineCommon->GetDxCommon();

	// 初回呼び出し時のみ、頂点バッファ・定数バッファを生成する(骨の線 + 全ジョイント分のローカル軸)
	if(!isLineResourceCreated_){
		const uint32_t maxLineVertexCount = static_cast<uint32_t>(joints.size()) * kVerticesPerLine * (1 + kAxisCount);

		lineVertexBuffer_ = dxCommon->CreateBufferResource(sizeof(LineCommon::Vertex) * maxLineVertexCount);
		lineVertexBuffer_->Map(0,nullptr,reinterpret_cast<void**>(&lineVertexData_));

		lineVertexBufferView_.BufferLocation = lineVertexBuffer_->GetGPUVirtualAddress();
		lineVertexBufferView_.SizeInBytes = sizeof(LineCommon::Vertex) * maxLineVertexCount;
		lineVertexBufferView_.StrideInBytes = sizeof(LineCommon::Vertex);

		lineWvpResource_ = dxCommon->CreateBufferResource(sizeof(Matrix4x4));
		lineWvpResource_->Map(0,nullptr,reinterpret_cast<void**>(&lineWvpData_));

		isLineResourceCreated_ = true;
	}

	uint32_t vertexCount = 0;
	for(const Joint& joint : joints){
		const Vector3 jointPosition = {joint.skeletonSpaceMatrix.m[3][0], joint.skeletonSpaceMatrix.m[3][1], joint.skeletonSpaceMatrix.m[3][2]};

		// 親ジョイントとの間を結ぶ骨の線を書き込む(ルートジョイントは親がいないため線を生成しない)
		if(drawBoneLines && joint.parent.has_value()){
			const Matrix4x4& parentMatrix = joints[*joint.parent].skeletonSpaceMatrix;
			const Vector3 parentPosition = {parentMatrix.m[3][0], parentMatrix.m[3][1], parentMatrix.m[3][2]};

			lineVertexData_[vertexCount].position = {jointPosition.x, jointPosition.y, jointPosition.z, 1.0f};
			lineVertexData_[vertexCount].color = kBoneDebugLineColor;
			++vertexCount;
			lineVertexData_[vertexCount].position = {parentPosition.x, parentPosition.y, parentPosition.z, 1.0f};
			lineVertexData_[vertexCount].color = kBoneDebugLineColor;
			++vertexCount;
		}

		// ジョイントのローカル軸(X, Y, Z)を書き込む
		if(drawLocalAxes){
			const Vector3 axisDirections[kAxisCount] = {
				Normalize(Vector3{joint.skeletonSpaceMatrix.m[0][0], joint.skeletonSpaceMatrix.m[0][1], joint.skeletonSpaceMatrix.m[0][2]}),
				Normalize(Vector3{joint.skeletonSpaceMatrix.m[1][0], joint.skeletonSpaceMatrix.m[1][1], joint.skeletonSpaceMatrix.m[1][2]}),
				Normalize(Vector3{joint.skeletonSpaceMatrix.m[2][0], joint.skeletonSpaceMatrix.m[2][1], joint.skeletonSpaceMatrix.m[2][2]}),
			};
			const Vector4 axisColors[kAxisCount] = {kAxisColorX, kAxisColorY, kAxisColorZ};

			for(uint32_t axisIndex = 0; axisIndex < kAxisCount; ++axisIndex){
				const Vector3 axisEnd = jointPosition + axisDirections[axisIndex] * kLocalAxisLength;

				lineVertexData_[vertexCount].position = {jointPosition.x, jointPosition.y, jointPosition.z, 1.0f};
				lineVertexData_[vertexCount].color = axisColors[axisIndex];
				++vertexCount;
				lineVertexData_[vertexCount].position = {axisEnd.x, axisEnd.y, axisEnd.z, 1.0f};
				lineVertexData_[vertexCount].color = axisColors[axisIndex];
				++vertexCount;
			}
		}
	}

	if(vertexCount == 0){
		return;
	}

	// ワールド×ビュープロジェクション行列を定数バッファへ書き込む
	*lineWvpData_ = Multiply(worldMatrix,viewProjectionMatrix);

	// 描画コマンドを発行する
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();
	commandList->SetGraphicsRootSignature(lineCommon->GetRootSignature());
	commandList->SetPipelineState(lineCommon->GetPipelineState());
	commandList->IASetVertexBuffers(0,1,&lineVertexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	commandList->SetGraphicsRootConstantBufferView(0,lineWvpResource_->GetGPUVirtualAddress());
	commandList->DrawInstanced(vertexCount,1,0,0);
}
// ↑骨デバッグ表示 追加

void Skeleton::ApplyAnimation(const Animation& animation,float animationTime){
	for(auto& joint : joints){
		// そのジョイントの現在の時間における補間済みアニメーションデータを取得
		NodeAnimation nodeAnim = AnimationController::GetInterpolatedNode(animation,joint.name,animationTime);

		// 補間計算済みのデータを使う
		if(!nodeAnim.rotateKeyframes.empty()){
			joint.transform.rotate = nodeAnim.rotateKeyframes[0].value;
		}
		if(!nodeAnim.translateKeyframes.empty()){
			joint.transform.translate = nodeAnim.translateKeyframes[0].value;
		}
		if(!nodeAnim.scaleKeyframes.empty()){
			joint.transform.scale = nodeAnim.scaleKeyframes[0].value;
		}

		// ローカル行列の再計算
		joint.localMatrix = MakeAffineMatrixQuaternion(joint.transform.scale,joint.transform.rotate,joint.transform.translate);
	}
}
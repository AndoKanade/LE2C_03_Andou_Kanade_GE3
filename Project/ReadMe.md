# 加点要素の実装内容

## 骨のデバッグ表示 (10点)

スキニングされた人型モデル（`humanObj_`）のスケルトンについて、各ジョイントとその親ジョイントを結ぶ線分をLINELISTで描画し、骨格構造を可視化する機能を実装した。

### 実装概要
- `Skeleton::DrawDebug`（`Engine/Graphics/3D/Animation/Skeleton.cpp`）にて、`Skeleton::joints`をループし、親を持つジョイントごとに「自身の`skeletonSpaceMatrix`の平行移動成分」と「親の`skeletonSpaceMatrix`の平行移動成分」を線分の頂点として動的頂点バッファへ書き込み、`D3D_PRIMITIVE_TOPOLOGY_LINELIST`で描画する。
- 線分描画専用の共通パイプライン（ルートシグネチャ・PSO）を`LineCommon`（`Engine/Graphics/3D/LineCommon.h/.cpp`）として新規に用意した。プロジェクト内に既存だった描画パイプラインはすべて三角形トポロジのみだったため、`D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE`を用いる新規パイプラインとして追加している。
- 専用シェーダー（`Engine/Graphics/Shaders/Line/Line.VS.hlsl`, `Line.PS.hlsl`）を新規作成し、単色（緑）でジョイント間を結ぶ線を描画する。
- `Game/scenes/GameScene`のImGui「Character Bone Control」内に「Bone Debug Display」チェックボックスを追加し、ON/OFFを切り替え可能にした。ONの場合、`GameScene::Draw()`内で`Skeleton::DrawDebug`を呼び出し、アニメーション再生中の骨格に追従する形でデバッグ線を描画する。

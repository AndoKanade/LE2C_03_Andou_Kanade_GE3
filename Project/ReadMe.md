# 加点要素の実装内容

## 骨のデバッグ表示 (10点)

スキニングされた人型モデル（`humanObj_`）のスケルトンについて、各ジョイントとその親ジョイントを結ぶ線分をLINELISTで描画し、骨格構造を可視化する機能を実装した。

### 実装概要
- `Skeleton::DrawDebug`（`Engine/Graphics/3D/Animation/Skeleton.cpp`）にて、`Skeleton::joints`をループし、親を持つジョイントごとに「自身の`skeletonSpaceMatrix`の平行移動成分」と「親の`skeletonSpaceMatrix`の平行移動成分」を線分の頂点として動的頂点バッファへ書き込み、`D3D_PRIMITIVE_TOPOLOGY_LINELIST`で描画する。
- 線分描画専用の共通パイプライン（ルートシグネチャ・PSO）を`LineCommon`（`Engine/Graphics/3D/LineCommon.h/.cpp`）として新規に用意した。プロジェクト内に既存だった描画パイプラインはすべて三角形トポロジのみだったため、`D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE`を用いる新規パイプラインとして追加している。
- 専用シェーダー（`Engine/Graphics/Shaders/Line/Line.VS.hlsl`, `Line.PS.hlsl`）を新規作成し、単色（緑）でジョイント間を結ぶ線を描画する。
- `Game/scenes/GameScene`のImGui「Character Bone Control」内に「Bone Debug Display」チェックボックスを追加し、ON/OFFを切り替え可能にした。ONの場合、`GameScene::Draw()`内で`Skeleton::DrawDebug`を呼び出し、アニメーション再生中の骨格に追従する形でデバッグ線を描画する。

## 動作確認方法（Release版exeでの操作キー／パッド）

Release構成では`USE_IMGUI`が定義されずImGuiが使用できないため、キー入力とゲームパッド（XInput、0番）のみで全操作・加点要素の動作確認ができるようにした。Debug/Development構成ではImGuiのチェックボックス・ボタンからも同じ操作が可能。

### タイトル画面（`Game/scenes/TitleScene.cpp`）

| キー | パッド | 効果 |
| --- | --- | --- |
| テンキー/数字キー 1 | 十字キー ↑ | ポストプロセス「Default」 |
| テンキー/数字キー 2 | 十字キー ↓ | ポストプロセス「BoxFilter」 |
| テンキー/数字キー 3 | 十字キー ← | ポストプロセス「Grayscale」 |
| テンキー/数字キー 4 | 十字キー → | ポストプロセス「Vignette」 |
| テンキー/数字キー 5 | A ボタン | ポストプロセス「GaussianBlur」 |
| テンキー/数字キー 6 | B ボタン | ポストプロセス「LuminanceOutline」 |
| テンキー/数字キー 7 | X ボタン | ポストプロセス「DepthOutline」 |
| テンキー/数字キー 8 | Y ボタン | ポストプロセス「RadialBlur」 |
| テンキー/数字キー 9 | LB (左ショルダー) | ポストプロセス「Dissolve」（アニメーション開始） |
| テンキー/数字キー 0 | RB (右ショルダー) | ポストプロセス「Random」 |
| Enter | 右スティック押し込み | グリッチ演出を発動 |
| Space | START ボタン | ゲームシーンへ遷移 |

### ゲームプレイ画面（`Game/scenes/GameScene.cpp`）

| キー | パッド | 効果 |
| --- | --- | --- |
| W / A / S / D | 左スティック | キャラクター移動 |
| F1 | Y ボタン | 骨のデバッグ表示 ON/OFF（キャラクターは半透明化） |
| F2 | X ボタン | ジョイントのローカル軸表示 ON/OFF |
| F3 | B ボタン | ジョイント名表示 ON/OFF |
| 1 | 十字キー ↑ | GPUパーティクル「Shockwave」を発生 |
| 2 | 十字キー ↓ | GPUパーティクル「Spark」を発生 |
| 3 | 十字キー ← | GPUパーティクル「Smoke」を発生 |
| 4 | 十字キー → | GPUパーティクル「Charge」を発生 |
| 5 | LB (左ショルダー) | GPUパーティクル「Aura」を発生 |
| 6 | RB (右ショルダー) | GPUパーティクル「Warp」を発生 |

## 不具合修正: Release版exeが起動直後に例外で落ちる

### 原因
`DXCommon::CompileShader`（`Engine/Base/DXCommon.cpp`）内で、`assert(SUCCEEDED(dxcUtils->LoadFile(...)))` のように**呼び出し自体をassertの引数に直接書いていた**ことが原因。
`assert`はNDEBUG定義時（Release構成）にマクロごと消える仕様のため、Release版では中の`LoadFile`呼び出し自体が実行されず、`shaderSource`が未初期化のまま次の行で`shaderSource->GetBufferPointer()`を呼び出しアクセス違反（例外）が発生していた。`shaderResult->GetOutput(DXC_OUT_OBJECT, ...)`も同様の書き方だったため同じ問題を抱えていた。

cdb（WinDbgのコマンドライン版）でRelease版exeを実際に起動して解析したところ、`ParticleManager::CreateGraphicsPipeline`が最初にコンパイルするシェーダー`Particle.VS.hlsl`の読み込み時にちょうどこの問題が表面化しており、「Particle.VSが怪しい」という見立ては結果的に正しかった（実際には特定のシェーダー固有の問題ではなく、`CompileShader`関数自体の共通のバグだった）。

### 修正内容
`HRESULT`変数で呼び出し結果を受けてから`assert(SUCCEEDED(hr))`でチェックする、プロジェクト内の他コードと同じ書き方に修正した。これによりRelease構成でも呼び出しが確実に実行されるようになった。

### 副次対応: Release版exeを単体で動かせるように
併せて、Release構成のビルド後処理（`MyGameEngine.vcxproj`のPostBuildEvent）に、シェーダーソース（`Engine/Graphics/Shaders`）とリソース（`resource`）フォルダを出力先(`$(TargetDir)`)へコピーする処理を追加した。これにより、Visual Studioを介さずに`generated/outputs/Release`フォルダのexeを直接ダブルクリックしても、相対パスで読み込んでいるシェーダー・モデル・テクスチャ等が正しく見つかるようになる。

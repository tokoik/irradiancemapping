# irradiancemapping - イラディアンスマッピングによる大域照明

## 1. 概要

本プログラムは、イラディアンス環境マップ（Irradiance Environment Map）を用いて、ディフューズ反射光の畳み込み計算結果を事前生成したテクスチャから高速に取得し、リアルタイムに環境光照度を反映するサンプルプログラムです。

- 移行元ブログ記事:
  - [イラディアンスマッピング (1) - 床井研究室](https://tokoik.github.io/blog/2015/08/26/)
  - [イラディアンスマッピング (2) - 床井研究室](https://tokoik.github.io/blog/2015/08/28/)

## 2. 対応環境

- **Windows**: Visual Studio 2019 / 2022 / 2026 (CMake 経由で GLFW を自動構成)
- **macOS**: Xcode (GLFW を自動ダウンロード、OpenGL Framework を使用)
- **Ubuntu Linux**: GCC / Make (システム標準の libglfw3-dev, libgl1-mesa-dev を使用)

## 3. ビルド手順

### Windows (Visual Studio)

```pwsh
cmake -B build -S .
cmake --build build --config Release
```

### macOS (Xcode)

```bash
cmake -B build -G Xcode
cmake --build build --config Release
```

### Ubuntu Linux (Makefile)

```bash
sudo apt-get update
sudo apt-get install -y libglfw3-dev libgl1-mesa-dev
cmake -B build -S .
cmake --build build
```

## 4. 起動方法

ビルド完了後、生成された実行ファイルを実行します。

- **Windows**: `build/Release/irradiancemapping.exe`
- **macOS**: `build/Release/irradiancemapping.app`
- **Linux**: `build/irradiancemapping`

## 5. 操作方法

- **マウス左ドラッグ**: シーンの視点回転
- **マウス右ドラッグ**: 視点の平行移動
- **マウスホイール**: ズーム
- **[SPACE]**: マッピング手法 / 表示モデルの切り替え
- **[q] / [Q] / [ESC]**: プログラムの終了

## 6. プログラムの解説

全天球画像（キューブマップまたは正距円錐図法）からディフューズ成分を球面調和関数または数値積分により前計算して求めたイラディアンスマップを参照し、スタンフォードバニー（`bunny.obj`）等のモデルに複雑な環境光ライティングを高速に適用しています。

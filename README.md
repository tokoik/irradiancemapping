irradiancemapping
-----------------

放射照度マッピングの解説用デモプログラム

    Copyright (c) 2011, 2012, 2013, 2014, 2015 Kohe Tokoi. All Rights Reserved.
    
    Permission is hereby granted, free of charge,  to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction,  including without limitation the rights
    to use, copy,  modify, merge,  publish, distribute,  sublicense,  and/or sell
    copies or substantial portions of the Software.
    
    The above  copyright notice  and this permission notice  shall be included in
    all copies or substantial portions of the Software.
    
    THE SOFTWARE  IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
    IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO THE WARRANTIES  OF MERCHANTABILITY,
    FITNESS  FOR  A PARTICULAR PURPOSE  AND NONINFRINGEMENT.  IN  NO EVENT  SHALL
    KOHE TOKOI  BE LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY,  WHETHER IN
    AN ACTION  OF CONTRACT,  TORT  OR  OTHERWISE,  ARISING  FROM,  OUT OF  OR  IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

* GLFW の version 3 を使っています
* シェーダを使わず OpenGL の regacy API だけで実装しています
* Linux 用 Makefile, Xcode 6 プロジェクト, Visual Studio 2013 ソリューション付き
* マウスの左ボタンドラッグでシーンを回転できます
* ホイールでカメラを前後移動できます
* 左右の矢印キーでテクスチャを切り替えられます
* 上下の矢印キーで明るさを調整できます
* 形状データには Alias OBJ ですがテッセレーションしないのであらかじめ三角形分割してください

## 放射照度マップの作成について

* main.cpp の記号定数 USEMAP を 0 にすると天空画像から放射照度マップと環境マップを作成します
* 天空画像には等距離射影方式の魚眼レンズで撮影した Targa (TGA) 形式の画像を指定してください
* 画像の中央の min(画像の幅, 画像の高さ, 定数 skysize) 画素の正方形を天空画像として使います
* 作成する画像の大きさは定数 mapsize に指定します
* 定数 ambient は天空画像の範囲外の明るさとして使用しています
* 定数 shininess を大きくすると環境マップがシャープになります

### 注意

放射照度マップの作成には, iMac Late 2013 (3.2 GHz Intel Core i5) で,
天空画像が 256x256, mapsize が 256 の設定で 1 枚あたり 5 分かかります.

放射照度マップは非常にぼけるので, 本当は天空画像の全面を平滑する必要はありません.
数十ポイントランダムサンプリングすれば十分だと思います.

映り込みをボケさせたくない場合は, 天空画像そのものを環境マップに使ってください.
その場合, 画像は 2^n x 2^n 画素の正方形である必要があります.

Windows だと Debug ビルドでは OBJ ファイルの読み込みに非常に時間が狩ります.
またいずれのプラットフォームでも, Debug ビルドでは放射照度マップの作成に時間がかかります.

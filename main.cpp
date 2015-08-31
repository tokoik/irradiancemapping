#define _USE_MATH_DEFINES
#define NOMINMAX
#include <cmath>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

// 事前計算したマップを使用するなら 1
#define USEMAP 1

// ウィンドウ関連の処理
#include "Window.h"

namespace
{
  //
  // 三角形分割した Alias OBJ 形式の形状データファイル
  //
  const char filename[] = "bunny.obj";

#if USEMAP
  //
  // 放射照度マップ
  //
  const char *const irrmaps[] =
  {
    "irrmap0.tga",
    "irrmap1.tga",
    "irrmap2.tga",
    "irrmap3.tga",
    "irrmap4.tga",
    "irrmap5.tga",
    "irrmap6.tga",
    "irrmap7.tga",
    "irrmap8.tga",
    "irrmap9.tga",
    "irrmapa.tga",
    "irrmapb.tga",
    "irrmapc.tga",
    "irrmapd.tga",
    "irrmape.tga",
    "irrmapf.tga",
    "irrmapg.tga",
    "irrmaph.tga"
  };

  //
  // 環境マップ
  //
  const char *const envmaps[] =
  {
    "envmap0.tga",
    "envmap1.tga",
    "envmap2.tga",
    "envmap3.tga",
    "envmap4.tga",
    "envmap5.tga",
    "envmap6.tga",
    "envmap7.tga",
    "envmap8.tga",
    "envmap9.tga",
    "envmapa.tga",
    "envmapb.tga",
    "envmapc.tga",
    "envmapd.tga",
    "envmape.tga",
    "envmapf.tga",
    "envmapg.tga",
    "envmaph.tga"
  };

  //
  // 放射照度マップの数
  //
  const size_t mapcount(sizeof irrmaps / sizeof irrmaps[0]);
#else
  //
  // 距離射影方式の魚眼レンズで撮影した天空画像
  //
  const char *const skymaps[] =
  {
#if 0
    "skymap0.tga",
    "skymap1.tga",
    "skymap2.tga",
    "skymap3.tga",
    "skymap4.tga",
    "skymap5.tga",
    "skymap6.tga",
    "skymap7.tga",
    "skymap8.tga",
#endif
    "skymap9.tga"
  };

  //
  // 天空画像の数
  //
  const size_t mapcount(sizeof skymaps / sizeof skymaps[0]);

  //
  // 大域環境光強度
  //
  const GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };

  //
  // 輝き係数
  //
  const GLfloat shininess(60.0f);

  //
  // 天空画像中の天空領域の直径の最大値
  //
  const GLsizei skysize(1024);

  //
  // 作成するテクスチャのサイズ
  //
  const GLsizei imapsize(256);
  const GLsizei emapsize(256);

  //
  // フィルタのサンプル数
  //
  const unsigned int isamples(256);
  const unsigned int esamples(256);
#endif

  //
  // 放物面マッピング用のテクスチャ変換行列
  //
  const GLfloat paraboloid[] =
  {
    -1.0f,  0.0f,  0.0f,  0.0f,
     1.0f,  1.0f,  0.0f,  2.0f,
     0.0f, -1.0f,  0.0f,  0.0f,
     1.0f,  1.0f,  0.0f,  2.0f,
  };

  //
  // 床の描画
  //
  void floor(int size, GLfloat height)
  {
    // タイル１枚の頂点データ
    static const GLfloat floorvert[] =
    {
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f,
      1.0f, 0.0f, 1.0f,
      1.0f, 0.0f, 0.0f
    };

    // タイルの拡散反射係数
    static const GLfloat floordiff[][4] =
    {
      { 0.6f, 0.6f, 0.6f, 1.0f },
      { 0.3f, 0.3f, 0.3f, 1.0f }
    };

    // タイルの鏡面反射係数
    static const GLfloat floorspec[] =
    {
      0.1f, 0.1f, 0.1f, 1.0f
    };

    // 床のポリゴンの頂点位置を頂点配列から取得する
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, floorvert);

    // 床のポリゴンの法線ベクトルは glNormal3f() で設定する
    glDisableClientState(GL_NORMAL_ARRAY);
    glNormal3f(0.0f, 1.0f, 0.0f);

    // 床のポリゴンの拡散反射光と鏡面反射光を鏡面反射係数で配分する
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, floorspec);

    // 床の描画
    for (int j = -size; j < size; ++j)
    {
      for (int i = -size; i < size; ++i)
      {
        // 床のポリゴンの拡散反射係数を primary color (頂点色) に設定する
        glColor3fv(floordiff[(i + j) & 1]);

        // 床のポリゴンを描画する
        glPushMatrix();
        glTranslatef(GLfloat(i), height, GLfloat(j));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glPopMatrix();
      }
    }
  }

  //
  // シーンの描画
  //
  void scene(GLuint ng, const GLuint (*group)[2], const GLfloat (*diff)[4], const GLfloat (*spec)[4],
    GLuint nv, const GLfloat (*pos)[3], const GLfloat (*norm)[3])
  {
    // オブジェクトの頂点位置を頂点配列から取得する
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, pos);

    // オブジェクトの法線ベクトルを頂点配列から取得する
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, norm);

    // オブジェクトの描画
    for (unsigned int g = 0; g < ng; ++g)
    {
      // 拡散反射光と鏡面反射光を鏡面反射係数で配分する
      glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, spec[g]);

      // 拡散反射係数を primary color (頂点色) に設定する
      glColor4fv(diff[g]);

      // オブジェクトを描画する
      glDrawArrays(GL_TRIANGLES, group[g][0], group[g][1]);
    }
  }

  //
  // テクスチャの作成
  //
  void createTexture(const GLubyte *buffer, GLsizei width, GLsizei height, GLenum format,
    const GLfloat *amb, GLuint tex)
  {
    // テクスチャオブジェクトにテクスチャを割り当てる
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, buffer);

    // テクスチャは線形補間する
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // 境界色を拡張する
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // テクスチャの境界色に大域環境光を設定する
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, amb);
  }

#if USEMAP
  //
  // 放射照度マップの読み込み
  //
  bool loadMap(const char *iname, const char *ename, GLuint imap, GLuint emap)
  {
    // 終了ステータス
    bool status(true);

    // 読み込んだ画像の幅と高さ, フォーマット
    GLsizei width, height;
    GLenum format;

    // 放射照度マップの読み込み
    GLubyte const *const itexture(ggLoadTga(iname, &width, &height, &format));

    // 画像が読み込めなければ終了
    if (!itexture) status = false;

    // 読み込んだ画像の左上隅の画素の色を大域環境光とする
    const GLfloat iamb[] = { itexture[2] / 255.0f, itexture[1] / 255.0f, itexture[0] / 255.0f };

    // テクスチャの作成
    createTexture(itexture, width, height, format, iamb, imap);

    // 読み込んだデータはもう使わないのでメモリを解放する
    delete[] itexture;

    // 放射照度マップの読み込み
    GLubyte const *const etexture(ggLoadTga(ename, &width, &height, &format));

    // 画像が読み込めなければ終了
    if (!etexture) status = false;

    // 読み込んだ画像の左上隅の画素の色を大域環境光とする
    const GLfloat eamb[] = { etexture[2] / 255.0f, etexture[1] / 255.0f, etexture[0] / 255.0f };

    // テクスチャの作成
    createTexture(etexture, width, height, format, eamb, emap);

    // 読み込んだデータはもう使わないのでメモリを解放する
    delete[] etexture;

    // すべて読み込み成功
    return status;
  }
#else
  //
  // 一様乱数発生 (Xorshift 法)
  //
  GLfloat xor128()
  {
    static unsigned int x(123456789);
    static unsigned int y(362436069);
    static unsigned int z(521288629);
    static unsigned int w(88675123);
    const unsigned int t(x ^ x << 11);

    // データの入れ替え
    x = y;
    y = z;
    z = w;

    //
    return GLfloat(w ^= w >> 19 ^ t ^ t >> 8) * 2.3283064e-10f;
  }

  //
  // サンプラーの作成
  //
  void createSampler(unsigned int samples, GLfloat(*sample)[4], GLfloat n)
  {
    // e ← 1 / (n + 1)
    const GLfloat e(1.0f / (n + 1.0f));

    for (unsigned int i = 0; i < samples; ++i)
    {
      const GLfloat y(pow(1.0f - xor128(), e));
      const GLfloat r(sqrt(1.0f - y * y));
      const GLfloat t(6.2831853f * xor128());
      const GLfloat x(r * cos(t)), z(r * sin(t));

      (*sample)[0] = x;
      (*sample)[1] = y;
      (*sample)[2] = z;
      (*sample)[3] = y; // (0, 1, 0) との内積
      ++sample;
    }
  }

  //
  // サンプラーの回転
  //
  void rotateSampler(unsigned int samples, const GLfloat (*sample)[4],
    const GLfloat x, const GLfloat y, const GLfloat z, GLfloat (*result)[4])
  {
    // a ← x^2 + z^2;
    const GLfloat a(x * x + z * z);

    // 回転する必要があるとき
    if (a > 0)
    {
      // l ← length(x, z);
      const GLfloat l(sqrt(a));

      // m ← [(x, y, z) x (0, 1, 0), (x, y, z), (x, y, z) x (0, 1, 0) x (x, y, z)]
      const GLfloat m00(-z / l), m10(0.0f), m20(x / l);
      const GLfloat m01(x), m11(y), m21(z);
      const GLfloat m02(-m20 * y), m12(l), m22(m00 * y);

      // 回転してコピー
      for (unsigned int i = 0; i < samples; ++i)
      {
        result[i][0] = m00 * sample[i][0] + m01 * sample[i][1] + m02 * sample[i][2];
        result[i][1] = m10 * sample[i][0] + m11 * sample[i][1] + m12 * sample[i][2];
        result[i][2] = m20 * sample[i][0] + m21 * sample[i][1] + m22 * sample[i][2];
        result[i][3] = sample[i][3];
      }

      return;
    }

    // 回転する必要がなければそのままコピー
    for (unsigned int i = 0; i < samples; ++i)
    {
      result[i][0] = sample[i][0];
      result[i][1] = sample[i][1];
      result[i][2] = sample[i][2];
      result[i][3] = sample[i][3];
    }
  }

  //
  // 平滑化
  //
  void smooth(const GLubyte *src, GLsizei width, GLsizei height, GLenum format,
    GLsizei cs, GLsizei ct, GLsizei rs, GLsizei rt, unsigned int samples,
    GLubyte *dst, GLsizei size, const GLfloat *amb, GLfloat shi)
  {
    // サンプラー
    GLfloat (*const sampler)[4](new GLfloat[samples][4]);
    createSampler(samples, sampler, shi);

    // 回転したサンプラーの保存先
    GLfloat (*const rsampler)[4](new GLfloat[samples][4]);

    // チャンネル数
    const int channels(format == GL_BGRA ? 4 : 3);

    // 大域環境光強度
    const GLfloat ramb(amb[0] * 255.0f), gamb(amb[1] * 255.0f), bamb(amb[2] * 255.0f);

    // 放射照度マップの各画素について
    for (int dt = 0; dt < size; ++dt)
    {
      std::cout << "Processing line: " << dt
        << " (" << std::fixed << std::setprecision(1) << float(dt) * 100.0f / float(size) << "%)"
        << std::endl;

      for (int ds = 0; ds < size; ++ds)
      {
        // この画素の放射照度マップの配列 dst のインデックス
        const int id((dt * size + ds) * 3);

        // この画素の放射照度マップ上の正規化された座標値 (-1 ≦ u, v ≦ 1)
        const float du(float(ds * 2) / float(size - 1) - 1.0f);
        const float dv(1.0f - float(dt * 2) / float(size - 1));

        // 放射照度マップを放物面マップとして参照するときのこの画素の方向ベクトル q
        const float qx(du);
        const float qy(1.0f - du * du - dv * dv);
        const float qz(dv);

        // この画素が放射照度マップの単位円外にあるとき
        if (qy <= 0.0f)
        {
          // 大域環境光を設定する
          dst[id + 0] = GLubyte(ramb);
          dst[id + 1] = GLubyte(gamb);
          dst[id + 2] = GLubyte(bamb);
          continue;
        }

        // サンプラーをこのベクトルの方向に向ける
        rotateSampler(samples, sampler, qx, qy, qz, rsampler);

        // このベクトルの方向を天頂とする半天球からの放射照度の総和
        float rsum(0.0f), gsum(0.0f), bsum(0.0f);

        for (unsigned int i = 0; i < samples; ++i)
        {
          // 天空に向かうベクトル
          const GLfloat &px(rsampler[i][0]);
          const GLfloat &py(rsampler[i][1]);
          const GLfloat &pz(rsampler[i][2]);
          const GLfloat &pq(rsampler[i][3]);  // p と q の内積

          // このベクトル p が天空画像の領域の外 (裏側) を指しているとき
          if (py <= 0.0f)
          {
            // 大域環境光を加算する
            rsum += ramb;
            gsum += gamb;
            bsum += bamb;
            continue;
          }

          // このベクトルの xz 平面上の長さの 2 乗
          const GLfloat l(1.0f - py * py);

          // このベクトルの xz 平面上の長さに対する天頂角から求めた天空画像の中心からの正規化された距離の比
          const GLfloat r(l > 0.0f ? acos(py) * 2.0f / (float(M_PI) * sqrt(l)) : 0.0f);

          // このベクトルの向いている方向の天空画像における正規化された座標値 (-1 ≦ u, v ≦ 1)
          const GLfloat su(px * r);
          const GLfloat sv(pz * r);

          // この画素の天空画像上の画素位置
          const int ss(int(round(float(rs) * su)) - cs);
          const int st(ct - int(round(float(rt) * sv)));

          // この画素の天空画像の配列 src のインデックス
          const int is((st * width + ss) * channels);

          // 天空画像 src の画素値を放射照度マップ dst の画素に加算する
          rsum += float(src[is + 2]);
          gsum += float(src[is + 1]);
          bsum += float(src[is + 0]);
        }

        // 放射照度マップの画素値の平均を求める
        dst[id + 0] = GLubyte(round(rsum / float(samples)));
        dst[id + 1] = GLubyte(round(gsum / float(samples)));
        dst[id + 2] = GLubyte(round(bsum / float(samples)));
      }
    }

    // サンプラに使ったメモリを開放する
    delete[] sampler;
    delete[] rsampler;
  }

  //
  // 放射照度マップの作成
  //
  bool createMap(const char *name, GLsizei diameter,
    GLuint imap, GLsizei isize, unsigned int isamples,
    GLuint emap, GLsizei esize, unsigned int esamples,
    const GLfloat *amb, GLfloat shi)
  {
    // 作成したテクスチャの数
    static int count(0);

    // 読み込んだ画像の幅と高さ, フォーマット
    GLsizei width, height;
    GLenum format;

    // 天空画像ファイルの読み込み
    GLubyte const *const texture(ggLoadTga(name, &width, &height, &format));

    // 画像が読み込めなければ終了
    if (!texture) return false;

    // この画像の中心位置
    const GLsizei cx(width / 2), cy(height / 2);

    // diameter, width, height の最小値の 1 / 2 を radius にする
    const GLsizei radius(std::min(diameter, std::min(width, height)) / 2);

    // 平滑した放射照度マップの一時保存先
    std::vector<GLubyte> itemp(isize * isize * 3);

    // 放射照度マップ用に平滑する
    smooth(texture, width, height, format, cx, cy, radius, radius, isamples, &itemp[0], isize, amb, 1.0f);

    // 放射照度マップのテクスチャを作成する
    createTexture(&itemp[0], isize, isize, GL_RGB, amb, imap);

    // 作成したテクスチャを保存する
    std::stringstream imapname;
    imapname << "irr" << std::setfill('0') << std::setw(5) << std::right << count << ".tga";
    ggSaveTga(isize, isize, 3, &itemp[0], imapname.str().c_str());

    // 平滑した環境マップの一時保存先
    std::vector<GLubyte> etemp(esize * esize * 3);

    // 環境マップ用に平滑する
    smooth(texture, width, height, format, cx, cy, radius, radius, esamples, &etemp[0], esize, amb, shi);

    // 環境マップのテクスチャを作成する
    createTexture(&etemp[0], esize, esize, GL_RGB, amb, emap);

    // 作成したテクスチャを保存する
    std::stringstream emapname;
    emapname << "env" << std::setfill('0') << std::setw(5) << std::right << count << ".tga";
    ggSaveTga(esize, esize, 3, &etemp[0], emapname.str().c_str());

    // 読み込んだデータはもう使わないのでメモリを解放する
    delete[] texture;

    // 作成したテクスチャの数を数える
    ++count;

    return true;
  }
#endif

  //
  // 放射照度マップに使うテクスチャうニットの設定
  //
  void irradiance()
  {
    // テクスチャ座標に法線ベクトルを使う
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);

    // 放射照度マップの値をかさ上げする Ce ← Cb + Ct
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);          // 加算
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_CONSTANT);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);   // Cb ← GL_TEXTURE_ENV_COLOR の RGB 値
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);   // Ct ← 放射照度マップの値

    // テクスチャ座標の変換行列に放物面マッピング用の変換行列を設定する
    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(paraboloid);
  }

  //
  // 拡散反射光強度の算出につかうテクスチャユニットの設定
  //
  void diffuse()
  {
    // 物体の色（頂点色の補間値）に前レイヤで求めた入射光強度をかける Cd ← Cv * Ce
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);     // 乗算
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);   // Cv ← 頂点色の補間値
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);   // Ce ← かさ上げした放射照度 (前レイヤ)
  }

  //
  // 環境マップの加算に使うテクスチャユニットの設定
  //
  void reflection()
  {
    // テクスチャ座標に反射ベクトルを使う
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);

    // 環境マップの値と前レイヤで求めた拡散反射光強度を鏡面反射係数で比例配分する C ← Ct * Cs + Cd * (1 - Cs)
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);  // 補間
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);   // Ct ← 環境マップの値
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);   // Cd ← 拡散反射光強度 (前レイヤ)
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);   // Cs ← 鏡面反射係数

    // テクスチャ座標の変換行列に放物面マッピング用の変換行列を設定する
    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(paraboloid);
  }

  //
  // プログラム終了時の処理
  //
  void cleanup()
  {
    // GLFW の終了処理
    glfwTerminate();
  }
}

//
// メインプログラム
//
int main()
{
  // GLFW を初期化する
  if (glfwInit() == GL_FALSE)
  {
    // 初期化に失敗した
    std::cerr << "Can't initialize GLFW" << std::endl;
    return 1;
  }

  // プログラム終了時の処理を登録する
  atexit(cleanup);

  // ウィンドウを作成する
  Window window("Irradiance Mapping", 960, 540);

  // OpenGL の初期設定
  glClearColor(0.3f, 0.5f, 0.8f, 0.0f);
  glEnable(GL_NORMALIZE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_MULTISAMPLE);

  // 陰影付けを無効にする
  glDisable(GL_LIGHTING);

  // テクスチャ
  GLuint imap[mapcount], emap[mapcount];
  glGenTextures(mapcount, imap);
  glGenTextures(mapcount, emap);

  // テクスチャの読み込み
  for (size_t i = 0; i < mapcount; ++i)
  {
#if USEMAP
    loadMap(irrmaps[i], envmaps[i], imap[i], emap[i]);
#else
    createMap(skymaps[i], skysize, imap[i], imapsize, isamples, emap[i], emapsize, esamples, ambient, shininess);
#endif
  }

  // 放射照度マップのかさ上げに使うテクスチャユニットの設定
  glActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  irradiance();

  // 放射照度マップのかさ上げに使うテクスチャユニットの設定
  glActiveTexture(GL_TEXTURE1);
  glEnable(GL_TEXTURE_2D);
  diffuse();

  // 環境マップの加算に使うテクスチャユニットの設定
  glActiveTexture(GL_TEXTURE2);
  glEnable(GL_TEXTURE_2D);
  reflection();

  // 材質データ
  GLuint ng;
  GLuint (*group)[2];
  GLfloat (*amb)[4], (*diff)[4], (*spec)[4], *shi;

  // 形状データ
  GLuint nv;
  GLfloat (*pos)[3], (*norm)[3];

  // 形状データの読み込み
  ggLoadObj(filename, ng, group, amb, diff, spec, shi, nv, pos, norm, false);

  // ウィンドウが開いている間繰り返す
  while (window.shouldClose() == GL_FALSE)
  {
    // ウィンドウを消去する
    window.clear();

    // テクスチャの選択
    const int select(window.getSelection() % mapcount);

    // 明るさ
    GLfloat brightness[4];
    window.getBrightness(brightness);

    // 放射照度マップのかさ上げ
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, imap[select]);
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, brightness);

    // 拡散反射光強度の算出
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, imap[select]);

    // 環境マッピング
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, emap[select]);

    // モデルビュー変換行列の設定
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 視点の移動
    glTranslatef(window.getPosition()[0], window.getPosition()[1], window.getPosition()[2]);

    // トラックボール処理による回転
    glMultMatrixf(window.getTb());

    // シーンの描画
    scene(ng, group, diff, spec, nv, pos, norm);

    // 床の描画
    floor(5, -1.0f);

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }
}

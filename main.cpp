#define _USE_MATH_DEFINES
#define NOMINMAX
#include <cmath>
#include <cstdlib>
#include <vector>
#include <iostream>

// ウィンドウ関連の処理
#include "Window.h"

namespace
{
  //
  // 三角形分割した Alias OBJ 形式の形状データファイル
  //
  const char filename[] = "bunny.obj";

  //
  // 光源の位置
  //
  const GLfloat lightPos[][4] =
  {
    { 0.0f, 5.0f, 10.0f, 1.0f },
    { 10.0f, 2.0f, 1.0f, 1.0f }
  };

  //
  // 光源の色
  //
  const GLfloat lightCol[][4] =
  {
    { 0.8f, 0.8f, 0.8f, 1.0f },
    { 0.4f, 0.4f, 0.4f, 1.0f }
  };

  //
  // 環境光の色
  //
  const GLfloat lightAmb[][4] =
  {
    { 0.1f, 0.1f, 0.1f, 1.0f },
    { 0.1f, 0.1f, 0.1f, 1.0f }
  };

  //
  // 光源の数
  //
  const size_t lightCount(sizeof lightPos / sizeof lightPos[0]);

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

    // 床のポリゴンの鏡面反射係数を設定する
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, floorspec);

    // 床の描画
    for (int j = -size; j < size; ++j)
    {
      for (int i = -size; i < size; ++i)
      {
        // 床のポリゴンの拡散反射係数を設定する
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, floordiff[(i + j) & 1]);

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
  void scene(GLuint ng, const GLuint (*group)[2], const GLfloat (*diff)[4], const GLfloat (*spec)[4], const GLfloat *shi,
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
      // 拡散反射係数を設定する
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diff[g]);

      // 鏡面反射係数と輝き係数を設定する
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec[g]);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shi[g] > 128.0f ? 128.0f : shi[g]);

      // オブジェクトを描画する
      glDrawArrays(GL_TRIANGLES, group[g][0], group[g][1]);
    }
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

  // 陰影付けを有効にする
  glEnable(GL_LIGHTING);

  // OpenGL の光源の設定
  for (size_t i = 0; i < lightCount; ++i)
  {
    const GLenum light(GL_LIGHT0 + i);
    glEnable(light);
    glLightfv(light, GL_POSITION, lightPos[i]);
    glLightfv(light, GL_DIFFUSE, lightCol[i]);
    glLightfv(light, GL_SPECULAR, lightCol[i]);
    glLightfv(light, GL_AMBIENT, lightAmb[i]);
  }

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

    // モデルビュー変換行列の設定
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 視点の移動
    glTranslatef(window.getPosition()[0], window.getPosition()[1], window.getPosition()[2]);

    // トラックボール処理による回転
    glMultMatrixf(window.getTb());

    // シーンの描画
    scene(ng, group, diff, spec, shi, nv, pos, norm);

    // 床の描画
    floor(5, -1.0f);

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }
}

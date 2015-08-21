#pragma once

#include <cstdlib>
#include <iostream>

#if defined(_WIN32)
#  pragma comment(lib, "glu32.lib")
#elif defined(__APPLE__)
#  define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#  include <OpenGL/gl.h>
#  include <OpenGL/glext.h>
#endif
#define GLFW_INCLUDE_GLU
#include "gg.h"
using namespace gg;

//
// ウィンドウ関連の処理
//
class Window
{
  // ウィンドウの識別子
  GLFWwindow *const window;

  // トラックボール処理
  GgTrackball tb;

  // 回転中心に対するカメラの位置
  GLfloat position[3];

public:

  // コンストラクタ
  Window(const char *title = "Game Graphics", int width = 640, int height = 480);

  // デストラクタ
  virtual ~Window();

  // ウィンドウを閉じるべきかを判定する
  int shouldClose() const
  {
    return glfwWindowShouldClose(window) | glfwGetKey(window, GLFW_KEY_ESCAPE);
  }

  // 画面クリア
  void clear() const
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  // カラーバッファを入れ替えてイベントを取り出す
  void swapBuffers();

  // キーボードをタイプした時の処理
  static void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods);

  // マウスボタンを操作したときの処理
  static void mouse(GLFWwindow *window, int button, int action, int mods);

  // マウスホイール操作時の処理
  static void wheel(GLFWwindow *window, double x, double y);

  // ウィンドウのサイズ変更時の処理
  static void resize(GLFWwindow *window, int width, int height);

  // トラックボールによる回転を取り出す
  const GLfloat *getTb() const
  {
    return tb.get();
  }

  // トラックボールによる回転を取り出す
  const GLfloat *getTbTransposed() const
  {
    return tb.getMatrix().transpose().get();
  }

  // 回転中心に対するカメラの位置を取り出す
  const GLfloat *getPosition() const
  {
    return position;
  }
};

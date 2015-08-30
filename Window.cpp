//
// Window 関連の処理
//
#include <cmath>
#include "Window.h"

// カメラの画角
const GLdouble fovy(40.0);

// 前方面の位置
const GLdouble zNear(1.0);

// 後方面の位置
const GLdouble zFar(20.0);

// 回転中心までの距離の初期値
const GLfloat initialPosition[] = { 0.0f, 0.0f, 5.0f };

// ホイールによる前後移動の速度
const GLfloat distanceStep(0.1f);

//
// コンストラクタ
//
Window::Window(const char *title, int width, int height)
  : window(glfwCreateWindow(width, height, title, NULL, NULL))
{
  if (window == NULL)
  {
    // ウィンドウが作成できなかった
    std::cerr << "Can't create GLFW window." << std::endl;
    exit(1);
  }

  // 現在のウィンドウを処理対象にする
  glfwMakeContextCurrent(window);

  // 作成したウィンドウに対する設定
  glfwSwapInterval(1);

  // ゲームグラフィックス特論の都合にもとづく初期化
  ggInit();

  // このインスタンスの this ポインタを記録しておく
  glfwSetWindowUserPointer(window, this);

  // キーボードを操作した時の処理
  glfwSetKeyCallback(window, keyboard);

  // マウスボタンを操作したときの処理
  glfwSetMouseButtonCallback(window, mouse);

  // マウスホイール操作時に呼び出す処理
  glfwSetScrollCallback(window, wheel);

  // ウィンドウのサイズ変更時に呼び出す処理を登録する
  glfwSetFramebufferSizeCallback(window, resize);

  // カメラの初期位置を設定する
  position[0] = -initialPosition[0];
  position[1] = -initialPosition[1];
  position[2] = -initialPosition[2];

  // ウィンドウの設定を初期化する
  resize(window, width, height);
}

//
// デストラクタ
//
Window::~Window()
{
  glfwDestroyWindow(window);
}

//
// カラーバッファを入れ替えてイベントを取り出す
//
void Window::swapBuffers()
{
  // カラーバッファを入れ替える
  glfwSwapBuffers(window);

  // OpenGL のエラーをチェックする
  ggError("SwapBuffers");

  // イベントを取り出す
  glfwPollEvents();

  // マウスの位置を調べる
  double x, y;
  glfwGetCursorPos(window, &x, &y);

  // 左ボタンドラッグ
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
  {
    // トラックボール回転
    tb.motion(static_cast<GLfloat>(x), static_cast<GLfloat>(y));
  }
}

//
// キーボードをタイプした時の処理
//
void Window::keyboard(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  // このインスタンスの this ポインタを得る
  Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

  if (instance)
  {
    if (action == GLFW_PRESS)
    {
      switch (key)
      {
        case GLFW_KEY_SPACE:
          break;
        case GLFW_KEY_BACKSPACE:
        case GLFW_KEY_DELETE:
          break;
        case GLFW_KEY_UP:
          break;
        case GLFW_KEY_DOWN:
          break;
        case GLFW_KEY_RIGHT:
          break;
        case GLFW_KEY_LEFT:
          break;
        default:
          break;
      }
    }
  }
}

//
// マウスボタンを操作したときの処理
//
void Window::mouse(GLFWwindow *window, int button, int action, int mods)
{
  // このインスタンスの this ポインタを得る
  Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

  if (instance)
  {
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    switch (button)
    {
      case GLFW_MOUSE_BUTTON_1:
        // トラックボール処理
        if (action)
        {
          // トラックボール処理開始
          instance->tb.start(static_cast<float>(x), static_cast<float>(y));
        }
        else
        {
          // トラックボール処理終了
          instance->tb.stop(static_cast<float>(x), static_cast<float>(y));
        }
        break;
      case GLFW_MOUSE_BUTTON_2:
        break;
      case GLFW_MOUSE_BUTTON_3:
        break;
      default:
        break;
    }
  }
}

//
// マウスホイール操作時の処理
//
void Window::wheel(GLFWwindow *window, double x, double y)
{
  // このインスタンスの this ポインタを得る
  Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

  if (instance)
  {
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL))
    {
      // Control キーが押されていれば左右位置を調整する
      instance->position[0] += distanceStep * static_cast<GLfloat>(y);
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT))
    {
      // Shift キーが押されていれば高さを調整する
      instance->position[1] += distanceStep * static_cast<GLfloat>(y);
    }
    else
    {
      // 何も押されていなければ
      if (fabs(x) > fabs(y))
      {
        // 左右位置を調整する
        instance->position[0] += distanceStep * static_cast<GLfloat>(x);
      }
      else
      {
        // 前後位置を調整する
        instance->position[2] += distanceStep * static_cast<GLfloat>(y);
      }
    }
  }
}

//
// ウィンドウのサイズ変更時の処理
//
void Window::resize(GLFWwindow *window, int width, int height)
{
  // このインスタンスの this ポインタを得る
  Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

  if (instance)
  {
    // ウィンドウ全体をビューポートにする
    glViewport(0, 0, width, height);

    // 投影変換行列を初期化する
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fovy, GLdouble(width) / GLdouble(height), zNear, zFar);

    // トラックボール処理の範囲を設定する
    instance->tb.region(width, height);
  }
}

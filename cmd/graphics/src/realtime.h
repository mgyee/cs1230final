#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <entt/entt.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

#include "utils/sceneparser.h"
#include "camera/camera.h"
#include "shapes/cone.h"
#include "shapes/cube.h"
#include "shapes/sphere.h"
#include "shapes/cylinder.h"
#include "clientwin.h"

struct glShape {
    RenderShapeData shape;
    unsigned long start;
    unsigned long length;
};

struct Position {
    glm::vec4 value;
};

struct Velocity {
    glm::vec4 value;
};

struct Renderable {
    // GLuint vao;
    // GLuint shader;
    unsigned long index;
    unsigned long vertexCount;

    glm::mat4 ctm;
    glm::vec4 cAmbient;
    glm::vec4 cDiffuse;
    glm::vec4 cSpecular;
    float shininess;
    glm::mat4 inverseTransposectm;
};

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);



public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    double m_devicePixelRatio;

    RenderData metaData;
    Camera m_camera;
    float aspectRatio;
    GLuint m_shader;
    GLuint m_vao;
    GLuint m_vbo = -1;
    std::vector<glShape> glShapes;
    Cone cone;
    Cube cube;
    Cylinder cylinder;
    Sphere sphere;

    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;

    void makeFBO();

    GLuint m_fbo_texture;
    GLuint m_fbo_renderbuffer;
    GLuint m_fbo;
    GLuint m_defaultFBO;
    GLuint m_texture_shader;

    float m_screen_width;
    float m_screen_height;

    int old_param1 = 0;
    int old_param2 = 0;

    // Final project 
    entt::registry registry;

    entt::entity camera_ent;

//TCPClient client;

    void updateVBO();
    void paintTexture(GLuint texture, bool pixelFilter, bool kernelFilter);
};

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
//#include "clientarm.h"
#include "clientwin.h"
#include "udpclientwin.h"
#include "udpclientarm.h"
#include <mutex>

struct glShape {
    RenderShapeData shape;
    unsigned long start;
    unsigned long length;
};

// struct Position {
//     glm::vec4 value;
// };

// struct Velocity {
//     glm::vec4 value;
// };

// struct Id {
// };

struct Player {
    int id;
    glm::vec4 position;
    glm::vec4 velocity;
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

struct SkyBox {
    unsigned int cubeMapTexture;
    GLuint vao;
    GLuint vbo;
    GLuint shader;
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
    std::mutex registry_mutex;

    // Player entity, needs an ID associated
    // when put into the registry, use -1 as shown below
    entt::entity camera_ent;

    float m_verticalVelocity = 0.0f;
    const float m_gravity = -9.8f;
    const float m_jumpSpeed = 5.0f;
    const float m_groundLevel = 1.0f;
    bool m_isJump = false;

    std::pair<bool, bool> isCollision(glm::vec4 camMin, glm::vec4 camMax);

    int my_id = -1;

//TCPClient client;

    void updateVBO();
    void paintTexture(GLuint texture, bool pixelFilter, bool kernelFilter);
    void run_client();

    // skybox parameters
    GLuint m_skybox_shader;
    GLuint m_skybox_vao;
    GLuint m_skybox_vbo;
    GLuint m_cubemap_texture;

    // fog parameters
    bool m_fogEnabled = false;
    glm::vec4 m_fogColor = glm::vec4(0.9f, 0.45f, 0.3f, 1.0f);
    float m_fogDensity = 0.12f;
    float m_fogStart = 3.0f;
    float m_fogEnd = 10.0f;
    float m_fogHeight = 0.5f;
    float m_fogBaseHeight = 0.5f;

    bool m_FXAAEnabled = false;

    void createSkybox();
    void loadCubeMap(std::vector<std::string> faces);
    void renderSkybox();
};

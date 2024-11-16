#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/shaderloader.h"

// ================== Project 5: Lights, Camera

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here

    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);

    glDeleteProgram(m_shader);

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here

    glClearColor(0, 0, 0, 1);

    m_shader = ShaderLoader::createShaderProgram("resources/shaders/default.vert", "resources/shaders/default.frag");

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(m_vao);

    glUseProgram(m_shader);

    for (auto &shape: glShapes) {
        glm::mat4 mvpMat = m_camera.getProjMatrix() * m_camera.getViewMatrix() * shape.shape.ctm;

        glUniformMatrix4fv(glGetUniformLocation(m_shader, "mvpMat"), 1, GL_FALSE, &mvpMat[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "modelMat"), 1, GL_FALSE, &shape.shape.ctm[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "invTransModelMat"), 1, GL_FALSE,
                           &glm::inverse(glm::transpose(shape.shape.ctm))[0][0]);

        glm::vec4 ambient = metaData.globalData.ka * shape.shape.primitive.material.cAmbient;
        glUniform4f(glGetUniformLocation(m_shader, "ambient"), ambient.r, ambient.g, ambient.b, ambient.a);

        glm::vec4 diffuse = metaData.globalData.kd * shape.shape.primitive.material.cDiffuse;
        glUniform4f(glGetUniformLocation(m_shader, "diffuse"), diffuse.r, diffuse.g, diffuse.b, diffuse.a);

        glm::vec4 specular = metaData.globalData.ks * shape.shape.primitive.material.cSpecular;
        glUniform4f(glGetUniformLocation(m_shader, "specular"), specular.r, specular.g, specular.b, specular.a);

        glUniform1f(glGetUniformLocation(m_shader, "shininess"), shape.shape.primitive.material.shininess);

        int i = 0;
        for (auto &light: metaData.lights) {
            if (light.type != LightType::LIGHT_DIRECTIONAL) {
                continue;
            }
            std::basic_string lightString = "lights[" + std::to_string(i) + "]";
            GLint loc = glGetUniformLocation(m_shader, (lightString + ".dir").c_str());
            glUniform3f(loc, light.dir.x, light.dir.y, light.dir.z);
            loc = glGetUniformLocation(m_shader, (lightString + ".color").c_str());
            glUniform4f(loc, light.color.r, light.color.g, light.color.b, light.color.a);
            i++;
        }
        glUniform1i(glGetUniformLocation(m_shader, "lightsCount"), i);

        glm::vec4 camPos = m_camera.getInverseViewMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0);

        glUniform4fv(glGetUniformLocation(m_shader, "camPos"), 1, &camPos[0]);

        glDrawArrays(GL_TRIANGLES, shape.start, shape.length);
    }

    glBindVertexArray(0);

    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    aspectRatio = float(w) / float(h);
    m_camera.setAspectRatio(aspectRatio);
    m_camera.setProjMatrix(settings.nearPlane, settings.farPlane);
}

void Realtime::sceneChanged() {

    SceneParser::parse(settings.sceneFilePath, metaData);
    // m_camera = Camera(metaData.cameraData.pos, metaData.cameraData.look, metaData.cameraData.up,
    //                 aspectRatio,
    //                 metaData.cameraData.heightAngle, metaData.cameraData.focalLength, metaData.cameraData.aperture);
    m_camera.setHeightAngle(metaData.cameraData.heightAngle);
    m_camera.setViewMatrix(metaData.cameraData.pos, metaData.cameraData.look, metaData.cameraData.up);
    m_camera.setProjMatrix(settings.nearPlane, settings.farPlane);
    updateVBO();
    update(); // asks for a PaintGL() call to occur
}

void Realtime::updateVBO() {
    if ((int)m_vbo < 0) {
        return;
    }
    std::vector<float> shapeData;
    unsigned long index = 0;
    glShapes.clear();

    for(auto &shape: metaData.shapes) {
        std::vector<float> tempData;

        switch (shape.primitive.type) {
        case PrimitiveType::PRIMITIVE_CONE:
            tempData = cone.generateShape();
            break;
        case PrimitiveType::PRIMITIVE_CYLINDER:
            tempData = cylinder.generateShape();
            break;
        case PrimitiveType::PRIMITIVE_SPHERE:
            tempData = sphere.generateShape();
            break;
        case PrimitiveType::PRIMITIVE_CUBE:
            tempData = cube.generateShape();
            break;
        case PrimitiveType::PRIMITIVE_MESH:
            break;
        }
        glShape glShape = {shape, index, tempData.size() / 6};
        glShapes.push_back(glShape);

        shapeData.insert(shapeData.end(), tempData.begin(), tempData.end());
        index += glShape.length;
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, shapeData.size() * sizeof(GLfloat), shapeData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Realtime::settingsChanged() {
    m_camera.setProjMatrix(settings.nearPlane, settings.farPlane);
    cone.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    cube.updateParams(settings.shapeParameter1);
    sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    cylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    updateVBO();
    update(); // asks for a PaintGL() call to occur
}

// ================== Project 6: Action!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around

    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}

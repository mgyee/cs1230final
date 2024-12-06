#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/shaderloader.h"

#include <numbers>

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

    glDeleteProgram(m_texture_shader);
    glDeleteVertexArrays(1, &m_fullscreen_vao);
    glDeleteBuffers(1, &m_fullscreen_vbo);

    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_defaultFBO = 2;

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
    m_texture_shader = ShaderLoader::createShaderProgram("resources/shaders/texture.vert", "resources/shaders/texture.frag");

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    // creating one entity
    // auto entity = registry.create();
    // registry.emplace<Position>(entity, glm::vec3(0.0f, 0.0f, 0.0f));
    // registry.emplace<Velocity>(entity, glm::vec3(0.0f, 0.0f, 0.0f));
    // registry.emplace<Renderable>(entity, m_vao, m_shader, 10);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(m_texture_shader);

    glUniform1i(glGetUniformLocation(m_texture_shader, "sampler"), 0);

    glUseProgram(0);

    std::vector<GLfloat> fullscreen_quad_data =
        { //     POSITIONS    //
            -1.f,  1.f, 0.0f,
            0.f,  1.f, 0.0f,

            -1.f, -1.f, 0.0f,
            0.f,  0.f, 0.0f,

            1.f, -1.f, 0.0f,
            1.f,  0.f, 0.0f,

            1.f,  1.f, 0.0f,
            1.f,  1.f, 0.0f,

            -1.f,  1.f, 0.0f,
            0.f,  1.f, 0.0f,

            1.f, -1.f, 0.0f,
            1.f,  0.f, 0.0f
        };

    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    // Task 14: modify the code below to add a second attribute to the vertex attribute array
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    makeFBO();
}

void Realtime::makeFBO(){
    // Task 19: Generate and bind an empty texture, set its min/mag filter interpolation, then unbind

    glGenTextures(1, &m_fbo_texture);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_screen_width, m_screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Task 20: Generate and bind a renderbuffer of the right size, set its format, then unbind

    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_screen_width, m_screen_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Task 18: Generate and bind an FBO

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Task 21: Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);

    // Task 22: Unbind the FBO

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glViewport(0, 0, m_screen_width, m_screen_height);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(m_vao);

    glUseProgram(m_shader);

    glm::vec4 camPos = m_camera.getInverseViewMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0);

    glUniform4fv(glGetUniformLocation(m_shader, "camPos"), 1, &camPos[0]);

    auto view = registry.view<Renderable>();
    for (auto entity : view) {
        // const auto &pos = view.get<Position>(entity);
        const auto &renderable = view.get<Renderable>(entity);
        
        glm::mat4 mvpMat = m_camera.getProjMatrix() * m_camera.getViewMatrix() * renderable.ctm;

        glUniformMatrix4fv(glGetUniformLocation(m_shader, "mvpMat"), 1, GL_FALSE, &mvpMat[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "modelMat"), 1, GL_FALSE, &renderable.ctm[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "invTransModelMat"), 1, GL_FALSE,
                           &renderable.inverseTransposectm[0][0]);

        glm::vec4 ambient = metaData.globalData.ka * renderable.cAmbient;
        glUniform4fv(glGetUniformLocation(m_shader, "ambient"), 1, &ambient[0]);

        glm::vec4 diffuse = metaData.globalData.kd * renderable.cDiffuse;
        glUniform4fv(glGetUniformLocation(m_shader, "diffuse"), 1, &diffuse[0]);

        glm::vec4 specular = metaData.globalData.ks * renderable.cSpecular;
        glUniform4fv(glGetUniformLocation(m_shader, "specular"), 1, &specular[0]);

        glUniform1f(glGetUniformLocation(m_shader, "shininess"), renderable.shininess);

        glDrawArrays(GL_TRIANGLES, renderable.index, renderable.vertexCount);
    }

    glBindVertexArray(0);

    glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(0);

    paintTexture(m_fbo_texture, settings.perPixelFilter, settings.kernelBasedFilter);

}

void Realtime::paintTexture(GLuint texture, bool pixelFilter, bool kernelFilter) {
    glUseProgram(m_texture_shader);
    // Task 32: Set your bool uniform on whether or not to filter the texture drawn

    glUniform1i(glGetUniformLocation(m_texture_shader, "pixelFilter"), pixelFilter);
    glUniform1i(glGetUniformLocation(m_texture_shader, "kernelFilter"), kernelFilter);

    glBindVertexArray(m_fullscreen_vao);
    // Task 10: Bind "texture" to slot 0

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {

    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;

    makeFBO();

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
    glUseProgram(m_shader);
    int i = 0;
    for (auto &light: metaData.lights) {

        std::basic_string lightString = "lights[" + std::to_string(i) + "]";

        GLint loc = glGetUniformLocation(m_shader, (lightString + ".dir").c_str());
        glUniform3fv(loc, 1, &light.dir[0]);

        loc = glGetUniformLocation(m_shader, (lightString + ".pos").c_str());
        glUniform3fv(loc, 1, &light.pos[0]);

        loc = glGetUniformLocation(m_shader, (lightString + ".color").c_str());
        glUniform4fv(loc, 1, &light.color[0]);

        loc = glGetUniformLocation(m_shader, (lightString + ".function").c_str());
        glUniform3fv(loc, 1, &light.function[0]);

        loc = glGetUniformLocation(m_shader, (lightString + ".angle").c_str());
        glUniform1f(loc, light.angle);

        loc = glGetUniformLocation(m_shader, (lightString + ".penumbra").c_str());
        glUniform1f(loc, light.penumbra);

        loc = glGetUniformLocation(m_shader, (lightString + ".type").c_str());
        switch (light.type) {
        case LightType::LIGHT_DIRECTIONAL:
            glUniform1i(loc, 0);
            break;
        case LightType::LIGHT_SPOT:
            glUniform1i(loc, 1);
            break;
        case LightType::LIGHT_POINT:
            glUniform1i(loc, 2);
            break;
        }
        i++;
    }
    glUniform1i(glGetUniformLocation(m_shader, "lightsCount"), i);
    glUseProgram(0);
    update(); // asks for a PaintGL() call to occur
}

void Realtime::updateVBO() {
    if ((int)m_vbo < 0) {
        return;
    }
    std::vector<float> shapeData;
    unsigned long index = 0;
    glShapes.clear();
    registry.clear();

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

        // update the registry
        auto newEntity = registry.create();
        // registry.emplace<Position>(newEntity, shape.primitive.transform.position);
        // registry.emplace<Velocity>(newEntity, shape.primitive.transform.velocity);
        
        Renderable newEntityRender = {index, tempData.size() / 6, 
                                            shape.ctm, 
                                            shape.primitive.material.cAmbient, 
                                            shape.primitive.material.cDiffuse, 
                                            shape.primitive.material.cSpecular, 
                                            shape.primitive.material.shininess,
                                            shape.inverseTransposectm};
        registry.emplace<Renderable>(newEntity, newEntityRender);
        index += glShape.length;
    }
    
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, shapeData.size() * sizeof(GLfloat), shapeData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Realtime::settingsChanged() {
    m_camera.setProjMatrix(settings.nearPlane, settings.farPlane);
    if (old_param1 != settings.shapeParameter1 || old_param2 != settings.shapeParameter2) {
        old_param1 = settings.shapeParameter1;
        old_param2 = settings.shapeParameter2;

        cone.updateParams(settings.shapeParameter1, settings.shapeParameter2);
        cube.updateParams(settings.shapeParameter1);
        sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
        cylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2);
        updateVBO();
    }
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

        if (m_mouseDown) {
            float x_angle = deltaX * std::numbers::pi / 500.f;
            glm::vec3 u_x(0, -1, 0);
            float c_x = glm::cos(x_angle);
            float s_x = glm::sin(x_angle);
            float one_minus_c_x = 1.f - c_x;

            glm::vec3 u_y = -glm::normalize(glm::cross(glm::vec3(metaData.cameraData.look),
                                                                glm::vec3(metaData.cameraData.up)));
            float y_angle = deltaY * std::numbers::pi / 500.f;
            float c_y = glm::cos(y_angle);
            float s_y = glm::sin(y_angle);
            float one_minus_c_y = 1.f - c_y;

            metaData.cameraData.look = glm::vec4(glm::mat3(c_y + pow(u_y.y,2)*one_minus_c_y,
                                                           u_y.x*u_y.y*one_minus_c_y+u_y.z*s_y,
                                                           u_y.x*u_y.z*one_minus_c_y-u_y.y*s_y,
                                                           u_y.x*u_y.y*one_minus_c_y-u_y.z*s_y,
                                                           c_y+pow(u_y.y,2)*one_minus_c_y,
                                                           u_y.y*u_y.z*one_minus_c_y+u_y.x*s_y,
                                                           u_y.x*u_y.z*one_minus_c_y+u_y.y*s_y,
                                                           u_y.y*u_y.z*one_minus_c_y-u_y.x*s_y,
                                                           c_y+pow(u_y.z,2)*one_minus_c_y) *
                                                glm::mat3(c_x + pow(u_x.x,2)*one_minus_c_x,
                                                          u_x.x*u_x.y*one_minus_c_x+u_x.z*s_x,
                                                          u_x.x*u_x.z*one_minus_c_x-u_x.y*s_x,
                                                          u_x.x*u_x.y*one_minus_c_x-u_x.z*s_x,
                                                          c_x+pow(u_x.y,2)*one_minus_c_x,
                                                          u_x.y*u_x.z*one_minus_c_x+u_x.x*s_x,
                                                          u_x.x*u_x.z*one_minus_c_x+u_x.y*s_x,
                                                          u_x.y*u_x.z*one_minus_c_x-u_x.x*s_x,
                                                          c_x+pow(u_x.z,2)*one_minus_c_x) *
                                                    glm::vec3(metaData.cameraData.look), 1.0);

            m_camera.setViewMatrix(metaData.cameraData.pos, metaData.cameraData.look, metaData.cameraData.up);
        }

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around

    float units = 5.f * deltaTime;
    
    // auto view = registry.view<Position, Velocity>();

    if (m_keyMap[Qt::Key_W]) {
        // auto &pos = view.get<Position>(camera_ent);
        // pos.value += units * glm::vec4(glm::normalize(glm::vec3(metaData.cameraData.look)), 0);
        glm::vec4 move = units * glm::vec4(glm::normalize(glm::vec3(metaData.cameraData.look)), 0);
        move.y = 0;
        metaData.cameraData.pos += move;
    }
    if (m_keyMap[Qt::Key_A]) {
        // auto &pos = view.get<Position>(camera_ent);
        // pos.value -= units * glm::normalize(glm::vec4(glm::cross(glm::vec3(metaData.cameraData.look),
        //                                               glm::vec3(metaData.cameraData.up)), 0));
        glm::vec4 move = units * glm::normalize(glm::vec4(glm::cross(glm::vec3(metaData.cameraData.look),
                                                        glm::vec3(metaData.cameraData.up)), 0));
        move.y = 0;
        metaData.cameraData.pos -= move;
    }
    if (m_keyMap[Qt::Key_S]) {
        // auto &pos = view.get<Position>(camera_ent);
        // pos.value -= units * glm::vec4(glm::normalize(glm::vec3(metaData.cameraData.look)), 0);
        glm::vec4 move = units * glm::vec4(glm::normalize(glm::vec3(metaData.cameraData.look)), 0);
        move.y = 0;
        metaData.cameraData.pos -= move;
    }
    if (m_keyMap[Qt::Key_D]) {
        // auto &pos = view.get<Position>(camera_ent);
        // pos.value += units * glm::normalize(glm::vec4(glm::cross(glm::vec3(metaData.cameraData.look),
        //                                               glm::vec3(metaData.cameraData.up)), 0));
        glm::vec4 move = units * glm::normalize(glm::vec4(glm::cross(glm::vec3(metaData.cameraData.look),
                                                                             glm::vec3(metaData.cameraData.up)), 0));
        move.y = 0;
        metaData.cameraData.pos += move;
    }
    // if (m_keyMap[Qt::Key_Control]) {
    //     // metaData.cameraData.pos -= units * glm::vec4(0, 1, 0, 0);
    // }
    // if (m_keyMap[Qt::Key_Space]) {
    //     auto &pos = view.get<Position>(camera_ent);
    //     auto &vel = view.get<Velocity>(camera_ent);
    //     if (pos.value.y == 0) {
    //         vel.value = glm::vec4(0, 5, 0, 0);
    //     }
    //     pos.value += deltaTime * vel.value;
    //     vel.value -= deltaTime * 2;

    //     if (pos.value.y <= 0) {
    //         pos.value.y = 0;
    //         vel.value = glm::vec4(0);
    //     }
    //     // metaData.cameraData.pos += units * glm::vec4(0, 1, 0, 0);
    // }

    if (m_isJumping) {
        m_verticalVelocity += m_gravity * deltaTime;

        metaData.cameraData.pos.y += m_verticalVelocity * deltaTime;

        if (metaData.cameraData.pos.y <= m_groundLevel) {
            metaData.cameraData.pos.y = m_groundLevel;
            m_verticalVelocity = 0.0f;
            m_isJumping = false;
        }
    }

    if (m_keyMap[Qt::Key_Space] && !m_isJumping) {
        m_isJumping = true;
        m_verticalVelocity = m_jumpSpeed;
    }

    m_camera.setViewMatrix(metaData.cameraData.pos, metaData.cameraData.look, metaData.cameraData.up);

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

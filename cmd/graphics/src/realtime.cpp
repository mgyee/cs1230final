#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "settings.h"
#include "utils/shaderloader.h"
#include <thread>
#include <chrono>

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
    // TCPClient client("127.0.0.1", 50050);
    // client.connectAndSend("Hello, Server!");
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

    glDeleteVertexArrays(1, &m_skybox_vao);
    glDeleteBuffers(1, &m_skybox_vbo);
    glDeleteTextures(1, &m_cubemap_texture);
    glDeleteProgram(m_skybox_shader);

    this->doneCurrent();
}

uint32_t htonf(float value) {
    union {
        float f;
        uint32_t i;
    } converter;
    converter.f = value;
    return htonl(converter.i);
}

// Serialize Player struct to network byte order
void serializePlayer(const Player& player, char* buffer) {
    // Serialize id (4 bytes)
    uint32_t networkId = htonl(player.id);
    memcpy(buffer, &networkId, sizeof(uint32_t));

    // Serialize position (4 * 4 = 16 bytes)
    uint32_t networkPositionX = htonf(player.position.x);
    uint32_t networkPositionY = htonf(player.position.y);
    uint32_t networkPositionZ = htonf(player.position.z);
    //uint32_t networkPositionW = htonf(player.position.w);

    memcpy(buffer + 4, &networkPositionX, sizeof(uint32_t));
    memcpy(buffer + 8, &networkPositionY, sizeof(uint32_t));
    memcpy(buffer + 12, &networkPositionZ, sizeof(uint32_t));
    //memcpy(buffer + 16, &networkPositionW, sizeof(uint32_t));

    // Serialize velocity (4 * 4 = 16 bytes)
    uint32_t networkVelocityX = htonf(player.velocity.x);
    uint32_t networkVelocityY = htonf(player.velocity.y);
    uint32_t networkVelocityZ = htonf(player.velocity.z);
    //uint32_t networkVelocityW = htonf(player.velocity.w);

    memcpy(buffer + 16, &networkVelocityX, sizeof(uint32_t));
    memcpy(buffer + 20, &networkVelocityY, sizeof(uint32_t));
    memcpy(buffer + 24, &networkVelocityZ, sizeof(uint32_t));
}

// Deserialize from network byte order back to host byte order
Player deserializePlayer(const char* buffer) {
    Player player;

    // Deserialize id
    uint32_t networkId;
    memcpy(&networkId, buffer, sizeof(uint32_t));
    player.id = ntohl(networkId);

    // Deserialize position
    uint32_t networkPositionX, networkPositionY, networkPositionZ, networkPositionW;
    memcpy(&networkPositionX, buffer + 4, sizeof(uint32_t));
    memcpy(&networkPositionY, buffer + 8, sizeof(uint32_t));
    memcpy(&networkPositionZ, buffer + 12, sizeof(uint32_t));

    union {
        uint32_t i;
        float f;
    } converter;

    converter.i = ntohl(networkPositionX);
    player.position.x = converter.f;

    converter.i = ntohl(networkPositionY);
    player.position.y = converter.f;

    converter.i = ntohl(networkPositionZ);
    player.position.z = converter.f;

    converter.i = ntohl(networkPositionW);
    player.position.w = 1.f;

    // Deserialize velocity (same process as position)
    uint32_t networkVelocityX, networkVelocityY, networkVelocityZ, networkVelocityW;
    memcpy(&networkVelocityX, buffer + 16, sizeof(uint32_t));
    memcpy(&networkVelocityY, buffer + 20, sizeof(uint32_t));
    memcpy(&networkVelocityZ, buffer + 24, sizeof(uint32_t));

    converter.i = ntohl(networkVelocityX);
    player.velocity.x = converter.f;

    converter.i = ntohl(networkVelocityY);
    player.velocity.y = converter.f;

    converter.i = ntohl(networkVelocityZ);
    player.velocity.z = converter.f;

    converter.i = ntohl(networkVelocityW);
    player.velocity.w = 0.f;

    return player;
}

void Realtime::run_client() {
    // should connect to the server
    UDPClient client("127.0.0.1", 12345, 5);
    char buffer[256] = {0};
    memset(buffer, 0, 256);
    int initialValue = -1;
    memcpy(buffer, &initialValue, sizeof(int));
    int status;
    bool success;


    success = client.sendMessage(buffer, 4);

    if (!success) {
        std::cout << "failed to send the connecting message" << std::endl;
        return;
    }
    // maybe no timeout on this one
    memset(buffer, 0, 256);
    status = client.readMessage(buffer, 256);

    if (status == -1) {
        std::cout << "closing because of err in read" << std::endl;
        return;
    }

    int value;
    memcpy(&value, buffer, sizeof(int));

    // this is techincally just for ints, so not sure about this one
    my_id = ntohl(value);
    std::cout << "got id: " << my_id << std::endl;


    // assuming that there is only one player at this point:
    auto view = registry.view<Player>();
    entt::entity myself;
    for (auto entity : view) {
        if (view.get<Player>(entity).id == -1) {
            myself = entity;
            view.get<Player>(entity).id = my_id;
            break;
        }
    }

    int updates;
    int total = 0;
    bool found[4] = {false, false, false, false};
    while (true) {
        memset(buffer, 0, 256);
        // make a message and then marshal
        // update message
        registry_mutex.lock();
        serializePlayer(registry.get<Player>(myself), buffer);
        success = client.sendMessage(buffer, 28);
        registry_mutex.unlock();
        if (!success) {
            std::cout << "tcp connection has been closed" << std::endl;
            return;
        }

        // update stuff; so read, timeout, update
        // double check status
        status = client.readMessage(buffer, 256);
        if (status == -1) {
            std::cout << "closing because of err in read" << std::endl;
            //break;
            return;
        } else if (status > 0) {
            // use the world state, take mutex
            // prep data from the buffer we get
            updates = status / 28;
            Player data[updates];

            int offset = 0;
            for (int i = 0; i < updates; i++) {
                data[i] = deserializePlayer(buffer + offset);
                offset += 28;
            }

            registry_mutex.lock();
            // update the registry
            // for every update that we got
            for (int i = 0; i < updates; i++) {
                // std::cout << "i value: " << i << std::endl;
                Player currPlayer = data[i];
                // don't update yourself
                if (currPlayer.id == my_id) {
                    continue;
                }
                // curr_id = id from the update
                //
                if (!found[currPlayer.id]) {
                    auto newEntity = registry.create();
                    registry.emplace<Player>(newEntity, currPlayer);
                    found[currPlayer.id] = true;
                    continue;
                }
                auto view = registry.view<Player>();
                for (auto entity : view) {
                    if (currPlayer.id == view.get<Player>(entity).id) {
                        // data on the right should be actual updates
                        // placeholders
                        auto &entPlayer = view.get<Player>(entity);
                        entPlayer.position = currPlayer.position;
                        entPlayer.velocity = currPlayer.velocity;
                        // std::cout << "Here is the player's id: " << entPlayer.id << std::endl;
                        // std::cout << "Here is the player's position.x: " << entPlayer.position.x << std::endl;
                        // std::cout << "Here is the player's position.y: " << entPlayer.position.y << std::endl;
                        // std::cout << "Here is the player's position.z: " << entPlayer.position.z << std::endl;
                        break;
                    }
                }
            }
            registry_mutex.unlock();
        } else if (status == 0) {
            // this will just continue because we will base our stuff off
            // current world state
            std::cerr << "Got to status 0\n";
            continue;
        }
        //
    }

    // std::this_thread::sleep_for(std::chrono::seconds(2));
    // std::cout << "now I am closing" << std::endl;
    // return;
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

    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");
    m_texture_shader = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/texture.frag");

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    // creating one entity

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

    createSkybox();
    GLenum error = glGetError();
    std::cout << "ERROR AFTER initializing " << error << "\n";

    camera_ent = registry.create();
    Player myself = {-1, metaData.cameraData.pos, glm::vec4(0.0)};
    registry.emplace<Player>(camera_ent, myself);

    // need to bind this instance of realtime for the thread
    std::thread myThread(std::bind(&Realtime::run_client, this));
    myThread.detach();

    // paintGL();
}

void Realtime::createSkybox() {
    GLenum error3 = glGetError();
    // std::cout << "ERROR at start of create Skybox " << error3 << "\n";
    m_skybox_shader = ShaderLoader::createShaderProgram("resources/shaders/skybox.vert", "resources/shaders/skybox.frag");

    std::vector<GLfloat> skyboxVertices = {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    GLenum error4 = glGetError();
    // std::cout << "ERROR after creating positions " << error4 << "\n";

    glGenVertexArrays(1, &m_skybox_vao);
    glGenBuffers(1, &m_skybox_vbo);

    GLenum error2 = glGetError();
    // std::cout << "ERROR after gen arrays and buffers " << error2 << "\n";

    glBindVertexArray(m_skybox_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_skybox_vbo);
    glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(GLfloat), skyboxVertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);

    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    std::vector<std::string> faces = {
        "resources/skybox/right.jpg",
        "resources/skybox/left.jpg",
        "resources/skybox/top.jpg",
        "resources/skybox/bottom.jpg",
        "resources/skybox/front.jpg",
        "resources/skybox/back.jpg"
    };
    GLenum error = glGetError();
    std::cout << "ERROR before loading cube map " << error << "\n";
    loadCubeMap(faces);
}

void Realtime::loadCubeMap(std::vector<std::string> faces) {
    glUseProgram(m_skybox_shader);
    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &m_cubemap_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_texture);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                         );
            stbi_image_free(data);
            // std::cout << "Succesfully loaded image" << std::endl; // this works
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // glBindTexture(GL_TEXTURE_CUBE_MAP, 2);
    glUseProgram(0);
    GLenum error = glGetError();
    std::cout << "ERROR after loading cube map " << error << "\n";
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

    renderSkybox();

    glBindVertexArray(m_vao);

    glUseProgram(m_shader);

    glUniform1i(glGetUniformLocation(m_shader, "fog"), m_fogEnabled);
    glUniform4fv(glGetUniformLocation(m_shader, "fogColor"), 1, &m_fogColor[0]);
    glUniform1f(glGetUniformLocation(m_shader, "fogDensity"), m_fogDensity);
    glUniform1f(glGetUniformLocation(m_shader, "fogStart"), m_fogStart);
    glUniform1f(glGetUniformLocation(m_shader, "fogEnd"), m_fogEnd);
    glUniform1f(glGetUniformLocation(m_shader, "fogHeight"), m_fogHeight);
    glUniform1f(glGetUniformLocation(m_shader, "fogBaseHeight"), m_fogBaseHeight);

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
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "viewMat"), 1, GL_FALSE, &m_camera.getViewMatrix()[0][0]);

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

    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // renderSkybox();
    GLenum error = glGetError();
    // std::cout << "ERROR AFTER RENDERING " << error << "\n";

}

void Realtime::renderSkybox() {

    // if (m_cubemap_texture == 0) {
    //     std::cerr << "ERROR: Invalid cubemap texture" << std::endl;
    //     return;
    // } else {
    //     std::cout << "valid cubemap texture\n";
    // }

    // std::cout << "VBO for skybox is " << m_skybox_vbo << " and vao for skybox is " << m_skybox_vao << std::endl;
    // Disable depth writing
    // glDepthMask(GL_FALSE);
    // glclear
    glDisable(GL_DEPTH_TEST);

    // Use skybox shader
    glUseProgram(m_skybox_shader);

    // Create view matrix without translation
    glm::mat4 view = glm::mat4(glm::mat3(m_camera.getViewMatrix()));

    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "projection"), 1, GL_FALSE, &m_camera.getProjMatrix()[0][0]);
    glUniform1i(glGetUniformLocation(m_skybox_shader, "fog"), m_fogEnabled);
    glUniform4fv(glGetUniformLocation(m_skybox_shader, "fogColor"), 1, &m_fogColor[0]);
    glUniform1f(glGetUniformLocation(m_skybox_shader, "fogDensity"), m_fogDensity);


    // Bind cubemap
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_texture);
    glUniform1i(glGetUniformLocation(m_skybox_shader, "skybox"), 2);

    // Draw skybox
    glBindVertexArray(m_skybox_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // Re-enable depth writing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glUseProgram(0);
}

void Realtime::paintTexture(GLuint texture, bool pixelFilter, bool kernelFilter) {
    glUseProgram(m_texture_shader);
    // Task 32: Set your bool uniform on whether or not to filter the texture drawn

    glUniform1i(glGetUniformLocation(m_texture_shader, "pixelFilter"), pixelFilter);
    glUniform1i(glGetUniformLocation(m_texture_shader, "kernelFilter"), kernelFilter);
    glUniform1i(glGetUniformLocation(m_texture_shader, "FXAAEnabled"), m_FXAAEnabled);

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
    registry_mutex.lock();
    registry.get<Player>(camera_ent).position = metaData.cameraData.pos;
    registry_mutex.unlock();
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

    // DEBUG: IF NOTHING SHOWS AFTER
    // if (my_id != -1) {
    //     return;
    // }
    std::vector<float> shapeData;
    unsigned long index = 0;
    glShapes.clear();
    registry_mutex.lock();
    // handle camera as a special case?
    registry.clear<Renderable>();

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
        glShape glShape = {shape, index, static_cast<unsigned long>(tempData.size() / 6)};
        glShapes.push_back(glShape);

        shapeData.insert(shapeData.end(), tempData.begin(), tempData.end());

        // update the registry
        auto newEntity = registry.create();
        // registry.emplace<Position>(newEntity, shape.primitive.transform.position);
        // registry.emplace<Velocity>(newEntity, shape.primitive.transform.velocity);
        
        Renderable newEntityRender = {index, static_cast<unsigned long>(tempData.size() / 6),
                                            shape.ctm, 
                                            shape.primitive.material.cAmbient, 
                                            shape.primitive.material.cDiffuse, 
                                            shape.primitive.material.cSpecular, 
                                            shape.primitive.material.shininess,
                                            shape.inverseTransposectm};
        registry.emplace<Renderable>(newEntity, newEntityRender);
        index += glShape.length;
    }
    registry_mutex.unlock();

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
    m_fogEnabled = settings.fog;
    m_FXAAEnabled = settings.FXAA;
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
        metaData.cameraData.pos += units * glm::normalize(glm::vec4(glm::cross(glm::vec3(metaData.cameraData.look),
                                                                glm::vec3(metaData.cameraData.up)), 0));
    }
    if (m_keyMap[Qt::Key_Control]) {
        metaData.cameraData.pos -= units * glm::vec4(0, 1, 0, 0);
    }

    registry_mutex.lock();
    registry.get<Player>(camera_ent).position = metaData.cameraData.pos;
    registry_mutex.unlock();
    // velocity can probably be taken from move
    // orientation vector????
    m_camera.setViewMatrix(metaData.cameraData.pos, metaData.cameraData.look, metaData.cameraData.up);

    update(); // asks for a PaintGL() call to occur

    //update(); // asks for a PaintGL() call to occur
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

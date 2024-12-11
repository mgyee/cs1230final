#include "cube.h"

void Cube::updateParams(int param1) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    setVertexData();
}

glm::vec2 planeUV(glm::vec3 normal, glm::vec3 position) {
    // left, [1][0]
    if (normal.x == 1.f) {
        return glm::vec2(-position.z + 0.5f, position.y + 0.5f);
    }

    // right [0][1]
    else if (normal.x == -1.f) {
        return glm::vec2(position.z + 0.5f, position.y + 0.5f);
    }

    // top [1][1]
    else if (normal.y == 1.f && normal.x == 0.f and normal.z == 0.f) {
        return glm::vec2(position.x + 0.5f, -position.z + 0.5f);
        //return glm::vec2(0.f, 0.f);
    }

    // bottom [1][2]
    else if (normal.y == -1.f) {
        return glm::vec2(position.x + 0.5f, position.z + 0.5f);
    }

    // front [0][0]
    else if (normal.z == 1.f) {
        return glm::vec2(position.x + 0.5f, position.y + 0.5f);
    }

    // back [0][2]
    else if (normal.z == -1.f) {
        return glm::vec2(-position.x + 0.5f, position.y + 0.5f);
    }

    return glm::vec2(-1.f, -1.f);
}

void insertVec2(std::vector<float> &data, glm::vec2 v) {
    data.push_back(v.x);
    data.push_back(v.y);
}


void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // Task 2: create a tile (i.e. 2 triangles) based on 4 given points.
    glm::vec3 normal = glm::normalize(glm::cross(bottomLeft - topLeft, bottomRight - bottomLeft));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertVec2(m_vertexData, planeUV(normal, topLeft));
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normal);
    insertVec2(m_vertexData, planeUV(normal, bottomLeft));
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);
    insertVec2(m_vertexData, planeUV(normal, bottomRight));

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);
    insertVec2(m_vertexData, planeUV(normal, bottomRight));
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normal);
    insertVec2(m_vertexData, planeUV(normal, topRight));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertVec2(m_vertexData, planeUV(normal, topLeft));
}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // Task 3: create a single side of the cube out of the 4
    //         given points and makeTile()
    // Note: think about how param 1 affects the number of triangles on
    //       the face of the cube
    float fm_param1 = float(m_param1);

    for (int i = 0; i < m_param1; i++) {
        for (int j = 0; j < m_param1; j++) {
            glm::vec3 topLeft_ = topLeft
                                 + (topRight - topLeft) * float(i) / fm_param1
                                 + (bottomLeft - topLeft) * float(j) / fm_param1;

            glm::vec3 topRight_ = topLeft
                                  + (topRight - topLeft) * float(i + 1) / fm_param1
                                  + (bottomLeft - topLeft) * float(j) / fm_param1;

            glm::vec3 bottomLeft_ = topLeft
                                    + (topRight - topLeft) * float(i) / fm_param1
                                    + (bottomLeft - topLeft) * float(j + 1) / fm_param1;

            glm::vec3 bottomRight_ = topLeft
                                     + (topRight - topLeft) * float(i + 1) / fm_param1
                                     + (bottomLeft - topLeft) * float(j + 1) / fm_param1;

            makeTile(topLeft_, topRight_, bottomLeft_, bottomRight_);
        }
    }

}

void Cube::setVertexData() {
    // Uncomment these lines for Task 2, then comment them out for Task 3:

    // makeTile(glm::vec3(-0.5f,  0.5f, 0.5f),
    //          glm::vec3( 0.5f,  0.5f, 0.5f),
    //          glm::vec3(-0.5f, -0.5f, 0.5f),
    //          glm::vec3( 0.5f, -0.5f, 0.5f));

    // Uncomment these lines for Task 3:

    makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3( 0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3( 0.5f, -0.5f, 0.5f));

    // Task 4: Use the makeFace() function to make all 6 sides of the cube

    makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f));


    makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f));


    makeFace(glm::vec3(0.5f,  0.5f, 0.5f),
             glm::vec3(0.5f,  0.5f, -0.5f),
             glm::vec3(0.5f, -0.5f, 0.5f),
             glm::vec3(0.5f, -0.5f, -0.5f));


    makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3(0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3(0.5f,  0.5f, 0.5f));

    makeFace(glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3(0.5f, -0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(0.5f, -0.5f, -0.5f));
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

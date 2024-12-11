#include "cone.h"

void Cone::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Cone::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // Task 5: Implement the makeTile() function for a Sphere
    // Note: this function is very similar to the makeTile() function for Cube,
    //       but the normals are calculated in a different way!
    glm::vec3 topLeftNormal = glm::normalize(glm::vec3(topLeft.x, sqrtf(pow(topLeft.x, 2) + pow(topLeft.z, 2)) / 2.f, topLeft.z));
    glm::vec3 topRightNormal = glm::normalize(glm::vec3(topRight.x, sqrtf(pow(topRight.x, 2) + pow(topRight.z, 2)) / 2.f, topRight.z));
    glm::vec3 botLeftNormal = glm::normalize(glm::vec3(bottomLeft.x, sqrtf(pow(bottomLeft.x, 2) + pow(bottomLeft.z, 2)) / 2.f, bottomLeft.z));
    glm::vec3 botRightNormal = glm::normalize(glm::vec3(bottomRight.x, sqrtf(pow(bottomRight.x, 2) + pow(bottomRight.z, 2)) / 2.f, bottomRight.z));
    if (topLeft.y == 0.5) {
        topLeftNormal = 0.5f * (botLeftNormal + botRightNormal);
        topLeftNormal.y = 0;
        topLeftNormal = glm::normalize(topLeftNormal);
        topLeftNormal.y = 0.5;
        topLeftNormal = glm::normalize(topLeftNormal);
        topRightNormal = topLeftNormal;
    }

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, topLeftNormal);
    m_vertexData.push_back(0.0f);
    m_vertexData.push_back(0.0f);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, botLeftNormal);
    m_vertexData.push_back(0.0f);
    m_vertexData.push_back(0.0f);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, botRightNormal);
    m_vertexData.push_back(0.0f);
    m_vertexData.push_back(0.0f);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, botRightNormal);
    m_vertexData.push_back(0.0f);
    m_vertexData.push_back(0.0f);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, topRightNormal);
    m_vertexData.push_back(0.0f);
    m_vertexData.push_back(0.0f);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, topLeftNormal);
    m_vertexData.push_back(0.0f);
    m_vertexData.push_back(0.0f);
}

void Cone::makeCapTile(glm::vec3 topLeft,
                       glm::vec3 topRight,
                       glm::vec3 bottomLeft,
                       glm::vec3 bottomRight) {
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::vec3(0, -1, 0));
    m_vertexData.push_back(0.0f);
    m_vertexData.push_back(0.0f);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, glm::vec3(0, -1, 0));
    m_vertexData.push_back(0.0f);
    m_vertexData.push_back(0.0f);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::vec3(0, -1, 0));
    m_vertexData.push_back(0.0f);
    m_vertexData.push_back(0.0f);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::vec3(0, -1, 0));
    m_vertexData.push_back(0.0f);
    m_vertexData.push_back(0.0f);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, glm::vec3(0, -1, 0));
    m_vertexData.push_back(0.0f);
    m_vertexData.push_back(0.0f);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::vec3(0, -1, 0));
    m_vertexData.push_back(0.0f);
    m_vertexData.push_back(0.0f);
}

void Cone::makeWedge(float currentTheta, float nextTheta) {
    // Task 6: create a single wedge of the sphere using the
    //         makeTile() function you implemented in Task 5
    // Note: think about how param 1 comes into play here!

    float latStep = 1.f / m_param1;
    for (int i = 0; i < m_param1; i++) {
        float currentLat = i * latStep;
        float nextLat = (i + 1) * latStep;

        float currentY = 0.5 - currentLat;
        float nextY = 0.5 - nextLat;

        glm::vec3 topLeft(0.5f * currentLat * glm::cos(currentTheta), currentY, -0.5f * currentLat * glm::sin(currentTheta));
        glm::vec3 topRight(0.5f * currentLat * glm::cos(nextTheta), currentY, -0.5f * currentLat * glm::sin(nextTheta));
        glm::vec3 bottomLeft(0.5f * nextLat * glm::cos(currentTheta), nextY, -0.5f * nextLat * glm::sin(currentTheta));
        glm::vec3 bottomRight(0.5f * nextLat * glm::cos(nextTheta), nextY, -0.5f * nextLat * glm::sin(nextTheta));

        makeTile(topLeft, topRight, bottomLeft, bottomRight);

        float currentRad = currentLat / 2.f;
        float nextRad = nextLat / 2.f;

        glm::vec3 topLeftBottomCap(currentRad * glm::cos(currentTheta), -0.5, -currentRad * glm::sin(currentTheta));
        glm::vec3 topRightBottomCap(currentRad * glm::cos(nextTheta), -0.5, -currentRad * glm::sin(nextTheta));
        glm::vec3 bottomLeftBottomCap(nextRad * glm::cos(currentTheta), -0.5, -nextRad * glm::sin(currentTheta));
        glm::vec3 bottomRightBottomCap(nextRad * glm::cos(nextTheta), -0.5, -nextRad * glm::sin(nextTheta));

        makeCapTile(topRightBottomCap, topLeftBottomCap, bottomRightBottomCap, bottomLeftBottomCap);
    }
}

void Cone::makeCone() {
    // Task 7: create a full sphere using the makeWedge() function you
    //         implemented in Task 6
    // Note: think about how param 2 comes into play here!

    float thetaStep = glm::radians(360.f / m_param2);

    for (int i = 0; i < m_param2; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;

        makeWedge(currentTheta, nextTheta);
    }
}


void Cone::setVertexData() {
    // TODO for Project 5: Lights, Camera

    makeCone();
}


// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cone::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

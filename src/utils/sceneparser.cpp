#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

void SceneParser::parse_helper(SceneNode *node,
                               glm::mat4 ctm,
                               std::vector<SceneLightData> &lights,
                               std::vector<RenderShapeData> &shapes) {
    for (SceneTransformation *transform: node->transformations) {
        switch (transform->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            ctm = glm::translate(ctm, transform->translate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            ctm = glm::scale(ctm, transform->scale);
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            ctm = glm::rotate(ctm, transform->angle, transform->rotate);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            ctm *= transform->matrix;
            break;
        }
    }

    for (ScenePrimitive *prim: node->primitives) {
        shapes.push_back(RenderShapeData{*prim, ctm});
    }
    for (SceneLight *light: node->lights) {
        lights.push_back(SceneLightData{light->id,
                                        light->type,
                                        light->color,
                                        light->function,
                                        ctm * glm::vec4(0, 0, 0, 1),
                                        ctm * light->dir,
                                        light->penumbra,
                                        light->angle,
                                        light->width,
                                        light->height});
    }


    for (SceneNode *child: node->children) {
        SceneParser::parse_helper(child, ctm, lights, shapes);
    }
}

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    // TODO: Use your Lab 5 code here

    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();

    // Task 6: populate renderData's list of primitives and their transforms.
    //         This will involve traversing the scene graph, and we recommend you
    //         create a helper function to do so!

    SceneNode *root = fileReader.getRootNode();
    renderData.shapes.clear();
    renderData.lights.clear();

    glm::mat4 ctm = glm::mat4(1);

    SceneParser::parse_helper(root, ctm, renderData.lights, renderData.shapes);

    return true;
}

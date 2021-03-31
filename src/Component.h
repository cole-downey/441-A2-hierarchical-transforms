#pragma once
#ifndef _Component_H_
#define _Component_H_

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"

class Component {
public:
    Component();
    Component(glm::vec3 joint_trans, glm::vec3 mesh_trans, glm::vec3 mesh_scale);
    ~Component();

    void Draw(std::shared_ptr<Program>& prog, std::shared_ptr<Shape>& shape, std::shared_ptr<MatrixStack>& MV, std::shared_ptr<Shape>& joint);
    void Rotate(char input);

    std::vector<Component> children;
    glm::vec3 joint_trans;
    glm::vec3 joint_angles;
    glm::vec3 mesh_trans;
    glm::vec3 mesh_scale;
    float rotate_amount = 0.2f;

    bool selected;
    double t;

    void startSpin(char direction);
private:
    // for spinning in place
    bool spinning;
    char spin_direction;
    float spin;

};

#endif
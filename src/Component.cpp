#include "Component.h"
#include <cmath>

using namespace std;

Component::Component() {
    joint_trans = { 0, 0, 0 };
    joint_angles = { 0, 0, 0 };
    mesh_trans = { 0, 0, 0 };
    mesh_scale = { 0, 0, 0 };
    selected = false;
    spinning = false;
    spin = 0;
}

Component::Component(glm::vec3 joint_trans, glm::vec3 mesh_trans, glm::vec3 mesh_scale) :
    joint_trans(joint_trans),
    mesh_trans(mesh_trans),
    mesh_scale(mesh_scale) {
    joint_angles = { 0, 0, 0 };
    selected = false;
    spinning = false;
    spin = 0;
}

Component::~Component() {
}

void Component::Draw(shared_ptr<Program>& prog, shared_ptr<Shape>& shape, shared_ptr<MatrixStack>& MV, shared_ptr<Shape>& joint) {
    // Draw parent
    MV->pushMatrix();
    MV->translate(joint_trans); // move joint
    MV->rotate(joint_angles.x, 1, 0, 0); // rotate x
    MV->rotate(joint_angles.y, 0, 1, 0); // rotate y
    MV->rotate(joint_angles.z, 0, 0, 1); // rotate z

    // spinning in place
    MV->pushMatrix();
    if (spinning) {
        spin += rotate_amount / 10.0f;
        switch (spin_direction) {
        case 'x':
            MV->rotate(spin, 1, 0, 0);
            break;
        case 'y':
            MV->rotate(spin, 0, 1, 0);
            break;
        case 'z':
            MV->rotate(spin, 0, 0, 1);
            break;
        }
    }

    // draw joint sphere
    MV->pushMatrix();
    MV->scale(1.5, 1.5, 1.5);
    glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
    joint->draw(prog);
    MV->popMatrix();

    MV->pushMatrix(); // switch to mesh
    MV->translate(mesh_trans); // move mesh
    MV->scale(mesh_scale); // scale
    if (selected) {
        t = glfwGetTime();
        float scale_amount = 1.05f + 0.025f * (float)sin(2 * 2 * 3.14 * t);
        MV->scale(scale_amount, scale_amount, scale_amount);
    }
    // draw mesh
    glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
    shape->draw(prog);
    MV->popMatrix();
    MV->popMatrix();

    // Draw children
    for (int i = 0; i < children.size(); i++) {
        children.at(i).Draw(prog, shape, MV, joint);
    }
    MV->popMatrix();
}

void Component::Rotate(char input) {
    // increment/decrement rotation by set amount
    switch (input) {
    case 'x':
        joint_angles.x += rotate_amount;
        break;
    case 'X':
        joint_angles.x -= rotate_amount;
        break;
    case 'y':
        joint_angles.y += rotate_amount;
        break;
    case 'Y':
        joint_angles.y -= rotate_amount;
        break;
    case 'z':
        joint_angles.z += rotate_amount;
        break;
    case 'Z':
        joint_angles.z -= rotate_amount;
        break;
    case 'r':
        joint_angles = { 0, 0, 0 };
        break;
    }
}

void Component::startSpin(char direction) {
    // start the spinning animation
    spin_direction = direction;
    spinning = true;
}
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Component.h"

using namespace std;

GLFWwindow* window; // Main application window
string RES_DIR = ""; // Where data files live
shared_ptr<Program> prog;
shared_ptr<Shape> shape;
shared_ptr<Shape> joint; // joint sphere

// data for the robot
Component robot;
vector<Component*> traversal; // stores pointers to depth-first tree traversal
int current_node = 0;

static void error_callback(int error, const char* description) {
	cerr << description << endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

static void character_callback(GLFWwindow* window, unsigned int key) {
	switch (key) {
	case 'x': case 'X':
	case 'y': case 'Y':
	case 'z': case 'Z':
	case 'r':
		traversal.at(current_node)->Rotate(key);
		break;
	case '.':
		traversal.at(current_node)->selected = false;
		if (current_node < traversal.size() - 1)
			current_node++;
		traversal.at(current_node)->selected = true;
		break;
	case ',':
		traversal.at(current_node)->selected = false;
		if (current_node > 0)
			current_node--;
		traversal.at(current_node)->selected = true;
		break;
	}
}

void build_traversal(Component* comp) {
	// goes through tree to build depth first traversal
	traversal.push_back(comp);
	for (int i = 0; i < comp->children.size(); i++) {
		build_traversal(&comp->children.at(i));
	}
}


static void init() {
	GLSL::checkVersion();

	// Set background color.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	// Initialize mesh.
	shape = make_shared<Shape>();
	shape->loadMesh(RES_DIR + "cube.obj");
	shape->init();

	// Initialize joint mesh.
	joint = make_shared<Shape>();
	joint->loadMesh(RES_DIR + "sphere.obj");
	joint->init();

	// Initialize the GLSL program.
	prog = make_shared<Program>();
	prog->setVerbose(true);
	prog->setShaderNames(RES_DIR + "vert.glsl", RES_DIR + "frag.glsl");
	prog->init();
	prog->addUniform("P");
	prog->addUniform("MV");
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->setVerbose(false);

	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);

	// create robot
	glm::vec3 body_size = { 3, 5, 2 };
	glm::vec3 arm_size = { 2, 1, 1 };
	glm::vec3 leg_size = { 1, 3, 1 };
	// body
	robot = Component({ 0, 2, 0 }, { 0, 0, 0 }, body_size);
	// head
	robot.children.push_back(Component({ 0, body_size.y / 2, 0 }, { 0, 0.75, 0 }, { 1.5, 1.5, 1.5 }));
	// right arm
	robot.children.push_back(Component({ -body_size.x / 2, (body_size.y - arm_size.y) / 2, 0 }, { -arm_size.x / 2, 0, 0 }, arm_size));
	robot.children.back().startSpin('x'); // start arm spinning in place
	// right hand
	robot.children.back().children.push_back(Component({ -arm_size.x, 0, 0 }, { -arm_size.x / 2, 0, 0 }, arm_size));
	// left arm
	robot.children.push_back(Component({ body_size.x / 2, (body_size.y - arm_size.y) / 2, 0 }, { arm_size.x / 2, 0, 0 }, arm_size));
	// left hand
	robot.children.back().children.push_back(Component({ arm_size.x, 0, 0 }, { arm_size.x / 2, 0, 0 }, arm_size));
	// right leg
	robot.children.push_back(Component({ -(body_size.x - leg_size.x) / 2, -body_size.y / 2, 0 }, { 0, -leg_size.y / 2, 0 }, leg_size));
	// right foot
	robot.children.back().children.push_back(Component({ 0, -leg_size.y, 0 }, { 0, -leg_size.y / 2, 0 }, leg_size));
	robot.children.back().children.back().startSpin('y'); // start leg spinning in place
	// left leg
	robot.children.push_back(Component({ (body_size.x - leg_size.x) / 2, -body_size.y / 2, 0 }, { 0, -leg_size.y / 2, 0 }, leg_size));
	// left foot
	robot.children.back().children.push_back(Component({ 0, -leg_size.y, 0 }, { 0, -leg_size.y / 2, 0 }, leg_size));

	build_traversal(&robot);
	robot.selected = true;

}

static void render() {
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = width / (float)height;
	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Create matrix stacks.
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	// Apply projection.
	P->pushMatrix();
	P->multMatrix(glm::perspective((float)(45.0 * M_PI / 180.0), aspect, 0.01f, 100.0f));
	// Apply camera transform.
	MV->pushMatrix();
	MV->translate(glm::vec3(0, 0, -18));

	// Draw mesh using GLSL.
	prog->bind();
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);

	// Draw robot
	robot.Draw(prog, shape, MV, joint);

	prog->unbind();

	// Pop matrix stacks.
	MV->popMatrix();
	P->popMatrix();

	GLSL::checkError(GET_FILE_LINE);
}

int main(int argc, char** argv) {
	if (argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RES_DIR = argv[1] + string("/");

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if (!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640 * 2, 480 * 2, "Cole Downey", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback
	glfwSetCharCallback(window, character_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

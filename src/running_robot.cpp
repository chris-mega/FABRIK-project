// Display a cube, using glDrawElements

#define _USE_MATH_DEFINES
#include "common.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <vector>
#include <iostream>

const char *WINDOW_TITLE = "Running Robot";
const double FRAME_RATE_MS = 1000.0 / 60.0;

typedef glm::vec4  color4;
typedef glm::vec4  point4;

const int SUBDIVISIONS = 3;

void icosphere(int sub, std::vector<glm::vec4> &vertices, std::vector<GLuint> &indices);
void initCylinder(std::vector<glm::vec4> &vertices, std::vector<GLuint> &indices);
void cubeOutline();
void drawCube(glm::vec4 color, glm::mat4 model_view);
void drawSphere(glm::vec4 color, glm::mat4 model_view);
void drawCylinder(glm::vec4 color, glm::mat4 model_view);
GLuint ico_indices_count, cyl_indices_count;

// These definitions are from the OpenGL Red Book example 2-13
#define X .525731112119133606
#define Z .850650808352039932
static GLfloat vdata[12][3] = {
   {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},
   {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
   {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}
};
static GLuint tindices[20][3] = {
   {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
   {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
   {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
   {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11}
};


// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
   point4(-0.5, -0.5,  0.5, 1.0),
   point4(-0.5,  0.5,  0.5, 1.0),
   point4(0.5,  0.5,  0.5, 1.0),
   point4(0.5, -0.5,  0.5, 1.0),
   point4(-0.5, -0.5, -0.5, 1.0),
   point4(-0.5,  0.5, -0.5, 1.0),
   point4(0.5,  0.5, -0.5, 1.0),
   point4(0.5, -0.5, -0.5, 1.0)
};


GLuint indices[] = {
	// each group of 3 is a triangle face
	1, 0, 3, 1, 2, 3,
	2, 3, 7, 2, 6, 7,
	3, 0, 4, 3, 7, 4,
	6, 5, 1, 6, 2, 1,
	4, 5, 6, 4, 7, 6,
	5, 4, 0, 5, 1, 0
};

struct object {
	char *type;
	float width;
	float height;
	glm::vec3 pos;
};
struct part {
	part* parent;
	part* sibling;
	part* child;
	object* object;

};

// Array of rotation angles (in degrees) for each coordinate axis
//enum { leftArm = 0, rightArm = 1, leftLeg = 2, rightLeg = 3, NumExtr = 4,};
//GLfloat  Moves[NumExtr] = { 0.0, 0.0, 0.0, 0.0 };

enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

GLuint ModelView, Projection, Color;
GLuint vPosition;
static std::vector<glm::vec4> ico_vertices;
static std::vector<GLuint> ico_indices;

part* testFirst = new part;
bool test = true;

part* leftArm = new part;
part* rightArm = new part;
part* leftLeg = new part;
part* rightLeg = new part;
part* retTest;
part* retRLeg;
part* retLLeg;
bool starAnim = false;
bool rightDone = false;
bool rightStart = true;

//----------------------------------------------------------------------------
glm::vec3 initTest = glm::vec3(-0.2 - (0.35 / 4), 0, 0);
glm::vec3 initRob = glm::vec3(0.25 / 2, 0.45 / 2 + 0.05, 0);
glm::vec3 initRobD = glm::vec3(0.05, -0.05, 0);
glm::vec3 destTest = glm::vec3(initTest.x + 0.35 + (0.35 / 2) - 0.3, initTest.y + 0.2, initTest.z + 0.2);
// OpenGL initialization

void createTestFirst() {
	part *fourth = new part;
	object* fop = new object;
	fop->height = 0.35 / 2;
	fop->width = 0.05;
	fop->pos = glm::vec3(initTest.x + 0.35 + (0.35 / 4), initTest.y, initTest.z);
	fourth->object = fop;
	fourth->child = NULL;

	part *third = new part;
	object* tp = new object;
	tp->height = 0.35 / 4;
	tp->width = 0.05;
	tp->pos = glm::vec3(initTest.x + 0.35, initTest.y, initTest.z);
	third->object = tp;
	third->child = fourth;

	part *second = new part;
	object* sp = new object;
	sp->height = 0.35 / 2;
	sp->width = 0.05;
	sp->pos = glm::vec3(initTest.x + (0.35 / 2), initTest.y, initTest.z);
	second->object = sp;
	second->child = third;

	object* fp = new object;
	fp->height = 0.35 / 2;
	fp->width = 0.05;
	fp->pos = initTest;
	testFirst->object = fp;
	testFirst->child = second;
	testFirst->parent = NULL;

	second->parent = testFirst;
	third->parent = second;
	fourth->parent = third;
}
void createRobot() {
	///////////////////////////////////////////
	part *rightArm4 = new part;
	object* fop = new object;
	fop->height = 0.35 / 2;
	fop->width = 0.05;
	fop->pos = glm::vec3(initRob.x + 0.025, initRob.y - 0.025 - 0.35 - 0.05, initRob.z);
	rightArm4->object = fop;
	rightArm4->child = NULL;

	part *rightArm3 = new part;
	object* tp = new object;
	tp->height = 0.05;
	tp->width = 0.05;
	tp->pos = glm::vec3(initRob.x + 0.025, initRob.y - 0.025 - 0.35, initRob.z);
	rightArm3->object = tp;
	rightArm3->child = rightArm4;

	part *rightArm2 = new part;
	object* sp = new object;
	sp->height = 0.35 / 2;
	sp->width = 0.05;
	sp->pos = glm::vec3(initRob.x + 0.025, initRob.y - 0.025 - 0.35 / 2, initRob.z);
	rightArm2->object = sp;
	rightArm2->child = rightArm3;

	object* fp = new object;
	fp->height = 0.35 / 2;
	fp->width = 0.05;
	fp->pos = glm::vec3(initRob.x + 0.025, initRob.y - 0.025, initRob.z);
	rightArm->object = fp;
	rightArm->child = rightArm2;
	rightArm->parent = NULL;

	rightArm2->parent = rightArm;
	rightArm3->parent = rightArm2;
	rightArm4->parent = rightArm3;

	//////////////////////////////////////////

	part *leftArm4 = new part;
	object* fopL = new object;
	fopL->height = 0.35 / 2;
	fopL->width = 0.05;
	fopL->pos = glm::vec3(-initRob.x - 0.025, initRob.y - 0.025 - 0.35 - 0.05, initRob.z);
	leftArm4->object = fopL;
	leftArm4->child = NULL;

	part *leftArm3 = new part;
	object* tpL = new object;
	tpL->height = 0.05;
	tpL->width = 0.05;
	tpL->pos = glm::vec3(-initRob.x - 0.025, initRob.y - 0.025 - 0.35, initRob.z);
	leftArm3->object = tpL;
	leftArm3->child = leftArm4;

	part *leftArm2 = new part;
	object* spL = new object;
	spL->height = 0.35 / 2;
	spL->width = 0.05;
	spL->pos = glm::vec3(-initRob.x - 0.025, initRob.y - 0.025 - 0.35 / 2, initRob.z);
	leftArm2->object = spL;
	leftArm2->child = leftArm3;

	object* fpL = new object;
	fpL->height = 0.35 / 2;
	fpL->width = 0.05;
	fpL->pos = glm::vec3(-initRob.x - 0.025, initRob.y - 0.025, initRob.z);
	leftArm->object = fpL;
	leftArm->child = leftArm2;
	leftArm->parent = NULL;

	leftArm2->parent = leftArm;
	leftArm3->parent = leftArm2;
	leftArm4->parent = leftArm3;

	//////////////////////////////////////////////////////////

	part *rightLeg4 = new part;
	object* fopRl = new object;
	fopRl->height = 0.35 / 2;
	fopRl->width = 0.05;
	fopRl->pos = glm::vec3(initRobD.x, initRobD.y - 0.35 - 0.05, initRobD.z);
	rightLeg4->object = fopRl;
	rightLeg4->child = NULL;

	part *rightLeg3 = new part;
	object* tpRl = new object;
	tpRl->height = 0.05;
	tpRl->width = 0.05;
	tpRl->pos = glm::vec3(initRobD.x, initRobD.y - 0.35, initRobD.z);
	rightLeg3->object = tpRl;
	rightLeg3->child = rightLeg4;

	part *rightLeg2 = new part;
	object* spRl = new object;
	spRl->height = 0.35 / 2;
	spRl->width = 0.05;
	spRl->pos = glm::vec3(initRobD.x, initRobD.y - 0.35 / 2, initRobD.z);
	rightLeg2->object = spRl;
	rightLeg2->child = rightLeg3;

	object* fpRl = new object;
	fpRl->height = 0.35 / 2;
	fpRl->width = 0.05;
	fpRl->pos = glm::vec3(initRobD.x, initRobD.y, initRobD.z);
	rightLeg->object = fpRl;
	rightLeg->child = rightLeg2;
	rightLeg->parent = NULL;

	rightLeg2->parent = rightLeg;
	rightLeg3->parent = rightLeg2;
	rightLeg4->parent = rightLeg3;

	//////////////////////////////////////////
	part *leftLeg4 = new part;
	object* fopLl = new object;
	fopLl->height = 0.35 / 2;
	fopLl->width = 0.05;
	fopLl->pos = glm::vec3(-initRobD.x, initRobD.y - 0.35 - 0.05, initRobD.z);
	leftLeg4->object = fopLl;
	leftLeg4->child = NULL;

	part *leftLeg3 = new part;
	object* tpLl = new object;
	tpLl->height = 0.05;
	tpLl->width = 0.05;
	tpLl->pos = glm::vec3(-initRobD.x, initRobD.y - 0.35, initRobD.z);
	leftLeg3->object = tpLl;
	leftLeg3->child = leftLeg4;

	part *leftLeg2 = new part;
	object* spLl = new object;
	spLl->height = 0.35 / 2;
	spLl->width = 0.05;
	spLl->pos = glm::vec3(-initRobD.x, initRobD.y - 0.35 / 2, initRobD.z);
	leftLeg2->object = spLl;
	leftLeg2->child = leftLeg3;

	object* fpLl = new object;
	fpLl->height = 0.35 / 2;
	fpLl->width = 0.05;
	fpLl->pos = glm::vec3(-initRobD.x, initRobD.y, initRobD.z);
	leftLeg->object = fpLl;
	leftLeg->child = leftLeg2;
	leftLeg->parent = NULL;

	leftLeg2->parent = leftLeg;
	leftLeg3->parent = leftLeg2;
	leftLeg4->parent = leftLeg3;
}
void
init()
{
	createTestFirst();
	createRobot();

	//sphere:
	icosphere(SUBDIVISIONS, ico_vertices, ico_indices);
	ico_indices_count = ico_indices.size();

	static std::vector<glm::vec4> cyl_vertices;
	static std::vector<GLuint> cyl_indices;
	initCylinder(cyl_vertices, cyl_indices);
	cyl_indices_count = cyl_indices.size();

	// Create a vertex array object
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint buffer;

	// Create and initialize a buffer object
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + ico_vertices.size() * sizeof(glm::vec4) + cyl_vertices.size() * sizeof(glm::vec4), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), ico_vertices.size() * sizeof(glm::vec4), &ico_vertices[0].x);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) + ico_vertices.size() * sizeof(glm::vec4), cyl_vertices.size() * sizeof(glm::vec4), &cyl_vertices[0].x);

	// Another for the index buffer
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) + ico_indices.size() * sizeof(GLuint) + cyl_indices.size() * sizeof(GLuint), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), ico_indices.size() * sizeof(GLuint), &ico_indices[0]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) + ico_indices.size() * sizeof(GLuint), cyl_indices.size() * sizeof(GLuint), &cyl_indices[0]);
	
	// Load shaders and use the resulting shader program
	GLuint program = InitShader("vshader_robot.glsl", "fshader_robot.glsl");
	glUseProgram(program);

	// set up vertex arrays
	vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	/*
	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(indices)));
	*/
	ModelView = glGetUniformLocation(program, "ModelView");
	Projection = glGetUniformLocation(program, "Projection");
	Color = glGetUniformLocation(program, "Color");
	
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

// Animation
float error = 0.025; //range of acceptable error
void calculateIK(part* last, glm::vec3 goal);
void calculateFK(part* init, glm::vec3 goal);
void calculateNotReachable(part* init, glm::vec3 goal);
glm::mat4 calcModelView(glm::mat4 model_view, float x1, float y1, float z1, float x2, float y2, float z2, glm::mat4 s);
void iterateTree(part* init, glm::mat4 model_view, glm::vec4 color);
part* copyJerar(part* root);


part* legalRot(part* check, glm::vec3 dest) {
	glm::vec3 pChck = check->object->pos;
	float scalar = dot(dest, (check->object->pos - check->parent->object->pos)) / check->object->height;
	glm::vec3 proj = scalar * normalize(check->object->pos - check->parent->object->pos);


	float angle = acos(dot(pChck, proj) / (glm::length(pChck)*glm::length(proj)));
	if (angle > M_PI / 4) {

		std::cout << " " << std::endl;
	}

	return check;
}

void calculateIK(part* last, glm::vec3 goal) {
	if (last != NULL) {
		object* pobj = new object;
		pobj->height = last->object->height;
		pobj->width = last->object->width;
		pobj->pos = goal;
		last->object = pobj;
		if (last->parent != NULL) {
			glm::vec3 ppos = last->parent->object->pos;
			glm::vec3 ri = last->object->pos - last->parent->object->pos;
			float lambda = last->parent->object->height / glm::length(ri);
			//glm::vec3 newPos = normalize(last->object->pos - last->parent->object->pos) * last->parent->object->height;
			glm::vec3 newPos = (1.f - lambda) * last->object->pos + lambda * last->parent->object->pos;
			last->parent->object->pos = newPos;
			legalRot(last, newPos);
			calculateIK(last->parent, newPos);
		}
	}
	
}



void calculateFK(part* init, glm::vec3 goal) {
	if (init != NULL) {
		object* pobj = new object;
		pobj->height = init->object->height;
		pobj->width = init->object->width;
		pobj->pos = goal;
		init->object = pobj;
		if (init->child != NULL) {
			glm::vec3 ppos = init->child->object->pos;
			glm::vec3 ri = init->child->object->pos - init->object->pos;
			float lambda = init->object->height / glm::length(ri);
			//glm::vec3 newPos = normalize(last->object->pos - last->parent->object->pos) * last->parent->object->height;
			glm::vec3 newPos = (1.f - lambda) * init->object->pos + lambda * init->child->object->pos;
			init->child->object->pos = newPos;
			calculateFK(init->child, newPos);
		}
	}
}


void calculateNotReachable(part* init, glm::vec3 goal) {
	if (init != NULL) {
		glm::vec3 ri = goal - init->object->pos;
		float lambda = init->object->height / glm::length(ri);
		glm::vec3 newPos = (1.f - lambda) * init->object->pos + lambda * goal;
		if (init->child != NULL) {
			init->child->object->pos = newPos;
			calculateNotReachable(init->child, goal);
		}
	}
}

part* copyJerar(part* root) {
	part *fourth = new part;
	object* fop = new object;
	fop->height = root->child->child->child->object->height;
	fop->width = root->child->child->child->object->width;
	fop->pos = root->child->child->child->object->pos;
	fourth->object = fop;
	fourth->child = NULL;

	part *third = new part;
	object* tp = new object;
	tp->height = root->child->child->object->height;
	tp->width = root->child->child->object->width;
	tp->pos = root->child->child->object->pos;
	third->object = tp;
	third->child = fourth;

	part *second = new part;
	object* sp = new object;
	sp->height = root->child->object->height;
	sp->width = root->child->object->width;
	sp->pos = root->child->object->pos;
	second->object = sp;
	second->child = third;

	part* first = new part;
	object* fp = new object;
	fp->height = root->object->height;
	fp->width = root->object->width;
	fp->pos = root->object->pos;
	first->object = fp;
	first->child = second;
	first->parent = NULL;

	second->parent = first;
	third->parent = second;
	fourth->parent = third;

	return first;
}

part* startAnimation(part* joint, glm::vec3 initP, glm::vec3 goalPos) {
	int maxDepth = 10; //limit depth to arrive to accuracy
	glm::vec3 orgPos = joint->object->pos;
	float distParts = 0;
	part* iter = copyJerar(joint);
	part** parts = new part*[4];
	for (int i = 0; i < 4; i++) {
		if(i<3)
			distParts += iter->object->height;
		parts[i] = iter;
		iter = iter->child;
	}
	glm::vec3 vector = goalPos - orgPos;
	float distToGoal = glm::length(vector);
	if (distToGoal > distParts) {
		calculateNotReachable(parts[0], goalPos);
		return parts[0];
	}
	else {
		float dif = glm::length(parts[3]->object->pos - goalPos);
		int depthCount = 0;
		while (dif > error && depthCount <= maxDepth) {
			calculateIK(parts[3], goalPos);
			calculateFK(parts[0], initP);
			dif = glm::length(parts[3]->object->pos - goalPos);
			depthCount++;
		}
		return parts[0];
	}
}

//----------------------------------------------------------------------------

void drawTest(glm::mat4 model_view, glm::vec4 color) {


	glm::vec3 dest = destTest; //move this coordinates wherever you want
	glm::mat4 mv = model_view;

	mv = model_view * glm::translate(glm::mat4(), initTest) * glm::scale(glm::mat4(), glm::vec3(error, error, error));
	color = glm::vec4(0, 0, 0, 1);
	drawSphere(color, mv);

	mv = model_view * glm::translate(glm::mat4(), dest) * glm::scale(glm::mat4(), glm::vec3(error, error, error));
	color = glm::vec4(0, 0, 0, 1);
	drawSphere(color, mv);
	

	if (!starAnim) {
		retTest = startAnimation(testFirst, initTest, dest);
		starAnim = true;
	}

	
	iterateTree(testFirst, model_view, color);
}

void iterateTree(part* init, glm::mat4 model_view, glm::vec4 color) {
	part* iter = init;
	glm::mat4 mv;
	for (int i = 0; i < 4; i++) {

		if (iter->child != NULL) {
			float x1 = iter->object->pos.x;
			float y1 = iter->object->pos.y;
			float z1 = iter->object->pos.z;
			float x2 = iter->child->object->pos.x;
			float y2 = iter->child->object->pos.y;
			float z2 = iter->child->object->pos.z;
			glm::mat4 s = glm::scale(glm::mat4(), glm::vec3(iter->object->width, iter->object->height, iter->object->width));
			mv = calcModelView(model_view, x1, y1, z1, x2, y2, z2, s);

		}
		if (i != 3) {
			if (i == 0)
				color = glm::vec4(1, 0, 0, 1);
			else if (i == 1)
				color = glm::vec4(0, 1, 0, 1);
			else
				color = glm::vec4(0, 0, 1, 1);
			//
			drawCube(color, mv);
			cubeOutline();
		}
		iter = iter->child;
	}
}



void drawRobot(glm::mat4 model_view, glm::vec4 color) {
	glm::mat4 mv = model_view;
	
	//WEIST
	mv = model_view * glm::scale(glm::mat4(), glm::vec3(0.2, 0.1, 0.2));
	color = glm::vec4(0, 0, 0, 1);
	drawCube(color, mv);
	cubeOutline();
	glm::mat4 transY = glm::translate(glm::mat4(), glm::vec3(0.0, 0.05, 0.0)); //

	//CHEST:
	transY = transY * glm::translate(glm::mat4(), glm::vec3(0, 0.45 / 4, 0)); //

	mv = model_view * transY * glm::scale(glm::mat4(), glm::vec3(0.25, 0.45 / 2, 0.25));
	color = glm::vec4(0.5, 0.5, 0.5, 1);
	drawCube(color, mv);
	cubeOutline();

	transY = transY * glm::translate(glm::mat4(), glm::vec3(0, 0.45 / 4, 0)); //
	glm::mat4 transX = glm::translate(glm::mat4(), glm::vec3(0.25 / 2, 0.0, 0.0)); //

	glm::mat4 armsY = transY;
	//Right:
	transX = transX * glm::translate(glm::mat4(), glm::vec3(0.025, 0, 0));
	mv = model_view * armsY * transX * glm::scale(glm::mat4(), glm::vec3(0.05, 0.05, 0.05));
	color = glm::vec4(0, 0, 0, 1);
	drawSphere(color, mv);
	//Left:
	transX = glm::translate(glm::mat4(), glm::vec3(-0.25/2 - 0.025, 0, 0));
	mv = model_view * armsY * transX * glm::scale(glm::mat4(), glm::vec3(0.05, 0.05, 0.05));
	color = glm::vec4(0, 0, 0, 1);
	drawSphere(color, mv);
	


	//HEAD:
	//neck:
	//rothead = ...
	transY = transY * glm::translate(glm::mat4(), glm::vec3(0, 0.05, 0));
	mv = model_view * transY * glm::scale(glm::mat4(), glm::vec3(0.075, 0.1, 0.075));
	color = glm::vec4(0.5, 0.5, 0.5, 1);
	drawCube(color, mv);
	cubeOutline();
	transY = transY * glm::translate(glm::mat4(), glm::vec3(0, 0.05, 0));

	//face:
	transY = transY * glm::translate(glm::mat4(), glm::vec3(0, 0.25 / 4, 0));
	mv = model_view * transY * glm::scale(glm::mat4(), glm::vec3(0.15, 0.25 / 2, 0.15));
	color = glm::vec4(0, 0, 0, 1);
	drawCube(color, mv);
	cubeOutline();
	///////////////////////////////////////////

	glm::vec3 destR = glm::vec3(initRobD.x, initRobD.y - 0.35/2 - 0.35/4, initRobD.z+0.1); //move this coordinates wherever you want
	mv = model_view * glm::translate(glm::mat4(), destR) * glm::scale(glm::mat4(), glm::vec3(error, error, error));
	color = glm::vec4(0, 0, 0, 1);
	drawSphere(color, mv);


	glm::vec3 destRR = glm::vec3(initRobD.x, initRobD.y - 0.35 - 0.05, initRobD.z); //move this coordinates wherever you want
	mv = model_view * glm::translate(glm::mat4(), destRR) * glm::scale(glm::mat4(), glm::vec3(error, error, error));
	color = glm::vec4(0, 0, 0, 1);
	drawSphere(color, mv);

	

	glm::vec3 destL = glm::vec3(-initRobD.x, initRobD.y - 0.35 / 2 - 0.35 / 4, initRobD.z + 0.1); //move this coordinates wherever you want
	mv = model_view * glm::translate(glm::mat4(), destL) * glm::scale(glm::mat4(), glm::vec3(error, error, error));
	color = glm::vec4(0, 0, 0, 1);
	drawSphere(color, mv);

	if (!starAnim) {
		retRLeg = startAnimation(rightLeg, initRobD, destR);
		retLLeg = startAnimation(rightLeg, initRobD, destL);
		starAnim = true;
	}

	

	iterateTree(leftArm, model_view, color);
	iterateTree(rightArm, model_view, color);
	iterateTree(leftLeg, model_view, color);
	iterateTree(rightLeg, model_view, color);
}

glm::mat4 calcModelView(glm::mat4 model_view, float x1, float y1, float z1, float x2, float y2, float z2, glm::mat4 s) {
	float x = x2 - x1;
	float y = y2 - y1;
	float z = z2 - z1;
	float size = sqrt((x * x) + (y * y) + (z * z));
	//float angleZ = acos(x / size);
	float angleX = 0;
	if(sqrt((x*x) + (z*z)) != 0 )
		angleX = acos(x / sqrt((x*x) + (z*z)));
	float angleZ = 0;
	if(x != 0)
		angleZ = atan(y/x);
	float angleY = 0;
	if(size != 0)
		angleY = acos(y / size);
	
	float mpi = M_PI / 2.f;
	if (abs(angleX) > mpi - 0.001)
		if(angleX > 0)
			angleX = angleX - mpi;
		else
			angleX = angleX + mpi;
	if (abs(angleY) > mpi - 0.001)
		if (angleY > 0)
			angleY = angleY - mpi;
		else
			angleY = angleY + mpi;
	if (abs(angleZ) > mpi - 0.001)
		if (angleZ > 0)
			angleZ = angleZ - mpi;
		else
			angleZ = angleZ + mpi;
			

	float centerX = x1 - x1 * 0.5 + x2 * 0.5;
	float centerY = y1 - y1 * 0.5 + y2 * 0.5;
	float centerZ = z1 - z1 * 0.5 + z2 * 0.5;
	glm::mat4 rx = glm::rotate(glm::mat4(), angleX, glm::vec3(1, 0, 0));
	glm::mat4 ry = glm::rotate(glm::mat4(), angleY, glm::vec3(0, 1, 0));
	glm::mat4 rz;
	if(test)
		rz = glm::rotate(glm::mat4(), glm::radians(90.f) + angleZ, glm::vec3(0, 0, 1));
	else
		rz = glm::rotate(glm::mat4(), angleZ, glm::vec3(0, 0, 1));
	glm::mat4 t = glm::translate(glm::mat4(), glm::vec3(centerX, centerY, centerZ));

	glm::mat4 mv = model_view * t * rx * rz * ry  * s;
	return mv;

}
void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//  Generate the model-view matrix
	glm::vec4 color;

	const glm::vec3 viewer_pos(0.0, 0.0, 2.0);
	glm::mat4 trans, model_view, lArmMove, rArmMove, lLegMove, rLegMove, scale, rotX, rotY;
	trans = glm::translate(trans, -viewer_pos);
	//create transformation matrix locally
	//lArmMove = glm::rotate(lArmMove, glm::radians(Moves[leftArm]), glm::vec3(1, 0, 0));
	//rArmMove = glm::rotate(rArmMove, glm::radians(Moves[rightArm]), glm::vec3(1, 0, 0));
	//lLegMove = glm::rotate(lLegMove, glm::radians(Moves[leftLeg]), glm::vec3(1, 0, 0));
	//rLegMove = glm::rotate(rLegMove, glm::radians(Moves[rightLeg]), glm::vec3(1, 0, 0));

	rotX = glm::rotate(rotX, glm::radians(Theta[Xaxis]), glm::vec3(0, 1, 0));
	rotY = glm::rotate(rotY, glm::radians(Theta[Yaxis]), glm::vec3(1, 0, 0));
	//scale = glm::translate(scale, glm::vec3(0, 0, Theta[Zaxis]));

	trans = trans * rotX * rotY;
	
	//start drawing robot:	

	model_view = trans;

	if (test)
		drawTest(model_view, color);
	else
		drawRobot(model_view, color);
	

	//FLOOR
	bool black = true;
	for (float z = -0.5; z <= 0.5; z += 0.25) {
		for (float x = -0.5; x <= 0.5; x += 0.25) {
			model_view = trans * glm::translate(glm::mat4(), glm::vec3(x, -0.55, z)) * glm::scale(glm::mat4(), glm::vec3(0.2,0.2,0.2));
			glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
			if (black)
				color = glm::vec4(0, 0, 0, 1);
			else
				color = glm::vec4(0, 0, 1, 1);
			black = !black;
			glUniform4fv(Color, 1, glm::value_ptr(color));
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)(18 * sizeof(GLuint)));			
		}
	}

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

float lerp(float min, float max, float frac) {
	//return (max - min) * frac + min;
	float asxdcv = (1 - frac) * min + frac * max;
	return asxdcv;
}

bool forward = true;
int step = 0;
void
update(void)
{
	if (starAnim) {
		if (test) {
			part* iter1 = testFirst;
			part* iter2 = retTest;
			for (int i = 0; i < 4;i++) {
				iter1->object->pos.x = lerp(iter1->object->pos.x, iter2->object->pos.x, 0.005);
				iter1->object->pos.y = lerp(iter1->object->pos.y, iter2->object->pos.y, 0.005);
				iter1->object->pos.z = lerp(iter1->object->pos.z, iter2->object->pos.z, 0.005);
				iter1 = iter1->child;
				iter2 = iter2->child;
			}
		}
		else {
			part* iterRL1 = rightLeg;
			part* iterRL2 = retRLeg;
			part* iterLL1 = leftLeg;
			part* iterLL2 = retLLeg;
			if (step < 2) {
				if (!rightDone) {
					for (int i = 0; i < 4;i++) {
						iterRL1->object->pos.x = lerp(iterRL1->object->pos.x, iterRL2->object->pos.x, 0.01);
						iterRL1->object->pos.y = lerp(iterRL1->object->pos.y, iterRL2->object->pos.y, 0.01);
						iterRL1->object->pos.z = lerp(iterRL1->object->pos.z, iterRL2->object->pos.z, 0.01);
						if (i != 3) {
							iterRL1 = iterRL1->child;
							iterRL2 = iterRL2->child;
						}
					}
					if (iterRL2->object->pos.x - iterRL1->object->pos.x < 0.01 && iterRL2->object->pos.y - iterRL1->object->pos.y < 0.01 && iterRL2->object->pos.z - iterRL1->object->pos.z < 0.01) {
						if(step == 0)
							retRLeg = startAnimation(rightLeg, rightLeg->object->pos, glm::vec3(initRobD.x, initRobD.y - 0.35 - 0.05, initRobD.z));
						else
							retRLeg = startAnimation(rightLeg, rightLeg->object->pos, glm::vec3(initRobD.x, initRobD.y - 0.35 / 2 - 0.35 / 4, initRobD.z + 0.1));
						//rightLeg = copyJerar(retRLeg);
						step++;
					}
				}
				else {
					std::cout << " " << std::endl;
				}
			}
			else {
				step = 0;
				rightDone = !rightDone;
			}
		}
	}
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
	GLfloat moveBy = 5.;
	switch (key) {
		case '8':
			destTest.y += 0.05;
			starAnim = false;
			break;
		case '2':
			destTest.y -= 0.05;
			starAnim = false;
			break;
		case '4':
			destTest.x -= 0.05;
			starAnim = false;
			break;
		case '6':
			destTest.x += 0.05;
			starAnim = false;
			break;
		case '7':
			destTest.z += 0.05;
			starAnim = false;
			break;
		case '3':
			destTest.z -= 0.05;
			starAnim = false;
			break;
		case 'a': case 'A':
			Theta[Xaxis] += moveBy;
			break;
		case 'd': case 'D':
			Theta[Xaxis] -= moveBy;
			break;
		case 'w': case 'W':
			Theta[Yaxis] += moveBy;
			break;
		case 's': case 'S':
			Theta[Yaxis] -= moveBy;
			break;
		case 't': case 'T':
			test = !test;
			if (test) {
				std::cout << "Mode: test" << std::endl;
				createTestFirst();
				starAnim = false;
			}
			else {
				std::cout << "Mode: robot" << std::endl;
				createRobot();
				starAnim = false;
			}

			break;
		case 033: // Escape Key
		case 'q': case 'Q':
			exit(EXIT_SUCCESS);
			break;
		}
}

//----------------------------------------------------------------------------

void cubeOutline() {
	glm::vec4 color = glm::vec4(1, 1, 1, 1);
	glUniform4fv(Color, 1, glm::value_ptr(color));
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0)); //change to cube portion in buffer
	for (int i = 0; i < sizeof(indices) / sizeof(GLuint); i += 3) {
		glDrawElements(GL_LINE_STRIP, 3, GL_UNSIGNED_INT, (void *)(i * sizeof(GLuint)));
	}
}
void drawCube(glm::vec4 color, glm::mat4 model_view) {
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0)); //change to cube portion in buffer
	glUniform4fv(Color, 1, glm::value_ptr(color));
	glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
}
void drawSphere(glm::vec4 color, glm::mat4 model_view) {
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices))); //change to sphere portion in buffer
	glUniform4fv(Color, 1, glm::value_ptr(color));
	glDrawElements(GL_TRIANGLES, ico_indices_count, GL_UNSIGNED_INT, BUFFER_OFFSET(sizeof(indices)));
}
void drawCylinder(glm::vec4 color, glm::mat4 model_view) {
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices) + ico_vertices.size() * sizeof(glm::vec4))); //change to cylinder portion in buffer
	glUniform4fv(Color, 1, glm::value_ptr(color));
	glDrawElements(GL_TRIANGLES, cyl_indices_count, GL_UNSIGNED_INT, BUFFER_OFFSET(sizeof(indices) + ico_indices.size() * sizeof(GLuint)));
}

void initCylinder(std::vector<glm::vec4> &vertices, std::vector<GLuint> &indices) {
	/*
		Got an idea for how to draw it in https://stackoverflow.com/questions/32883572/how-to-draw-cylinder-in-modern-opengl
		Set the algorithm to something similar to the one provided for icosphere
	*/
	GLuint limit = 360;
	for (int i = 0; i < limit; ++i) {
		GLfloat u = i / (GLfloat) limit;
		GLfloat zVal = 0.5 * sin(2 * M_PI*u);
		GLfloat xVal = 0.5 * cos(2 * M_PI*u);
		int m0 = vertices.size();
		int m1 = m0 + 1;
		vertices.push_back(glm::vec4(xVal, 0.5, zVal, 1));
		vertices.push_back(glm::vec4(xVal, -0.5, zVal, 1));
		if (m0 >= 2) {
			indices.push_back(m0);
			indices.push_back(m0 - 1);
			indices.push_back(m0);
			indices.push_back(m1);
			//circle top
			indices.push_back(m0);
			indices.push_back(m0 - 2);
			indices.push_back(limit);
			//circle bottom
			indices.push_back(m1);
			indices.push_back(m1 - 2);
			indices.push_back(limit + 1);
		}
		indices.push_back(m0);
		indices.push_back(m1);
	}
	//center of circle top and bottom
	vertices.push_back(glm::vec4(0, 0.5, 0, 1));
	vertices.push_back(glm::vec4(0, -0.5, 0, 1));
}

void icosphere(int sub, std::vector<glm::vec4> &vertices, std::vector<GLuint> &indices) {
	for (GLfloat *v : vdata) {
		vertices.push_back(glm::vec4(v[0], v[1], v[2], 1));
	}
	indices.assign((GLuint *)tindices, (GLuint *)tindices + (sizeof(tindices) / sizeof(GLuint)));

	// use a regular subdivision (each tri into 4 equally-sized)
	for (int s = 0; s < sub; s++) {
		int isize = indices.size();
		for (int tri = 0; tri < isize; tri += 3) {
			int i0 = indices[tri];
			int i1 = indices[tri + 1];
			int i2 = indices[tri + 2];

			// create midpoints and "push" them out to the unit sphere
			// (but exclude the homogeneous coordinate)
			glm::vec3 midpoint0(glm::normalize(glm::vec3(vertices[i0] + vertices[i1]) * 0.5f));
			glm::vec3 midpoint1(glm::normalize(glm::vec3(vertices[i1] + vertices[i2]) * 0.5f));
			glm::vec3 midpoint2(glm::normalize(glm::vec3(vertices[i2] + vertices[i0]) * 0.5f));

			// add the midpoints to the vertices list
			int m0 = vertices.size();
			int m1 = m0 + 1;
			int m2 = m0 + 2;
			vertices.push_back(glm::vec4(midpoint0, 1));
			vertices.push_back(glm::vec4(midpoint1, 1));
			vertices.push_back(glm::vec4(midpoint2, 1));

			// now the four triangles
			indices[tri + 1] = m0;
			indices[tri + 2] = m2;
			indices.push_back(m0);
			indices.push_back(i1);
			indices.push_back(m1);
			indices.push_back(m0);
			indices.push_back(m1);
			indices.push_back(m2);
			indices.push_back(m2);
			indices.push_back(m1);
			indices.push_back(i2);
		}
	}
}

//----------------------------------------------------------------------------

void
mouse(int button, int state, int x, int y)
{
	
}

//----------------------------------------------------------------------------

void
reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	GLfloat aspect = GLfloat(width) / height;
	glm::mat4  projection = glm::perspective(glm::radians(45.0f), aspect, 0.5f, 3.0f);

	glUniformMatrix4fv(Projection, 1, GL_FALSE, glm::value_ptr(projection));
}

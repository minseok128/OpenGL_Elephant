//
// Display a elephant
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.  We us an orthographic projection
//   as the default projetion.

#include "cube.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

glm::mat4 projectMat;
glm::mat4 viewMat;

GLuint pvmMatrixID;

float rotAngleWorldx = 4.123f;
float rotAngleWorldy = 6.25f;
float rotAngleWorldz = 4.375f;

float rotAngleLeg = 0.0f;
int isDrawingCar = false;

typedef glm::vec4 color4;
typedef glm::vec4 point4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
	point4(-0.5, -0.5, 0.5, 1.0),
	point4(-0.5, 0.5, 0.5, 1.0),
	point4(0.5, 0.5, 0.5, 1.0),
	point4(0.5, -0.5, 0.5, 1.0),
	point4(-0.5, -0.5, -0.5, 1.0),
	point4(-0.5, 0.5, -0.5, 1.0),
	point4(0.5, 0.5, -0.5, 1.0),
	point4(0.5, -0.5, -0.5, 1.0)};

// RGBA colors
color4 vertex_colors[8] = {
	color4(0.0, 0.0, 0.0, 1.0), // black
	color4(0.0, 1.0, 1.0, 1.0), // cyan
	color4(1.0, 0.0, 1.0, 1.0), // magenta
	color4(1.0, 1.0, 0.0, 1.0), // yellow
	color4(1.0, 0.0, 0.0, 1.0), // red
	color4(0.0, 1.0, 0.0, 1.0), // green
	color4(0.0, 0.0, 1.0, 1.0), // blue
	color4(1.0, 1.0, 1.0, 1.0)	// white
};

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices
int Index = 0;
void quad(int a, int b, int c, int d)
{
	colors[Index] = vertex_colors[a];
	points[Index] = vertices[a];
	Index++;
	colors[Index] = vertex_colors[b];
	points[Index] = vertices[b];
	Index++;
	colors[Index] = vertex_colors[c];
	points[Index] = vertices[c];
	Index++;
	colors[Index] = vertex_colors[a];
	points[Index] = vertices[a];
	Index++;
	colors[Index] = vertex_colors[c];
	points[Index] = vertices[c];
	Index++;
	colors[Index] = vertex_colors[d];
	points[Index] = vertices[d];
	Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void colorcube()
{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
}

//----------------------------------------------------------------------------

// OpenGL initialization
void init()
{
	colorcube();

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
				 NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("src/vshader.glsl", "src/fshader.glsl");
	glUseProgram(program);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
						  BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
						  BUFFER_OFFSET(sizeof(points)));

	pvmMatrixID = glGetUniformLocation(program, "mPVM");

	projectMat = glm::perspective(glm::radians(65.0f), 1.0f, 0.1f, 100.0f);
	viewMat = glm::lookAt(glm::vec3(0, 0, 4), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

void drawLeg(glm::mat4 worldRotMat, glm::mat4 bodyMat)
{
	glm::mat4 modelMat, pvmMat, scaleMat, identityMat = glm::mat4(1.0f);
	glm::mat4 legMat;
	glm::vec3 eachLegPos[4];

	int eachLeglDir[4];
	const float xPos = 0.6f, yPos = 0.4f, zPos = 0.4f;

	eachLegPos[0] = glm::vec3(xPos, yPos, -zPos);	// rear right
	eachLegPos[1] = glm::vec3(xPos, -yPos, -zPos);	// rear left
	eachLegPos[2] = glm::vec3(-xPos, yPos, -zPos);	// front right
	eachLegPos[3] = glm::vec3(-xPos, -yPos, -zPos); // front left
	eachLeglDir[0] = 1;
	eachLeglDir[1] = -1;
	eachLeglDir[2] = -1;
	eachLeglDir[3] = 1;

	// 허벅지
	for (int i = 0; i < 4; i++)
	{
		// 허벅지
		legMat = glm::translate(identityMat, eachLegPos[i]);
		legMat = glm::translate(legMat, glm::vec3(0.0f, 0.0f, 0.5f)); // 상단 끝 부분을 회전 축으로 이동
		legMat = glm::rotate(legMat, -rotAngleLeg * 60.0f * eachLeglDir[i], glm::vec3(0, 1, 0));
		legMat = glm::translate(legMat, glm::vec3(0.0f, 0.0f, -0.5f)); // 다시 원래 위치로 되돌리기
		scaleMat = glm::scale(identityMat, glm::vec3(0.5, 0.5, 0.5));
		pvmMat = projectMat * viewMat * worldRotMat * bodyMat * legMat * scaleMat;
		glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);

		// 무릎
		modelMat = glm::translate(legMat, glm::vec3(0, 0, -0.25));
		scaleMat = glm::scale(identityMat, glm::vec3(0.45, 0.375, 0.2));
		pvmMat = projectMat * viewMat * worldRotMat * bodyMat * modelMat * scaleMat;
		glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);

		// 종아리
		modelMat = glm::translate(modelMat, glm::vec3(0.0, 0.0, -0.31));
		modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.0f, 0.5f)); // 상단 끝 부분을 회전 축으로 이동
		modelMat = glm::rotate(modelMat, -rotAngleLeg * 50.0f * eachLeglDir[i], glm::vec3(0, 1, 0));
		modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.0f, -0.5f)); // 다시 원래 위치로 되돌리기
		scaleMat = glm::scale(identityMat, glm::vec3(0.35, 0.35, 0.5));
		pvmMat = projectMat * viewMat * worldRotMat * bodyMat * modelMat * scaleMat;
		glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);

		// 발
		modelMat = glm::translate(modelMat, glm::vec3(0.025, 0.0, -0.25));
		modelMat = glm::rotate(modelMat, -rotAngleLeg * 75.0f * eachLeglDir[i], glm::vec3(0, 1, 0));
		scaleMat = glm::scale(identityMat, glm::vec3(0.5, 0.36, 0.175));
		pvmMat = projectMat * viewMat * worldRotMat * bodyMat * modelMat * scaleMat;
		glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	}
}

void drawHead(glm::mat4 worldRotMat, glm::mat4 bodyMat)
{
	glm::mat4 modelMat, pvmMat, scaleMat, identityMat = glm::mat4(1.0f);
	glm::mat4 headMat, noseMat;

	headMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.25f, .0f, -0.2f));
	headMat = glm::rotate(headMat, rotAngleLeg * 35.0f, glm::vec3(1.0f, 0.0f, 0.0f));

	// 머리
	modelMat = glm::translate(identityMat, glm::vec3(0.75, 0, 0.45));
	modelMat = glm::rotate(modelMat, -0.25f, glm::vec3(0, 1, 0));
	scaleMat = glm::scale(identityMat, glm::vec3(0.65, 0.6, 0.65));
	pvmMat = projectMat * viewMat * worldRotMat * bodyMat * headMat * modelMat * scaleMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// 귀
	for (int i = 0; i < 2; i++)
	{
		int sign = i == 0 ? 1 : -1;
		modelMat = glm::translate(identityMat, glm::vec3(0.7, 0.5 * sign, 0.45));
		modelMat = glm::rotate(modelMat, -0.25f, glm::vec3(1 * sign, 1, -1 * sign));
		scaleMat = glm::scale(identityMat, glm::vec3(0.125, 0.65, 0.65));
		pvmMat = projectMat * viewMat * worldRotMat * bodyMat * headMat * modelMat * scaleMat;
		glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	}

	// 상아
	for (int i = 0; i < 2; i++)
	{
		int sign = i == 0 ? 1 : -1;
		modelMat = glm::translate(identityMat, glm::vec3(0.8, 0.275 * sign, 0.0));
		modelMat = glm::rotate(modelMat, -.35f, glm::vec3(-1 * sign, 1, -1 * sign));
		scaleMat = glm::scale(identityMat, glm::vec3(0.1, 0.1, 0.65));
		pvmMat = projectMat * viewMat * worldRotMat * bodyMat * headMat * modelMat * scaleMat;
		glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	}

	// 코1
	noseMat = glm::translate(identityMat, glm::vec3(0.85, 0, 0.0));
	// noseMat = glm::rotate(noseMat, 0, glm::vec3(0, 1, 0));
	noseMat = glm::rotate(noseMat, -rotAngleLeg * 35.0f, glm::vec3(0, 0, 1));
	scaleMat = glm::scale(identityMat, glm::vec3(0.45, 0.45, 0.65));
	pvmMat = projectMat * viewMat * worldRotMat * bodyMat * headMat * noseMat * scaleMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// 코2
	noseMat = glm::translate(noseMat, glm::vec3(0, 0, -0.5));
	noseMat = glm::rotate(noseMat, 0.1f, glm::vec3(0, 1, 0));
	noseMat = glm::rotate(noseMat, -rotAngleLeg * 35.0f, glm::vec3(0, 0, 1));
	scaleMat = glm::scale(identityMat, glm::vec3(0.35, 0.35, 0.45));
	pvmMat = projectMat * viewMat * worldRotMat * bodyMat * headMat * noseMat * scaleMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// 코3
	noseMat = glm::translate(noseMat, glm::vec3(0, 0, -0.3));
	noseMat = glm::rotate(noseMat, 0.15f, glm::vec3(0, 1, 0));
	noseMat = glm::rotate(noseMat, -rotAngleLeg * 35.0f, glm::vec3(0, 0, 1));
	scaleMat = glm::scale(identityMat, glm::vec3(0.225, 0.225, 0.4));
	pvmMat = projectMat * viewMat * worldRotMat * bodyMat * headMat * noseMat * scaleMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

void drawBody(glm::mat4 worldRotMat, glm::mat4 bodyMat)
{
	glm::mat4 modelMat, pvmMat, scaleMat, identityMat = glm::mat4(1.0f);

	// 몸통
	scaleMat = glm::scale(identityMat, glm::vec3(1.4, 1, 0.9));
	pvmMat = projectMat * viewMat * worldRotMat * bodyMat * scaleMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// 꼬리
	modelMat = glm::translate(identityMat, glm::vec3(-0.8, 0, 0));
	modelMat = glm::rotate(modelMat, 0.35f, glm::vec3(0, 1, 0));
	scaleMat = glm::scale(identityMat, glm::vec3(0.1, 0.1, 0.75));
	pvmMat = projectMat * viewMat * worldRotMat * bodyMat * modelMat * scaleMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

void drawElephant(glm::mat4 worldRotMat)
{
	glm::mat4 bodyMat = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
	bodyMat = glm::rotate(bodyMat, -rotAngleLeg * 10.0f, glm::vec3(1, 0, 0));

	drawBody(worldRotMat, bodyMat); // 몸통 그리기
	drawHead(worldRotMat, bodyMat); // 머리 그리기
	drawLeg(worldRotMat, bodyMat);	// 다리 그리기
}

void display(void)
{
	glm::mat4 worldRotMat, pvmMat;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	worldRotMat = glm::rotate(glm::mat4(1.0f), rotAngleWorldx, glm::vec3(1.0f, 0.0f, 0.0f));
	worldRotMat *= glm::rotate(glm::mat4(1.0f), rotAngleWorldy, glm::vec3(0.0f, 1.0f, 0.0f));
	worldRotMat *= glm::rotate(glm::mat4(1.0f), rotAngleWorldz, glm::vec3(0.0f, 0.0f, 1.0f));

	drawElephant(worldRotMat);

	glutSwapBuffers();
}

//----------------------------------------------------------------------------
void idle()
{
	static int prevTime = glutGet(GLUT_ELAPSED_TIME);
	int currTime = glutGet(GLUT_ELAPSED_TIME);

	if (abs(currTime - prevTime) >= 20)
	{
		rotAngleLeg = glm::radians(cos(currTime / 100.0f) * 360.0f / 2000.0f);

		prevTime = currTime;
		glutPostRedisplay();
	}
}

//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case '1':
		rotAngleWorldx += 0.125f;
		break;
	case '2':
		rotAngleWorldy += 0.125f;
		break;
	case '3':
		rotAngleWorldz += 0.125f;
		break;
	case 033: // Escape key
	case 'q':
	case 'Q':
		exit(EXIT_SUCCESS);
		break;
	}
}

//----------------------------------------------------------------------------

void resize(int w, int h)
{
	float ratio = (float)w / (float)h;
	glViewport(0, 0, w, h);

	projectMat = glm::perspective(glm::radians(65.0f), ratio, 0.1f, 100.0f);

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(700, 700);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("Color Elephant");

	glewInit();

	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(resize);
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}

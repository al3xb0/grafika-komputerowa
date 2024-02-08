#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include <vector>
#include "SOIL.h"

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"

#include "Box.cpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <random>


namespace texture {
	GLuint earth;
	GLuint clouds;
	GLuint moon;
	GLuint ship;
	GLuint jup;
	GLuint dome;
	GLuint sun;
	GLuint saturn;
	GLuint venus;
	GLuint arena;
	GLuint pbr;
	GLuint pbrNormal;

	GLuint grid;

	GLuint earthNormal;
	GLuint asteroidNormal;
	GLuint shipNormal;
	GLuint hud;
	GLuint mars;
	GLuint neptun;
	GLuint mercury;
}

GLuint screenWidth, screenHeight;

GLuint program;
GLuint programEarth;
GLuint programDome;
GLuint programSun;
GLuint programTex;
GLuint programShip;
GLuint programSkybox;
GLuint programPBR;



GLuint programSSAO;
GLuint programSSAOBlur;

std::vector<glm::vec3> ssaoKernel;
GLuint noiseTexture;


GLuint ssaoTexture;
GLuint ssaoBlurTexture;
GLuint ssaoFrameBuffer;
GLuint ssaoBlurFrameBuffer;
GLuint gFrameBuffer;
GLuint gDepthBuffer;
GLuint gColorBuffers[6];


glm::mat4 cameraMatrix, perspectiveMatrix;



const unsigned int width = 800;
const unsigned int height = 800;


Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;
Core::RenderContext modelContext;
Core::RenderContext cubeContext;



glm::vec3 cameraPos = glm::vec3(-4.f, 0, 0);
glm::vec3 cameraDir = glm::vec3(1.f, 0.f, 0.f);

glm::vec3 spaceshipPos = glm::vec3(-4.f, 0, 0);

glm::vec3 spaceshipDir = glm::vec3(1.f, 0.f, 0.f);

float angle = glm::radians(180.0f);


float phi = glm::radians(45.0f);

GLuint VAO, VBO;

GLuint quadVAO, quadVBO;

float aspectRatio = 1.f;
float exposition = 1.f;
float appLoadingTime;
float exposure = 1.0f;


float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;
bool firstMouse = true;

glm::vec3 moonPos;
glm::vec3 relativePos = glm::vec3(0.f,0.f,0.f);
glm::vec3 lightPos = glm::vec3(0, 0, 0);
glm::vec3 lightColor = glm::vec3(0.9, 0.7, 0.8) * 1000;
glm::vec3 spotlightPos = glm::vec3(0, 0, 0);
glm::vec3 spotlightConeDir = glm::vec3(0, 0, 0);
glm::vec3 spotlightColor = glm::vec3(0.5, 0.9, 0.8) * 100;
float spotlightPhi = 3.14;


float ambientFactor = 0.15f;
float diffuseFactor = 0.75f;
float radius = 0.5f;
float bias = 0.025f;


struct Star {
	glm::vec3 position;
	glm::vec3 velocity;
	float speed;
};

std::vector<Star> stars;




void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}


void createStar() {
	Star star = Star();
	
	glm::vec3 playerForward = glm::normalize(spaceshipDir);
	glm::vec3 spawnPosition = cameraPos + playerForward * glm::linearRand(5.0f, 30.0f) + glm::vec3(0.0f, 10.0f, 0.0f);


	star.position = spawnPosition;
	star.velocity = glm::vec3(0.0f, -1.0f, 0.0f);
	star.speed = glm::linearRand(0.001f, 0.005f);

	stars.push_back(star);
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; 

	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;


	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;


	spaceshipDir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	spaceshipDir.y = sin(glm::radians(pitch));
	spaceshipDir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	spaceshipDir = glm::normalize(spaceshipDir);
}



glm::mat4 createCameraMatrix()
{
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide, cameraDir));
	glm::mat4 cameraRotrationMatrix = glm::mat4({
		cameraSide.x,cameraSide.y,cameraSide.z,0,
		cameraUp.x,cameraUp.y,cameraUp.z ,0,
		-cameraDir.x,-cameraDir.y,-cameraDir.z,0,
		0.,0.,0.,1.,
		});
	cameraRotrationMatrix = glm::transpose(cameraRotrationMatrix);
	glm::mat4 cameraMatrix = cameraRotrationMatrix * glm::translate(-cameraPos);

	return cameraMatrix;
}




GLuint loadSkybox(const std::vector<std::string>& faces) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (GLuint i = 0; i < faces.size(); i++) {
		unsigned char* data = SOIL_load_image(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			SOIL_free_image_data(data);
		}
		else {
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			SOIL_free_image_data(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

std::vector<std::string> skyboxFaces = {
		"textures/skybox/space_rt.png",
		"textures/skybox/space_lf.png",
		"textures/skybox/space_up.png",
		"textures/skybox/space_dn.png",
		"textures/skybox/space_bk.png",
		"textures/skybox/space_ft.png"
};

GLuint skyboxTexture;

glm::mat4 createPerspectiveMatrix()
{

	glm::mat4 perspectiveMatrix;
	float n = 0.05;
	float f = 20.;
	float a1 = glm::min(aspectRatio, 1.f);
	float a2 = glm::min(1 / aspectRatio, 1.f);
	perspectiveMatrix = glm::mat4({
		1,0.,0.,0.,
		0.,aspectRatio,0.,0.,
		0.,0.,(f + n) / (n - f),2 * f * n / (n - f),
		0.,0.,-1.,0.,
		});


	perspectiveMatrix = glm::transpose(perspectiveMatrix);

	return perspectiveMatrix;
}




void drawObjectColor(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color) {
	GLuint prog = program;
	glUseProgram(prog);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(prog, "color"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(prog, "lightPos"), 0, 0, 0);
	Core::DrawContext(context);
	glUseProgram(0);

}

void drawObjectPBR(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color, GLuint textureID, GLuint normalMapID) {

	GLuint prog = programPBR;
	glUseProgram(prog);

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform1f(glGetUniformLocation(prog, "exposition"), exposition);
	glUniform3f(glGetUniformLocation(prog, "color"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(prog, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(prog, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(glGetUniformLocation(prog, "lightColor"), lightColor.x, lightColor.y, lightColor.z);
	glUniform1f(glGetUniformLocation(prog, "exposure"), exposure);


	glUniform3f(glGetUniformLocation(prog, "spotlightConeDir"), spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	glUniform3f(glGetUniformLocation(prog, "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
	glUniform3f(glGetUniformLocation(prog, "spotlightColor"), spotlightColor.x, spotlightColor.y, spotlightColor.z);
	glUniform1f(glGetUniformLocation(prog, "spotlightPhi"), spotlightPhi);

	Core::SetActiveTexture(textureID, "colorTexture", prog, 0);

	Core::SetActiveTexture(normalMapID, "normalSampler", prog, 1);

	Core::DrawContext(context);
	glUseProgram(0);
}

void drawEarth(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint normalMapID) {
	GLuint prog = programEarth;
	glUseProgram(prog);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform3f(glGetUniformLocation(prog, "lightPos"), 0, 0, 0);
	glUniform1f(glGetUniformLocation(prog, "exposure"), exposure);

	

	Core::SetActiveTexture(textureID, "colorTexture", prog, 0);

	Core::SetActiveTexture(normalMapID, "normalSampler", prog, 1);

	
	Core::DrawContext(context);
	glUseProgram(0);

}

void drawShip(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID) {
	GLuint prog = programShip;
	glUseProgram(prog);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform3f(glGetUniformLocation(prog, "lightPos"), 0, 0, 0);
	glUniform1f(glGetUniformLocation(prog, "exposure"), exposure);
	Core::SetActiveTexture(textureID, "colorTexture", prog, 0);
	//Core::SetActiveTexture(texture::scratches, "scratches", prog, 1);
	//Core::SetActiveTexture(texture::rust, "scratches", prog, 2);

	Core::DrawContext(context);
	glUseProgram(0);

}


void drawSun(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID) {
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f - appLoadingTime;
	GLuint prog = programSun;
	glUseProgram(prog);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform3f(glGetUniformLocation(prog, "lightPos"), 0, 0, 0);
	glUniform3f(glGetUniformLocation(prog, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(glGetUniformLocation(prog, "time"), time);
	glUniform1f(glGetUniformLocation(prog, "exposure"), exposure);
	Core::SetActiveTexture(textureID, "colorTexture", prog, 0);

	Core::DrawContext(context);
	glUseProgram(0);
}




void drawSkybox(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID)
{
	GLuint prog = programSkybox;

	glUseProgram(prog);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	Core::SetActiveTexture(textureID, "textureSampler", program, 0);
	glUniform3f(glGetUniformLocation(prog, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawContext(context);

	glUseProgram(0);
}

void drawObjectTexture(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID)
{

	GLuint prog = programTex;
	glUseProgram(prog);

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);


	glUniform3f(glGetUniformLocation(prog, "lightPos"), 0, 0, 0);
	glUniform3f(glGetUniformLocation(prog, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(glGetUniformLocation(prog, "exposure"), exposure);
	
	


	Core::SetActiveTexture(textureID, "colorTexture", prog, 0);


	Core::DrawContext(context);

	glUseProgram(0);
}


void drawObjectDome(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID)
{

	GLuint prog = programDome;
	glUseProgram(prog);

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(prog, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);


	glUniform3f(glGetUniformLocation(prog, "lightPos"), 0, 0, 0);
	glUniform3f(glGetUniformLocation(prog, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(glGetUniformLocation(prog, "exposure"), exposure);
	


	Core::SetActiveTexture(textureID, "colorTexture", prog, 0);


	Core::DrawContext(context);

	glUseProgram(0);
}

//void HUD()
//{
//	glBindTexture(GL_TEXTURE_2D, texture::hud);
//	glColor3f(1.0, 1.0, 1.0);
//	glBegin(GL_QUADS);
//	glTexCoord2f(0.0, 1.0); glVertex2f(0.05, 0.05);
//	glTexCoord2f(1.0, 1.0); glVertex2f(0.3, 0.05);
//	glTexCoord2f(1.0, 0.0); glVertex2f(0.3, 0.15);
//	glTexCoord2f(0.0, 0.0); glVertex2f(0.05, 0.15);
//	glEnd();
//}

//void drawHUD()
//{
//	glMatrixMode(GL_PROJECTION);
//	glPushMatrix();
//	glLoadIdentity();
//	gluOrtho2D(0.0, 1.0, 1.0, 0.0);
//	glMatrixMode(GL_MODELVIEW);
//	glPushMatrix();
//	glLoadIdentity();
//	HUD();
//	glMatrixMode(GL_PROJECTION);
//	glPopMatrix();
//	glMatrixMode(GL_MODELVIEW);
//	glPopMatrix();
//}


void renderScene(GLFWwindow* window)
{

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		exposure *= 1.001;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		exposure /= 1.001;

	glm::mat4 transformation;

	float time = glfwGetTime();

	for (int i = 0; i < stars.size(); i++) {
		drawObjectTexture(sphereContext, glm::translate(stars[i].position) * glm::scale(glm::vec3(0.03f)), texture::venus);
	}

	for (int i = 0; i < stars.size(); i++) {
		stars[i].position += stars[i].velocity * stars[i].speed;
	}

	for (int i = 0; i < stars.size(); i++) {

		if (stars[i].position.y < cameraPos.y) {
			stars.erase(stars.begin() + i);
			i--;
		}
	}



	moonPos = glm::vec3((glm::eulerAngleY(time / 6) * glm::translate(glm::vec3(6.f, 0, 0)) * glm::eulerAngleY(time / 10) * glm::translate(glm::vec3(1.f, 0, 0))) * glm::vec4(0.f, 0.f, 0.f, 1.f));

	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});

	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	glBindFramebuffer(GL_FRAMEBUFFER, gFrameBuffer);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	
	glDisable(GL_DEPTH_TEST);
	drawSkybox(cubeContext, glm::translate(cameraPos), skyboxTexture);

	glEnable(GL_DEPTH_TEST);

	drawSun(sphereContext, glm::mat4(), texture::sun);

	drawObjectPBR(sphereContext, glm::eulerAngleY(time / 3) * glm::translate(glm::vec3(7.f, 0, 0)) * glm::scale(glm::vec3(0.55f)) * glm::translate(glm::vec3(1.2f, 0, 0)),
		glm::vec3(0.7, 0.4, 0.4), texture::pbr, texture::pbrNormal);
	drawEarth(sphereContext, glm::eulerAngleY(time / 6) * glm::translate(glm::vec3(6.f, 0, 0)) * glm::scale(glm::vec3(0.7f)), texture::earth, texture::earthNormal);
	drawObjectTexture(modelContext,
		glm::eulerAngleY(time / 6) * glm::translate(glm::vec3(6.f, 0, 0)) * glm::eulerAngleY(time / 10) * glm::translate(glm::vec3(1.f, 0, 0)) * glm::scale(glm::vec3(0.08f)),
		texture::arena);
	//drawObjectDome(sphereContext,
		//glm::eulerAngleY(time / 6) * glm::translate(glm::vec3(6.f, 0, 0)) * glm::eulerAngleY(time / 10) * glm::translate(glm::vec3(1.f, 0, 0)) * glm::scale(glm::vec3(0.3f)),
		//texture::dome);


	drawObjectTexture(sphereContext, glm::eulerAngleY(time / 2) * glm::translate(glm::vec3(2.f, 0, 0)) * glm::scale(glm::vec3(0.3f)), texture::mercury);
	drawObjectTexture(sphereContext, glm::eulerAngleY(time / 4) * glm::translate(glm::vec3(10.f, 0, 0)) * glm::scale(glm::vec3(0.5f)), texture::mars);
	drawObjectTexture(sphereContext, glm::eulerAngleY(time / 5) * glm::translate(glm::vec3(14.f, 0, 0)) * glm::scale(glm::vec3(0.9f)), texture::jup);
	drawObjectTexture(sphereContext, glm::eulerAngleY(time / 6) * glm::translate(glm::vec3(20.f, 0, 0)) * glm::scale(glm::vec3(0.9f)), texture::neptun);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFrameBuffer);
	
	glUseProgram(programSSAO);
	for (int i = 0; i < 64; i++)
	{
		std::string uniformName = "samples[" + std::to_string(i) + "]";
		glUniform3f(glGetUniformLocation(programSSAO, uniformName.c_str()), ssaoKernel[i].x, ssaoKernel[i].y, ssaoKernel[i].z);
	}
	glUniformMatrix4fv(glGetUniformLocation(programSSAO, "perspectiveMatrix"), 1, GL_FALSE, (float*)&perspectiveMatrix);
	Core::SetActiveTexture(gColorBuffers[1], "gPosition", programSSAO, 0);
	Core::SetActiveTexture(gColorBuffers[2], "gNormal", programSSAO, 1);
	Core::SetActiveTexture(noiseTexture, "texNoise", programSSAO, 2);
	glUniform1f(glGetUniformLocation(programSSAO, "radius"), radius);
	glUniform1f(glGetUniformLocation(programSSAO, "bias"), bias);
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFrameBuffer);

	glUseProgram(programSSAOBlur);
	Core::SetActiveTexture(ssaoTexture, "ssaoInput", programSSAOBlur, 0);
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//drawHUD();
	
	glfwSwapBuffers(window);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	screenWidth = width;
	screenHeight = height;
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
}
void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}


void initSSAOKernel()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distr1(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distr2(0.0f, 1.0f);
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(distr1(gen), distr1(gen), distr2(gen));
		sample = glm::normalize(sample);
		sample *= distr2(gen);
		float scale = float(i) / 64.0;

		scale = glm::lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}
}

void initSSAONoise()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distr(-1.0f, 1.0f);
	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(distr(gen), distr(gen), 0.0f);
		ssaoNoise.push_back(noise);
	}
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void initSSAOFrameBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFrameBuffer);
	glGenTextures(1, &ssaoTexture);
	glBindTexture(GL_TEXTURE_2D, ssaoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTexture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initSSAOBlurFrameBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFrameBuffer);
	glGenTextures(1, &ssaoBlurTexture);
	glBindTexture(GL_TEXTURE_2D, ssaoBlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurTexture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initFrameBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, gFrameBuffer);
	glGenTextures(2,gColorBuffers);

	glBindTexture(GL_TEXTURE_2D, gColorBuffers[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gColorBuffers[1], 0);

	glBindTexture(GL_TEXTURE_2D, gColorBuffers[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gColorBuffers[2], 0);


	glGenRenderbuffers(1, &gDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, gDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenWidth);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepthBuffer);


	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
	auto gStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (gStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete: " << gStatus << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


void init(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



	program = shaderLoader.CreateProgram("shaders/shader_earth.vert", "shaders/shader_earth.frag");
	programEarth = shaderLoader.CreateProgram("shaders/shader_earth.vert", "shaders/shader_earth.frag");
	programDome = shaderLoader.CreateProgram("shaders/shader_dome.vert", "shaders/shader_dome.frag");
	programTex = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_sun.vert", "shaders/shader_sun.frag");
	programShip = shaderLoader.CreateProgram("shaders/shader_ship.vert", "shaders/shader_ship.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_sky.vert", "shaders/shader_sky.frag");
	programPBR = shaderLoader.CreateProgram("shaders/shader_pbr.vert", "shaders/shader_pbr.frag");
	programSSAO = shaderLoader.CreateProgram("shaders/shader_ssao.vert", "shaders/shader_ssao.frag");
	programSSAOBlur = shaderLoader.CreateProgram("shaders/shader_ssao_blur.vert", "shaders/shader_ssao_blur.frag");


	texture::pbr = Core::LoadTexture("textures/1AMR.png");
	texture::pbrNormal = Core::LoadTexture("textures/1Normal.png");
	texture::earth = Core::LoadTexture("textures/earth.png");
	texture::clouds = Core::LoadTexture("textures/clouds.jpg");
	texture::moon = Core::LoadTexture("textures/moon2.jpg");
	texture::ship = Core::LoadTexture("textures/spaceship2.jpg");
	texture::jup = Core::LoadTexture("textures/jup.jpg");
	texture::dome = Core::LoadTexture("textures/grid2.jpg");
	texture::sun = Core::LoadTexture("textures/sun.jpg");
	texture::saturn = Core::LoadTexture("textures/saturn.jpg");
	texture::venus = Core::LoadTexture("textures/star.jpg");
	texture::arena = Core::LoadTexture("textures/moon2.jpg");
	texture::hud = Core::LoadTexture("textures/hud.png");
	texture::mercury = Core::LoadTexture("textures/mercury.jpg");
	texture::mars = Core::LoadTexture("textures/mars.jpg");
	texture::neptun = Core::LoadTexture("textures/nep.jpg");

	texture::grid = Core::LoadTexture("textures/grid.png");

	texture::earthNormal = Core::LoadTexture("textures/earth_normalmap.png");
	texture::asteroidNormal = Core::LoadTexture("textures/rust_normal.jpg");
	texture::shipNormal = Core::LoadTexture("textures/spaceship_normal.jpg");

	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/ggg.obj", shipContext);
	loadModelToContext("./models/complex.obj", modelContext);
	loadModelToContext("./models/cube.obj", cubeContext);



	skyboxTexture = loadSkybox(skyboxFaces);

	
	initFrameBuffer();
	initSSAOFrameBuffer();
	initSSAOBlurFrameBuffer();
	initSSAOKernel();
	initSSAONoise();


	appLoadingTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
	shaderLoader.DeleteProgram(programEarth);
	shaderLoader.DeleteProgram(programDome);
	shaderLoader.DeleteProgram(programTex);
	shaderLoader.DeleteProgram(programSun);
	shaderLoader.DeleteProgram(programShip);
	shaderLoader.DeleteProgram(programSkybox);
	shaderLoader.DeleteProgram(programPBR);
	shaderLoader.DeleteProgram(programSSAO);
	shaderLoader.DeleteProgram(programSSAOBlur);
}

//obsluga wejscia
void processInput(GLFWwindow* window)
{
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::vec3(0.f, 1.f, 0.f);
	float angleSpeed = 0.005f;
	float moveSpeed = 0.005f;
	float relSpeed = 0.0005;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		spaceshipPos += spaceshipDir * moveSpeed;
		glm::vec3 newRelativePos1 = relativePos + spaceshipDir * relSpeed;
		if (glm::distance(moonPos, moonPos + newRelativePos1 + glm::vec3(0, 0.1, 0)) <= 0.25 && newRelativePos1.y >= moonPos.y - 0.05) {
			relativePos = newRelativePos1;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		spaceshipPos -= spaceshipDir * moveSpeed;
		glm::vec3 newRelativePos2 = relativePos - spaceshipDir * relSpeed;
		if (glm::distance(moonPos, moonPos + newRelativePos2 + glm::vec3(0, 0.1, 0)) <= 0.25 && newRelativePos2.y >= moonPos.y-0.05) {
			relativePos = newRelativePos2;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		createStar();
	}
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		spaceshipPos += spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		spaceshipPos -= spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		spaceshipPos += spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		spaceshipPos -= spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		spaceshipDir = glm::vec3(glm::eulerAngleY(angleSpeed) * glm::vec4(spaceshipDir, 0));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		spaceshipDir = glm::vec3(glm::eulerAngleY(-angleSpeed) * glm::vec4(spaceshipDir, 0));

	// cameraPos = spaceshipPos - 1 * spaceshipDir + glm::vec3(0, 1, 0) * 0.7f;

	//cameraDir = glm::normalize(-cameraPos);
	
	cameraPos = moonPos + relativePos + glm::vec3(0, 0.2, 0);
	cameraDir = spaceshipDir;


}

// funkcja jest glowna petla
void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window, mouse_callback);

		renderScene(window);
		glfwPollEvents();
	}
}
//}
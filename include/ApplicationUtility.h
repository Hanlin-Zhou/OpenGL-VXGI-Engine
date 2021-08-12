#include <iostream>
#include <glad/glad.h>


float lerp(float a, float b, float f);

unsigned int initQuad(float size, float dist);


void renderQuad(unsigned int VAO);


unsigned int loadTexturePath(const char* path);


unsigned int loadHDRIPath(const char* path);


unsigned int bindDepthMap(unsigned int FBO, int width, int height);


void attachRBOToBuffer(unsigned int FBO, int width, int height, GLenum component, GLenum attachment, unsigned int samples);

unsigned int genCubeMap(int width, int height, GLenum format, GLenum component);


unsigned int bindCubeDepthMap(unsigned int FBO, int width, int height);


unsigned int bindColorBuffer(unsigned int FBO, int width, int height, GLenum attachment, GLint format, unsigned int samples, GLint option);


unsigned int initSkybox();

void renderSkybox(unsigned int VAO);
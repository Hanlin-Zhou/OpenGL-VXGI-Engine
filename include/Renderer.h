#pragma once
#include <ApplicationUtility.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/string_cast.hpp>
#include <Vector>
#include <random>
#include <shader.h>
#include <string>
#include <Model.h>
#include <Light.h>
#include <Camera.h>


class Renderer {
private:
	// 0 : Need to confirm setting
	// 1 : Need to do pre-calculation
	// 2 : Running. Need to draw next frame
	unsigned int state;

	// settings
	bool MSAA;
	unsigned int MSAASample;
	bool PCSS;
	bool PeterPan;
	bool SSAO;
	bool HDR;
	bool SkyBox;
	bool ShowDebug;
	bool GLDebugOutput;
	bool ShowTexture;
	bool ShowNormal;
	unsigned int renderWidth;
	unsigned int renderHeight;
	unsigned int shadowWidth;
	unsigned int shadowHeight;

	bool SVOGI;

	Model myModel;

	// Path to Model and HDRI
	std::string modelPath;
	std::string HDRIPath;

	// Matrices
	glm::mat4 view_mat;
	glm::mat4 proj_mat;

	// ---OpenGL GLints and related misc---
	// G Buffer
	unsigned int gBuffer;
	unsigned int gPosition;
	unsigned int gNormal;
	unsigned int gAlbedoSpec;
	unsigned int gShadow;
	Shader gBufferGeoPass;
	Shader gBufferLightPass;
	Shader gBufferCombine;

	// Down sampled G Buffer
	unsigned int ds_gBuffer;
	unsigned int ds_gPosition;
	unsigned int ds_gNormal;
	unsigned int ds_gAlbedoSpec;
	unsigned int ds_gViewPos;
	Shader DownSampleShader;

	// SSAO Buffer
	unsigned int ssaoFBO;
	unsigned int ssaoOut;
	std::vector<glm::vec3> ssaoKernel;
	Shader SSAO_Shader;
	
	// Post Processing Buffer
	unsigned int PostProcessingFBO;
	unsigned int NoiseTexture;
	unsigned int PostProcessingOut;
	Shader BlurShader;

	// Cube Shadow FBO and Shadow Projection Matrix
	unsigned int DepthCubeFBO;
	unsigned int DepthCubeMap;
	glm::mat4 SProj;
	Shader PCSSDepthShader;

	// SkyBox / HDRI
	unsigned int HdriFBO;
	unsigned int SkyBoxCubeMap;
	unsigned int SkyBoxOut;
	unsigned int HDRIwidth;
	Shader SkyboxShader;
	Shader HdriConvert;

	// SVOGI
	unsigned int SVOGIFBO;
	unsigned int SVOGI3DTEX;
	float MaxCoord;
	int VoxelSize;
	Shader VoxelizeShader;

	// SVOGI Visualizer
	unsigned int VoxelVisFBO;
	unsigned int VoxelVisFrontFace;
	unsigned int VoxelVisBackFace;
	unsigned int VoxelVisOut;

	Shader VoxelVisFaceShader;
	Shader VoxelVisTraceShader;
	

	// Debug
	unsigned int DebugOut;
	Shader DebugShader;

	// misc
	unsigned int quadVAO;
	unsigned int DEBUGquadVAO;
	unsigned int SkyBoxVAO;

	void initializeBuffers();
	void loadModel();
	void loadHDRI(bool loaded);
	void LoadShaders();
	void Draw();

public:
	static Camera cam;
	Light myLight;

	Renderer() {};
	Renderer(unsigned int width, unsigned int height);
	~Renderer();
	void run();
	unsigned int getState();
	
	void updateMats();
	void setWidthHeight(unsigned int width, unsigned int height);

	friend void RendererInitSetting(Renderer &renderer);
	friend void LoadMenu(Renderer &renderer);
	friend void RendererMenu(Renderer &renderer);
	friend void DebugMenu(Renderer &renderer);
};
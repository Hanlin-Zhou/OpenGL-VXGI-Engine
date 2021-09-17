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
#include <GLFW/glfw3.h>

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
	bool ShadowBluring;
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
	unsigned int gTangent;
	Shader gBufferGeoPass;
	Shader gBufferLightPass;
	Shader gBufferCombine;

	// Shadow
	unsigned int ShadowBuffer;
	unsigned int ShadowRaw;
	unsigned int ShadowBlur;
	unsigned int ShadowOut;

	// Down sampled G Buffer
	unsigned int ds_gBuffer;
	unsigned int ds_gPosition;
	unsigned int ds_gNormal;
	unsigned int ds_gAlbedoSpec;
	unsigned int ds_gTangent;
	unsigned int ds_gMSAA;
	unsigned int ds_gViewPos;
	Shader DownSampleShader;
	Shader MSAADetectShader;

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
	unsigned int Albedo3D;
	unsigned int Normal3D;
	unsigned int Radiance3D;
	float MaxCoord;
	unsigned int vLevel;
	unsigned int workgroupsize;
	int VoxelSize;
	glm::mat4 VoxelProjectMat;
	Shader VoxelizeShader;
	Shader LightInjectionShader;
	Shader MipmapShader;
	Shader ConeTracingShader;

	float GI_OcclusionOffsetFactor;
	float GI_DiffuseOffsetFactor;
	float GI_SpecularOffsetFactor;
	float GI_SpecularAperture;
	float GI_DiffuseAperture;
	float GI_OcculsionAperture;
	float GI_stepSize;
	float GI_DiffuseConeAngleMix;

	// SVOGI Visualizer
	unsigned int VoxelVisFBO;
	unsigned int VoxelVisFrontFace;
	unsigned int VoxelVisBackFace;
	unsigned int VoxelVisOut;

	Shader VoxelVisFaceShader;
	Shader VoxelVisTraceShader;
	

	// Debug
	unsigned int DebugOut;
	float DebugWindowSize;
	Shader DebugShader;

	// misc
	unsigned int quadVAO;
	unsigned int DEBUGquadVAO;
	unsigned int SkyBoxVAO;

	void initializeBuffers();
	void loadModel();
	void loadHDRI(bool loaded);
	void voxelize();
	void gBufferDraw();
	void SSAODraw();
	void BlurDraw(unsigned int texture);
	void SkyBoxDraw();
	void ShadowMapDraw();
	void LightInjection();
	void MipmapBuild(unsigned int level);
	void ConeTrace(unsigned int buffer);
	void gBufferLightingPassDraw();
	void gBufferCombineDraw(unsigned int buffer);
	void VoxelVisualize();
	void DebugWindowDraw(unsigned int texture);
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
	friend void SVOGIDebugMenu(Renderer& renderer);
};

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
#include <iomanip>
#include <json.hpp>
#include <DirectionalLight.h>
#include <PointLight.h>

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

	std::vector<Model> modelList;

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
	unsigned int PointDepthCubeFBO;
	unsigned int DirectionalDepthFBO;
	unsigned int PointDepthCubeMap;
	unsigned int DirectionalDepthMap;
	glm::mat4 PointShadowProj;
	glm::mat4 DirectionalShadowProj;
	glm::mat4 DirectionalLightSpaceMatrix;
	Shader PointDepthShader;
	Shader DirectionalDepthShader;

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
	unsigned int DynamicAlbedo3D;
	unsigned int DynamicNormal3D;
	unsigned int Radiance3D;
	glm::vec3 MaxVec;
	glm::vec3 MinVec;
	float MaxCoord;
	unsigned int vLevel;
	unsigned int workgroupsize;
	int VoxelSize;
	glm::mat4 VoxelProjectMat;
	Shader VoxelizeShader;
	Shader LightInjectionShader;
	Shader MipmapShader;
	Shader ConeTracingShader;

	float GI_SpecularOffsetFactor;
	float GI_SpecularAperture;
	float GI_SpecularMaxT;

	float GI_DiffuseOffsetFactor;
	float GI_DiffuseAperture;
	float GI_DiffuseConeAngleMix;
	float GI_DiffuseMaxT;

	float GI_OcclusionOffsetFactor;
	float GI_OcculsionAperture;
	float GI_DirectionalMaxT;

	float GI_stepSize;

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
	std::vector<std::string> DebugViewsName = {"gPosition", "gNormal", "gAlbedoSpec", "gTangent", "ShadowRaw", "ShadowBlur", "ShadowOut", 
				"ds_gPosition", "ds_gNormal", "ds_gAlbedoSpec", "ds_gTangent", "ds_gMSAA", "ds_gViewPos", "ssaoOut", 
				"PostProcessingOut", "PointDepthCubeMap", "DirectionalDepthMap", "SkyBoxOut", "VoxelVisOut"};
	std::vector<unsigned int> DebugViewsID;
	std::vector<std::string> VoxelDebugViewsName = { "Albedo3D", "Normal3D", "DynamicAlbedo3D", "DynamicNormal3D", "Radiance3D"};
	std::vector<unsigned int> VoxelDebugViewsID;
	unsigned int CurrentDebugView;
	unsigned int VoxelCurrentDebugView;

	// misc
	unsigned int quadVAO;
	unsigned int DEBUGquadVAO;
	unsigned int SkyBoxVAO;

	void initializeBuffers();
	void loadModel();
	void addModel();
	void loadHDRI(bool loaded);
	void voxelizeStatic();
	void voxelizeDynamic();
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
	void VoxelVisualize(unsigned int voxel);
	void DebugWindowDraw(unsigned int texture);
	void LoadShaders();
	void Draw();
	void ModelListDraw(Shader shader);
	void SaveInit();

public:
	static Camera cam;
	PointLight myPointLight;
	DirectionalLight myDirectionalLight;

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
	friend void ModelMenu(Renderer& renderer);
	friend void DebugMenu(Renderer &renderer);
	friend void SVOGIDebugMenu(Renderer& renderer);
};

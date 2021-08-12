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


class Renderer {
public:
	// 0 : Need to confirm setting
	// 1 : Need to do pre-calculation
	// 2 : Running. Need to draw next frame
	unsigned int state;

	// Camera 
	glm::vec3 camPos;

	// Light
	Light myLight;

	// settings
	bool MSAA;
	unsigned int MSAASample;
	bool PCSS;
	bool PeterPan;
	bool SSAO;
	bool HDR;
	bool SkyBox;
	bool Debug;
	unsigned int renderWidth;
	unsigned int renderHeight;
	unsigned int shadowWidth;
	unsigned int shadowHeight;

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
	Shader gBufferGeoPass;
	Shader gBufferLightPass;

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
	Shader SkyboxShader;

	// misc
	unsigned int quadVAO;
	unsigned int DEBUGquadVAO;
	unsigned int SkyBoxVAO;

	void initializeBuffers();
	void loadModel();
	void LoadShaders();
	void Draw();

// public:
	Renderer() {};
	Renderer(unsigned int width, unsigned int height);
	~Renderer();
	void run();
	unsigned int getState();
	
	void update(glm::mat4 view, glm::mat4 proj, glm::vec3 camP, unsigned int width, unsigned int height);
	void setWidthHeight(unsigned int width, unsigned int height);
	friend void InitRendererMenu(Renderer* renderer);
};

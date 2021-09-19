#include <Renderer.h>


Renderer::Renderer(unsigned int width, unsigned int height) {
	state = 0;

	myPointLight = PointLight();
	myDirectionalLight = DirectionalLight();

	using json = nlohmann::json;
	std::ifstream inputfile("./json/RendererInitSetting.json");
	json RIsetting;
	inputfile >> RIsetting;

	modelPath = RIsetting["modelPath"];
	HDRIPath = RIsetting["HDRIPath"];

	MSAA = RIsetting["MSAA"];
	MSAASample = RIsetting["MSAASample"];
	PCSS = RIsetting["PCSS"];
	ShadowBluring = false;
	PeterPan = false;
	SSAO = RIsetting["SSAO"];
	HDR = false;
	SkyBox = RIsetting["SkyBox"];
	ShowDebug = false;
	GLDebugOutput = RIsetting["GLDebugOutput"];
	ShowTexture = true;
	ShowNormal = true;
	renderWidth = width;
	renderHeight = height;
	
	shadowWidth = 2048;
	shadowHeight = 2048;

	SVOGI = RIsetting["SVOGI"];

	vLevel = 8;
	VoxelSize = pow(2, 8);
	workgroupsize = VoxelSize / 8;

	HDRIwidth = 1024;

	DebugWindowSize = -1.0;

	GI_OcclusionOffsetFactor = 3.0;
	GI_DiffuseOffsetFactor = 1.5;
	GI_SpecularOffsetFactor = 3.0;
	GI_SpecularAperture = 0.10;
	GI_DiffuseAperture = 1.04;
	GI_OcculsionAperture = 0.10;
	GI_stepSize = 0.3;
	GI_DiffuseConeAngleMix = 0.666;

	MaxCoord = 0.0;
}

Renderer::~Renderer() {
	// TODO
}

void Renderer::run() {
	if (state == 2) {
		updateMats();
		Draw();
	}
	else if (state == 1) {
		loadModel();
		LoadShaders();
		initializeBuffers();
		state = 2;
	}
	else {
		glViewport(0, 0, renderWidth, renderHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		RendererInitSetting(*this);
	}
}

void Renderer::initializeBuffers() {
	if (GLDebugOutput) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(MessageCallback, 0);
	}
	// prim
	quadVAO = initQuad(-1.0, 0.7);
	SkyBoxVAO = initSkybox();

	// G Buffer
	glEnable(GL_MULTISAMPLE);
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	unsigned int attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, 
									GL_COLOR_ATTACHMENT4};
	glDrawBuffers(5, attachments);

	gPosition = bindColorBuffer(gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT0, GL_RGBA32F, MSAASample, GL_LINEAR);
	gNormal = bindColorBuffer(gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT1, GL_RGBA32F, MSAASample, GL_LINEAR);
	gAlbedoSpec = bindColorBuffer(gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT2, GL_RGBA32F, MSAASample, GL_LINEAR);
	gTangent = bindColorBuffer(gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT3, GL_RGBA32F, MSAASample, GL_LINEAR);
	attachRBOToBuffer(gBuffer, renderWidth, renderHeight, GL_DEPTH_COMPONENT32, GL_DEPTH_ATTACHMENT, MSAASample);

	// Shadow FBO
	glGenFramebuffers(1, &ShadowBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ShadowBuffer);
	ShadowRaw = bindColorBuffer(ShadowBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT0, GL_R8, MSAASample, GL_LINEAR);
	ShadowBlur = bindColorBuffer(ShadowBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT1, GL_R8, MSAASample, GL_LINEAR);

	// Down sampled G Buffer
	glGenFramebuffers(1, &ds_gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ds_gBuffer);
	unsigned int ds_attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, 
										GL_COLOR_ATTACHMENT4};
	glDrawBuffers(4, ds_attachments);
	if (MSAA) {
		ds_gPosition = bindColorBuffer(ds_gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT0, GL_RGBA32F, 1, GL_LINEAR);
		ds_gNormal = bindColorBuffer(ds_gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT1, GL_RGBA32F, 1, GL_LINEAR);
		ds_gAlbedoSpec = bindColorBuffer(ds_gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT2, GL_RGBA32F, 1, GL_LINEAR);
		ds_gTangent = bindColorBuffer(ds_gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT3, GL_RGBA32F, 1, GL_LINEAR);
		ds_gMSAA = bindColorBuffer(ds_gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT4, GL_R8, 1, GL_LINEAR);
	}
	else {
		ds_gPosition = gPosition;
		ds_gNormal = gNormal;
		ds_gAlbedoSpec = gAlbedoSpec;
		ds_gTangent = gTangent;
	}
	

	// SSAO
	if (SSAO) {
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
		std::default_random_engine generator;
		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator)
			);
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			float scale = (float)i / 64.0;
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			ssaoKernel.push_back(sample);
		}

		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		glGenTextures(1, &NoiseTexture);
		glBindTexture(GL_TEXTURE_2D, NoiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


		glGenFramebuffers(1, &ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		ssaoOut = bindColorBuffer(ssaoFBO, renderWidth, renderHeight, GL_COLOR_ATTACHMENT0, GL_RED, 1, GL_LINEAR);
		ds_gViewPos = bindColorBuffer(ds_gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT5, GL_RGBA32F, 1, GL_LINEAR);
	}

	// Post Processing Buffer
	if (SSAO | SkyBox) {
		glGenFramebuffers(1, &PostProcessingFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, PostProcessingFBO);
		PostProcessingOut = bindColorBuffer(PostProcessingFBO, renderWidth, renderHeight, GL_COLOR_ATTACHMENT0, GL_RED, 1, GL_LINEAR);
	}

	// DepthCube / Cube Shadow
	{
		if (SVOGI) {
			shadowWidth = 2 * VoxelSize;
			shadowHeight = 2 * VoxelSize;
		}
		else {
			shadowWidth = 2048;
			shadowHeight = 2048;
		}
		glGenFramebuffers(1, &PointDepthCubeFBO);
		PointDepthCubeMap = bindCubeDepthMap(PointDepthCubeFBO, shadowWidth, shadowHeight);
		PointShadowProj = glm::perspective(glm::radians(90.0), (double)shadowWidth / (double)shadowHeight, (double)cam.near_plane, (double)cam.far_plane);
		glGenFramebuffers(1, &DirectionalDepthFBO);
		DirectionalDepthMap = bindDepthMap(DirectionalDepthFBO, 2048, 2048);
		float coord = 1.5 * MaxCoord;
		DirectionalShadowProj = glm::ortho(-coord, coord, -coord, coord, 0.01f, 2 * coord);
	}

	// HDRI
	if (SkyBox) {
		loadHDRI(false);
	}

	if (SVOGI) {
		glGenFramebuffers(1, &VoxelVisFBO);
		VoxelVisFrontFace = bindColorBuffer(VoxelVisFBO, renderWidth, renderHeight, GL_COLOR_ATTACHMENT0, GL_RGBA32F, 1, GL_LINEAR);
		VoxelVisBackFace = bindColorBuffer(VoxelVisFBO, renderWidth, renderHeight, GL_COLOR_ATTACHMENT1, GL_RGBA32F, 1, GL_LINEAR);
		VoxelVisOut = bindColorBuffer(VoxelVisFBO, renderWidth, renderHeight, GL_COLOR_ATTACHMENT2, GL_RGBA32F, 1, GL_LINEAR);

		glGenFramebuffers(1, &SVOGIFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, SVOGIFBO);

		glGenTextures(1, &Albedo3D);
		glBindTexture(GL_TEXTURE_3D, Albedo3D);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, VoxelSize, VoxelSize, VoxelSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, VoxelSize, VoxelSize, VoxelSize);
		glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, Albedo3D, 0, 0);

		glGenTextures(1, &Normal3D);
		glBindTexture(GL_TEXTURE_3D, Normal3D);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, VoxelSize, VoxelSize, VoxelSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, VoxelSize, VoxelSize, VoxelSize);
		glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_3D, Normal3D, 0, 0);

		glGenTextures(1, &DynamicAlbedo3D);
		glBindTexture(GL_TEXTURE_3D, DynamicAlbedo3D);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, VoxelSize, VoxelSize, VoxelSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, VoxelSize, VoxelSize, VoxelSize);
		glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_3D, DynamicAlbedo3D, 0, 0);

		glGenTextures(1, &DynamicNormal3D);
		glBindTexture(GL_TEXTURE_3D, DynamicNormal3D);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, VoxelSize, VoxelSize, VoxelSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, VoxelSize, VoxelSize, VoxelSize);
		glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_3D, DynamicNormal3D, 0, 0);

		glGenTextures(1, &Radiance3D);
		glBindTexture(GL_TEXTURE_3D, Radiance3D);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, VoxelSize, VoxelSize, VoxelSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexStorage3D(GL_TEXTURE_3D, vLevel, GL_RGBA8, VoxelSize, VoxelSize, VoxelSize);
		glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_3D, Radiance3D, 0, 0);

		voxelizeStatic();
	}
}


void Renderer::Draw() {
	gBufferDraw();

	if (SSAO) {
		SSAODraw();
		BlurDraw(ssaoOut);
	}

	if (SkyBox) {
		SkyBoxDraw();
	}

	if (PCSS || SVOGI) {
		ShadowMapDraw();
	}

	if (SVOGI) {
		voxelizeDynamic();
		LightInjection();
		MipmapBuild(6);
		ConeTrace(0);
	}
	
	if (PCSS) {
		gBufferLightingPassDraw();
		gBufferCombineDraw(0);
	}

	if (SVOGI && ShowDebug) {
		VoxelVisualize();
	}

	if (ShowDebug) {
		DebugWindowDraw(VoxelVisOut);
	}
}


void Renderer::loadModel() {
	// Model new_model = Model(modelPath.c_str());
	modelList.clear();
	modelList.emplace_back(modelPath.c_str());
	MaxCoord = modelList[0].max_pos + 1.0;
	if (SVOGI) {
		modelList[0].isStatic = true;
		modelList[0].scale = 50.0 / modelList[0].max_pos;
		MaxCoord = 50.0;
		glm::mat4 SVOGIproj = glm::ortho(-MaxCoord, MaxCoord, -MaxCoord, MaxCoord, 0.1f, 2.0f * MaxCoord + 0.1f);
		VoxelProjectMat = SVOGIproj * glm::lookAt(glm::vec3(0.0f, 0.0f, MaxCoord + 0.1f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	
}


void Renderer::addModel() {
	// Model new_model = Model(modelPath.c_str());
	modelList.emplace_back(modelPath.c_str());
}


void Renderer::ModelListDraw(Shader shader) {
	for (int i = 0; i < modelList.size(); i++)
	{
		glm::mat4 modelMat = modelList[i].getModelMatrix();
		shader.setMat4("model", glm::value_ptr(modelMat), false);
		modelList[i].Draw(shader, ShowTexture, ShowNormal);
	}
}


void Renderer::gBufferDraw() {
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glViewport(0, 0, renderWidth, renderHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// G Buffer Geometry Pass
	glViewport(0, 0, renderWidth, renderHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gBufferGeoPass.use();
	gBufferGeoPass.setMat4("proj", glm::value_ptr(proj_mat), false);
	gBufferGeoPass.setMat4("view", glm::value_ptr(view_mat), false);
	gBufferGeoPass.setInt("MSAA_Sample", MSAASample);
	ModelListDraw(gBufferGeoPass);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	// Down sampled G Buffer
	if (MSAA) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ds_gBuffer);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, renderWidth, renderHeight, 0, 0, renderWidth, renderHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, renderWidth, renderHeight, 0, 0, renderWidth, renderHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glDrawBuffer(GL_COLOR_ATTACHMENT2);
		glReadBuffer(GL_COLOR_ATTACHMENT2);
		glBlitFramebuffer(0, 0, renderWidth, renderHeight, 0, 0, renderWidth, renderHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glDrawBuffer(GL_COLOR_ATTACHMENT3);
		glReadBuffer(GL_COLOR_ATTACHMENT3);
		glBlitFramebuffer(0, 0, renderWidth, renderHeight, 0, 0, renderWidth, renderHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		glDrawBuffer(GL_COLOR_ATTACHMENT4);
		MSAADetectShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gNormal);
		MSAADetectShader.setInt("gNormal", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ds_gNormal);
		MSAADetectShader.setInt("ds_gNormal", 1);
		renderQuad(quadVAO);
	}
}


void Renderer::SSAODraw() {
	DownSampleShader.use();
	glBindFramebuffer(GL_FRAMEBUFFER, ds_gBuffer);
	glDrawBuffer(GL_COLOR_ATTACHMENT3);
	DownSampleShader.setMat4("view", glm::value_ptr(view_mat), false);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ds_gPosition);
	DownSampleShader.setInt("ds_gPosition", 0);
	renderQuad(quadVAO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// SSAO
	glViewport(0, 0, renderWidth, renderHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	SSAO_Shader.use();
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (unsigned int i = 0; i < 64; i++) {
		SSAO_Shader.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ds_gPosition);
	SSAO_Shader.setInt("gPosition", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ds_gNormal);
	SSAO_Shader.setInt("gNormal", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, NoiseTexture);
	SSAO_Shader.setInt("texNoise", 2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, ds_gViewPos);
	SSAO_Shader.setInt("gViewPos", 3);
	SSAO_Shader.setMat4("proj", glm::value_ptr(proj_mat), false);
	SSAO_Shader.setMat4("view", glm::value_ptr(view_mat), false);
	renderQuad(quadVAO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Renderer::BlurDraw(unsigned int texture) {
	glViewport(0, 0, renderWidth, renderHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, PostProcessingFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	BlurShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	BlurShader.setInt("Input", 0);
	BlurShader.setInt("sampleSize", 2);
	glEnable(GL_DEPTH_TEST);
	renderQuad(quadVAO);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Renderer::SkyBoxDraw() {
	glViewport(0, 0, renderWidth, renderHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, PostProcessingFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	SkyboxShader.use();
	SkyboxShader.setMat4("proj", glm::value_ptr(proj_mat), false);
	SkyboxShader.setMat4("view", glm::value_ptr(view_mat), false);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, SkyBoxCubeMap);
	renderSkybox(SkyBoxVAO);
}


void Renderer::ShadowMapDraw() {
	glm::vec3 lightpos = myPointLight.position;
	std::vector<glm::mat4> shadowTransforms;
	shadowTransforms.push_back(PointShadowProj * glm::lookAt(lightpos, lightpos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(PointShadowProj * glm::lookAt(lightpos, lightpos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(PointShadowProj * glm::lookAt(lightpos, lightpos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	shadowTransforms.push_back(PointShadowProj * glm::lookAt(lightpos, lightpos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	shadowTransforms.push_back(PointShadowProj * glm::lookAt(lightpos, lightpos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(PointShadowProj * glm::lookAt(lightpos, lightpos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

	glViewport(0, 0, shadowWidth, shadowHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, PointDepthCubeFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	PointDepthShader.use();
	PointDepthShader.setVec3("lightPos", glm::value_ptr(lightpos));
	for (unsigned int i = 0; i < 6; ++i)
		PointDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
	PointDepthShader.setFloat("far_plane", cam.far_plane);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	if (PeterPan) {
		glCullFace(GL_FRONT);
		ModelListDraw(PointDepthShader);
		glCullFace(GL_BACK);
	}
	else {
		ModelListDraw(PointDepthShader);
	}
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// Directional
	glViewport(0, 0, 2048, 2048);
	glBindFramebuffer(GL_FRAMEBUFFER, DirectionalDepthFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glm::vec3 DPos = 1.5f * MaxCoord * -glm::normalize(myDirectionalLight.normal);
	glm::vec3 up = glm::vec3(1.0f, 0.0f, 0.0f);

	myDirectionalLight.position = DPos;
	glm::mat4 DView = glm::lookAt(DPos, glm::vec3(0.0f, 0.0f, 0.0f), up);
	DirectionalLightSpaceMatrix = DirectionalShadowProj * DView;
	DirectionalDepthShader.use();
	DirectionalDepthShader.setMat4("lightSpaceMatrix", glm::value_ptr(DirectionalLightSpaceMatrix), false);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	if (PeterPan) {
		glCullFace(GL_FRONT);
		ModelListDraw(DirectionalDepthShader);
		glCullFace(GL_BACK);
	}
	else {
		ModelListDraw(DirectionalDepthShader);
	}
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}


void Renderer::LightInjection() {
	glBindFramebuffer(GL_FRAMEBUFFER, SVOGIFBO);
	float zero[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glClearTexImage(Radiance3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);
	LightInjectionShader.use();
	LightInjectionShader.setVec3("PointLightPos", glm::value_ptr(myPointLight.position));
	LightInjectionShader.setFloat("PointLightStrength", myPointLight.strength);
	LightInjectionShader.setMat4("lightSpaceMatrix", glm::value_ptr(DirectionalLightSpaceMatrix), false);
	LightInjectionShader.setFloat("DirectionalLightStrength", myDirectionalLight.strength);
	LightInjectionShader.setInt("VoxelSize", VoxelSize);
	LightInjectionShader.setFloat("far_plane", cam.far_plane);
	LightInjectionShader.setFloat("MaxCoord", MaxCoord);
	
	glBindImageTexture(0, Radiance3D, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, Albedo3D);
	LightInjectionShader.setInt("Albedo3D", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, Normal3D);
	LightInjectionShader.setInt("Normal3D", 2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, DynamicAlbedo3D);
	LightInjectionShader.setInt("DynamicAlbedo3D", 3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_3D, DynamicNormal3D);
	LightInjectionShader.setInt("DynamicNormal3D", 4);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, PointDepthCubeMap);
	LightInjectionShader.setInt("PointDepthMap", 5);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, DirectionalDepthMap);
	LightInjectionShader.setInt("DirectionalDepthMap", 6);

	glDispatchCompute(workgroupsize, workgroupsize, workgroupsize);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Renderer::MipmapBuild(unsigned int level) {
	glBindFramebuffer(GL_FRAMEBUFFER, SVOGIFBO);
	MipmapShader.use();
	for (int mipLevel = 1; mipLevel < level; mipLevel++) {
		glBindImageTexture(0, Radiance3D, mipLevel, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
		glBindImageTexture(1, Radiance3D, mipLevel - 1, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
		int div = pow(2, mipLevel);
		glDispatchCompute(workgroupsize / div, workgroupsize / div, workgroupsize / div);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Renderer::ConeTrace(unsigned int buffer) {
	glBindFramebuffer(GL_FRAMEBUFFER, buffer);
	glViewport(0, 0, renderWidth, renderHeight);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	ConeTracingShader.use();
	glm::vec3 temp_campos = cam.getPosition();
	glm::vec3 temp_lightpos = myPointLight.position;
	glm::vec3 temp_dir = -glm::normalize(myDirectionalLight.normal);
	ConeTracingShader.setVec3("PointLightPos", glm::value_ptr(temp_lightpos));
	ConeTracingShader.setFloat("PointLightStrength", myPointLight.strength);
	ConeTracingShader.setVec3("DirectionalLightDirection", glm::value_ptr(temp_dir));
	ConeTracingShader.setFloat("DirectionalLightStrength", myDirectionalLight.strength);
	ConeTracingShader.setVec3("viewPos", glm::value_ptr(temp_campos));
	if (MSAA) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gPosition);
		ConeTracingShader.setInt("gPosition", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gNormal);
		ConeTracingShader.setInt("gNormal", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gAlbedoSpec);
		ConeTracingShader.setInt("gAlbedoSpec", 2);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gTangent);
		ConeTracingShader.setInt("gTangent", 4);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, ds_gMSAA);
		ConeTracingShader.setInt("ds_gNormal", 7);
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		ConeTracingShader.setInt("gPosition", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		ConeTracingShader.setInt("gNormal", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		ConeTracingShader.setInt("gAlbedoSpec", 2);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, gTangent);
		ConeTracingShader.setInt("gTangent", 4);
		
	}
	

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_3D, Radiance3D);
	ConeTracingShader.setInt("Radiance3D", 5);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, SkyBoxOut);
	ConeTracingShader.setInt("skybox", 6);

	ConeTracingShader.setFloat("MaxCoord", MaxCoord);
	ConeTracingShader.setInt("VoxelSize", VoxelSize);
	ConeTracingShader.setMat4("ProjectMat", VoxelProjectMat);
	ConeTracingShader.setFloat("OcclusionOffsetFactor", GI_OcclusionOffsetFactor);
	ConeTracingShader.setFloat("DiffuseOffsetFactor", GI_DiffuseOffsetFactor);
	ConeTracingShader.setFloat("SpecularOffsetFactor", GI_SpecularOffsetFactor);
	ConeTracingShader.setFloat("SpecularAperture", GI_SpecularAperture);
	ConeTracingShader.setFloat("DiffuseAperture", GI_DiffuseAperture);
	ConeTracingShader.setFloat("OcculsionAperture", GI_OcculsionAperture);
	ConeTracingShader.setFloat("DiffuseConeAngleMix", GI_DiffuseConeAngleMix);
	ConeTracingShader.setFloat("stepSize", GI_stepSize);
	float VoxelCellSize = MaxCoord * 2.0 / VoxelSize;
	ConeTracingShader.setFloat("VoxelCellSize", VoxelCellSize);
	glEnable(GL_FRAMEBUFFER_SRGB);
	renderQuad(quadVAO);
	glDisable(GL_FRAMEBUFFER_SRGB);
}


void Renderer::voxelizeStatic() {
	float zero[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glClearTexImage(Albedo3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);
	glClearTexImage(Normal3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);
	glBindFramebuffer(GL_FRAMEBUFFER, SVOGIFBO);
	std::cout << "STATIC max coord = " << MaxCoord << std::endl;
	VoxelizeShader.use();
	VoxelizeShader.setMat4("ProjectMat", VoxelProjectMat);
	VoxelizeShader.setInt("VoxelSize", VoxelSize);
	glBindImageTexture(0, Albedo3D, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(1, Normal3D, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glViewport(0, 0, VoxelSize, VoxelSize);
	for (int i = 0; i < modelList.size(); i++)
	{
		if (modelList[i].isStatic) {
			glm::mat4 modelMat = modelList[i].getModelMatrix();
			VoxelizeShader.setMat4("model", glm::value_ptr(modelMat), false);
			modelList[i].Draw(VoxelizeShader, ShowTexture, ShowNormal);
		}
	}
	glBindTexture(GL_TEXTURE_3D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Renderer::voxelizeDynamic() {
	float zero[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glClearTexImage(DynamicAlbedo3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);
	glClearTexImage(DynamicNormal3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);
	glBindFramebuffer(GL_FRAMEBUFFER, SVOGIFBO);
	VoxelizeShader.use();
	VoxelizeShader.setMat4("ProjectMat", VoxelProjectMat);
	VoxelizeShader.setInt("VoxelSize", VoxelSize);
	glBindImageTexture(0, DynamicAlbedo3D, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(1, DynamicNormal3D, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glViewport(0, 0, VoxelSize, VoxelSize);
	for (int i = 0; i < modelList.size(); i++)
	{
		if (!modelList[i].isStatic) {
			glm::mat4 modelMat = modelList[i].getModelMatrix();
			VoxelizeShader.setMat4("model", glm::value_ptr(modelMat), false);
			modelList[i].Draw(VoxelizeShader, ShowTexture, ShowNormal);
		}
	}
	glBindTexture(GL_TEXTURE_3D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void Renderer::gBufferLightingPassDraw() {
	glViewport(0, 0, renderWidth, renderHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, ShadowBuffer);
	gBufferLightPass.use();
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	if (MSAA) {
		glBindImageTexture(0, ShadowRaw, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R8);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gPosition);
		gBufferLightPass.setInt("gPosition", 0);
		gBufferLightPass.setInt("MSAA_Sample", 4);
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		gBufferLightPass.setInt("gPosition", 0);
	}
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, PointDepthCubeMap);
	gBufferLightPass.setInt("depthCubemap", 3);
	glm::vec3 temp_lightpos = myPointLight.position;
	gBufferLightPass.setVec3("lightPos", glm::value_ptr(temp_lightpos));
	gBufferLightPass.setFloat("far_plane", cam.far_plane);
	renderQuad(quadVAO);

	if (ShadowBluring && MSAA) {
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		BlurShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ShadowRaw);
		BlurShader.setInt("Input", 0);
		BlurShader.setInt("sampleSize", 2);
		renderQuad(quadVAO);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		ShadowOut = ShadowBlur;
	}
	else {
		ShadowOut = ShadowRaw;
	}
}


void Renderer::gBufferCombineDraw(unsigned int buffer) {
	glViewport(0, 0, renderWidth, renderHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, buffer);
	gBufferCombine.use();
	if (MSAA) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gPosition);
		gBufferCombine.setInt("gPosition", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gNormal);
		gBufferCombine.setInt("gNormal", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gAlbedoSpec);
		gBufferCombine.setInt("gAlbedoSpec", 2);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ShadowOut);
		gBufferCombine.setInt("Shadow", 3);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, ds_gMSAA);
		gBufferCombine.setInt("ds_gMSAA", 6);
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		gBufferCombine.setInt("gPosition", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		gBufferCombine.setInt("gNormal", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		gBufferCombine.setInt("gAlbedoSpec", 2);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, ShadowOut);
		gBufferCombine.setInt("Shadow", 3);
	}

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, PostProcessingOut);
	gBufferCombine.setInt("SSAO", 4);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, SkyBoxOut);
	gBufferCombine.setInt("skybox", 5);
	glm::vec3 temp_campos = cam.getPosition();
	glm::vec3 temp_lightpos = myPointLight.position;
	gBufferCombine.setVec3("PointLightPos", glm::value_ptr(temp_lightpos));
	gBufferCombine.setVec3("viewPos", glm::value_ptr(temp_campos));
	gBufferCombine.setFloat("PointLightStrength", myPointLight.strength);
	gBufferCombine.setBool("HDR", HDR);
	gBufferCombine.setInt("MSAA_Sample", MSAASample);
	glEnable(GL_FRAMEBUFFER_SRGB);
	renderQuad(quadVAO);
	glDisable(GL_FRAMEBUFFER_SRGB);
}


void Renderer::VoxelVisualize() {
	glViewport(0, 0, renderWidth, renderHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, VoxelVisFBO);
	VoxelVisFaceShader.use();
	VoxelVisFaceShader.setMat4("proj", glm::value_ptr(proj_mat), false);
	VoxelVisFaceShader.setMat4("view", glm::value_ptr(view_mat), false);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glCullFace(GL_BACK);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	renderSkybox(SkyBoxVAO);

	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glCullFace(GL_FRONT);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	renderSkybox(SkyBoxVAO);
	glCullFace(GL_BACK);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	VoxelVisTraceShader.use();
	glDrawBuffer(GL_COLOR_ATTACHMENT2);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, VoxelVisBackFace);
	VoxelVisTraceShader.setInt("textureBack", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, VoxelVisFrontFace);
	VoxelVisTraceShader.setInt("textureFront", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, Radiance3D);
	VoxelVisTraceShader.setInt("tex3D", 2);
	glm::vec3 temp_campos = cam.getPosition();
	VoxelVisTraceShader.setVec3("cameraPosition", glm::value_ptr(temp_campos));
	renderQuad(quadVAO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Renderer::DebugWindowDraw(unsigned int texture) {
	DEBUGquadVAO = initQuad(DebugWindowSize, 0.5);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	DebugShader.use();
	DebugShader.setInt("debug", 0);
	DebugShader.setFloat("near_plane", cam.near_plane);
	DebugShader.setFloat("far_plane", cam.far_plane);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	renderQuad(DEBUGquadVAO);
}


void Renderer::loadHDRI(bool loaded) {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	if (loaded) {
		unsigned int hdri_2DTexture = loadHDRIPath(HDRIPath.c_str());
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0), 1.0, 0.1, 10.0);
		glm::mat4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};
		glBindFramebuffer(GL_FRAMEBUFFER, HdriFBO);
		HdriConvert.use();
		HdriConvert.setInt("equirectangularMap", 0);
		HdriConvert.setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdri_2DTexture);
		glViewport(0, 0, HDRIwidth, HDRIwidth);

		for (unsigned int i = 0; i < 6; ++i)
		{
			HdriConvert.setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, SkyBoxCubeMap, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			renderSkybox(SkyBoxVAO);
		}
		glViewport(0, 0, renderWidth, renderHeight);
	}
	else {
		unsigned int hdri_2DTexture = loadHDRIPath(HDRIPath.c_str());
		glGenFramebuffers(1, &HdriFBO);
		SkyBoxCubeMap = genCubeMap(HDRIwidth, HDRIwidth, GL_RGB32F, GL_RGB);
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0), 1.0, 0.1, 10.0);
		glm::mat4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};
		glBindFramebuffer(GL_FRAMEBUFFER, HdriFBO);
		HdriConvert.use();
		HdriConvert.setInt("equirectangularMap", 0);
		HdriConvert.setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdri_2DTexture);
		glViewport(0, 0, HDRIwidth, HDRIwidth);

		for (unsigned int i = 0; i < 6; ++i)
		{
			HdriConvert.setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, SkyBoxCubeMap, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			renderSkybox(SkyBoxVAO);
		}
		glViewport(0, 0, renderWidth, renderHeight);
		SkyBoxOut = bindColorBuffer(PostProcessingFBO, renderWidth, renderHeight, GL_COLOR_ATTACHMENT1, GL_RGBA32F, 1, GL_LINEAR);
		SkyBox = true;
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}


void Renderer::LoadShaders() {
	// G Buffer
	gBufferGeoPass = Shader("./shader/gBuffer.vert", "./shader/gBuffer.frag");
	//gBufferCombine = Shader("./shader/gBuffer.vert", "./shader/Combine.frag");
	if (MSAA) {
		gBufferCombine = Shader("./shader/deferred_shading.vert", "./shader/MSCombine.frag");
		MSAADetectShader = Shader("./shader/deferred_shading.vert", "./shader/MSAADetect.frag");
	}
	else {
		gBufferCombine = Shader("./shader/deferred_shading.vert", "./shader/Combine.frag");
	}

	// Down Sample
	if (SSAO) {
		DownSampleShader = Shader("./shader/deferred_shading.vert", "./shader/downSample.frag");
	}

	// SSAO
	if (SSAO) {
		SSAO_Shader = Shader("./shader/deferred_shading.vert", "./shader/SSAO.frag");
	}

	// Post Processing Buffer
	if (SSAO) {
		BlurShader = Shader("./shader/deferred_shading.vert", "./shader/Blur.frag");
	}

	// PCSS
	if (PCSS || SVOGI) {
		PointDepthShader = Shader("./shader/cubeDepth.vert", "./shader/cubeDepth.frag", "./shader/cubeDepth.geom");
		DirectionalDepthShader = Shader("./shader/directionalDepth.vert", "./shader/empty.frag");
		if (MSAA) {
			gBufferLightPass = Shader("./shader/deferred_shading.vert", "./shader/MSCubePCSS.frag");
		}
		else {
			gBufferLightPass = Shader("./shader/deferred_shading.vert", "./shader/CubePCSS.frag");
		}
	}
	if (SVOGI) {
		VoxelizeShader = Shader("./shader/voxelize.vert", "./shader/voxelize.frag", "./shader/voxelize.geom");
		VoxelVisFaceShader = Shader("./shader/VoxelVisFace.vert", "./shader/VoxelVisFace.frag");
		VoxelVisTraceShader = Shader("./shader/VoxelVisTrace.vert", "./shader/VoxelVisTrace.frag");
		LightInjectionShader = Shader(nullptr, nullptr, nullptr, "./shader/lightInjection.comp");
		MipmapShader = Shader(nullptr, nullptr, nullptr, "./shader/MipMap.comp");
		if (MSAA) {
			ConeTracingShader = Shader("./shader/deferred_shading.vert", "./shader/MSConeTracing.frag");
		}
		else {
			ConeTracingShader = Shader("./shader/deferred_shading.vert", "./shader/ConeTracing.frag");
		}
		
	}

	// HDRI
	
	SkyboxShader = Shader("./shader/skybox.vert", "./shader/skybox.frag");
	HdriConvert = Shader("./shader/hdriConvert.vert", "./shader/hdriConvert.frag");

	// Debug
	DebugShader = Shader("./shader/quad.vert", "./shader/debug.frag");
}


unsigned int Renderer::getState() {
	return state;
}


void Renderer::setWidthHeight(unsigned int width, unsigned int height) {
	renderWidth = width;
	renderHeight = height;
}


void Renderer::updateMats() {
	proj_mat = cam.getProjMat();
	view_mat = cam.getViewMat();
}


void Renderer::SaveInit() {
	std::ofstream outputfile("./json/RendererInitSetting.json");
	using json = nlohmann::json;
	json RIsetting;
	RIsetting["MSAA"] = MSAA;
	RIsetting["MSAASample"] = MSAASample;
	RIsetting["SVOGI"] = SVOGI;
	RIsetting["PCSS"] = PCSS;
	RIsetting["SSAO"] = SSAO;
	RIsetting["SkyBox"] = SkyBox;
	RIsetting["GLDebugOutput"] = GLDebugOutput;
	RIsetting["modelPath"] = modelPath;
	RIsetting["HDRIPath"] = HDRIPath;
	outputfile << std::setw(4) << RIsetting << std::endl;
}


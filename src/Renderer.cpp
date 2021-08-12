#include <Renderer.h>

Renderer::Renderer(unsigned int width, unsigned int height) {
	state = 0;
	myLight = Light();

	//
	renderWidth = width;
	renderHeight = height; 

	//
	// modelPath = "./model/sponza/sponza.obj";
	modelPath = "./model/opengl_render_testing.obj";
	HDRIPath = "./hdri/purple_sky.hdr";

	MSAA = true;
	MSAASample = 4;
	PCSS = true;
	SSAO = true;
	SkyBox = true;
	Debug = false;
	shadowWidth = 2048;
	shadowHeight = 2048;

	HDRIwidth = 1024;


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
		RendererInitSetting(this);
	}
}

void Renderer::initializeBuffers() {
	// prim
	quadVAO = initQuad(-1.0, 0.7);
	DEBUGquadVAO = initQuad(-1.0, 0.5);
	SkyBoxVAO = initSkybox();

	// G Buffer
	glEnable(GL_MULTISAMPLE);
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	gPosition = bindColorBuffer(gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT0, GL_RGBA32F, MSAASample, GL_LINEAR);
	gNormal = bindColorBuffer(gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT1, GL_RGBA32F, MSAASample, GL_LINEAR);
	gAlbedoSpec = bindColorBuffer(gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT2, GL_RGBA32F, MSAASample, GL_LINEAR);
	attachRBOToBuffer(gBuffer, renderWidth, renderHeight, GL_DEPTH_COMPONENT32, GL_DEPTH_ATTACHMENT, MSAASample);
	// Down sampled G Buffer
	if (MSAA) {
		glGenFramebuffers(1, &ds_gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, ds_gBuffer);
		unsigned int ds_attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, ds_attachments);

		ds_gPosition = bindColorBuffer(ds_gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT0, GL_RGBA32F, 1, GL_LINEAR);
		ds_gNormal = bindColorBuffer(ds_gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT1, GL_RGBA32F, 1, GL_LINEAR);
		ds_gAlbedoSpec = bindColorBuffer(ds_gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT2, GL_RGBA32F, 1, GL_LINEAR);
	}
	else {
		ds_gPosition = gPosition;
		ds_gNormal = gNormal;
		ds_gAlbedoSpec = gAlbedoSpec;
	}
	ds_gViewPos = bindColorBuffer(ds_gBuffer, renderWidth, renderHeight, GL_COLOR_ATTACHMENT3, GL_RGBA32F, 1, GL_LINEAR);

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
	}

	// Post Processing Buffer
	if (SSAO | SkyBox) {
		glGenFramebuffers(1, &PostProcessingFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, PostProcessingFBO);
		PostProcessingOut = bindColorBuffer(PostProcessingFBO, renderWidth, renderHeight, GL_COLOR_ATTACHMENT0, GL_RGBA32F, 1, GL_LINEAR);
	}

	// DepthCube / Cube Shadow
	if (PCSS) {
		glGenFramebuffers(1, &DepthCubeFBO);
		DepthCubeMap = bindCubeDepthMap(DepthCubeFBO, shadowWidth, shadowHeight);
		SProj = glm::perspective(glm::radians(90.0), (double)shadowWidth / (double)shadowHeight, 0.1, 80.0);
	}

	// HDRI
	if (SkyBox) {
		loadHDRI(false);
	}
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
}


void Renderer::Draw() {
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
	myModel.Draw(gBufferGeoPass);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
	}
	// viewpos
	DownSampleShader.use();
	glDrawBuffer(GL_COLOR_ATTACHMENT3);
	DownSampleShader.setMat4("view", glm::value_ptr(view_mat), false);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ds_gPosition);
	DownSampleShader.setInt("ds_gPosition", 0);
	renderQuad(quadVAO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// SSAO
	if (SSAO) {
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

	// Post Processing
	if (SSAO) {
		glViewport(0, 0, renderWidth, renderHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, PostProcessingFBO);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		BlurShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoOut);
		BlurShader.setInt("ssaoInput", 0);
		BlurShader.setInt("sampleSize", 2);
		renderQuad(quadVAO);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// SkyBox
	if (SkyBox) {
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
		glDisable(GL_CULL_FACE);
		renderSkybox(SkyBoxVAO);
		glEnable(GL_CULL_FACE);
	}

	// PCSS
	if (PCSS) {
		std::vector<glm::mat4> shadowTransforms;
		shadowTransforms.push_back(SProj * glm::lookAt(myLight.position, myLight.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(SProj * glm::lookAt(myLight.position, myLight.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(SProj * glm::lookAt(myLight.position, myLight.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		shadowTransforms.push_back(SProj * glm::lookAt(myLight.position, myLight.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		shadowTransforms.push_back(SProj * glm::lookAt(myLight.position, myLight.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(SProj * glm::lookAt(myLight.position, myLight.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
		
		glViewport(0, 0, shadowWidth, shadowHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, DepthCubeFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		PCSSDepthShader.use();
		PCSSDepthShader.setVec3("lightPos", glm::value_ptr(myLight.position));
		for (unsigned int i = 0; i < 6; ++i)
			PCSSDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		PCSSDepthShader.setFloat("far_plane", 80.0);
		myModel.Draw(PCSSDepthShader);
	}

	// G Buffer Lighting Pass
	glViewport(0, 0, renderWidth, renderHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	gBufferLightPass.use();
	if (MSAA) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gPosition);
		gBufferLightPass.setInt("gPosition", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gNormal);
		gBufferLightPass.setInt("gNormal", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gAlbedoSpec);
		gBufferLightPass.setInt("gAlbedoSpec", 2);
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		gBufferLightPass.setInt("gPosition", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		gBufferLightPass.setInt("gNormal", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		gBufferLightPass.setInt("gAlbedoSpec", 2);
	}
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, DepthCubeMap);
	gBufferLightPass.setInt("depthCubemap", 3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, PostProcessingOut);
	gBufferLightPass.setInt("SSAO", 4);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, SkyBoxOut);
	gBufferLightPass.setInt("skybox", 5);
	glm::vec3 temp_pos = cam.getPosition();
	gBufferLightPass.setVec3("lightPos", glm::value_ptr(myLight.position));
	gBufferLightPass.setVec3("viewPos", glm::value_ptr(temp_pos));
	gBufferLightPass.setFloat("far_plane", 80.0);
	gBufferLightPass.setBool("HDR", HDR);
	gBufferLightPass.setInt("MSAA_Sample", MSAASample);
	glEnable(GL_FRAMEBUFFER_SRGB);
	renderQuad(quadVAO);
	glDisable(GL_FRAMEBUFFER_SRGB);
}


void Renderer::loadModel() {
	myModel = Model(modelPath.c_str());
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
	if (MSAA) {
		gBufferLightPass = Shader("./shader/deferred_shading.vert", "./shader/MSgCubePCSS.frag");
	}
	else {
		gBufferLightPass = Shader("./shader/deferred_shading.vert", "./shader/CubePCSS.frag");
	}

	// Down Sample
	if (MSAA) {
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
	if (PCSS) {
		PCSSDepthShader = Shader("./shader/cubeDepth.vert", "./shader/cubeDepth.frag", "./shader/cubeDepth.geom");
	}

	// HDRI
	
	SkyboxShader = Shader("./shader/skybox.vert", "./shader/skybox.frag");
	HdriConvert = Shader("./shader/hdriConvert.vert", "./shader/hdriConvert.frag");
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
#include <Model.h>
#include "stb_image.h"

void Model::Draw(Shader& shader, bool showTex, bool showNorm) {
	for (unsigned int i = 0; i < meshes.size(); i++) {
		meshes[i].Draw(shader, showTex, showNorm);
	}
}


void Model::loadModel(std::string path) {
	Assimp::Importer import;
	import.SetPropertyFloat(AI_CONFIG_PP_CT_MAX_SMOOTHING_ANGLE, 0.f);
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_JoinIdenticalVertices);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of("\\"));
	if (directory.length() == path.length()) {
		directory = path.substr(0, path.find_last_of("/"));
	}
	processNode(scene->mRootNode, scene);
}


void Model::processNode(aiNode* node, const aiScene* scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	for (unsigned i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}


Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	// vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		glm::vec3 pos_vector;
		pos_vector.x = mesh->mVertices[i].x;
		pos_vector.y = mesh->mVertices[i].y;
		pos_vector.z = mesh->mVertices[i].z;
		vertex.Position = pos_vector;
		updateMaxPos(pos_vector);

		glm::vec3 nor_vector;
		nor_vector.x = mesh->mNormals[i].x;
		nor_vector.y = mesh->mNormals[i].y;
		nor_vector.z = mesh->mNormals[i].z;
		vertex.Normal = nor_vector;

		glm::vec3 tan_vector;
		if (mesh->HasTangentsAndBitangents()) {
			tan_vector.x = mesh->mBitangents[i].x;
			tan_vector.y = mesh->mBitangents[i].y;
			tan_vector.z = mesh->mBitangents[i].z;
		}
		else {
			if (abs(glm::dot(glm::vec3(1.0, 0.0, 0.0), vertex.Normal)) > 0.99) {
				tan_vector = glm::normalize(glm::cross(glm::vec3(0.0, 1.0, 0.0), vertex.Normal));
			}
			else {
				tan_vector = glm::normalize(glm::cross(glm::vec3(1.0, 0.0, 0.0), vertex.Normal));
			}
			std::cout << "No Tangent info detected. Generating random Tangent." << std::endl;
		}
		vertex.Tangent = tan_vector;

		if (mesh->mTextureCoords[0]) 
		{
			glm::vec2 tex_vector;
			tex_vector.x = mesh->mTextureCoords[0][i].x;
			tex_vector.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCord = tex_vector;
		}
		else {
			vertex.TexCord = glm::vec2(0.0f, 0.0f);
			std::cout << "No UV Coordinates. Setting to (0.0, 0.0)" << std::endl;
		}
			
		vertices.push_back(vertex);
	}
	// indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// material
	glm::vec3 color = glm::vec3(1.0, 1.0, 1.0);
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// 1. diffuse maps
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// 2. specular maps
		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		// 3. normal maps
		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		// 4. height maps
		std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
		// 5. opacity maps
		std::vector<Texture> opacityMaps = loadMaterialTextures(material, aiTextureType_OPACITY, "texture_opacity");
		textures.insert(textures.end(), opacityMaps.begin(), opacityMaps.end());

		aiColor3D diffuse_color(0.f, 0.f, 0.f);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color);
		color = glm::vec3(diffuse_color[0], diffuse_color[1], diffuse_color[2]);

	}
	return Mesh(vertices, indices, textures, color);
}


std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++)
		{
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{   // if texture hasn't been loaded already, load it
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), directory, typeName);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture); // add to loaded textures
		}
	}
	return textures;
}


unsigned int Model::TextureFromFile(const char* path, const std::string& directory, std::string type)
{
	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(false);
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		GLenum internalFormat;
		if (nrComponents == 1) {
			internalFormat = GL_RED;
			format = GL_RED;
		}
		else if (nrComponents == 2) {
			internalFormat = GL_RG;
			format = GL_RG;
		}
		else if (nrComponents == 3) {
			format = GL_RGB;
			if (type == "texture_normal") {
				internalFormat = GL_RGB32F;
			}
			else if(type == "texture_diffuse") {
				internalFormat = GL_SRGB;
			}
			else {
				internalFormat = GL_RGB;
			}
		}
		else if (nrComponents == 4) {
			format = GL_RGBA;
			if (type == "texture_normal") {
				internalFormat = GL_RGBA32F;
			}
			else if (type == "texture_diffuse") {
				internalFormat = GL_SRGB_ALPHA;
			}
			else {
				internalFormat = GL_RGBA;
			}
		}
		
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		if (nrComponents <= 2){
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void Model::updateMaxPos(glm::vec3 pos) {
	float max_comp = glm::compMax(glm::abs(pos));
	max_pos = fmax(max_pos, max_comp);
	max_vec = glm::max(pos, max_vec);
	min_vec = glm::min(pos, min_vec);
}


glm::mat4 Model::getModelMatrix() {
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(scale, scale, scale));
	modelMatrix = glm::translate(modelMatrix, offset);
	return modelMatrix;
}
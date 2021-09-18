#include <Mesh.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <gtx/component_wise.hpp >
#include <gtc/matrix_transform.hpp>
#include <gtx/string_cast.hpp>

class Model
{
public:
	Model() {}
	Model(const char* path) {
		loadModel(path);
		name = path;
	}
	void Draw(Shader& shader, bool showTex, bool showNorm);
	float max_pos = 0.0;
	float scale = 1.0;
	glm::vec3 offset = glm::vec3(0.0, 0.0, 0.0);
	glm::mat4 getModelMatrix();

	std::vector<Mesh> meshes;
	std::vector<Texture> textures_loaded;
	std::string directory;
	std::string name;
	bool isStatic = false;

private:
	void updateMaxPos(glm::vec3 pos);
	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	unsigned int TextureFromFile(const char* path, const std::string& directory, std::string type);
};

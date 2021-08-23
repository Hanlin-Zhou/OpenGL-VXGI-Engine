#include <Mesh.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <gtx/component_wise.hpp >


class Model
{
public:
	Model() {}
	Model(const char* path) {
		loadModel(path);
	}
	void Draw(Shader& shader, bool showTex, bool showNorm);
	float max_pos = 0.0;;
private:
	std::vector<Mesh> meshes;
	std::vector<Texture> textures_loaded;
	std::string directory;

	void updateMaxPos(glm::vec3 pos);
	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	unsigned int TextureFromFile(const char* path, const std::string& directory, std::string type);
};

#include <Vertex.h>
#include <MyTexture.h>
#include <vector>
#include <shader.h>

class Mesh {
	public:
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
		void Draw(Shader& shader, bool showTex, bool showNorm);
	private:
		unsigned int VAO, VBO, EBO;

		void setupMesh();

};

#include <Vertex.h>
#include <MyTexture.h>
#include <vector>
#include <shader.h>

class Mesh {
	public:
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;
		glm::vec3 color;

		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, glm::vec3 color);
		void Draw(Shader& shader, bool showTex, bool showNorm);
	private:
		unsigned int VAO, VBO, EBO;

		void setupMesh();

};

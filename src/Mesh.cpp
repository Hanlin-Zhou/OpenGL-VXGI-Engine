#include <Mesh.h>
//#include <cstddef>


Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	setupMesh();
}


void Mesh::setupMesh() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCord));

	glBindVertexArray(0);
}


void Mesh::Draw(Shader& shader) {
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;
	unsigned int opacityNr = 1;
	shader.setBool("diffuse", false);
	shader.setBool("specular", false);
	shader.setBool("normal", false);
	shader.setBool("height", false);
	shader.setBool("opacity", false);
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i+8); // activate proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = textures[i].type;
		if (name == "texture_diffuse") {
			number = std::to_string(diffuseNr++);
			shader.setBool("diffuse", true);
		}
		else if (name == "texture_specular") {
			number = std::to_string(specularNr++);
			shader.setBool("specular", true);
		}
		else if (name == "texture_normal") {
			number = std::to_string(normalNr++);
			shader.setBool("normal", true);
		}
		else if (name == "texture_height") {
			number = std::to_string(heightNr++);
			shader.setBool("height", true);
		}
		else if (name == "texture_opacity") {
			number = std::to_string(opacityNr++);
			shader.setBool("opacity", true);
		}
		shader.setInt((name + number).c_str(), i + 8);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);

	// draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
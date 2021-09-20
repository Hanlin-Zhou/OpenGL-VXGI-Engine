#include <Mesh.h>


Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, glm::vec3 color) {
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	this->color = color;

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

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

	glBindVertexArray(0);
}


void Mesh::Draw(Shader& shader, bool showTex, bool showNorm) {
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;
	unsigned int opacityNr = 1;

	float default_1[4] = { 1.0,1.0,1.0,1.0 };
	float default_color[4] = {color[0], color[1], color[2], 1.0};
	float default_0[4] = { 0.0,0.0,0.0,0.0 };
	unsigned int default_tex_1, default_tex_0, default_tex_color;
	glGenTextures(1, &default_tex_1);
	glGenTextures(1, &default_tex_0);
	glGenTextures(1, &default_tex_color);
	glBindTexture(GL_TEXTURE_2D, default_tex_1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, default_1);
	glBindTexture(GL_TEXTURE_2D, default_tex_0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, default_0);
	glBindTexture(GL_TEXTURE_2D, default_tex_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, 1, 1, 0, GL_RGBA, GL_FLOAT, default_color);

	glActiveTexture(GL_TEXTURE10); // diff
	glBindTexture(GL_TEXTURE_2D, default_tex_color);
	glActiveTexture(GL_TEXTURE11); // spec
	glBindTexture(GL_TEXTURE_2D, default_tex_0);
	glActiveTexture(GL_TEXTURE12); // norm
	glBindTexture(GL_TEXTURE_2D, default_tex_0);
	glActiveTexture(GL_TEXTURE13); // height
	glBindTexture(GL_TEXTURE_2D, default_tex_0);
	glActiveTexture(GL_TEXTURE14); // opac
	glBindTexture(GL_TEXTURE_2D, default_tex_1);

	for (unsigned int i = 0; i < textures.size(); i++)
	{
		// glActiveTexture(GL_TEXTURE0 + i+8); // activate proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = textures[i].type;
		if (name == "texture_diffuse" && showTex) {
			glActiveTexture(GL_TEXTURE10);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		else if (name == "texture_specular" && showTex) {
			glActiveTexture(GL_TEXTURE11);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		else if (name == "texture_normal" && showNorm) {
			glActiveTexture(GL_TEXTURE12);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		else if (name == "texture_height") {
			glActiveTexture(GL_TEXTURE13);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		else if (name == "texture_opacity") {
			glActiveTexture(GL_TEXTURE14);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
	}
	shader.setInt("texture_diffuse1", 10);
	shader.setInt("texture_specular1", 11);
	shader.setInt("texture_normal1", 12);
	shader.setInt("texture_height1", 13);
	shader.setInt("texture_opacity1", 14);
	glActiveTexture(GL_TEXTURE0);

	// draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
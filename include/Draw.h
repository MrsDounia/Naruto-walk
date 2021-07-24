#define INDICE_TO_PTR(x) ((void*)(x))

//Les structures material et light
struct Material {
    glm::vec3 color;
    float ka;
    float kd;
    float ks;
    float alpha;
};

struct Light {
    glm::vec3 color;
    glm::vec3 position;
};

//Fonctions generateVBO, genere un VBO d'une geometry et la dessine avec Draw

GLuint generateVBO(const Geometry& forme)
{
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    {
        glBufferData(GL_ARRAY_BUFFER, (3 * 2 + 2) * sizeof(float) * forme.getNbVertices(), NULL, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float) * forme.getNbVertices(), forme.getVertices());
        glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float) * forme.getNbVertices(), 3 * sizeof(float) * forme.getNbVertices(), forme.getNormals());
        glBufferSubData(GL_ARRAY_BUFFER, (3 * 2) * sizeof(float) * forme.getNbVertices(), 2 * sizeof(float) * forme.getNbVertices(), forme.getUVs());
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return VBO;
}

void Draw(const glm::mat4& mvp, Shader* shader, GLuint VBO, const Geometry& forme, GLuint textureID, Material& material, Light& light, const glm::mat4& modelMatrix, const glm::vec3& cameraPos)
{
    ////////Use shader 
    glUseProgram(shader->getProgramID());
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        GLint vPosition = glGetAttribLocation(shader->getProgramID(), "vPosition");
        glVertexAttribPointer(vPosition, 3, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(vPosition);

        GLint vNormal = glGetAttribLocation(shader->getProgramID(), "vNormal");
        glVertexAttribPointer(vNormal, 3, GL_FLOAT, 0, 0, INDICE_TO_PTR(3 * forme.getNbVertices() * sizeof(float)));
        glEnableVertexAttribArray(vNormal);

        GLint vUV = glGetAttribLocation(shader->getProgramID(), "vUV");
        glVertexAttribPointer(vUV, 2, GL_FLOAT, 0, 0, INDICE_TO_PTR((3 + 3) * forme.getNbVertices() * sizeof(float)));
        glEnableVertexAttribArray(vUV);

        //VBO : 0  position  3  normal 6 uv 8

        glm::vec4 constantes(material.ka, material.kd, material.ks, material.alpha);
        GLint uConstantes = glGetUniformLocation(shader->getProgramID(), "uConstantes");
        glUniform4fv(uConstantes, 1, glm::value_ptr(constantes));

        GLint uColorMtl = glGetUniformLocation(shader->getProgramID(), "uColorMtl");
        glUniform3fv(uColorMtl, 1, glm::value_ptr(material.color));

        GLint uLightColor = glGetUniformLocation(shader->getProgramID(), "uLightColor");
        glUniform3fv(uLightColor, 1, glm::value_ptr(light.color));

        GLint uLightPosition = glGetUniformLocation(shader->getProgramID(), "uLightPosition");
        glUniform3fv(uLightPosition, 1, glm::value_ptr(light.position));

        GLint uMVP = glGetUniformLocation(shader->getProgramID(), "uMVP");
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(mvp));

        GLint uModelMatrix = glGetUniformLocation(shader->getProgramID(), "uModelMatrix");
        glUniformMatrix4fv(uModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        glm::mat3 invModelMatrix = glm::inverse(glm::mat3(modelMatrix));
        GLint uInvModelMatrix = glGetUniformLocation(shader->getProgramID(), "uInvModelMatrix");
        glUniformMatrix3fv(uInvModelMatrix, 1, GL_FALSE, glm::value_ptr(invModelMatrix));

        GLint uCameraPos = glGetUniformLocation(shader->getProgramID(), "uCameraPos");
        glUniform3fv(uCameraPos, 1, glm::value_ptr(cameraPos));

        //travail avec la premiere texture que peut supporter le shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);// GL_TEXTURE0 vaut textureID

        GLint uTexture = glGetUniformLocation(shader->getProgramID(), "uTexture");
        glUniform1i(uTexture, 0);

        glDrawArrays(GL_TRIANGLES, 0, forme.getNbVertices());
        glBindBuffer(GL_ARRAY_BUFFER, 0); //Close the VBO
    }
    glUseProgram(0); //Close the program

}

//Fonction generateObjectVBO, genere un VBO d'un fichier .obj et le dessine avec DrawObject

GLuint generateObjectVBO(size_t nb_vertices, std::vector<glm::vec3> vertices, std::vector<glm::vec2> uvs, std::vector<glm::vec3> normals)
{
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    {
        glBufferData(GL_ARRAY_BUFFER, (3 * 2 + 2) * sizeof(float) * nb_vertices, NULL, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float) * nb_vertices, vertices.data());
        glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float) * nb_vertices, 3 * sizeof(float) * nb_vertices, normals.data());
        glBufferSubData(GL_ARRAY_BUFFER, (3 * 2) * sizeof(float) * nb_vertices, 2 * sizeof(float) * nb_vertices, uvs.data());
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return VBO;

}

void DrawObject(const glm::mat4& mvp, Shader* shader, GLuint VBO, const glm::mat4& modelMatrix, size_t nb_vertices, GLuint textureID) {
    glUseProgram(shader->getProgramID());
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        //Dire comment est ranger ke VBO : 0  position  3  normal 6 uv 8
        GLint vPosition = glGetAttribLocation(shader->getProgramID(), "vPosition");
        glVertexAttribPointer(vPosition, 3, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(vPosition);

        GLint vNormal = glGetAttribLocation(shader->getProgramID(), "vNormal");
        glVertexAttribPointer(vNormal, 3, GL_FLOAT, 0, 0, INDICE_TO_PTR(3 * nb_vertices * sizeof(float)));
        glEnableVertexAttribArray(vNormal);

        GLint vUV = glGetAttribLocation(shader->getProgramID(), "vUV");
        glVertexAttribPointer(vUV, 2, GL_FLOAT, 0, 0, INDICE_TO_PTR((3 + 3) * nb_vertices * sizeof(float)));
        glEnableVertexAttribArray(vUV);

        //Send matrices to shader
        GLint uMVP = glGetUniformLocation(shader->getProgramID(), "uMVP");
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(mvp));

        GLint uModelMatrix = glGetUniformLocation(shader->getProgramID(), "uModelMatrix");
        glUniformMatrix4fv(uModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        //travail avec la premiere texture que peut supporter le shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);// GL_TEXTURE0 vaut textureID

        GLint uTexture = glGetUniformLocation(shader->getProgramID(), "uTexture");
        glUniform1i(uTexture, 0);

        glDrawArrays(GL_TRIANGLES, 0, nb_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0); //Close the VBO
    }
    glUseProgram(0); //Close the program

}

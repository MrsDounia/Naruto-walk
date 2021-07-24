#pragma once

bool loadOBJ(
    const char* path,
    std::vector<glm::vec3>& out_vertices,
    std::vector<glm::vec2>& out_uvs,
    std::vector<glm::vec3>& out_normals,
    size_t& nbVertices
) {
    printf("Loading OBJ file %s...\n", path);

    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    FILE* file = fopen(path, "r");
    if (file == NULL) {
        printf("Could not open file... \n");
        getchar();
        return false;
    }

    while (1) {

        char lineHeader[128];
        // lire le premier mot de la ligne
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // fin du fichier

        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            uv.y = -uv.y; 
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            std::string vertex1, vertex2, vertex3; 
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            //Recup�re les index et les range dans 3 tableaux : vertices, normal, uvs: 
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
            if (matches != 9) {
                printf("File can't be read by our simple parser \n");
                fclose(file);
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);

            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);

            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
        }
        else {
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }

    }
    // pour chaque point de chaque triangle
    for (unsigned int i = 0; i < vertexIndices.size(); i++) {

        // obtenir les attributs avec les indices
        glm::vec3 vertex = temp_vertices[vertexIndices[i] - 1];
        glm::vec3 normal = temp_normals[normalIndices[i] - 1];
        glm::vec2 uv = temp_uvs[uvIndices[i] - 1];


        // on range les attributs dans le buffer
        out_vertices.push_back(vertex);
        out_normals.push_back(normal);
        out_uvs.push_back(uv);

    }
    nbVertices = out_vertices.size();
    fclose(file);
    return true;
}
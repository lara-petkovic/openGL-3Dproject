#include "model.h"

static unsigned
_LoadMaterialTexture(const struct aiMaterial* material, const char* resourcePath, enum aiTextureType type) {
    if (material) {
        struct aiString Path;
        if (aiGetMaterialTexture(material, type, 0, &Path, 0, 0, 0, 0, 0, 0) == AI_SUCCESS) {
            size_t FullPathLength = strlen(resourcePath) + strlen(Path.data);
            const char* FullMaterialTexturePath = calloc(FullPathLength + 1, sizeof(char));
            if (!FullMaterialTexturePath) {
                fprintf(stderr, "Failed to allocate space for material texture path\n");
                return LOAD_FAIL;
            }
            strcat(FullMaterialTexturePath, resourcePath);
            strcat(FullMaterialTexturePath, Path.data);
            return LoadImageToTexture(FullMaterialTexturePath);
        }
    }

    return 0;
}

int
LoadModel(const char* filePath, Model* model) {

    if (!filePath) {
        fprintf(stderr, "Model file path invalid\n");
        return LOAD_FAIL;
    }

    const struct aiScene* Scene = aiImportFile(filePath, POSTPROCESS_FLAGS);
    if(!Scene) {
        fprintf(stderr, "Failed to load Assimp scene: %s\n", aiGetErrorString());
        return LOAD_FAIL;
    }

    // NOTE(Jovan): Allocate all mesh buffers
    model->NumMeshes = Scene->mNumMeshes;
    model->MeshBuffers = (MeshBuffer*)calloc(model->NumMeshes, sizeof(MeshBuffer));
    if(!model->MeshBuffers) {
        fprintf(stderr, "Failed to allocate mesh buffer memory.\n");
        return LOAD_FAIL;
    }

    unsigned ElementsPerVertex = 8;
    // NOTE(Jovan): Attempt to load vertex information into buffers
    for(unsigned MeshIdx = 0; MeshIdx < model->NumMeshes; ++MeshIdx) {
        MeshBuffer* CurrMeshBuffer = &model->MeshBuffers[MeshIdx];
        const struct aiMesh* CurrMesh = Scene->mMeshes[MeshIdx];
        unsigned NumVertices = CurrMesh->mNumVertices;
        float* Vertices = (float*)calloc((size_t)ElementsPerVertex * (size_t)NumVertices, sizeof(float));

        if (!Vertices) {
            fprintf(stderr, "Failed to allocate vertices.\n");
            return LOAD_FAIL;
        }
       
        char HasTextureCoords = !!CurrMesh->mTextureCoords;
        for(unsigned VertIdx = 0; VertIdx < NumVertices; ++VertIdx) {
            Vertices[ElementsPerVertex * VertIdx] = CurrMesh->mVertices[VertIdx].x;
            Vertices[ElementsPerVertex * VertIdx + 1] = CurrMesh->mVertices[VertIdx].y;
            Vertices[ElementsPerVertex * VertIdx + 2] = CurrMesh->mVertices[VertIdx].z;

            Vertices[ElementsPerVertex * VertIdx + 3] = CurrMesh->mNormals[VertIdx].x;
            Vertices[ElementsPerVertex * VertIdx + 4] = CurrMesh->mNormals[VertIdx].y;
            Vertices[ElementsPerVertex * VertIdx + 5] = CurrMesh->mNormals[VertIdx].z;
            
            Vertices[ElementsPerVertex * VertIdx + 6] = HasTextureCoords ? CurrMesh->mTextureCoords[0][VertIdx].x : 0.0f;
            Vertices[ElementsPerVertex * VertIdx + 7] = HasTextureCoords ? CurrMesh->mTextureCoords[0][VertIdx].y : 0.0f;
        }

        unsigned NumFaces = CurrMesh->mNumFaces;
        if(!NumFaces) continue;

        unsigned* Indices = (unsigned*)calloc(3 * (size_t)NumFaces, sizeof(unsigned));
        if(!Indices) {
            fprintf(stderr, "Failed to allocate indices.\n");
            return LOAD_FAIL;
        }

        for(unsigned FaceIdx = 0; FaceIdx < NumFaces; ++FaceIdx) {
            const struct aiFace* CurrFace = &CurrMesh->mFaces[FaceIdx];
            Indices[3 * FaceIdx] = CurrFace->mIndices[0];
            Indices[3 * FaceIdx + 1] = CurrFace->mIndices[1];
            Indices[3 * FaceIdx + 2] = CurrFace->mIndices[2];
        }

        CurrMeshBuffer->VerticesCount = NumVertices;
        CurrMeshBuffer->IndicesCount = 3 * NumFaces;

        const char* LastSlash = strrchr(filePath, '/');
        size_t Length = LastSlash - filePath + 1;
        char* ResourcePath = calloc(Length + 1, sizeof(char));

        if (!ResourcePath) {
            fprintf(stderr, "Failed to allocate resource path for model: %s\n", filePath);
            return LOAD_FAIL;
        }

        strncpy(ResourcePath, filePath, Length);
        const struct aiMaterial* Material = Scene->mMaterials[CurrMesh->mMaterialIndex];
        CurrMeshBuffer->DiffuseTexture = _LoadMaterialTexture(Material, ResourcePath, aiTextureType_DIFFUSE);
        CurrMeshBuffer->SpecularTexture = _LoadMaterialTexture(Material, ResourcePath, aiTextureType_SPECULAR);
        free(ResourcePath);

        GLsizei Stride = ElementsPerVertex * sizeof(float);
        glGenVertexArrays(1, &CurrMeshBuffer->VAO);
        glBindVertexArray(CurrMeshBuffer->VAO);
        glGenBuffers(1, &CurrMeshBuffer->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, CurrMeshBuffer->VBO);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)NumVertices * Stride, Vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(LAYOUT_POSITION, 3, GL_FLOAT, GL_FALSE, Stride, (void*)0);
        glEnableVertexAttribArray(LAYOUT_POSITION);
        glVertexAttribPointer(LAYOUT_NORMALS, 3, GL_FLOAT, GL_FALSE, Stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(LAYOUT_NORMALS);
        glVertexAttribPointer(LAYOUT_UV, 2, GL_FLOAT, GL_FALSE, Stride, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(LAYOUT_UV);

        if(CurrMeshBuffer->IndicesCount) {
            glGenBuffers(1, &CurrMeshBuffer->EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CurrMeshBuffer->EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, CurrMeshBuffer->IndicesCount * sizeof(unsigned), Indices, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        if(NumVertices) {
            free(Vertices);
        }

        if(NumFaces){
            free(Indices);
        }
    }

    fprintf(stdout, "Loaded %d meshes for %s.\n", model->NumMeshes, filePath);

    aiReleaseImport(Scene);
    return LOAD_SUCCESS;
}

void
FreeModelResources(Model* model) {
    fprintf(stdout, "Freeing model\n");
    for(unsigned MeshIdx = 0; MeshIdx < model->NumMeshes; ++MeshIdx) {
        MeshBuffer* CurrMesh = &model->MeshBuffers[MeshIdx];
        fprintf(stdout, "Freeing VBO\n");
        if(CurrMesh->VerticesCount) {
            glDeleteBuffers(1, &CurrMesh->VBO);
        }
        fprintf(stdout, "Freeing EBO\n");
        if(CurrMesh->IndicesCount) {
            glDeleteBuffers(1, &CurrMesh->EBO);
        }
        fprintf(stdout, "Freeing VAO\n");
        glDeleteVertexArrays(1, &CurrMesh->VAO);
    }
    fprintf(stdout, "Freeing meshes\n");
    free(model->MeshBuffers);
}

void
RenderModel(const Model* model) {
    for(unsigned MeshIdx = 0; MeshIdx < model->NumMeshes; ++MeshIdx) {
        const MeshBuffer* CurrBuffer = &model->MeshBuffers[MeshIdx];

        if (CurrBuffer->DiffuseTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, CurrBuffer->DiffuseTexture);
        }

        if (CurrBuffer->SpecularTexture) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, CurrBuffer->SpecularTexture);
        }

        glBindVertexArray(CurrBuffer->VAO);
        if(CurrBuffer->IndicesCount) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CurrBuffer->EBO);
            glDrawElements(GL_TRIANGLES, CurrBuffer->IndicesCount, GL_UNSIGNED_INT, (void*)0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            continue;
        }

        glDrawArrays(GL_TRIANGLES, 0, CurrBuffer->VerticesCount);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindVertexArray(0);
    }
}
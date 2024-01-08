/**
 * @file model.h
 * @author Jovan Ivosevic
 * @brief Model loading via Assimp for OpenGL
 * @version 0.1
 * @date 2022-11-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef MODEL_H
#define MODEL_H

#define POSTPROCESS_FLAGS (aiProcess_Triangulate)
// TODO(Jovan): If textures don't show properly, use this instead
// #define POSTPROCESS_FLAGS (aiProcess_Triangulate | aiProcess_FlipUVs)
#define LOAD_SUCCESS 1
#define LOAD_FAIL 0
#define LAYOUT_POSITION 0
#define LAYOUT_NORMALS 1
#define LAYOUT_UV 2

#include <stdlib.h>
#include <stdio.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>
#include <string.h>
#include "texture.h"

/**
 * @brief Internal buffer representation for each model mesh
 * 
 */
typedef struct MeshBuffer {
    unsigned VAO;
    unsigned VBO;
    unsigned EBO;
    unsigned VerticesCount;
    unsigned IndicesCount;
    unsigned DiffuseTexture;
    unsigned SpecularTexture;
} MeshBuffer;

/**
 * @brief Model struct
 * 
 */
typedef struct Model {
    unsigned NumMeshes;
    MeshBuffer* MeshBuffers;
} Model;

#endif

/**
 * @brief Loads model
 * 
 * @param filePath Relative model file path
 * @param model Model struct which will contain result. Should be allocated beforehand.
 * @return int Success, 0 - FAIL, 1 - SUCCESS
 */
int LoadModel(const char* filePath, Model* model);
/**
 * @brief Attempts to free all memory and GL resources occupied by model. Does not free model struct itself.
 * 
 * @param model Model to be deallocated
 */
void FreeModelResources(Model* model);

/**
 * @brief Attempts to render all model meshes via GL interface
 * 
 * @param model Model to be rendered
 */
void RenderModel(const Model* model);

static unsigned _LoadMaterialTexture(const struct aiMaterial* material, const char* resourcePath, enum aiTextureType type);
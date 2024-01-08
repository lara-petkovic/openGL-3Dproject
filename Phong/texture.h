/**
 * @file texture.h
 * @author Jovan Ivosevic
 * @brief Texture loading
 *
 */
#pragma once
#include <stdio.h>
#include <GL/glew.h>

static const char* MISSING_TEXTURE_PATH = "res/missing_textures.png";
/**
 * @brief Loads image file and creates an OpenGL texture.
 * NOTE: Try avoiding .jpg and other lossy compression formats as
 * they are uncompressed during loading and the memory benefit is
 * negated with the addition of loss of quality
 *
 * @param filePath Image file path
 * @returns TextureID
*/
unsigned LoadImageToTexture(const char* filePath);
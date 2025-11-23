#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

/**
 * @brief Simple font renderer using stb_truetype and OpenGL textures
 *
 * Generates a texture atlas for ASCII characters and renders text using textured quads.
 */
class FontRenderer
{
public:
    /**
     * @brief Initializes font renderer with a TrueType font
     * @param fontPath Path to .ttf font file
     * @param fontSize Font size in pixels for atlas generation
     */
    FontRenderer(const char *fontPath, float fontSize);

    /**
     * @brief Destructor - cleans up OpenGL resources
     */
    ~FontRenderer();

    /**
     * @brief Renders text at specified position
     * @param text Text to render
     * @param x X position in normalized device coordinates (-1 to 1)
     * @param y Y position in normalized device coordinates (-1 to 1)
     * @param scale Scale factor for rendering
     * @param color Text color (RGB)
     * @param shaderProgram Shader program to use for rendering
     * @param vao VAO to bind for rendering
     * @param vbo VBO to use for vertex data
     */
    void RenderText(const std::string &text,
        float x,
        float y,
        float scale,
        const glm::vec3 &color,
        GLuint shaderProgram,
        GLuint vao,
        GLuint vbo);

    /**
     * @brief Calculates the width of text in normalized device coordinates
     * @param text Text to measure
     * @param scale Scale factor
     * @return Width in NDC
     */
    float GetTextWidth(const std::string &text, float scale) const;

private:
    struct Character
    {
        float x0, y0, x1, y1; // Atlas texture coordinates
        float xOffset, yOffset; // Glyph offset
        float advance;          // Horizontal advance
        float width, height;    // Glyph dimensions
    };

    GLuint fontTexture;
    std::unordered_map<char, Character> characters;
    float fontSize;
    float ascent, descent, lineGap;

    void GenerateFontAtlas(const unsigned char *fontData, size_t fontDataSize);
};

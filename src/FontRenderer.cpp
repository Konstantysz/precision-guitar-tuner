#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "FontRenderer.h"
#include <Logger.h>
#include <fstream>
#include <vector>

FontRenderer::FontRenderer(const char *fontPath, float fontSize) : fontSize(fontSize), fontTexture(0)
{
    // Load font file
    std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        LOG_ERROR("Failed to open font file: {}", fontPath);
        return;
    }

    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> fontData(fileSize);
    file.read(reinterpret_cast<char *>(fontData.data()), fileSize);
    file.close();

    LOG_INFO("Loaded font file: {} ({} bytes)", fontPath, fileSize);

    GenerateFontAtlas(fontData.data(), fileSize);
}

FontRenderer::~FontRenderer()
{
    if (fontTexture != 0)
    {
        glDeleteTextures(1, &fontTexture);
    }
}

void FontRenderer::GenerateFontAtlas(const unsigned char *fontData, size_t fontDataSize)
{
    // Initialize stb_truetype
    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontData, stbtt_GetFontOffsetForIndex(fontData, 0)))
    {
        LOG_ERROR("Failed to initialize font");
        return;
    }

    // Get font metrics
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);

    float scale = stbtt_ScaleForPixelHeight(&font, fontSize);
    this->ascent = ascent * scale;
    this->descent = descent * scale;
    this->lineGap = lineGap * scale;

    // Calculate atlas size (8x16 grid for 128 ASCII characters)
    const int atlasWidth = 512;
    const int atlasHeight = 512;
    std::vector<unsigned char> atlasData(atlasWidth * atlasHeight, 0);

    int x = 0, y = 0;
    int rowHeight = 0;

    // Generate glyphs for ASCII characters (32-126)
    for (int c = 32; c < 127; c++)
    {
        int glyphIndex = stbtt_FindGlyphIndex(&font, c);

        int width, height, xOffset, yOffset;
        unsigned char *bitmap =
            stbtt_GetGlyphBitmap(&font, 0, scale, glyphIndex, &width, &height, &xOffset, &yOffset);

        if (bitmap == nullptr)
        {
            // Space or invisible character
            int advance, leftBearing;
            stbtt_GetGlyphHMetrics(&font, glyphIndex, &advance, &leftBearing);

            characters[static_cast<char>(c)] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                advance * scale, 0.0f, 0.0f };
            continue;
        }

        // Check if we need to move to next row
        if (x + width >= atlasWidth)
        {
            x = 0;
            y += rowHeight;
            rowHeight = 0;
        }

        // Copy bitmap to atlas
        for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                int atlasX = x + col;
                int atlasY = y + row;
                if (atlasX < atlasWidth && atlasY < atlasHeight)
                {
                    atlasData[atlasY * atlasWidth + atlasX] = bitmap[row * width + col];
                }
            }
        }

        // Calculate texture coordinates
        float tx0 = static_cast<float>(x) / atlasWidth;
        float ty0 = static_cast<float>(y) / atlasHeight;
        float tx1 = static_cast<float>(x + width) / atlasWidth;
        float ty1 = static_cast<float>(y + height) / atlasHeight;

        // Get advance width
        int advance, leftBearing;
        stbtt_GetGlyphHMetrics(&font, glyphIndex, &advance, &leftBearing);

        characters[static_cast<char>(c)] = { tx0, ty0, tx1, ty1, static_cast<float>(xOffset),
            static_cast<float>(yOffset), advance * scale, static_cast<float>(width),
            static_cast<float>(height) };

        stbtt_FreeBitmap(bitmap, nullptr);

        x += width + 1; // Add 1px padding
        rowHeight = std::max(rowHeight, height + 1);
    }

    // Create OpenGL texture
    glGenTextures(1, &fontTexture);
    glBindTexture(GL_TEXTURE_2D, fontTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, atlasData.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    LOG_INFO("Generated font atlas: {}x{} with {} characters", atlasWidth, atlasHeight, characters.size());
}

void FontRenderer::RenderText(const std::string &text,
    float x,
    float y,
    float scale,
    const glm::vec3 &color,
    GLuint shaderProgram,
    GLuint vao,
    GLuint vbo)
{
    if (fontTexture == 0)
    {
        return;
    }

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, fontTexture);

    float currentX = x;

    for (char c : text)
    {
        auto it = characters.find(c);
        if (it == characters.end())
        {
            continue;
        }

        const Character &ch = it->second;

        // Calculate quad position
        float xpos = currentX + ch.xOffset * scale / fontSize;
        float ypos = y + ch.yOffset * scale / fontSize;
        float width = ch.width * scale / fontSize;
        float height = ch.height * scale / fontSize;

        // Vertex data: position (2) + texcoord (2) + color (3)
        float vertices[] = {
            // positions                    // tex coords   // colors
            xpos, ypos, ch.x0, ch.y1, color.r, color.g, color.b,                              // bottom-left
            xpos + width, ypos, ch.x1, ch.y1, color.r, color.g, color.b,                      // bottom-right
            xpos + width, ypos + height, ch.x1, ch.y0, color.r, color.g, color.b,             // top-right
            xpos, ypos, ch.x0, ch.y1, color.r, color.g, color.b,                              // bottom-left
            xpos + width, ypos + height, ch.x1, ch.y0, color.r, color.g, color.b,             // top-right
            xpos, ypos + height, ch.x0, ch.y0, color.r, color.g, color.b                      // top-left
        };

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        currentX += ch.advance * scale / fontSize;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

float FontRenderer::GetTextWidth(const std::string &text, float scale) const
{
    float width = 0.0f;
    for (char c : text)
    {
        auto it = characters.find(c);
        if (it != characters.end())
        {
            width += it->second.advance * scale / fontSize;
        }
    }
    return width;
}

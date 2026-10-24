#include <iostream>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include "HUD.h"
#include "AssetPath.h"

// Font de mapa de bits 8x8 per a dígits '0'..'9', ':' i espai
// 1 bit = píxel de tinta fosca (estil retro arcade), 0 bit = transparent
static const unsigned char FONT_BITMAPS[12][8] = {
	// '0' (anell net sense barra diagonal)
	{ 0b01111000,
	  0b10000100,
	  0b10000100,
	  0b10000100,
	  0b10000100,
	  0b10000100,
	  0b01111000,
	  0b00000000 },
	// '1'
	{ 0b00110000,
	  0b01110000,
	  0b00110000,
	  0b00110000,
	  0b00110000,
	  0b00110000,
	  0b01111000,
	  0b00000000 },
	// '2'
	{ 0b01111000,
	  0b10000100,
	  0b00001000,
	  0b00010000,
	  0b00100000,
	  0b01000000,
	  0b11111100,
	  0b00000000 },
	// '3'
	{ 0b01111000,
	  0b10000100,
	  0b00001000,
	  0b00111000,
	  0b00000100,
	  0b10000100,
	  0b01111000,
	  0b00000000 },
	// '4'
	{ 0b00011000,
	  0b00111000,
	  0b01011000,
	  0b10011000,
	  0b11111100,
	  0b00011000,
	  0b00011000,
	  0b00000000 },
	// '5'
	{ 0b11111100,
	  0b10000000,
	  0b11111000,
	  0b00000100,
	  0b00000100,
	  0b10000100,
	  0b01111000,
	  0b00000000 },
	// '6'
	{ 0b01111000,
	  0b10000100,
	  0b10000000,
	  0b11111000,
	  0b10000100,
	  0b10000100,
	  0b01111000,
	  0b00000000 },
	// '7'
	{ 0b11111100,
	  0b00000100,
	  0b00001000,
	  0b00010000,
	  0b00100000,
	  0b00100000,
	  0b00100000,
	  0b00000000 },
	// '8'
	{ 0b01111000,
	  0b10000100,
	  0b10000100,
	  0b01111000,
	  0b10000100,
	  0b10000100,
	  0b01111000,
	  0b00000000 },
	// '9'
	{ 0b01111000,
	  0b10000100,
	  0b10000100,
	  0b01111100,
	  0b00000100,
	  0b10000100,
	  0b01111000,
	  0b00000000 },
	// ':'
	{ 0b00000000,
	  0b01100000,
	  0b01100000,
	  0b00000000,
	  0b01100000,
	  0b01100000,
	  0b00000000,
	  0b00000000 },
	// ' '
	{ 0b00000000,
	  0b00000000,
	  0b00000000,
	  0b00000000,
	  0b00000000,
	  0b00000000,
	  0b00000000,
	  0b00000000 }
};

HUD::HUD()
{
	shaderProgram = NULL;
	whiteTexId = 0;
	fontTexId = 0;
	vao = 0;
	vbo = 0;
	posLocation = 0;
	texCoordLocation = 0;
	barPinkColor = glm::vec4(0.933f, 0.482f, 0.690f, 1.0f); // #EE7BB0 per defecte
	texWidth = 24;
	texHeight = 16;
}

HUD::~HUD()
{
	if(whiteTexId != 0)
		glDeleteTextures(1, &whiteTexId);
	if(fontTexId != 0)
		glDeleteTextures(1, &fontTexId);
	if(vao != 0)
		glDeleteVertexArrays(1, &vao);
	if(vbo != 0)
		glDeleteBuffers(1, &vbo);
}

void HUD::init(ShaderProgram *program)
{
	shaderProgram = program;

	// 1. Carregar la textura d'icones barra.png (graella de 8x8 píxels per icona)
	textureBarra.loadFromFile(assetPath("images/barra.png"), TEXTURE_PIXEL_FORMAT_RGBA);
	textureBarra.setWrapS(GL_CLAMP_TO_EDGE);
	textureBarra.setWrapT(GL_CLAMP_TO_EDGE);
	textureBarra.setMinFilter(GL_NEAREST);
	textureBarra.setMagFilter(GL_NEAREST);

	// Obtenir les dimensions exactes en píxels de barra.png
	int w = 0, h = 0, channels = 0;
	unsigned char *imgData = stbi_load(assetPath("images/barra.png").c_str(), &w, &h, &channels, 4);
	if(imgData != NULL)
	{
		texWidth = w;
		texHeight = h;
		if(imgData[0] > 180 && imgData[1] > 80 && imgData[2] > 140)
			barPinkColor = glm::vec4(imgData[0] / 255.f, imgData[1] / 255.f, imgData[2] / 255.f, 1.0f);
		else
			barPinkColor = glm::vec4(238.f / 255.f, 123.f / 255.f, 176.f / 255.f, 1.0f);
		stbi_image_free(imgData);
	}
	else
	{
		texWidth = textureBarra.width() > 0 ? textureBarra.width() : 24;
		texHeight = textureBarra.height() > 0 ? textureBarra.height() : 16;
		barPinkColor = glm::vec4(238.f / 255.f, 123.f / 255.f, 176.f / 255.f, 1.0f);
	}

	// 2. Crear textura blanca 1x1 per dibuixar la barra rosa de fons
	unsigned char whitePixel[4] = { 255, 255, 255, 255 };
	glGenTextures(1, &whiteTexId);
	glBindTexture(GL_TEXTURE_2D, whiteTexId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

	// 3. Crear textura d'atlas per a la font retro de 96x8 píxels (12 glifs de 8x8)
	unsigned char fontData[96 * 8 * 4];
	std::memset(fontData, 0, sizeof(fontData));

	for(int glyph = 0; glyph < 12; glyph++)
	{
		for(int row = 0; row < 8; row++)
		{
			unsigned char byte = FONT_BITMAPS[glyph][row];
			for(int col = 0; col < 8; col++)
			{
				bool bitOn = (byte & (1 << (7 - col))) != 0;
				int px = glyph * 8 + col;
				int py = row;
				int index = (py * 96 + px) * 4;
				if(bitOn)
				{
					// Color fosc arcade retro (#1E281E) amb opacitat màxima
					fontData[index + 0] = 30;  // R
					fontData[index + 1] = 42;  // G
					fontData[index + 2] = 30;  // B
					fontData[index + 3] = 255; // A
				}
				else
				{
					fontData[index + 0] = 0;
					fontData[index + 1] = 0;
					fontData[index + 2] = 0;
					fontData[index + 3] = 0;
				}
			}
		}
	}

	glGenTextures(1, &fontTexId);
	glBindTexture(GL_TEXTURE_2D, fontTexId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 96, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontData);

	// 4. Crear VAO i VBO per a la renderització dels quads
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	posLocation = program->bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	texCoordLocation = program->bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void *)(2 * sizeof(float)));
}

void HUD::renderQuad(float x, float y, float width, float height, float u0, float v0, float u1, float v1)
{
	float vertices[24] = {
		x, y, u0, v0,
		x + width, y, u1, v0,
		x + width, y + height, u1, v1,
		x, y, u0, v0,
		x + width, y + height, u1, v1,
		x, y + height, u0, v1
	};

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 24 * sizeof(float), vertices);

	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void HUD::renderIcon(HUDIconType type, float x, float y, float size)
{
	int col = 0, row = 0;

	switch(type)
	{
	case HUD_ICON_CLOCK:  // Rellotge: fila 1, col 2
		col = 2; row = 1;
		break;
	case HUD_ICON_LIVES:  // Cap de Bomberman en marc: fila 0, col 2
		col = 2; row = 0;
		break;
	case HUD_ICON_TARGET: // 'T': fila 1, col 0
		col = 0; row = 1;
		break;
	case HUD_ICON_BOMB:   // Bomba negra: fila 0, col 1
		col = 1; row = 0;
		break;
	case HUD_ICON_POWER:  // Cap blanc / patí: fila 0, col 0
		col = 0; row = 0;
		break;
	case HUD_ICON_HEART:  // Cor: fila 1, col 1
		col = 1; row = 1;
		break;
	}

	// Càlcul basat en píxels reals: cada icona fa exactament 8x8 píxels
	float tw = (texWidth > 0) ? (float)texWidth : 24.0f;
	float th = (texHeight > 0) ? (float)texHeight : 16.0f;

	float u0 = (float)(col * 8) / tw;
	float u1 = (float)((col + 1) * 8) / tw;
	float v0 = (float)(row * 8) / th;
	float v1 = (float)((row + 1) * 8) / th;

	glEnable(GL_TEXTURE_2D);
	textureBarra.use();
	shaderProgram->setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	renderQuad(x, y, size, size, u0, v0, u1, v1);
}

void HUD::renderChar(char c, float x, float y, float width, float height)
{
	int glyphIdx = 11; // Espai per defecte
	if(c >= '0' && c <= '9')
		glyphIdx = c - '0';
	else if(c == ':')
		glyphIdx = 10;

	float u0 = (float)(glyphIdx * 8) / 96.0f;
	float u1 = (float)((glyphIdx + 1) * 8) / 96.0f;
	float v0 = 0.0f;
	float v1 = 1.0f;

	glBindTexture(GL_TEXTURE_2D, fontTexId);
	shaderProgram->setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	renderQuad(x, y, width, height, u0, v0, u1, v1);
}

void HUD::renderString(const std::string &str, float x, float y, float charAdvance)
{
	float curX = x;
	for(size_t i = 0; i < str.size(); ++i)
	{
		char c = str[i];
		if(c == ':')
		{
			renderChar(':', curX, y, 8.0f, 8.0f);
			curX += 4.0f;
		}
		else if(c >= '0' && c <= '9')
		{
			renderChar(c, curX, y, 8.0f, 8.0f);
			curX += charAdvance;
		}
		else
		{
			curX += 4.0f;
		}
	}
}

void HUD::render(int timeRemainingSeconds, int lives, int enemiesLeft, int maxBombs, int flameRadius)
{
	if(shaderProgram == NULL)
		return;

	// Establir projecció ortogràfica estàtica de pantalla completa (240x160)
	glm::mat4 hudProj = glm::ortho(0.0f, 240.0f, 160.0f, 0.0f);
	shaderProgram->setUniformMatrix4f("projection", hudProj);
	shaderProgram->setUniformMatrix4f("modelview", glm::mat4(1.0f));
	shaderProgram->setUniform2f("texCoordDispl", 0.0f, 0.0f);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// 1. Barra rosa estreta de 10px d'alçada: x=[0..240], y=[150..160] (amb 1px de marge superior i inferior)
	const float barY = 150.0f;
	const float barHeight = 10.0f;

	glBindTexture(GL_TEXTURE_2D, whiteTexId);
	shaderProgram->setUniform4f("color", barPinkColor.r, barPinkColor.g, barPinkColor.b, 1.0f);
	renderQuad(0.0f, barY, 240.0f, barHeight, 0.0f, 0.0f, 1.0f, 1.0f);

	// 2. Icones i text 8x8 (mida 1:1 nativa, centrats verticalment a y = 151)
	const float itemY = 151.0f;
	const float iconSize = 8.0f;
	const float charAdvance = 6.0f;
	const float iconTextGap = 2.0f;

	// Formatar el temps restant (ex: "3:35")
	int minutes = timeRemainingSeconds / 60;
	int seconds = timeRemainingSeconds % 60;
	if(minutes > 9) minutes = 9;
	if(minutes < 0) minutes = 0;
	if(seconds < 0) seconds = 0;
	char timeBuffer[16];
	snprintf(timeBuffer, sizeof(timeBuffer), "%d:%02d", minutes, seconds);

	// Grup 1: Rellotge + Temps (ex: 3:35)
	float g1X = 16.0f;
	renderIcon(HUD_ICON_CLOCK, g1X, itemY, iconSize);
	renderString(timeBuffer, g1X + iconSize + iconTextGap, itemY, charAdvance);

	// Grup 2: Vides (cap de bomberman en marc) + quantitat (ex: 2)
	char livesBuffer[16];
	snprintf(livesBuffer, sizeof(livesBuffer), "%d", std::max(0, lives));
	float g2X = 76.0f;
	renderIcon(HUD_ICON_LIVES, g2X, itemY, iconSize);
	renderString(livesBuffer, g2X + iconSize + iconTextGap, itemY, charAdvance);

	// Grup 3: Enemics restants ('T') + quantitat (ex: 4)
	char enemiesBuffer[16];
	snprintf(enemiesBuffer, sizeof(enemiesBuffer), "%d", std::max(0, enemiesLeft));
	float g3X = 120.0f;
	renderIcon(HUD_ICON_TARGET, g3X, itemY, iconSize);
	renderString(enemiesBuffer, g3X + iconSize + iconTextGap, itemY, charAdvance);

	// Grup 4: Bombes simultànies + quantitat (ex: 2)
	char bombsBuffer[16];
	snprintf(bombsBuffer, sizeof(bombsBuffer), "%d", std::max(1, maxBombs));
	float g4X = 162.0f;
	renderIcon(HUD_ICON_BOMB, g4X, itemY, iconSize);
	renderString(bombsBuffer, g4X + iconSize + iconTextGap, itemY, charAdvance);

	// Grup 5: Potència de foc / patí + quantitat (ex: 2)
	char flameBuffer[16];
	snprintf(flameBuffer, sizeof(flameBuffer), "%d", std::max(1, flameRadius));
	float g5X = 204.0f;
	renderIcon(HUD_ICON_POWER, g5X, itemY, iconSize);
	renderString(flameBuffer, g5X + iconSize + iconTextGap, itemY, charAdvance);

	glDisable(GL_BLEND);
}

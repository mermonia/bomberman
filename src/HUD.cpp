#include <iostream>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <vector>
#include <cmath>
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include "HUD.h"
#include "AssetPath.h"

// Font de mapa de bits 8x8 per a caràcters ASCII 32 (' ') fins a 90 ('Z') → 59 glifs
static const unsigned char FONT_8x8[59][8] = {
	// 32 ' '
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	// 33 '!'
	{0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00},
	// 34 '"'
	{0x6C,0x6C,0x00,0x00,0x00,0x00,0x00,0x00},
	// 35 '#'
	{0x6C,0xFE,0x6C,0x6C,0xFE,0x6C,0x00,0x00},
	// 36 '$'
	{0x18,0x7E,0xC0,0x7C,0x06,0xFC,0x18,0x00},
	// 37 '%'
	{0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00,0x00},
	// 38 '&'
	{0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00},
	// 39 '\''
	{0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00},
	// 40 '('
	{0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00},
	// 41 ')'
	{0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00},
	// 42 '*'
	{0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00},
	// 43 '+'
	{0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00},
	// 44 ','
	{0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30},
	// 45 '-'
	{0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00},
	// 46 '.'
	{0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00},
	// 47 '/'
	{0x06,0x0C,0x18,0x30,0x60,0xC0,0x00,0x00},
	// 48 '0'
	{0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00},
	// 49 '1'
	{0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00},
	// 50 '2'
	{0x3C,0x66,0x06,0x0C,0x18,0x30,0x7E,0x00},
	// 51 '3'
	{0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00},
	// 52 '4'
	{0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C,0x00},
	// 53 '5'
	{0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00},
	// 54 '6'
	{0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00},
	// 55 '7'
	{0x7E,0x06,0x0C,0x18,0x30,0x30,0x30,0x00},
	// 56 '8'
	{0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00},
	// 57 '9'
	{0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00},
	// 58 ':'
	{0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00},
	// 59 ';'
	{0x00,0x18,0x18,0x00,0x18,0x18,0x30,0x00},
	// 60 '<'
	{0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00},
	// 61 '='
	{0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00},
	// 62 '>'
	{0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00},
	// 63 '?'
	{0x3C,0x66,0x06,0x0C,0x18,0x00,0x18,0x00},
	// 64 '@'
	{0x3C,0x66,0x6E,0x6E,0x60,0x66,0x3C,0x00},
	// 65 'A'
	{0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00},
	// 66 'B'
	{0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00},
	// 67 'C'
	{0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00},
	// 68 'D'
	{0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00},
	// 69 'E'
	{0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00},
	// 70 'F'
	{0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00},
	// 71 'G'
	{0x3C,0x66,0x60,0x6E,0x66,0x66,0x3E,0x00},
	// 72 'H'
	{0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00},
	// 73 'I'
	{0x7E,0x18,0x18,0x18,0x18,0x18,0x7E,0x00},
	// 74 'J'
	{0x3E,0x0C,0x0C,0x0C,0x0C,0x6C,0x38,0x00},
	// 75 'K'
	{0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00},
	// 76 'L'
	{0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00},
	// 77 'M'
	{0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0x00},
	// 78 'N'
	{0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00},
	// 79 'O'
	{0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
	// 80 'P'
	{0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00},
	// 81 'Q'
	{0x3C,0x66,0x66,0x66,0x6A,0x6C,0x36,0x00},
	// 82 'R'
	{0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66,0x00},
	// 83 'S'
	{0x3C,0x66,0x70,0x3C,0x0E,0x66,0x3C,0x00},
	// 84 'T'
	{0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00},
	// 85 'U'
	{0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
	// 86 'V'
	{0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00},
	// 87 'W'
	{0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00},
	// 88 'X'
	{0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00},
	// 89 'Y'
	{0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00},
	// 90 'Z'
	{0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00},
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

	// 3. Crear textura d'atlas per a la font retro de 472x8 píxels (59 glifs de 8x8)
	const int GLYPH_COUNT = 59;
	const int AW = GLYPH_COUNT * 8;
	const int AH = 8;
	std::vector<unsigned char> fontData(AW * AH * 4, 0);

	for(int glyph = 0; glyph < GLYPH_COUNT; glyph++)
	{
		for(int row = 0; row < 8; row++)
		{
			unsigned char byte = FONT_8x8[glyph][row];
			for(int col = 0; col < 8; col++)
			{
				bool bitOn = (byte & (1 << (7 - col))) != 0;
				int px = glyph * 8 + col;
				int py = row;
				int index = (py * AW + px) * 4;
				if(bitOn)
				{
					fontData[index + 0] = 255;
					fontData[index + 1] = 255;
					fontData[index + 2] = 255;
					fontData[index + 3] = 255;
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, AW, AH, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontData.data());

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

void HUD::renderChar(char c, float x, float y, float width, float height, const glm::vec4 &color)
{
	if(c >= 'a' && c <= 'z')
		c = (char)(c - 32);

	int glyphIdx = 0; // ' ' per defecte
	if(c >= 32 && c <= 90)
		glyphIdx = c - 32;

	float u0 = (float)(glyphIdx * 8) / 472.0f;
	float u1 = (float)((glyphIdx + 1) * 8) / 472.0f;
	float v0 = 0.0f;
	float v1 = 1.0f;

	glBindTexture(GL_TEXTURE_2D, fontTexId);
	shaderProgram->setUniform4f("color", color.r, color.g, color.b, color.a);
	renderQuad(x, y, width, height, u0, v0, u1, v1);
}

void HUD::renderString(const std::string &str, float x, float y, float charAdvance, float width, float height, const glm::vec4 &color)
{
	float curX = x;
	for(size_t i = 0; i < str.size(); ++i)
	{
		char c = str[i];
		if(c == ':')
		{
			renderChar(':', curX, y, width, height, color);
			curX += 4.0f;
		}
		else if(c == ' ')
		{
			curX += charAdvance * 0.6f;
		}
		else
		{
			renderChar(c, curX, y, width, height, color);
			curX += charAdvance;
		}
	}
}

void HUD::renderExitPrompt(float animTime)
{
	// Animació suau de parpelleig/pulsació estil retro arcade
	float pulse = 0.5f + 0.5f * std::sin(animTime * 0.007f);

	// Rètol centrat a dalt de tot de la pantalla (x = 120, y = 8)
	const float bannerW = 68.0f;
	const float bannerH = 16.0f;
	const float bannerX = 120.0f - bannerW * 0.5f; // 86.0f
	const float bannerY = 8.0f;

	// 1. Caixa translúcida fosca de fons
	glBindTexture(GL_TEXTURE_2D, whiteTexId);
	shaderProgram->setUniform4f("color", 0.04f, 0.05f, 0.10f, 0.88f);
	renderQuad(bannerX, bannerY, bannerW, bannerH, 0.0f, 0.0f, 1.0f, 1.0f);

	// 2. Vora daurada pampalluguejant (1px)
	glm::vec4 borderCol(1.0f, 0.84f, 0.20f, 0.70f + 0.30f * pulse);
	shaderProgram->setUniform4f("color", borderCol.r, borderCol.g, borderCol.b, borderCol.a);
	renderQuad(bannerX, bannerY, bannerW, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	renderQuad(bannerX, bannerY + bannerH - 1.0f, bannerW, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	renderQuad(bannerX, bannerY, 1.0f, bannerH, 0.0f, 0.0f, 1.0f, 1.0f);
	renderQuad(bannerX + bannerW - 1.0f, bannerY, 1.0f, bannerH, 0.0f, 0.0f, 1.0f, 1.0f);

	// 3. Detall de cantonades brillants blanques (estil arcade vintage)
	shaderProgram->setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	renderQuad(bannerX, bannerY, 2.0f, 2.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	renderQuad(bannerX + bannerW - 2.0f, bannerY, 2.0f, 2.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	renderQuad(bannerX, bannerY + bannerH - 2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	renderQuad(bannerX + bannerW - 2.0f, bannerY + bannerH - 2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// 4. Paraula "EXIT" flanquejada per fletxes "> EXIT <" amb ombra fosca
	glm::vec4 textCol(1.0f, 0.85f + 0.15f * pulse, 0.25f + 0.75f * pulse, 1.0f);
	glm::vec4 arrowCol(1.0f, 0.40f + 0.40f * pulse, 0.20f, 0.90f);
	glm::vec4 shadowCol(0.0f, 0.0f, 0.0f, 0.85f);

	// Ombra de contrast
	renderChar('>', bannerX + 7.0f, bannerY + 5.0f, 8.0f, 8.0f, shadowCol);
	renderString("EXIT", bannerX + 19.0f, bannerY + 5.0f, 7.5f, 8.0f, 8.0f, shadowCol);
	renderChar('<', bannerX + bannerW - 13.0f, bannerY + 5.0f, 8.0f, 8.0f, shadowCol);

	// Caràcters principals brillants
	renderChar('>', bannerX + 6.0f, bannerY + 4.0f, 8.0f, 8.0f, arrowCol);
	renderString("EXIT", bannerX + 18.0f, bannerY + 4.0f, 7.5f, 8.0f, 8.0f, textCol);
	renderChar('<', bannerX + bannerW - 14.0f, bannerY + 4.0f, 8.0f, 8.0f, arrowCol);
}

void HUD::render(int timeRemainingSeconds, int lives, int enemiesLeft, int maxBombs, int flameRadius, bool bShowExit, float animTime)
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
	const glm::vec4 hudTextColor(30.f / 255.f, 42.f / 255.f, 30.f / 255.f, 1.0f);

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
	renderString(timeBuffer, g1X + iconSize + iconTextGap, itemY, charAdvance, 8.0f, 8.0f, hudTextColor);

	// Grup 2: Vides (cap de bomberman en marc) + quantitat (ex: 2)
	char livesBuffer[16];
	snprintf(livesBuffer, sizeof(livesBuffer), "%d", std::max(0, lives));
	float g2X = 76.0f;
	renderIcon(HUD_ICON_LIVES, g2X, itemY, iconSize);
	renderString(livesBuffer, g2X + iconSize + iconTextGap, itemY, charAdvance, 8.0f, 8.0f, hudTextColor);

	// Grup 3: Enemics restants ('T') + quantitat (ex: 4)
	char enemiesBuffer[16];
	snprintf(enemiesBuffer, sizeof(enemiesBuffer), "%d", std::max(0, enemiesLeft));
	float g3X = 120.0f;
	renderIcon(HUD_ICON_TARGET, g3X, itemY, iconSize);
	renderString(enemiesBuffer, g3X + iconSize + iconTextGap, itemY, charAdvance, 8.0f, 8.0f, hudTextColor);

	// Grup 4: Bombes simultànies + quantitat (ex: 2)
	char bombsBuffer[16];
	snprintf(bombsBuffer, sizeof(bombsBuffer), "%d", std::max(1, maxBombs));
	float g4X = 162.0f;
	renderIcon(HUD_ICON_BOMB, g4X, itemY, iconSize);
	renderString(bombsBuffer, g4X + iconSize + iconTextGap, itemY, charAdvance, 8.0f, 8.0f, hudTextColor);

	// Grup 5: Potència de foc / patí + quantitat (ex: 2)
	char flameBuffer[16];
	snprintf(flameBuffer, sizeof(flameBuffer), "%d", std::max(1, flameRadius));
	float g5X = 204.0f;
	renderIcon(HUD_ICON_POWER, g5X, itemY, iconSize);
	renderString(flameBuffer, g5X + iconSize + iconTextGap, itemY, charAdvance, 8.0f, 8.0f, hudTextColor);

	// 3. Sobreimprimir la paraula EXIT fins que el jugador arriba a la porta
	if(bShowExit)
	{
		renderExitPrompt(animTime);
	}

	glDisable(GL_BLEND);
}

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

HUD::HUD()
{
	shaderProgram = NULL;
	barPinkColor = glm::vec4(0.933f, 0.482f, 0.690f, 1.0f);
	texWidth = 24;
	texHeight = 16;
}

HUD::~HUD()
{
}

void HUD::init(ShaderProgram *program)
{
	shaderProgram = program;

	textureBarra.loadFromFile(assetPath("images/barra.png"), TEXTURE_PIXEL_FORMAT_RGBA);
	textureBarra.setWrapS(GL_CLAMP_TO_EDGE);
	textureBarra.setWrapT(GL_CLAMP_TO_EDGE);
	textureBarra.setMinFilter(GL_NEAREST);
	textureBarra.setMagFilter(GL_NEAREST);

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

	font.init();
	quads.init(*program);
}

void HUD::renderIcon(HUDIconType type, float x, float y, float size)
{
	int col = 0, row = 0;

	switch(type)
	{
	case HUD_ICON_CLOCK:
		col = 2; row = 1;
		break;
	case HUD_ICON_LIVES:
		col = 2; row = 0;
		break;
	case HUD_ICON_TARGET:
		col = 0; row = 1;
		break;
	case HUD_ICON_BOMB:
		col = 1; row = 0;
		break;
	case HUD_ICON_POWER:
		col = 0; row = 0;
		break;
	case HUD_ICON_HEART:
		col = 1; row = 1;
		break;
	}

	float tw = (texWidth > 0) ? (float)texWidth : 24.0f;
	float th = (texHeight > 0) ? (float)texHeight : 16.0f;

	float u0 = (float)(col * 8) / tw;
	float u1 = (float)((col + 1) * 8) / tw;
	float v0 = (float)(row * 8) / th;
	float v1 = (float)((row + 1) * 8) / th;

	glEnable(GL_TEXTURE_2D);
	textureBarra.use();
	shaderProgram->setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	quads.render(x, y, size, size, u0, v0, u1, v1);
}

void HUD::renderChar(char c, float x, float y, float width, float height, const glm::vec4 &color)
{
	float u0, u1;
	font.glyphCoords(c, u0, u1);

	font.use();
	shaderProgram->setUniform4f("color", color.r, color.g, color.b, color.a);
	quads.render(x, y, width, height, u0, 0.0f, u1, 1.0f);
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
	float pulse = 0.5f + 0.5f * std::sin(animTime * 0.007f);

	const float bannerW = 68.0f;
	const float bannerH = 16.0f;
	const float bannerX = 120.0f - bannerW * 0.5f;
	const float bannerY = 8.0f;

	shaderProgram->setUniform4f("color", 0.04f, 0.05f, 0.10f, 0.88f);
	quads.renderSolid(bannerX, bannerY, bannerW, bannerH);

	glm::vec4 borderCol(1.0f, 0.84f, 0.20f, 0.70f + 0.30f * pulse);
	shaderProgram->setUniform4f("color", borderCol.r, borderCol.g, borderCol.b, borderCol.a);
	quads.renderSolid(bannerX, bannerY, bannerW, 1.0f);
	quads.renderSolid(bannerX, bannerY + bannerH - 1.0f, bannerW, 1.0f);
	quads.renderSolid(bannerX, bannerY, 1.0f, bannerH);
	quads.renderSolid(bannerX + bannerW - 1.0f, bannerY, 1.0f, bannerH);

	shaderProgram->setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	quads.renderSolid(bannerX, bannerY, 2.0f, 2.0f);
	quads.renderSolid(bannerX + bannerW - 2.0f, bannerY, 2.0f, 2.0f);
	quads.renderSolid(bannerX, bannerY + bannerH - 2.0f, 2.0f, 2.0f);
	quads.renderSolid(bannerX + bannerW - 2.0f, bannerY + bannerH - 2.0f, 2.0f, 2.0f);

	glm::vec4 textCol(1.0f, 0.85f + 0.15f * pulse, 0.25f + 0.75f * pulse, 1.0f);
	glm::vec4 arrowCol(1.0f, 0.40f + 0.40f * pulse, 0.20f, 0.90f);
	glm::vec4 shadowCol(0.0f, 0.0f, 0.0f, 0.85f);

	renderChar('>', bannerX + 7.0f, bannerY + 5.0f, 8.0f, 8.0f, shadowCol);
	renderString("EXIT", bannerX + 19.0f, bannerY + 5.0f, 7.5f, 8.0f, 8.0f, shadowCol);
	renderChar('<', bannerX + bannerW - 13.0f, bannerY + 5.0f, 8.0f, 8.0f, shadowCol);

	renderChar('>', bannerX + 6.0f, bannerY + 4.0f, 8.0f, 8.0f, arrowCol);
	renderString("EXIT", bannerX + 18.0f, bannerY + 4.0f, 7.5f, 8.0f, 8.0f, textCol);
	renderChar('<', bannerX + bannerW - 14.0f, bannerY + 4.0f, 8.0f, 8.0f, arrowCol);
}

void HUD::render(int timeRemainingSeconds, int lives, int enemiesLeft, int maxBombs, int flameRadius, bool bHasHeart, bool bShowExit, float animTime, bool bGodMode)
{
	if(shaderProgram == NULL)
		return;

	glm::mat4 hudProj = glm::ortho(0.0f, 240.0f, 160.0f, 0.0f);
	shaderProgram->setUniformMatrix4f("projection", hudProj);
	glm::mat4 hudModelview = glm::mat4(1.0f);
	shaderProgram->setUniformMatrix4f("modelview", hudModelview);
	shaderProgram->setUniform2f("texCoordDispl", 0.0f, 0.0f);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const float barY = 150.0f;
	const float barHeight = 10.0f;

	shaderProgram->setUniform4f("color", barPinkColor.r, barPinkColor.g, barPinkColor.b, 1.0f);
	quads.renderSolid(0.0f, barY, 240.0f, barHeight);

	const float itemY = 151.0f;
	const float iconSize = 8.0f;
	const float charAdvance = 6.0f;
	const float iconTextGap = 2.0f;
	const glm::vec4 hudTextColor(30.f / 255.f, 42.f / 255.f, 30.f / 255.f, 1.0f);

	int minutes = timeRemainingSeconds / 60;
	int seconds = timeRemainingSeconds % 60;
	if(minutes > 9) minutes = 9;
	if(minutes < 0) minutes = 0;
	if(seconds < 0) seconds = 0;
	char timeBuffer[16];
	snprintf(timeBuffer, sizeof(timeBuffer), "%d:%02d", minutes, seconds);

	float g1X = 16.0f;
	renderIcon(HUD_ICON_CLOCK, g1X, itemY, iconSize);
	renderString(timeBuffer, g1X + iconSize + iconTextGap, itemY, charAdvance, 8.0f, 8.0f, hudTextColor);

	if(bGodMode)
	{
		float godX = g1X + iconSize + iconTextGap + float(strlen(timeBuffer)) * charAdvance + 4.0f;
		renderString("GOD", godX, itemY, charAdvance, 8.0f, 8.0f, hudTextColor);
	}

	char livesBuffer[16];
	snprintf(livesBuffer, sizeof(livesBuffer), "%d", std::max(0, lives));
	float g2X = 76.0f;
	renderIcon(HUD_ICON_LIVES, g2X, itemY, iconSize);
	renderString(livesBuffer, g2X + iconSize + iconTextGap, itemY, charAdvance, 8.0f, 8.0f, hudTextColor);

	if(bHasHeart)
	{
		float heartX = g2X + iconSize + iconTextGap + float(strlen(livesBuffer)) * charAdvance + 3.0f;
		renderIcon(HUD_ICON_HEART, heartX, itemY, iconSize);
	}

	char enemiesBuffer[16];
	snprintf(enemiesBuffer, sizeof(enemiesBuffer), "%d", std::max(0, enemiesLeft));
	float g3X = 120.0f;
	renderIcon(HUD_ICON_TARGET, g3X, itemY, iconSize);
	renderString(enemiesBuffer, g3X + iconSize + iconTextGap, itemY, charAdvance, 8.0f, 8.0f, hudTextColor);

	char bombsBuffer[16];
	snprintf(bombsBuffer, sizeof(bombsBuffer), "%d", std::max(1, maxBombs));
	float g4X = 162.0f;
	renderIcon(HUD_ICON_BOMB, g4X, itemY, iconSize);
	renderString(bombsBuffer, g4X + iconSize + iconTextGap, itemY, charAdvance, 8.0f, 8.0f, hudTextColor);

	char flameBuffer[16];
	snprintf(flameBuffer, sizeof(flameBuffer), "%d", std::max(1, flameRadius));
	float g5X = 204.0f;
	renderIcon(HUD_ICON_POWER, g5X, itemY, iconSize);
	renderString(flameBuffer, g5X + iconSize + iconTextGap, itemY, charAdvance, 8.0f, 8.0f, hudTextColor);

	if(bShowExit)
	{
		renderExitPrompt(animTime);
	}

	glDisable(GL_BLEND);
}

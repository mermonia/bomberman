#ifndef _HUD_INCLUDE
#define _HUD_INCLUDE

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Texture.h"
#include "ShaderProgram.h"

enum HUDIconType
{
	HUD_ICON_CLOCK = 0,
	HUD_ICON_LIVES,
	HUD_ICON_TARGET,
	HUD_ICON_BOMB,
	HUD_ICON_POWER,
	HUD_ICON_HEART
};

class HUD
{
public:
	HUD();
	~HUD();

	void init(ShaderProgram *program);
	void render(int timeRemainingSeconds, int lives, int enemiesLeft, int maxBombs, int flameRadius, bool bHasHeart = false, bool bShowExit = false, float animTime = 0.0f);

private:
	void renderQuad(float x, float y, float width, float height, float u0, float v0, float u1, float v1);
	void renderIcon(HUDIconType type, float x, float y, float size = 8.0f);
	void renderChar(char c, float x, float y, float width = 8.0f, float height = 8.0f, const glm::vec4 &color = glm::vec4(1.0f));
	void renderString(const std::string &str, float x, float y, float charAdvance = 6.0f, float width = 8.0f, float height = 8.0f, const glm::vec4 &color = glm::vec4(1.0f));
	void renderExitPrompt(float animTime);

private:
	ShaderProgram *shaderProgram;
	Texture textureBarra;
	GLuint whiteTexId;
	GLuint fontTexId;
	GLuint vao;
	GLuint vbo;
	GLint posLocation;
	GLint texCoordLocation;
	glm::vec4 barPinkColor;
	int texWidth;
	int texHeight;
};

#endif // _HUD_INCLUDE

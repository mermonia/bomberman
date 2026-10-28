#ifndef _MAINMENU_INCLUDE
#define _MAINMENU_INCLUDE

#include <GL/glew.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "ShaderProgram.h"

enum GameState
{
	STATE_MENU = 0,
	STATE_PLAY,
	STATE_INSTRUCTIONS,
	STATE_CREDITS,
	STATE_GAME_OVER
};

enum MenuAction
{
	MENU_NONE = 0,
	MENU_PLAY,
	MENU_INSTRUCTIONS,
	MENU_CREDITS,
	MENU_QUIT
};

struct SpriteRect
{
	int x, y, w, h;
};

class MainMenu
{
public:
	MainMenu();
	~MainMenu();

	void init();
	void update(int deltaTime);
	void render(GameState screenState);
	MenuAction handleKey(int key);
	void setGameInProgress(bool inProgress) { bGameInProgress = inProgress; }

	void startGameOver();
	bool isGameOverFinished() const { return bGameOverDone; }

private:
	void initShaders();
	void loadSpriteSheet();

	void renderQuad(float x, float y, float w, float h,
	                float u0, float v0, float u1, float v1);
	void renderSolidQuad(float x, float y, float w, float h, const glm::vec4 &col);

	void renderChar(char c, float x, float y, float scale, const glm::vec4 &col);
	void renderString(const std::string &s, float x, float y,
	                  float scale, float spacing, const glm::vec4 &col);
	float stringPixelWidth(const std::string &s, float scale, float spacing);
	void renderStringCentered(const std::string &s, float cx, float y,
	                          float scale, float spacing, const glm::vec4 &col);

	void drawMenuScreen();
	void drawInstructionsScreen();
	void drawCreditsScreen();
	void drawGameOverScreen();

	void renderRegion(int srcX, int srcY, int srcW, int srcH,
	                  float dstX, float dstY, float dstW, float dstH);

private:
	ShaderProgram texProgram;
	GLuint menuTexId;
	GLuint whiteTexId;
	GLuint fontTexId;

	GLuint vao, vbo;
	GLint posLoc, texLoc;

	int sheetW, sheetH;

	int selectedOption;
	int animTimer;
	bool bGameInProgress;
	int gameOverTimer;
	bool bGameOverDone;

	SpriteRect logoRect;
	SpriteRect treeRect;
	SpriteRect copyrightRect;
	SpriteRect hudsonRect;
	SpriteRect gameOverRect;
	SpriteRect fallenBombermanRect;

	struct Dot { float x, y, speed, phase; };
	std::vector<Dot> dots;
};

#endif

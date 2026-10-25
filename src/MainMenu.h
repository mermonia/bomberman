#ifndef _MAINMENU_INCLUDE
#define _MAINMENU_INCLUDE

#include <GL/glew.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "ShaderProgram.h"

// Game states managed by Game via MainMenu
enum GameState
{
	STATE_MENU = 0,
	STATE_PLAY,
	STATE_INSTRUCTIONS,
	STATE_CREDITS,
	STATE_GAME_OVER
};

// What the menu selection resolved to
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

	// Low-level drawing helpers
	void renderQuad(float x, float y, float w, float h,
	                float u0, float v0, float u1, float v1);
	void renderSolidQuad(float x, float y, float w, float h, const glm::vec4 &col);

	// Font helpers (built-in 8×8 bitmap font, A-Z 0-9 and basic punctuation)
	void renderChar(char c, float x, float y, float scale, const glm::vec4 &col);
	void renderString(const std::string &s, float x, float y,
	                  float scale, float spacing, const glm::vec4 &col);
	float stringPixelWidth(const std::string &s, float scale, float spacing);
	void renderStringCentered(const std::string &s, float cx, float y,
	                          float scale, float spacing, const glm::vec4 &col);

	// Screen-specific rendering
	void drawMenuScreen();
	void drawInstructionsScreen();
	void drawCreditsScreen();
	void drawGameOverScreen();

	// Sprite-region helper: renders a rectangle from the sprite-sheet
	void renderRegion(int srcX, int srcY, int srcW, int srcH,
	                  float dstX, float dstY, float dstW, float dstH);

private:
	ShaderProgram texProgram;
	GLuint menuTexId;   // menus_color.png with green removed
	GLuint whiteTexId;  // 1×1 white pixel
	GLuint fontTexId;   // bitmap font atlas

	GLuint vao, vbo;
	GLint posLoc, texLoc;

	int sheetW, sheetH; // pixel dimensions of menu sprite sheet

	int selectedOption;
	int animTimer;       // ms counter for animations
	bool bGameInProgress;
	int gameOverTimer;
	bool bGameOverDone;

	SpriteRect logoRect;
	SpriteRect treeRect;
	SpriteRect copyrightRect;
	SpriteRect hudsonRect;
	SpriteRect gameOverRect;
	SpriteRect fallenBombermanRect;

	// Little twinkling dots in the background
	struct Dot { float x, y, speed, phase; };
	std::vector<Dot> dots;
};


#endif // _MAINMENU_INCLUDE

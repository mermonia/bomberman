#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Scene.h"
#include "MainMenu.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define WINDOW_WIDTH 1440
#define WINDOW_HEIGHT 960

#define FADE_OUT_TIME 200
#define FADE_IN_TIME 200
#define FADE_STEPS 4

enum TransitionPhase
{
	FADE_NONE = 0,
	FADE_OUT,
	FADE_IN
};

class Game
{

private:
	Game() : bPlay(true), state(STATE_MENU), bSceneInited(false), fadePhase(FADE_NONE), fadeTimer(0), pendingState(STATE_MENU), bPendingNewGame(false), bPendingResetScene(false), bPendingGameOver(false), bPendingLevel(false), fadeVao(0), fadeVbo(0), fadePosLocation(-1) {}

public:
	static Game &instance()
	{
		static Game G;

		return G;
	}

	void init();
	bool update(int deltaTime);
	void render();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMove(int x, int y);
	void mousePress(int button);
	void mouseRelease(int button);

	bool getKey(int key) const;

	GameState getState() const { return state; }
	void changeState(GameState target);

private:
	void initFade();
	void updateFade(int deltaTime);
	void applyTransition();
	void renderFade();

private:
	bool bPlay;
	bool keys[GLFW_KEY_LAST+1];
	GameState state;
	bool bSceneInited;
	Scene scene;
	MainMenu mainMenu;

	TransitionPhase fadePhase;
	int fadeTimer;
	GameState pendingState;
	bool bPendingNewGame;
	bool bPendingResetScene;
	bool bPendingGameOver;
	bool bPendingLevel;
	ShaderProgram fadeProgram;
	GLuint fadeVao;
	GLuint fadeVbo;
	GLint fadePosLocation;

};

#endif


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"


void Game::init()
{
	bPlay = true;
	state = STATE_MENU;
	bSceneInited = false;

	for(int i = 0; i <= GLFW_KEY_LAST; i++)
		keys[i] = false;

	glClearColor(0.04f, 0.05f, 0.10f, 1.0f);

	mainMenu.init();
	// scene.init() is deferred until the player presses JUGAR
}

bool Game::update(int deltaTime)
{
	switch(state)
	{
	case STATE_MENU:
	case STATE_INSTRUCTIONS:
	case STATE_CREDITS:
		mainMenu.setGameInProgress(bSceneInited);
		mainMenu.update(deltaTime);
		break;
	case STATE_PLAY:
		scene.update(deltaTime);
		if(scene.isGameOver())
		{
			mainMenu.startGameOver();
			state = STATE_GAME_OVER;
		}
		else if(scene.isGameWon())
		{
			bSceneInited = false;
			state = STATE_MENU;
		}
		break;
	case STATE_GAME_OVER:
		mainMenu.update(deltaTime);
		if(mainMenu.isGameOverFinished())
		{
			bSceneInited = false;
			state = STATE_MENU;
		}
		break;
	}

	return bPlay;
}

void Game::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch(state)
	{
	case STATE_MENU:
	case STATE_INSTRUCTIONS:
	case STATE_CREDITS:
	case STATE_GAME_OVER:
		mainMenu.render(state);
		break;
	case STATE_PLAY:
		scene.render();
		break;
	}
}

void Game::keyPressed(int key)
{
	if(key < 0 || key > GLFW_KEY_LAST)
		return;
	keys[key] = true;

	switch(state)
	{
	case STATE_MENU:
	{
		MenuAction action = mainMenu.handleKey(key);
		if(action == MENU_PLAY)
		{
			if(!bSceneInited)
			{
				scene.init();
				bSceneInited = true;
			}
			state = STATE_PLAY;
		}
		else if(action == MENU_INSTRUCTIONS)
			state = STATE_INSTRUCTIONS;
		else if(action == MENU_CREDITS)
			state = STATE_CREDITS;
		else if(action == MENU_QUIT || key == GLFW_KEY_ESCAPE)
			bPlay = false;
		break;
	}
	case STATE_PLAY:
		if(key == GLFW_KEY_ESCAPE)
			state = STATE_MENU;
		break;
	case STATE_INSTRUCTIONS:
	case STATE_CREDITS:
		if(key == GLFW_KEY_ESCAPE || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
			state = STATE_MENU;
		break;
	case STATE_GAME_OVER:
		if(key == GLFW_KEY_ESCAPE || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
		{
			bSceneInited = false;
			state = STATE_MENU;
		}
		break;
	}
}

void Game::keyReleased(int key)
{
	if(key >= 0 && key <= GLFW_KEY_LAST)
		keys[key] = false;
}

void Game::mouseMove(int x, int y)
{
}

void Game::mousePress(int button)
{
}

void Game::mouseRelease(int button)
{
}

bool Game::getKey(int key) const
{
	if(key < 0 || key > GLFW_KEY_LAST)
		return false;
	return keys[key];
}


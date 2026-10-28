#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Game.h"
#include "AssetPath.h"
#include "AudioManager.h"

void Game::init()
{
	bPlay = true;
	state = STATE_MENU;
	bSceneInited = false;

	for(int i = 0; i <= GLFW_KEY_LAST; i++)
		keys[i] = false;

	glClearColor(0.04f, 0.05f, 0.10f, 1.0f);

	mainMenu.init();

	initFade();

	AudioManager::instance().init();
	AudioManager::instance().playMusic(MUSIC_TITLE);
}

bool Game::update(int deltaTime)
{
	AudioManager::instance().update();

	if(fadePhase != FADE_NONE)
	{
		updateFade(deltaTime);
		return bPlay;
	}

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
			bPendingGameOver = true;
			changeState(STATE_GAME_OVER);
		}
		else if(scene.isGameWon())
		{
			bPendingResetScene = true;
			changeState(STATE_MENU);
		}
		else if(scene.hasPendingLevel())
		{
			bPendingLevel = true;
			changeState(STATE_PLAY);
		}
		break;
	case STATE_GAME_OVER:
		mainMenu.update(deltaTime);
		if(mainMenu.isGameOverFinished())
		{
			bPendingResetScene = true;
			changeState(STATE_MENU);
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

	if(fadePhase != FADE_NONE)
		renderFade();
}

void Game::keyPressed(int key)
{
	if(key < 0 || key > GLFW_KEY_LAST)
		return;
	keys[key] = true;

	if(fadePhase != FADE_NONE)
		return;

	switch(state)
	{
	case STATE_MENU:
	{
		MenuAction action = mainMenu.handleKey(key);
		if(action == MENU_PLAY)
		{
			if(!bSceneInited)
				bPendingNewGame = true;
			changeState(STATE_PLAY);
		}
		else if(action == MENU_INSTRUCTIONS)
			changeState(STATE_INSTRUCTIONS);
		else if(action == MENU_CREDITS)
			changeState(STATE_CREDITS);
		else if(action == MENU_QUIT || key == GLFW_KEY_ESCAPE)
			bPlay = false;
		break;
	}
	case STATE_PLAY:
		if(key == GLFW_KEY_ESCAPE)
			changeState(STATE_MENU);
		break;
	case STATE_INSTRUCTIONS:
	case STATE_CREDITS:
		if(key == GLFW_KEY_ESCAPE || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
			changeState(STATE_MENU);
		break;
	case STATE_GAME_OVER:
		if(key == GLFW_KEY_ESCAPE || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
		{
			bPendingResetScene = true;
			changeState(STATE_MENU);
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
	if(fadePhase != FADE_NONE)
		return false;
	return keys[key];
}

void Game::changeState(GameState target)
{
	if(fadePhase != FADE_NONE)
		return;
	pendingState = target;
	fadePhase = FADE_OUT;
	fadeTimer = 0;
}

void Game::initFade()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, assetPath("shaders/simple.vert"));
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, assetPath("shaders/simple.frag"));
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	fadeProgram.init();
	fadeProgram.addShader(vShader);
	fadeProgram.addShader(fShader);
	fadeProgram.link();
	if(!fadeProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << fadeProgram.log() << endl << endl;
	}
	fadeProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();

	float vertices[12] = {
		0.f, 0.f, 1.f, 0.f, 1.f, 1.f,
		0.f, 0.f, 1.f, 1.f, 0.f, 1.f
	};

	glGenVertexArrays(1, &fadeVao);
	glBindVertexArray(fadeVao);
	glGenBuffers(1, &fadeVbo);
	glBindBuffer(GL_ARRAY_BUFFER, fadeVbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertices, GL_STATIC_DRAW);
	fadePosLocation = fadeProgram.bindVertexAttribute("position", 2, 2 * sizeof(float), 0);
	glBindVertexArray(0);
}

void Game::updateFade(int deltaTime)
{
	int phaseTime = (fadePhase == FADE_OUT) ? FADE_OUT_TIME : FADE_IN_TIME;
	int step = phaseTime / FADE_STEPS;

	if(deltaTime > step)
		deltaTime = step;
	if(deltaTime > phaseTime - fadeTimer)
		deltaTime = phaseTime - fadeTimer;
	fadeTimer += deltaTime;

	if(fadeTimer < phaseTime)
		return;

	if(fadePhase == FADE_OUT)
	{
		applyTransition();
		fadePhase = FADE_IN;
		fadeTimer = 0;
	}
	else
	{
		fadePhase = FADE_NONE;
		fadeTimer = 0;
	}
}

void Game::applyTransition()
{
	if(bPendingNewGame)
	{
		scene.init();
		bSceneInited = true;
	}
	if(bPendingResetScene)
		bSceneInited = false;
	if(bPendingGameOver)
		mainMenu.startGameOver();
	if(bPendingLevel)
		scene.applyPendingLevel();

	bPendingNewGame = false;
	bPendingResetScene = false;
	bPendingGameOver = false;
	bPendingLevel = false;

	mainMenu.setGameInProgress(bSceneInited);
	if(state == STATE_GAME_OVER && pendingState != STATE_GAME_OVER)
		AudioManager::instance().stopMusic();
	state = pendingState;

	if(state == STATE_PLAY)
		AudioManager::instance().playMusic(AudioManager::musicForLevel(scene.getCurrentLevel()));
	else if(state == STATE_GAME_OVER)
		AudioManager::instance().playMusic(JINGLE_GAME_OVER);
	else
		AudioManager::instance().playMusic(MUSIC_TITLE);
}

void Game::renderFade()
{
	float alpha;
	if(fadePhase == FADE_OUT)
		alpha = float((fadeTimer * FADE_STEPS + FADE_OUT_TIME - 1) / FADE_OUT_TIME) / float(FADE_STEPS);
	else
		alpha = 1.0f - float((fadeTimer * FADE_STEPS) / FADE_IN_TIME) / float(FADE_STEPS);

	glm::mat4 projection = glm::ortho(0.f, 1.f, 1.f, 0.f);
	glm::mat4 modelview = glm::mat4(1.0f);

	fadeProgram.use();
	fadeProgram.setUniformMatrix4f("projection", projection);
	fadeProgram.setUniformMatrix4f("modelview", modelview);
	fadeProgram.setUniform4f("color", 0.0f, 0.0f, 0.0f, alpha);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(fadeVao);
	glEnableVertexAttribArray(fadePosLocation);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
}

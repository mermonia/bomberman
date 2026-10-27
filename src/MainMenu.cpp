#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include "MainMenu.h"
#include "AssetPath.h"

// ─── Built-in 8×8 bitmap font ────────────────────────────────────────────────
// Covers ASCII 32 (' ') through 90 ('Z') → 59 glyphs.
// Each glyph is 8 rows of 8 bits (MSB = leftmost pixel).
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

static char toFontChar(char c)
{
	if(c >= 'a' && c <= 'z') return (char)(c - 32);
	if(c >= 32 && c <= 90)   return c;
	return ' ';
}


// ─── Constructor / Destructor ────────────────────────────────────────────────

MainMenu::MainMenu()
	: menuTexId(0), whiteTexId(0), fontTexId(0),
	  vao(0), vbo(0), posLoc(0), texLoc(0),
	  sheetW(0), sheetH(0),
	  selectedOption(0), animTimer(0), bGameInProgress(false),
	  gameOverTimer(0), bGameOverDone(false),
	  logoRect{26, 95, 115, 98},
	  treeRect{735, 18, 155, 126},
	  copyrightRect{12, 215, 145, 30},
	  hudsonRect{25, 470, 130, 130},
	  gameOverRect{10, 10, 175, 75},
	  fallenBombermanRect{560, 415, 52, 32}
{
}

static SpriteRect findContiguousBlock(const unsigned char *data, int W, int H,
                                      int x0, int y0, int x1, int y1_max,
                                      int kr, int kg, int kb)
{
	int startY = -1;
	int endY = -1;
	int minX = x1;
	int maxX = x0;

	for(int y = y0; y <= y1_max && y < H; y++)
	{
		int nonBgInRow = 0;
		int rowMinX = x1, rowMaxX = x0;

		for(int x = x0; x <= x1 && x < W; x++)
		{
			int idx = (y * W + x) * 4;
			int r = data[idx + 0], g = data[idx + 1], b = data[idx + 2], a = data[idx + 3];
			bool isBg = (a == 0) || (std::abs(r - kr) <= 12 && std::abs(g - kg) <= 12 && std::abs(b - kb) <= 12);
			if(!isBg)
			{
				nonBgInRow++;
				if(x < rowMinX) rowMinX = x;
				if(x > rowMaxX) rowMaxX = x;
			}
		}

		if(nonBgInRow > 5)
		{
			if(startY == -1) startY = y;
			endY = y;
			if(rowMinX < minX) minX = rowMinX;
			if(rowMaxX > maxX) maxX = rowMaxX;
		}
		else if(startY != -1)
		{
			// Reached empty background row under the illustration
			break;
		}
	}

	if(startY == -1 || minX > maxX)
		return {x0, y0, x1 - x0 + 1, 140};

	return {minX, startY, maxX - minX + 1, endY - startY + 1};
}

static SpriteRect findBounds(const unsigned char *data, int W, int H,
                                 int x0, int y0, int x1, int y1,
                                 int kr, int kg, int kb)
{
	int minX = x1, maxX = x0, minY = y1, maxY = y0;
	for(int y = y0; y <= y1 && y < H; y++)
	{
		for(int x = x0; x <= x1 && x < W; x++)
		{
			int idx = (y * W + x) * 4;
			int r = data[idx + 0], g = data[idx + 1], b = data[idx + 2], a = data[idx + 3];
			if(a > 0 && !(std::abs(r - kr) <= 12 && std::abs(g - kg) <= 12 && std::abs(b - kb) <= 12))
			{
				if(x < minX) minX = x;
				if(x > maxX) maxX = x;
				if(y < minY) minY = y;
				if(y > maxY) maxY = y;
			}
		}
	}
	if(minX > maxX || minY > maxY)
		return {x0, y0, x1 - x0 + 1, y1 - y0 + 1};
	return {minX, minY, maxX - minX + 1, maxY - minY + 1};
}

static SpriteRect findSpriteBetweenBlankLines(const unsigned char *data, int W, int H,
                                              int x0, int y0, int x1, int y1_max,
                                              int kr, int kg, int kb)
{
	int startY = -1;
	int endY = -1;
	int minX = x1;
	int maxX = x0;

	for(int y = y0; y <= y1_max && y < H; y++)
	{
		int nonBgInRow = 0;
		int rowMinX = x1, rowMaxX = x0;

		for(int x = x0; x <= x1 && x < W; x++)
		{
			int idx = (y * W + x) * 4;
			int r = data[idx + 0], g = data[idx + 1], b = data[idx + 2], a = data[idx + 3];
			bool isBg = (a == 0) || (std::abs(r - kr) <= 12 && std::abs(g - kg) <= 12 && std::abs(b - kb) <= 12);
			if(!isBg)
			{
				nonBgInRow++;
				if(x < rowMinX) rowMinX = x;
				if(x > rowMaxX) rowMaxX = x;
			}
		}

		if(nonBgInRow > 0)
		{
			if(startY == -1) startY = y;
			endY = y;
			if(rowMinX < minX) minX = rowMinX;
			if(rowMaxX > maxX) maxX = rowMaxX;
		}
		else if(startY != -1)
		{
			// Reached pure background row under the sprite
			break;
		}
	}

	if(startY == -1 || minX > maxX)
		return {x0, y0, x1 - x0 + 1, y1_max - y0 + 1};

	return {minX, startY, maxX - minX + 1, endY - startY + 1};
}

MainMenu::~MainMenu()
{
	if(menuTexId)  glDeleteTextures(1, &menuTexId);
	if(whiteTexId) glDeleteTextures(1, &whiteTexId);
	if(fontTexId)  glDeleteTextures(1, &fontTexId);
	if(vbo) glDeleteBuffers(1, &vbo);
	if(vao) glDeleteVertexArrays(1, &vao);
	texProgram.free();
}


// ─── Initialization ──────────────────────────────────────────────────────────

void MainMenu::init()
{
	initShaders();

	// 1×1 white texture for solid-colour quads
	unsigned char white[4] = {255, 255, 255, 255};
	glGenTextures(1, &whiteTexId);
	glBindTexture(GL_TEXTURE_2D, whiteTexId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);

	// VAO / VBO (6 vertices × 4 floats = one dynamic quad)
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

	posLoc = texProgram.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	texLoc = texProgram.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(posLoc);
	glEnableVertexAttribArray(texLoc);
	glBindVertexArray(0);

	// Build font texture atlas: 59 glyphs × 8px = 472 × 8 RGBA
	const int GLYPH_COUNT = 59;
	const int AW = GLYPH_COUNT * 8;
	const int AH = 8;
	std::vector<unsigned char> atlas(AW * AH * 4, 0);
	for(int g = 0; g < GLYPH_COUNT; g++)
	{
		for(int row = 0; row < 8; row++)
		{
			unsigned char byte = FONT_8x8[g][row];
			for(int col = 0; col < 8; col++)
			{
				bool on = (byte & (0x80 >> col)) != 0;
				int idx = ((row * AW) + (g * 8 + col)) * 4;
				atlas[idx + 0] = 255;
				atlas[idx + 1] = 255;
				atlas[idx + 2] = 255;
				atlas[idx + 3] = on ? 255 : 0;
			}
		}
	}
	glGenTextures(1, &fontTexId);
	glBindTexture(GL_TEXTURE_2D, fontTexId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, AW, AH, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas.data());

	loadSpriteSheet();

	// Background stars / dots
	srand((unsigned)time(NULL));
	dots.clear();
	for(int i = 0; i < 35; i++)
	{
		Dot d;
		d.x = float(rand() % 240);
		d.y = float(rand() % 160);
		d.speed = 0.006f + float(rand() % 20) / 1200.0f;
		d.phase = float(rand() % 628) / 100.0f;
		dots.push_back(d);
	}
}

void MainMenu::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, assetPath("shaders/texture.vert"));
	if(!vShader.isCompiled())
	{
		std::cerr << "MainMenu vertex shader error" << std::endl;
		std::cerr << vShader.log() << std::endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, assetPath("shaders/texture.frag"));
	if(!fShader.isCompiled())
	{
		std::cerr << "MainMenu fragment shader error" << std::endl;
		std::cerr << fShader.log() << std::endl;
	}
	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if(!texProgram.isLinked())
	{
		std::cerr << "MainMenu shader link error" << std::endl;
		std::cerr << texProgram.log() << std::endl;
	}
	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}

void MainMenu::loadSpriteSheet()
{
	std::string paths[] = {
		assetPath("images/example_levels/menus_color.png"),
		assetPath("images/menus_color.png"),
		"assets/images/example_levels/menus_color.png",
		"assets/images/menus_color.png",
		"../assets/images/example_levels/menus_color.png"
	};

	int ch = 0;
	unsigned char *data = NULL;
	for(const std::string &p : paths)
	{
		data = stbi_load(p.c_str(), &sheetW, &sheetH, &ch, 4);
		if(data) break;
	}

	if(!data)
	{
		std::cerr << "MainMenu: cannot load menus_color.png" << std::endl;
		sheetW = sheetH = 0;
		return;
	}

	int kr = data[0], kg = data[1], kb = data[2];

	// Detect sprite boundaries from un-cleared sprite sheet:
	// - logoRect: captures POCKET BOMBERMAN and (C) HUDSON SOFT, stopping before purple box (y=196)
	logoRect = findBounds(data, sheetW, sheetH, 10, 95, 175, 196, kr, kg, kb);
	treeRect = findContiguousBlock(data, sheetW, sheetH, 730, 15, 920, 175, kr, kg, kb);
	hudsonRect = findBounds(data, sheetW, sheetH, 10, 460, 175, 610, kr, kg, kb);
	gameOverRect = findBounds(data, sheetW, sheetH, 5, 5, 195, 92, kr, kg, kb);
	fallenBombermanRect = findSpriteBetweenBlankLines(data, sheetW, sheetH, 540, 408, 645, 472, kr, kg, kb);

	// Chroma-key: remove the green background.
	int total = sheetW * sheetH;
	for(int i = 0; i < total; i++)
	{
		int r = data[i * 4 + 0], g = data[i * 4 + 1], b = data[i * 4 + 2];
		if(std::abs(r - kr) <= 12 && std::abs(g - kg) <= 12 && std::abs(b - kb) <= 12)
			data[i * 4 + 3] = 0;
	}

	glGenTextures(1, &menuTexId);
	glBindTexture(GL_TEXTURE_2D, menuTexId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sheetW, sheetH, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);
}


// ─── Update ──────────────────────────────────────────────────────────────────

void MainMenu::startGameOver()
{
	gameOverTimer = 0;
	bGameOverDone = false;
}

void MainMenu::update(int deltaTime)
{
	animTimer += deltaTime;
	gameOverTimer += deltaTime;
	if(gameOverTimer >= 6500)
		bGameOverDone = true;

	for(size_t i = 0; i < dots.size(); i++)
	{
		dots[i].y += dots[i].speed * float(deltaTime);
		if(dots[i].y > 160.0f)
		{
			dots[i].y = -2.0f;
			dots[i].x = float(rand() % 240);
		}
	}
}


// ─── Input ───────────────────────────────────────────────────────────────────

MenuAction MainMenu::handleKey(int key)
{
	if(key == GLFW_KEY_UP || key == GLFW_KEY_W)
	{
		selectedOption = (selectedOption + 3) % 4;
		return MENU_NONE;
	}
	if(key == GLFW_KEY_DOWN || key == GLFW_KEY_S)
	{
		selectedOption = (selectedOption + 1) % 4;
		return MENU_NONE;
	}
	if(key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
	{
		switch(selectedOption)
		{
		case 0: return MENU_PLAY;
		case 1: return MENU_INSTRUCTIONS;
		case 2: return MENU_CREDITS;
		case 3: return MENU_QUIT;
		}
	}
	return MENU_NONE;
}


// ─── Low-level drawing ──────────────────────────────────────────────────────

void MainMenu::renderQuad(float x, float y, float w, float h,
                           float u0, float v0, float u1, float v1)
{
	float verts[24] = {
		x,     y,     u0, v0,
		x + w, y,     u1, v0,
		x + w, y + h, u1, v1,
		x,     y,     u0, v0,
		x + w, y + h, u1, v1,
		x,     y + h, u0, v1,
	};
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
	glEnableVertexAttribArray(posLoc);
	glEnableVertexAttribArray(texLoc);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void MainMenu::renderSolidQuad(float x, float y, float w, float h, const glm::vec4 &col)
{
	glBindTexture(GL_TEXTURE_2D, whiteTexId);
	texProgram.setUniform4f("color", col.r, col.g, col.b, col.a);
	renderQuad(x, y, w, h, 0, 0, 1, 1);
}

void MainMenu::renderRegion(int srcX, int srcY, int srcW, int srcH,
                             float dstX, float dstY, float dstW, float dstH)
{
	if(menuTexId == 0 || sheetW <= 0 || sheetH <= 0) return;
	glBindTexture(GL_TEXTURE_2D, menuTexId);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	float u0 = float(srcX) / float(sheetW);
	float v0 = float(srcY) / float(sheetH);
	float u1 = float(srcX + srcW) / float(sheetW);
	float v1 = float(srcY + srcH) / float(sheetH);
	renderQuad(dstX, dstY, dstW, dstH, u0, v0, u1, v1);
}


// ─── Font rendering ─────────────────────────────────────────────────────────

void MainMenu::renderChar(char c, float x, float y, float scale, const glm::vec4 &col)
{
	char fc = toFontChar(c);
	int idx = fc - 32;
	if(idx < 0 || idx >= 59) idx = 0;

	const float TOTAL = 59.0f;
	float u0 = float(idx) / TOTAL;
	float u1 = float(idx + 1) / TOTAL;

	glBindTexture(GL_TEXTURE_2D, fontTexId);
	texProgram.setUniform4f("color", col.r, col.g, col.b, col.a);
	renderQuad(x, y, 8.0f * scale, 8.0f * scale, u0, 0.0f, u1, 1.0f);
}

void MainMenu::renderString(const std::string &s, float x, float y,
                             float scale, float spacing, const glm::vec4 &col)
{
	float cx = x;
	for(size_t i = 0; i < s.size(); i++)
	{
		// Drop shadow
		renderChar(s[i], cx + scale, y + scale, scale, glm::vec4(0.0f, 0.0f, 0.0f, 0.65f));
		renderChar(s[i], cx, y, scale, col);
		cx += spacing;
	}
}

float MainMenu::stringPixelWidth(const std::string &s, float scale, float spacing)
{
	if(s.empty()) return 0.0f;
	return float(s.size()) * spacing;
}

void MainMenu::renderStringCentered(const std::string &s, float cx, float y,
                                     float scale, float spacing, const glm::vec4 &col)
{
	float w = stringPixelWidth(s, scale, spacing);
	renderString(s, cx - w * 0.5f, y, scale, spacing, col);
}


// ─── Main render dispatch ───────────────────────────────────────────────────

void MainMenu::render(GameState screenState)
{
	texProgram.use();

	// Fixed 240×160 orthographic projection
	glm::mat4 proj = glm::ortho(0.0f, 240.0f, 160.0f, 0.0f);
	texProgram.setUniformMatrix4f("projection", proj);
	glm::mat4 mv = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", mv);
	texProgram.setUniform2f("texCoordDispl", 0.0f, 0.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Deep arcade dark navy background
	renderSolidQuad(0, 0, 240, 160, glm::vec4(0.03f, 0.04f, 0.10f, 1.0f));

	// Twinkling stars
	for(size_t i = 0; i < dots.size(); i++)
	{
		float a = 0.25f + 0.25f * std::sin(dots[i].phase + float(animTimer) * 0.005f);
		renderSolidQuad(dots[i].x, dots[i].y, 1.5f, 1.5f, glm::vec4(0.55f, 0.65f, 0.95f, a));
	}

	switch(screenState)
	{
	case STATE_MENU:         drawMenuScreen();         break;
	case STATE_INSTRUCTIONS: drawInstructionsScreen(); break;
	case STATE_CREDITS:      drawCreditsScreen();      break;
	case STATE_GAME_OVER:    drawGameOverScreen();     break;
	default: break;
	}

	glDisable(GL_BLEND);
}


// ─── Menu screen ────────────────────────────────────────────────────────────

void MainMenu::drawMenuScreen()
{
	// ── 1. Logo "POCKET BOMBERMAN" ──
	if(logoRect.w > 0 && logoRect.h > 0)
	{
		float scaleX = 170.0f / float(logoRect.w);
		float scaleY = 46.0f / float(logoRect.h);
		float logoScale = std::min(scaleX, scaleY);
		float dw = float(logoRect.w) * logoScale;
		float dh = float(logoRect.h) * logoScale;
		float dx = (240.0f - dw) * 0.5f;
		float dy = 6.0f;
		renderRegion(logoRect.x, logoRect.y, logoRect.w, logoRect.h, dx, dy, dw, dh);
	}

	// ── 2. Right-side Card: Iconic Tree Scene with Bomberman sleeping ──
	const float cardX = 138.0f, cardY = 56.0f;
	const float cardW = 90.0f, cardH = 76.0f;
	{
		// Card background
		renderSolidQuad(cardX, cardY, cardW, cardH, glm::vec4(0.04f, 0.06f, 0.15f, 0.95f));

		// Tree scene inside the card (perfectly fitted, no PAUSE, no gaps)
		if(treeRect.w > 0 && treeRect.h > 0)
		{
			renderRegion(treeRect.x, treeRect.y, treeRect.w, treeRect.h,
			             cardX + 3.0f, cardY + 3.0f, cardW - 6.0f, cardH - 6.0f);
		}

		// Gold outer border
		glm::vec4 gold(0.85f, 0.68f, 0.18f, 1.0f);
		renderSolidQuad(cardX, cardY, cardW, 1, gold);
		renderSolidQuad(cardX, cardY + cardH - 1, cardW, 1, gold);
		renderSolidQuad(cardX, cardY, 1, cardH, gold);
		renderSolidQuad(cardX + cardW - 1, cardY, 1, cardH, gold);

		// Inner highlight frame
		glm::vec4 hl(0.25f, 0.40f, 0.70f, 0.45f);
		renderSolidQuad(cardX + 2, cardY + 2, cardW - 4, 1, hl);
		renderSolidQuad(cardX + 2, cardY + cardH - 3, cardW - 4, 1, hl);
		renderSolidQuad(cardX + 2, cardY + 2, 1, cardH - 4, hl);
		renderSolidQuad(cardX + cardW - 3, cardY + 2, 1, cardH - 4, hl);
	}

	// ── 3. Left-side Menu Box ──
	const float boxX = 12.0f, boxY = 56.0f;
	const float boxW = 120.0f, boxH = 76.0f;

	// Dark semi-transparent fill
	renderSolidQuad(boxX, boxY, boxW, boxH, glm::vec4(0.05f, 0.07f, 0.18f, 0.92f));

	// Gold outer frame
	glm::vec4 gold(0.85f, 0.68f, 0.18f, 1.0f);
	renderSolidQuad(boxX, boxY, boxW, 1, gold);
	renderSolidQuad(boxX, boxY + boxH - 1, boxW, 1, gold);
	renderSolidQuad(boxX, boxY, 1, boxH, gold);
	renderSolidQuad(boxX + boxW - 1, boxY, 1, boxH, gold);

	// Inner highlight line
	glm::vec4 hl(0.25f, 0.40f, 0.70f, 0.45f);
	renderSolidQuad(boxX + 2, boxY + 2, boxW - 4, 1, hl);
	renderSolidQuad(boxX + 2, boxY + boxH - 3, boxW - 4, 1, hl);
	renderSolidQuad(boxX + 2, boxY + 2, 1, boxH - 4, hl);
	renderSolidQuad(boxX + boxW - 3, boxY + 2, 1, boxH - 4, hl);

	// ── 4. Option Labels ──
	const std::string playLabel = bGameInProgress ? "CONTINUAR" : "JUGAR";
	const std::string labels[4] = { playLabel, "INSTRUCCIONS", "CREDITS", "SORTIR" };
	float pulse = 0.82f + 0.18f * std::sin(float(animTimer) * 0.008f);

	for(int i = 0; i < 4; i++)
	{
		float oy = boxY + 9.0f + float(i) * 16.0f;
		bool sel = (i == selectedOption);
		glm::vec4 col = sel
			? glm::vec4(1.0f * pulse, 0.92f * pulse, 0.20f, 1.0f)
			: glm::vec4(0.75f, 0.78f, 0.85f, 0.85f);

		if(sel)
		{
			// Pulsing cursor arrow
			renderString(">", boxX + 6.0f, oy, 1.0f, 7.0f, glm::vec4(1.0f, 0.35f, 0.20f, pulse));
		}

		renderString(labels[i], boxX + 16.0f, oy, 0.95f, 6.2f, col);
	}

	// ── 5. Bottom Prompt (replaces unwanted purple box) ──
	float blink = 0.5f + 0.5f * std::sin(float(animTimer) * 0.006f);
	renderStringCentered("PREM ENTER PER SELECCIONAR", 120.0f, 144.0f,
	                     0.82f, 5.0f, glm::vec4(0.5f * blink + 0.4f, 0.6f * blink + 0.4f, 0.85f * blink + 0.15f, 1.0f));
}


// ─── Instructions screen ───────────────────────────────────────────────────

void MainMenu::drawInstructionsScreen()
{
	renderStringCentered("INSTRUCCIONS", 120, 8, 1.1f, 9.0f,
	                     glm::vec4(1.0f, 0.85f, 0.2f, 1.0f));

	float bx = 10, by = 22, bw = 220, bh = 120;
	renderSolidQuad(bx, by, bw, bh, glm::vec4(0.05f, 0.07f, 0.18f, 0.94f));
	glm::vec4 bc(0.3f, 0.55f, 0.95f, 1.0f);
	renderSolidQuad(bx, by, bw, 1, bc);
	renderSolidQuad(bx, by + bh - 1, bw, 1, bc);
	renderSolidQuad(bx, by, 1, bh, bc);
	renderSolidQuad(bx + bw - 1, by, 1, bh, bc);

	glm::vec4 w(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec4 cyan(0.4f, 0.9f, 1.0f, 1.0f);
	glm::vec4 yellow(0.95f, 0.92f, 0.35f, 1.0f);

	float y = 28.0f;
	renderString("CONTROLS:", 16, y, 1.0f, 6.5f, cyan); y += 11;
	renderString("FLETXES: MOURE I SALTAR",   16, y, 0.85f, 5.2f, w); y += 9;
	renderString("A: PLANTAR BOMBA",          16, y, 0.85f, 5.2f, w); y += 9;
	renderString("ESC: PAUSA / MENU",         16, y, 0.85f, 5.2f, w); y += 13;

	renderString("TECLES DE CORRECCIO:", 16, y, 1.0f, 6.5f, cyan); y += 11;
	renderString("G: MODE DEU (INVULNERABLE)", 16, y, 0.85f, 5.2f, yellow); y += 9;
	renderString("K: ELIMINAR TOTS ENEMICS",  16, y, 0.85f, 5.2f, yellow); y += 9;
	renderString("1-4: SALTAR DE NIVELL",     16, y, 0.85f, 5.2f, yellow);

	float blink = 0.5f + 0.5f * std::sin(float(animTimer) * 0.006f);
	renderStringCentered("PREM ESC O ENTER PER TORNAR", 120, 147,
	                     0.85f, 5.2f, glm::vec4(blink, blink, blink, 1.0f));
}


// ─── Credits screen ─────────────────────────────────────────────────────────

void MainMenu::drawCreditsScreen()
{
	renderStringCentered("CREDITS", 120, 8, 1.1f, 9.0f,
	                     glm::vec4(1.0f, 0.85f, 0.2f, 1.0f));

	float bx = 12, by = 22, bw = 216, bh = 120;
	renderSolidQuad(bx, by, bw, bh, glm::vec4(0.05f, 0.07f, 0.18f, 0.94f));
	glm::vec4 bc(0.85f, 0.65f, 0.2f, 1.0f);
	renderSolidQuad(bx, by, bw, 1, bc);
	renderSolidQuad(bx, by + bh - 1, bw, 1, bc);
	renderSolidQuad(bx, by, 1, bh, bc);
	renderSolidQuad(bx + bw - 1, by, 1, bh, bc);

	glm::vec4 w(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec4 cyan(0.4f, 0.9f, 1.0f, 1.0f);
	glm::vec4 grey(0.75f, 0.78f, 0.85f, 1.0f);

	float y = 28.0f;
	renderStringCentered("FIB - UPC   VIDEOJOCS", 120, y, 1.0f, 6.5f, cyan); y += 11;
	renderStringCentered("CURS 2026/27 Q1",       120, y, 0.85f, 5.2f, grey); y += 13;
	renderStringCentered("BOMBERMAN 2D",          120, y, 1.0f, 6.5f, glm::vec4(1.0f, 0.9f, 0.3f, 1.0f)); y += 12;
	renderStringCentered("DESENVOLUPAT PER",      120, y, 0.85f, 5.2f, w); y += 9;
	renderStringCentered("CARLES I DANIEL",        120, y, 0.85f, 5.2f, w); y += 13;

	renderStringCentered("GRAFICS ORIGINALS:",    120, y, 0.85f, 5.2f, glm::vec4(0.5f, 0.8f, 1.0f, 1.0f)); y += 9;
	renderStringCentered("1998 HUDSON SOFT",      120, y, 0.80f, 4.8f, grey); y += 8;
	renderStringCentered("SPRITES: FRET & WOPPET",120, y, 0.80f, 4.8f, grey);

	// Hudson bee logo badge
	if(hudsonRect.w > 0 && hudsonRect.h > 0)
	{
		renderRegion(hudsonRect.x, hudsonRect.y, hudsonRect.w, hudsonRect.h, 184.0f, 84.0f, 34.0f, 34.0f);
	}

	float blink = 0.5f + 0.5f * std::sin(float(animTimer) * 0.006f);
	renderStringCentered("PREM ESC O ENTER PER TORNAR", 120, 147,
	                     0.85f, 5.2f, glm::vec4(blink, blink, blink, 1.0f));
}


// ─── Game Over screen ────────────────────────────────────────────────────────

void MainMenu::drawGameOverScreen()
{
	// Deep dramatic vignette background
	renderSolidQuad(0, 0, 240, 160, glm::vec4(0.06f, 0.02f, 0.04f, 1.0f));

	// Drifting ember particles (subtle red/orange sparks)
	for(size_t i = 0; i < dots.size(); i++)
	{
		float a = 0.35f + 0.35f * std::sin(dots[i].phase + float(animTimer) * 0.007f);
		renderSolidQuad(dots[i].x, dots[i].y, 1.5f, 1.5f, glm::vec4(1.0f, 0.28f, 0.15f, a));
	}

	// 1. 3D curved "GAME OVER" banner from sprite sheet
	float goW = float(gameOverRect.w);
	float goH = float(gameOverRect.h);
	if(goW <= 0 || goH <= 0) { goW = 150.0f; goH = 65.0f; }
	float aspect = goW / goH;
	float dw = 140.0f;
	float dh = dw / aspect;
	float dx = (240.0f - dw) * 0.5f;
	float dy = 12.0f + 2.0f * std::sin(float(animTimer) * 0.003f);

	renderRegion(gameOverRect.x, gameOverRect.y, gameOverRect.w, gameOverRect.h, dx, dy, dw, dh);

	// 2. Fallen Bomberman with swirl eyes
	float fbW = float(fallenBombermanRect.w);
	float fbH = float(fallenBombermanRect.h);
	if(fbW <= 0 || fbH <= 0) { fbW = 50.0f; fbH = 30.0f; }
	float fAspect = fbW / fbH;
	float fdw = 50.0f;
	float fdh = fdw / fAspect;
	float fdx = (240.0f - fdw) * 0.5f;
	float fdy = dy + dh + 6.0f;

	// Ground shadow
	renderSolidQuad(fdx + 5.0f, fdy + fdh - 1.0f, fdw - 10.0f, 2.5f, glm::vec4(0.0f, 0.0f, 0.0f, 0.45f));
	renderRegion(fallenBombermanRect.x, fallenBombermanRect.y, fallenBombermanRect.w, fallenBombermanRect.h, fdx, fdy, fdw, fdh);

	// 3. Subtitles and prompts
	renderStringCentered("FI DE LA PARTIDA", 120.0f, fdy + fdh + 8.0f,
	                     0.95f, 6.0f, glm::vec4(1.0f, 0.40f, 0.30f, 1.0f));

	float pulse = 0.5f + 0.5f * std::sin(float(animTimer) * 0.007f);
	renderStringCentered("PREM ENTER PER TORNAR AL MENU", 120.0f, 140.0f,
	                     0.80f, 4.8f, glm::vec4(1.0f * pulse + 0.2f, 0.9f * pulse + 0.2f, 0.3f, 1.0f));
}

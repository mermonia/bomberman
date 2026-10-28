#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "TileMap.h"
#include "AssetPath.h"

using namespace std;

#define BARRIER_COLLAPSE_STEP 70

TileMap *TileMap::createTileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program)
{
	TileMap *map = new TileMap(levelFile, minCoords, program);

	return map;
}

TileMap::TileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program)
{
	map = NULL;
	dynamicSolid = NULL;
	zeroBased = false;
	airTile = 0;
	playerSpawn = glm::ivec2(-1, -1);
	doorTile = glm::ivec2(-1, -1);
	doorTileId = -1;
	bushTiles.clear();
	fadingBushes.clear();
	fadingVao = 0;
	fadingVbo = 0;
	renderMinCoords = minCoords;
	renderProgram = &program;
	barrierTopTile = -1;
	barrierBodyTile = -1;
	barrierBaseTile = -1;
	loadLevel(levelFile);
	prepareArrays(minCoords, program);
}

TileMap::~TileMap()
{
	if(map != NULL)
		delete [] map;
	if(dynamicSolid != NULL)
		delete [] dynamicSolid;
	free();
}

void TileMap::update(int deltaTime)
{
	bool bRebuild = false;

	for(int i = int(collapsingBarriers.size()) - 1; i >= 0; i--)
	{
		BarrierCollapse &collapse = collapsingBarriers[i];
		collapse.timer += deltaTime;
		while(collapse.timer >= BARRIER_COLLAPSE_STEP && collapse.clearY >= collapse.topY)
		{
			collapse.timer -= BARRIER_COLLAPSE_STEP;
			map[collapse.clearY * mapSize.x + collapse.tileX] = airTile;
			collapse.clearY--;
			bRebuild = true;
		}
		if(collapse.clearY < collapse.topY)
			collapsingBarriers.erase(collapsingBarriers.begin() + i);
	}

	for(int i = int(fadingBushes.size()) - 1; i >= 0; i--)
	{
		fadingBushes[i].timer -= deltaTime;
		if(fadingBushes[i].timer <= 0)
			fadingBushes.erase(fadingBushes.begin() + i);
	}

	if(bRebuild)
	{
		free();
		prepareArrays(renderMinCoords, *renderProgram);
	}
}

void TileMap::render() const
{
	glEnable(GL_TEXTURE_2D);
	tilesheet.use();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLES, 0, 6 * nTiles);

	if(!fadingBushes.empty() && renderProgram != NULL && fadingVao != 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(fadingVao);
		glBindBuffer(GL_ARRAY_BUFFER, fadingVbo);
		glEnableVertexAttribArray(posLocation);
		glEnableVertexAttribArray(texCoordLocation);

		for(unsigned int b = 0; b < fadingBushes.size(); b++)
		{
			const FadingBush &fb = fadingBushes[b];
			float alpha = float(fb.timer) / float(fb.totalTime);
			if(alpha < 0.f) alpha = 0.f;
			if(alpha > 1.f) alpha = 1.f;

			float x0 = renderMinCoords.x + fb.tileX * tileSize;
			float y0 = renderMinCoords.y + fb.tileY * tileSize;
			float x1 = x0 + blockSize;
			float y1 = y0 + blockSize;

			int tileIdx = zeroBased ? fb.tileId : (fb.tileId - 1);
			float u0 = float(tileIdx % tilesheetSize.x) / tilesheetSize.x;
			float v0 = float(tileIdx / tilesheetSize.x) / tilesheetSize.y;
			float u1 = u0 + tileTexSize.x;
			float v1 = v0 + tileTexSize.y;

			float vertices[24] = {
				x0, y0, u0, v0,
				x1, y0, u1, v0,
				x1, y1, u1, v1,
				x0, y0, u0, v0,
				x1, y1, u1, v1,
				x0, y1, u0, v1
			};

			glBufferSubData(GL_ARRAY_BUFFER, 0, 24 * sizeof(float), vertices);
			renderProgram->setUniform4f("color", 1.0f, 1.0f, 1.0f, alpha);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		glBindVertexArray(0);
		glDisable(GL_BLEND);
		renderProgram->setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	}

	glDisable(GL_TEXTURE_2D);
}

void TileMap::free()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	if(fadingVao != 0)
	{
		glDeleteVertexArrays(1, &fadingVao);
		glDeleteBuffers(1, &fadingVbo);
		fadingVao = 0;
		fadingVbo = 0;
	}
}

bool TileMap::loadLevel(const string &levelFile)
{
	ifstream fin;
	string line, tilesheetFile;
	stringstream sstream;
	char tile;

	fin.open(levelFile.c_str());
	if(!fin.is_open())
		return false;
	fadingBushes.clear();
	getline(fin, line);
	if(line.compare(0, 7, "TILEMAP") != 0)
		return false;
	getline(fin, line);
	sstream.clear();
	sstream.str(line);
	sstream >> mapSize.x >> mapSize.y;
	getline(fin, line);
	sstream.clear();
	sstream.str(line);
	sstream >> tileSize >> blockSize;
	getline(fin, line);
	sstream.clear();
	sstream.str(line);
	sstream >> tilesheetFile;
	tilesheet.loadFromFile(assetPath(tilesheetFile), TEXTURE_PIXEL_FORMAT_RGBA);
	tilesheet.setWrapS(GL_CLAMP_TO_EDGE);
	tilesheet.setWrapT(GL_CLAMP_TO_EDGE);
	tilesheet.setMinFilter(GL_NEAREST);
	tilesheet.setMagFilter(GL_NEAREST);
	getline(fin, line);
	sstream.clear();
	sstream.str(line);
	sstream >> tilesheetSize.x >> tilesheetSize.y;
	tileTexSize = glm::vec2(1.f / tilesheetSize.x, 1.f / tilesheetSize.y);

	int declaredAirTile = -1;
	playerSpawn = glm::ivec2(-1, -1);
	enemySpawns.clear();

	streampos rowStart = fin.tellg();
	while(getline(fin, line))
	{
		if(!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		string keyword;
		sstream.clear();
		sstream.str(line);
		sstream >> keyword;

		if(keyword == "AIR")
		{
			sstream >> declaredAirTile;
		}
		else if(keyword == "BARRIER")
		{
			sstream >> barrierTopTile >> barrierBodyTile >> barrierBaseTile;
		}
		else if(keyword == "PLAYER")
		{
			sstream >> playerSpawn.x >> playerSpawn.y;
		}
		else if(keyword == "DOOR")
		{
			sstream >> doorTile.x >> doorTile.y;
		}
		else if(keyword == "BUSH")
		{
			int bTile;
			while(sstream >> bTile)
				bushTiles.push_back(bTile);
		}
		else if(keyword == "ENEMY")
		{
			EnemySpawn spawn;
			sstream >> spawn.type >> spawn.tile.x >> spawn.tile.y;
			enemySpawns.push_back(spawn);
		}
		else
		{
			break;
		}

		rowStart = fin.tellg();
	}
	fin.clear();
	fin.seekg(rowStart);

	zeroBased = (tilesheetSize.x > 2 || tilesheetFile.find("blocks4") != string::npos);
	airTile = (declaredAirTile >= 0) ? declaredAirTile : (zeroBased ? 2 : 0);

	map = new int[mapSize.x * mapSize.y];
	dynamicSolid = new bool[mapSize.x * mapSize.y];
	clearDynamicSolids();
	if(zeroBased)
	{
		for(int j=0; j<mapSize.y; j++)
		{
			for(int i=0; i<mapSize.x; i++)
			{
				fin >> map[j*mapSize.x+i];
			}
		}
	}
	else
	{
		for(int j=0; j<mapSize.y; j++)
		{
			for(int i=0; i<mapSize.x; i++)
			{
				fin.get(tile);
				if(tile == ' ')
					map[j*mapSize.x+i] = 0;
				else
					map[j*mapSize.x+i] = tile - int('0');
			}
			while(fin.get(tile) && tile != '\n')
				;
		}
	}
	fin.close();

	if(bushTiles.empty())
	{
		if(tilesheetFile.find("blocks1") != string::npos)
			bushTiles.push_back(12);
		else if(tilesheetFile.find("blocks2") != string::npos)
		{
			bushTiles.push_back(11);
			bushTiles.push_back(19);
		}
		else if(tilesheetFile.find("blocks3") != string::npos)
		{
			bushTiles.push_back(8);
		}
		else if(tilesheetFile.find("blocks4") != string::npos)
		{
			bushTiles.push_back(1);
		}
	}

	if(doorTile.x < 0)
	{
		int targetDoorId = -1;
		if(tilesheetFile.find("blocks1") != string::npos)
			targetDoorId = 20;
		else if(tilesheetFile.find("blocks2") != string::npos)
			targetDoorId = 13;
		else if(tilesheetFile.find("blocks3") != string::npos)
			targetDoorId = 10;
		else if(tilesheetFile.find("blocks4") != string::npos)
			targetDoorId = 3;
		else if(tilesheetFile.find("blocks5") != string::npos)
			targetDoorId = 12;

		if(targetDoorId >= 0)
		{
			for(int j = 0; j < mapSize.y; j++)
			{
				for(int i = 0; i < mapSize.x; i++)
				{
					if(map[j * mapSize.x + i] == targetDoorId)
					{
						doorTile = glm::ivec2(i, j);
						doorTileId = targetDoorId;
						break;
					}
				}
				if(doorTile.x >= 0)
					break;
			}
		}
	}
	else
	{
		if(doorTile.x >= 0 && doorTile.x < mapSize.x && doorTile.y >= 0 && doorTile.y < mapSize.y)
			doorTileId = map[doorTile.y * mapSize.x + doorTile.x];
	}

	return true;
}

bool TileMap::isSolidTile(int tile) const
{
	return (tile != airTile);
}

void TileMap::prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program)
{
	int tile;
	glm::vec2 posTile, texCoordTile[2];
	vector<float> vertices;

	nTiles = 0;
	for(int j=0; j<mapSize.y; j++)
	{
		for(int i=0; i<mapSize.x; i++)
		{
			tile = map[j * mapSize.x + i];
			if(tile > 0 || (zeroBased && tile >= 0))
			{
				nTiles++;
				posTile = glm::vec2(minCoords.x + i * tileSize, minCoords.y + j * tileSize);
				int tileIdx = zeroBased ? tile : (tile - 1);
				texCoordTile[0] = glm::vec2(float(tileIdx % tilesheetSize.x) / tilesheetSize.x, float(tileIdx / tilesheetSize.x) / tilesheetSize.y);
				texCoordTile[1] = texCoordTile[0] + tileTexSize;
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				vertices.push_back(posTile.x); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[1].y);
			}
		}
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * nTiles * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	posLocation = program.bindVertexAttribute("position", 2, 4*sizeof(float), 0);
	texCoordLocation = program.bindVertexAttribute("texCoord", 2, 4*sizeof(float), (void *)(2*sizeof(float)));

	glGenVertexArrays(1, &fadingVao);
	glBindVertexArray(fadingVao);
	glGenBuffers(1, &fadingVbo);
	glBindBuffer(GL_ARRAY_BUFFER, fadingVbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	program.bindVertexAttribute("position", 2, 4*sizeof(float), 0);
	program.bindVertexAttribute("texCoord", 2, 4*sizeof(float), (void *)(2*sizeof(float)));
	glBindVertexArray(0);
}

bool TileMap::collisionMoveLeft(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int x, y0, y1;

	x = pos.x / tileSize;
	y0 = pos.y / tileSize;
	y1 = (pos.y + size.y - 1) / tileSize;
	if(x < 0) return true;
	if(x >= mapSize.x) return false;
	if(y0 < 0) y0 = 0;
	if(y1 >= mapSize.y) y1 = mapSize.y - 1;

	for(int y=y0; y<=y1; y++)
	{
		if(isBlockedTile(x, y))
			return true;
	}

	return false;
}

bool TileMap::collisionMoveRight(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int x, y0, y1;

	x = (pos.x + size.x - 1) / tileSize;
	y0 = pos.y / tileSize;
	y1 = (pos.y + size.y - 1) / tileSize;
	if(x >= mapSize.x) return true;
	if(x < 0) return false;
	if(y0 < 0) y0 = 0;
	if(y1 >= mapSize.y) y1 = mapSize.y - 1;

	for(int y=y0; y<=y1; y++)
	{
		if(isBlockedTile(x, y))
			return true;
	}

	return false;
}

bool TileMap::collisionMoveDown(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const
{
	int x0, x1, y;

	x0 = pos.x / tileSize;
	x1 = (pos.x + size.x - 1) / tileSize;
	y = (pos.y + size.y - 1) / tileSize;
	if(x0 < 0) x0 = 0;
	if(x1 >= mapSize.x) x1 = mapSize.x - 1;
	if(y < 0 || y >= mapSize.y) return false;

	for(int x=x0; x<=x1; x++)
	{
		if(isBlockedTile(x, y))
		{
			*posY = tileSize * y - size.y;
			return true;
		}
	}

	return false;
}

bool TileMap::collisionMoveUp(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const
{
	int x0, x1, y;

	x0 = pos.x / tileSize;
	x1 = (pos.x + size.x - 1) / tileSize;
	y = pos.y / tileSize;
	if(y < 0)
	{
		*posY = 0;
		return true;
	}
	if(y >= mapSize.y) return false;
	if(x0 < 0) x0 = 0;
	if(x1 >= mapSize.x) x1 = mapSize.x - 1;

	for(int x=x0; x<=x1; x++)
	{
		if(isBlockedTile(x, y))
		{
			*posY = tileSize * (y + 1);
			return true;
		}
	}

	return false;
}

bool TileMap::isGround(int tileX, int tileY) const
{
	if(tileX < 0 || tileX >= mapSize.x || tileY < 0 || tileY >= mapSize.y)
		return false;
	return isSolidTile(map[tileY * mapSize.x + tileX]);
}

bool TileMap::isBlockedTile(int tileX, int tileY) const
{
	if(tileX < 0 || tileX >= mapSize.x || tileY < 0 || tileY >= mapSize.y)
		return false;

	if(tileX == doorTile.x && tileY == doorTile.y)
		return false;

	return isSolidTile(map[tileY * mapSize.x + tileX]) || dynamicSolid[tileY * mapSize.x + tileX];
}

bool TileMap::isFadingBush(int tileX, int tileY) const
{
	for(unsigned int i = 0; i < fadingBushes.size(); i++)
	{
		if(fadingBushes[i].tileX == tileX && fadingBushes[i].tileY == tileY)
			return true;
	}
	return false;
}

bool TileMap::isBush(int tileX, int tileY) const
{
	if(tileX < 0 || tileX >= mapSize.x || tileY < 0 || tileY >= mapSize.y)
		return false;
	if(isFadingBush(tileX, tileY))
		return true;
	int t = map[tileY * mapSize.x + tileX];
	for(unsigned int i = 0; i < bushTiles.size(); i++)
	{
		if(bushTiles[i] == t)
			return true;
	}
	return false;
}

bool TileMap::destroyBush(int tileX, int tileY)
{
	for(unsigned int i = 0; i < fadingBushes.size(); i++)
	{
		if(fadingBushes[i].tileX == tileX && fadingBushes[i].tileY == tileY)
		{
			fadingBushes.erase(fadingBushes.begin() + i);
			break;
		}
	}
	if(tileX < 0 || tileX >= mapSize.x || tileY < 0 || tileY >= mapSize.y)
		return false;
	int t = map[tileY * mapSize.x + tileX];
	bool isB = false;
	for(unsigned int i = 0; i < bushTiles.size(); i++)
	{
		if(bushTiles[i] == t)
		{
			isB = true;
			break;
		}
	}
	if(!isB)
		return false;
	map[tileY * mapSize.x + tileX] = airTile;
	free();
	prepareArrays(renderMinCoords, *renderProgram);
	return true;
}

bool TileMap::destroyBushFade(int tileX, int tileY, int fadeTimeMs)
{
	if(isFadingBush(tileX, tileY))
		return false;
	if(tileX < 0 || tileX >= mapSize.x || tileY < 0 || tileY >= mapSize.y)
		return false;
	int t = map[tileY * mapSize.x + tileX];
	bool isB = false;
	for(unsigned int i = 0; i < bushTiles.size(); i++)
	{
		if(bushTiles[i] == t)
		{
			isB = true;
			break;
		}
	}
	if(!isB)
		return false;

	FadingBush fb;
	fb.tileX = tileX;
	fb.tileY = tileY;
	fb.tileId = t;
	fb.timer = fadeTimeMs;
	fb.totalTime = fadeTimeMs;
	fadingBushes.push_back(fb);

	map[tileY * mapSize.x + tileX] = airTile;
	free();
	prepareArrays(renderMinCoords, *renderProgram);
	return true;
}

void TileMap::clearDynamicSolids()
{
	for(int i = 0; i < mapSize.x * mapSize.y; i++)
		dynamicSolid[i] = false;
}

void TileMap::setDynamicSolid(int tileX, int tileY)
{
	if(tileX < 0 || tileX >= mapSize.x || tileY < 0 || tileY >= mapSize.y)
		return;
	dynamicSolid[tileY * mapSize.x + tileX] = true;
}

bool TileMap::isBarrierSegment(int tile) const
{
	return (barrierBodyTile >= 0 && tile == barrierBodyTile) || (barrierBaseTile >= 0 && tile == barrierBaseTile);
}

void TileMap::triggerBarrier(int tileX, int tileY)
{
	if(barrierTopTile < 0)
		return;
	if(tileX < 0 || tileX >= mapSize.x || tileY < 0 || tileY >= mapSize.y)
		return;
	if(map[tileY * mapSize.x + tileX] != barrierTopTile)
		return;

	for(unsigned int i = 0; i < collapsingBarriers.size(); i++)
	{
		if(collapsingBarriers[i].tileX == tileX && collapsingBarriers[i].topY == tileY)
			return;
	}

	BarrierCollapse collapse;
	collapse.tileX = tileX;
	collapse.topY = tileY;
	collapse.clearY = tileY;
	collapse.timer = 0;
	while(collapse.clearY + 1 < mapSize.y && isBarrierSegment(map[(collapse.clearY + 1) * mapSize.x + tileX]))
		collapse.clearY++;
	collapsingBarriers.push_back(collapse);
}


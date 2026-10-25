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
	glDisable(GL_TEXTURE_2D);
}

void TileMap::free()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
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
			bushTiles.push_back(8); // Nomes les roques al nivell 3
		}
		else if(tilesheetFile.find("blocks4") != string::npos)
		{
			bushTiles.push_back(1); // Nomes les cares al nivell 4
		}
	}

	// Detecció automàtica de la porta del nivell si no s'ha especificat amb la directiva DOOR
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
	glm::vec2 posTile, texCoordTile[2], halfTexel;
	vector<float> vertices;
	
	nTiles = 0;
	halfTexel = glm::vec2(0.5f / tilesheet.width(), 0.5f / tilesheet.height());
	for(int j=0; j<mapSize.y; j++)
	{
		for(int i=0; i<mapSize.x; i++)
		{
			tile = map[j * mapSize.x + i];
			if(tile > 0 || (zeroBased && tile >= 0))
			{
				// Non-empty tile
				nTiles++;
				posTile = glm::vec2(minCoords.x + i * tileSize, minCoords.y + j * tileSize);
				int tileIdx = zeroBased ? tile : (tile - 1);
				texCoordTile[0] = glm::vec2(float(tileIdx % tilesheetSize.x) / tilesheetSize.x, float(tileIdx / tilesheetSize.x) / tilesheetSize.y);
				texCoordTile[1] = texCoordTile[0] + tileTexSize;
				//texCoordTile[0] += halfTexel;
				texCoordTile[1] -= halfTexel;
				// First triangle
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				// Second triangle
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
}

// Collision tests for axis aligned bounding boxes.
// Method collisionMoveDown also corrects Y coordinate if the box is
// already intersecting a tile below.

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
			if(*posY - tileSize * y + size.y <= 10)
			{
				*posY = tileSize * y - size.y;
				return true;
			}
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

	// La porta no bloqueja el pas per permetre posar-te davant de la porta
	if(tileX == doorTile.x && tileY == doorTile.y)
		return false;

	return isSolidTile(map[tileY * mapSize.x + tileX]) || dynamicSolid[tileY * mapSize.x + tileX];
}

bool TileMap::isBush(int tileX, int tileY) const
{
	if(tileX < 0 || tileX >= mapSize.x || tileY < 0 || tileY >= mapSize.y)
		return false;
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
	if(!isBush(tileX, tileY))
		return false;
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































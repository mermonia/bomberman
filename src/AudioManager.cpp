#include <iostream>
#include <sstream>
#include <filesystem>
#include <Audio/Device.hpp>
#include <Audio/Sound.hpp>
#include "AudioManager.h"
#include "AssetPath.h"


static const char *soundFiles[SFX_COUNT] =
{
	"audio/sfx_bomb_place.wav",
	"audio/sfx_explosion.wav",
	"audio/sfx_jump.wav",
	"audio/sfx_pickup.wav",
	"audio/sfx_enemy_die.wav",
	"audio/sfx_door_open.wav",
	"audio/sfx_door_enter.wav",
	"audio/sfx_menu_move.wav",
	"audio/sfx_menu_select.wav",
	"audio/sfx_boss_hit.wav",
	"audio/sfx_boss_death.wav"
};

static const char *musicFiles[MUSIC_COUNT] =
{
	"audio/music_title.ogg",
	"audio/music_level1.ogg",
	"audio/music_level2.ogg",
	"audio/music_level3.ogg",
	"audio/music_level4.ogg",
	"audio/music_boss.ogg",
	"audio/jingle_miss.ogg",
	"audio/jingle_stage_clear.ogg",
	"audio/jingle_game_over.ogg"
};


AudioManager::AudioManager()
{
	bAvailable = false;
	currentMusic = MUSIC_NONE;
	queuedMusic = MUSIC_NONE;
	for(int s = 0; s < SFX_COUNT; s++)
	{
		nextVoice[s] = 0;
		for(int v = 0; v < AUDIO_VOICES; v++)
			voices[s][v] = NULL;
	}
	for(int m = 0; m < MUSIC_COUNT; m++)
		tracks[m] = NULL;
}

AudioManager::~AudioManager()
{
	for(int s = 0; s < SFX_COUNT; s++)
	{
		for(int v = 0; v < AUDIO_VOICES; v++)
		{
			delete voices[s][v];
			voices[s][v] = NULL;
		}
	}
	for(int m = 0; m < MUSIC_COUNT; m++)
	{
		delete tracks[m];
		tracks[m] = NULL;
	}
}

void AudioManager::init()
{
	if(bAvailable)
		return;

	ostringstream captured;
	streambuf *previous = cerr.rdbuf(captured.rdbuf());

	try
	{
		Audio::Device::getListener(0);
		bAvailable = captured.str().empty();

		if(bAvailable)
		{
			for(int s = 0; s < SFX_COUNT; s++)
				for(int v = 0; v < AUDIO_VOICES; v++)
					voices[s][v] = loadFile(soundFiles[s], false);
			for(int m = 0; m < MUSIC_COUNT; m++)
				tracks[m] = loadFile(musicFiles[m], true);
		}
	}
	catch(...)
	{
		bAvailable = false;
	}

	cerr.rdbuf(previous);
}

Audio::Sound *AudioManager::loadFile(const string &file, bool bMusic)
{
	string path = assetPath(file);
	error_code error;
	if(!filesystem::exists(filesystem::path(path), error))
		return NULL;

	ostringstream captured;
	streambuf *previous = cerr.rdbuf(captured.rdbuf());
	Audio::Sound *sound = NULL;
	try
	{
		sound = new Audio::Sound(filesystem::path(path), bMusic ? Audio::Sound::Type::Music : Audio::Sound::Type::Sound);
	}
	catch(...)
	{
		sound = NULL;
	}
	cerr.rdbuf(previous);

	if(sound != NULL && !captured.str().empty())
	{
		delete sound;
		sound = NULL;
	}
	if(sound != NULL)
		sound->setVolume(bMusic ? AUDIO_MUSIC_VOLUME : AUDIO_SOUND_VOLUME);

	return sound;
}

void AudioManager::update()
{
	if(!bAvailable || queuedMusic == MUSIC_NONE)
		return;

	if(currentMusic == MUSIC_NONE || !isTrackPlaying(currentMusic))
	{
		MusicId next = queuedMusic;
		queuedMusic = MUSIC_NONE;
		startTrack(next);
	}
}

void AudioManager::playSound(SoundId id)
{
	if(!bAvailable || id < 0 || id >= SFX_COUNT)
		return;

	Audio::Sound *voice = voices[id][nextVoice[id]];
	nextVoice[id] = (nextVoice[id] + 1) % AUDIO_VOICES;
	if(voice != NULL)
		voice->replay();
}

void AudioManager::playMusic(MusicId id)
{
	if(!bAvailable || id < 0 || id >= MUSIC_COUNT)
		return;

	if(isJingle(id))
	{
		queuedMusic = MUSIC_NONE;
		startTrack(id);
		return;
	}

	if(queuedMusic == id)
		return;
	if(currentMusic == id && queuedMusic == MUSIC_NONE)
		return;

	if(currentMusic != MUSIC_NONE && isJingle(currentMusic) && isTrackPlaying(currentMusic))
	{
		queuedMusic = id;
		return;
	}

	queuedMusic = MUSIC_NONE;
	startTrack(id);
}

void AudioManager::stopMusic()
{
	if(!bAvailable)
		return;

	if(currentMusic != MUSIC_NONE && tracks[currentMusic] != NULL)
		tracks[currentMusic]->stop();
	currentMusic = MUSIC_NONE;
	queuedMusic = MUSIC_NONE;
}

MusicId AudioManager::musicForLevel(int level)
{
	switch(level)
	{
	case 2: return MUSIC_LEVEL2;
	case 3: return MUSIC_LEVEL3;
	case 4: return MUSIC_LEVEL4;
	case 5: return MUSIC_BOSS;
	default: return MUSIC_LEVEL1;
	}
}

bool AudioManager::isJingle(MusicId id) const
{
	return id == JINGLE_MISS || id == JINGLE_STAGE_CLEAR || id == JINGLE_GAME_OVER;
}

bool AudioManager::isTrackPlaying(MusicId id) const
{
	if(id == MUSIC_NONE || tracks[id] == NULL)
		return false;
	return tracks[id]->isPlaying();
}

void AudioManager::startTrack(MusicId id)
{
	if(currentMusic != MUSIC_NONE && tracks[currentMusic] != NULL)
		tracks[currentMusic]->stop();

	currentMusic = id;
	if(tracks[id] == NULL)
		return;

	tracks[id]->setLooping(!isJingle(id));
	tracks[id]->replay();
}

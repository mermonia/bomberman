#ifndef _AUDIOMANAGER_INCLUDE
#define _AUDIOMANAGER_INCLUDE


#include <string>


#define AUDIO_VOICES 4
#define AUDIO_MUSIC_VOLUME 0.6f
#define AUDIO_SOUND_VOLUME 1.0f


enum SoundId
{
	SFX_BOMB_PLACE = 0,
	SFX_EXPLOSION,
	SFX_JUMP,
	SFX_PICKUP,
	SFX_ENEMY_DIE,
	SFX_DOOR_OPEN,
	SFX_DOOR_ENTER,
	SFX_MENU_MOVE,
	SFX_MENU_SELECT,
	SFX_BOSS_HIT,
	SFX_BOSS_DEATH,
	SFX_COUNT
};

enum MusicId
{
	MUSIC_NONE = -1,
	MUSIC_TITLE = 0,
	MUSIC_LEVEL1,
	MUSIC_LEVEL2,
	MUSIC_LEVEL3,
	MUSIC_LEVEL4,
	MUSIC_BOSS,
	JINGLE_MISS,
	JINGLE_STAGE_CLEAR,
	JINGLE_GAME_OVER,
	MUSIC_COUNT
};


namespace Audio
{
	class Sound;
}


class AudioManager
{

private:
	AudioManager();

public:
	static AudioManager &instance()
	{
		static AudioManager A;

		return A;
	}

	~AudioManager();

	void init();
	void update();

	void playSound(SoundId id);
	void playMusic(MusicId id);
	void stopMusic();

	static MusicId musicForLevel(int level);

private:
	Audio::Sound *loadFile(const std::string &file, bool bMusic);
	bool isJingle(MusicId id) const;
	bool isTrackPlaying(MusicId id) const;
	void startTrack(MusicId id);

private:
	bool bAvailable;
	Audio::Sound *voices[SFX_COUNT][AUDIO_VOICES];
	int nextVoice[SFX_COUNT];
	Audio::Sound *tracks[MUSIC_COUNT];
	MusicId currentMusic;
	MusicId queuedMusic;

};


#endif // _AUDIOMANAGER_INCLUDE

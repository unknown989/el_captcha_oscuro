#pragma once
#include <SDL2/SDL_mixer.h>
#include <map>
#include <string>


class Music {
public:
  static Music &getInstance() {
    static Music instance;
    return instance;
  }

  // Initialize music system
  bool init(int frequency = 44100, int channels = 2, int chunksize = 2048) {
    if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, channels, chunksize) < 0) {
      return false;
    }
    return true;
  }

  // Load a music track
  bool loadMusic(const std::string &name, const std::string &path) {
    Mix_Music *music = Mix_LoadMUS(path.c_str());
    if (!music) {
      return false;
    }
    musicTracks[name] = music;
    return true;
  }

  // Play a loaded music track with fade-in
  void playMusic(const std::string &name, int loops = -1, int fadeInMs = 1000) {
    if (musicTracks.find(name) == musicTracks.end()) {
      return;
    }

    // If music is already playing, fade it out first
    if (Mix_PlayingMusic()) {
      fadeOutAndPlay(name, loops, fadeInMs);
    } else {
      Mix_FadeInMusic(musicTracks[name], loops, fadeInMs);
    }
    currentTrack = name;
  }

  // Stop music with fade-out
  void stopMusic(int fadeOutMs = 1000) { Mix_FadeOutMusic(fadeOutMs); }

  // Set music volume (0-128)
  void setVolume(int volume) { Mix_VolumeMusic(volume); }

  // Get current volume
  int getVolume() { return Mix_VolumeMusic(-1); }

  // Cleanup
  ~Music() {
    for (auto &track : musicTracks) {
      if (track.second) {
        Mix_FreeMusic(track.second);
      }
    }
    musicTracks.clear();
    Mix_CloseAudio();
  }
  

private:
  Music() {} // Private constructor for singleton
  Music(const Music &) = delete;
  Music &operator=(const Music &) = delete;

  std::map<std::string, Mix_Music *> musicTracks;
  std::string currentTrack;
  

  // Helper function to fade out current music and play new track
  void fadeOutAndPlay(const std::string &name, int loops, int fadeInMs) {
    Mix_FadeOutMusic(1000);
    // Wait for fade out to complete
    while (Mix_PlayingMusic()) {
      SDL_Delay(100);
    }
    Mix_FadeInMusic(musicTracks[name], loops, fadeInMs);
  }
};

// Helper macro for easier access
#define MUSIC Music::getInstance()
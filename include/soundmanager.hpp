#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_log.h> // Make sure SDL_log is included (often via SDL.h)
#include <map>
#include <string>
#include <vector>
#include <optional> // Used for storing channel (optional because loading might fail)

// Structure to hold both the chunk and its dedicated channel
struct SoundEffectInfo {
    Mix_Chunk* chunk = nullptr;
    int channel = -1; // -1 indicates no channel assigned yet or failed assignment
};

class SoundManager {
public:
    // --- Singleton Access ---
    static SoundManager& getInstance() {
        static SoundManager instance;
        return instance;
    }


    bool init(int frequency = 44100, int channels = 2, int chunksize = 2048, int numMixChannels = 16) {
        int flags = MIX_INIT_MP3 | MIX_INIT_OGG;
        int initted = Mix_Init(flags);
        if ((initted & flags) != flags) {
            SDL_Log("SoundManager Warning: Failed to initialize required mixer subsystems! Error: %s", Mix_GetError());
        }

        if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, channels, chunksize) < 0) {
            SDL_Log("SoundManager Error: Failed to open audio! Error: %s", Mix_GetError());
            Mix_Quit();
            return false;
        }

        int allocatedChannels = Mix_AllocateChannels(numMixChannels);
        totalMixChannels = Mix_AllocateChannels(-1); // Store the actual number allocated
        if (allocatedChannels < numMixChannels) {
             SDL_Log("SoundManager Warning: Requested %d channels, but only %d were allocated.", numMixChannels, allocatedChannels);
        }
        if (totalMixChannels <= 0) {
            SDL_Log("SoundManager Error: Failed to allocate any mixing channels!");
            Mix_CloseAudio();
            Mix_Quit();
            return false;
        }
        SDL_Log("SoundManager: Initialized with %d sound effect channels.", totalMixChannels);
        nextAvailableChannel = 0; // Start assigning from channel 0

        setMusicVolume(MIX_MAX_VOLUME);
        setSoundEffectVolume(MIX_MAX_VOLUME);

        return true;
    }

    ~SoundManager() {
        SDL_Log("SoundManager: Shutting down...");
        Mix_HaltMusic();
        Mix_HaltChannel(-1);

        for (auto& track : musicTracks) {
            if (track.second) {
                Mix_FreeMusic(track.second);
            }
        }
        musicTracks.clear();

        // Free sound effects (chunks are now inside the map's value)
        for (auto& pair : soundEffectMap) {
            if (pair.second.chunk) {
                Mix_FreeChunk(pair.second.chunk);
            }
        }
        soundEffectMap.clear();

        Mix_CloseAudio();
        Mix_Quit();
        SDL_Log("SoundManager: Cleaned up.");
    }

    bool loadMusic(const std::string& name, const std::string& path) {
        if (musicTracks.count(name)) {
             SDL_Log("SoundManager Warning: Music '%s' already loaded.", name.c_str());
             return true;
        }
        Mix_Music* music = Mix_LoadMUS(path.c_str());
        if (!music) {
            SDL_Log("SoundManager Error: Failed to load music '%s' from %s! Error: %s", name.c_str(), path.c_str(), Mix_GetError());
            return false;
        }
        musicTracks[name] = music;
        SDL_Log("SoundManager: Loaded music '%s'.", name.c_str());
        return true;
    }

    void playMusic(const std::string& name, int loops = -1, int fadeInMs = 1000) {
        auto it = musicTracks.find(name);
        if (it == musicTracks.end() || it->second == nullptr) {
            SDL_Log("SoundManager Error: Cannot play music '%s'. Not loaded or invalid.", name.c_str());
            return;
        }

        if (Mix_PlayingMusic() && currentTrack != name) {
            fadeOutAndPlay(name, loops, fadeInMs);
        }
        else if (Mix_PlayingMusic() && currentTrack == name) {
             SDL_Log("SoundManager: Music '%s' is already playing.", name.c_str());
        }
        else {
            if (Mix_FadeInMusic(it->second, loops, fadeInMs) == -1) {
                 SDL_Log("SoundManager Error: Failed to play music '%s'! Error: %s", name.c_str(), Mix_GetError());
            } else {
                currentTrack = name;
                SDL_Log("SoundManager: Playing music '%s'.", name.c_str());
            }
        }
    }

    void stopMusic(int fadeOutMs = 1000) {
        if (Mix_PlayingMusic()) {
            SDL_Log("SoundManager: Stopping music with %dms fade.", fadeOutMs);
            Mix_FadeOutMusic(fadeOutMs);
            currentTrack = "";
        } else {
             SDL_Log("SoundManager: No music playing to stop.");
        }
    }
    void setMusicVolume(int volume) {
        if (volume < 0) volume = 0;
        if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
        Mix_VolumeMusic(volume);
        musicVolume = volume;
    }

    int getMusicVolume() const {
        return musicVolume;
    }


    // --- Sound Effect Handling ---

    /**
     * @brief Loads a sound effect and dedicates the next available channel to it.
     * @param name Unique identifier for the sound effect.
     * @param path Path to the sound effect file.
     * @return True on success (loaded and channel assigned), false otherwise.
     */
    bool loadSoundEffect(const std::string& name, const std::string& path) {
        if (soundEffectMap.count(name)) {
             SDL_Log("SoundManager Warning: Sound Effect '%s' already loaded.", name.c_str());
             // Return true if already loaded successfully before
             return soundEffectMap[name].chunk != nullptr && soundEffectMap[name].channel != -1;
        }

        // Check if we have channels left to assign
        if (nextAvailableChannel >= totalMixChannels) {
            SDL_Log("SoundManager Error: Cannot load SFX '%s'. No more channels available for dedication (%d allocated).", name.c_str(), totalMixChannels);
            return false;
        }

        Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
        if (!chunk) {
            SDL_Log("SoundManager Error: Failed to load sound effect chunk '%s' from %s! Error: %s", name.c_str(), path.c_str(), Mix_GetError());
            // Store an entry indicating load failure (optional, but helps avoid retry attempts)
            soundEffectMap[name] = {nullptr, -1};
            return false;
        }

        // Assign the next channel and store info
        int dedicatedChannel = nextAvailableChannel++;
        Mix_VolumeChunk(chunk, sfxVolume); // Apply current global SFX volume
        soundEffectMap[name] = {chunk, dedicatedChannel};

        SDL_Log("SoundManager: Loaded SFX '%s' and assigned to channel %d.", name.c_str(), dedicatedChannel);
        return true;
    }

    /**
     * @brief Plays a loaded sound effect on its dedicated channel.
     * @param name Identifier of the sound effect to play.
     * @param loops Number of times to loop (0 for play once).
     * @return The dedicated channel the sound was played on, or -1 on error.
     */
    int playSoundEffect(const std::string& name, int loops = 0) {
        auto it = soundEffectMap.find(name);
        if (it == soundEffectMap.end()) {
            SDL_Log("SoundManager Error: Cannot play SFX '%s'. Not loaded.", name.c_str());
            return -1;
        }

        const SoundEffectInfo& info = it->second;

        if (!info.chunk || info.channel < 0) {
             SDL_Log("SoundManager Error: Cannot play SFX '%s'. Chunk invalid or no channel assigned (load failed?).", name.c_str());
            return -1;
        }

        // Play specifically on the dedicated channel
        int playedChannel = Mix_PlayChannel(info.channel, info.chunk, loops);

        if (playedChannel == -1) {
             // This might happen if the channel system has an issue, though less likely than channel contention with -1.
             SDL_Log("SoundManager Error: Failed to play SFX '%s' on its dedicated channel %d! Error: %s", name.c_str(), info.channel, Mix_GetError());
             return -1; // Return -1 as playing failed
        } else if (playedChannel != info.channel) {
             // This case *shouldn't* happen with Mix_PlayChannel(specific_channel,...)
             SDL_Log("SoundManager Warning: Played SFX '%s' on channel %d but expected dedicated channel %d.", name.c_str(), playedChannel, info.channel);
             // Still return the actual channel it played on? Or the expected one? Return expected for consistency.
             return info.channel;
        }

        // Success
        // Optional: SDL_Log("SoundManager: Played SFX '%s' on dedicated channel %d", name.c_str(), info.channel);
        return info.channel; // Return the dedicated channel number
    }

    /**
     * @brief Stops all currently playing sound effects immediately.
     */
    void stopAllSoundEffects() {
        Mix_HaltChannel(-1);
    }

    /**
     * @brief Stops the specific sound effect associated with 'name' by halting its dedicated channel.
     * @param name Identifier of the sound effect to stop.
     */
    void stopSoundEffect(const std::string& name) {
        auto it = soundEffectMap.find(name);
        if (it != soundEffectMap.end() && it->second.channel >= 0) {
            Mix_HaltChannel(it->second.channel);
             // Optional: SDL_Log("SoundManager: Stopped SFX '%s' on dedicated channel %d", name.c_str(), it->second.channel);
        } else {
            //  SDL_Log("SoundManager Warning: Cannot stop SFX '%s'. Not loaded or no channel assigned.", name.c_str());
        }
    }

    void setSoundEffectVolume(int volume) {
        if (volume < 0) volume = 0;
        if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;

        sfxVolume = volume;

        // Apply the new volume to all currently loaded sound chunks
        for (auto const& [name, info] : soundEffectMap) {
            if (info.chunk) {
                Mix_VolumeChunk(info.chunk, sfxVolume);
            }
        }
    }

    int getSoundEffectVolume() const {
        return sfxVolume;
    }


private:
    // --- Private Constructor for Singleton ---
    SoundManager() : musicVolume(MIX_MAX_VOLUME), sfxVolume(MIX_MAX_VOLUME), currentTrack(""), totalMixChannels(0), nextAvailableChannel(0) {}
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    // --- Data Members ---
    std::map<std::string, Mix_Music*> musicTracks;
    // Map SFX name to its info (chunk + dedicated channel)
    std::map<std::string, SoundEffectInfo> soundEffectMap;
    std::string currentTrack;
    int musicVolume;
    int sfxVolume;
    int totalMixChannels;     // Total number of mixing channels allocated
    int nextAvailableChannel; // Index of the next channel to assign

    // --- Helper Functions --- (Fade logic remains the same)
    void fadeOutAndPlay(const std::string& name, int loops, int fadeInMs) {
        const int fadeOutMs = 500;
        SDL_Log("SoundManager: Fading out current music (%s) to play '%s'.", currentTrack.c_str(), name.c_str());
        Mix_FadeOutMusic(fadeOutMs);

        Uint32 start = SDL_GetTicks();
        while (Mix_PlayingMusic() && (SDL_GetTicks() - start) < (Uint32)fadeOutMs + 100) {
            SDL_Delay(50);
        }

         if(Mix_PlayingMusic()){
              SDL_Log("SoundManager Warning: Music fadeout (%dms) incomplete or took longer than expected. Halting music.", fadeOutMs);
              Mix_HaltMusic();
         }

        auto it = musicTracks.find(name);
        if (it != musicTracks.end() && it->second != nullptr) {
            if (Mix_FadeInMusic(it->second, loops, fadeInMs) == -1) {
                SDL_Log("SoundManager Error: Failed to play music '%s' after fade out! Error: %s", name.c_str(), Mix_GetError());
                 currentTrack = "";
            } else {
                currentTrack = name;
                SDL_Log("SoundManager: Playing music '%s'.", name.c_str());
            }
        } else {
             SDL_Log("SoundManager Error: Music '%s' not found for fadeOutAndPlay.", name.c_str());
             currentTrack = "";
        }
    }
};

// Helper macro for easier access
#define SOUND_MANAGER SoundManager::getInstance()
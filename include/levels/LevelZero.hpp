#pragma once
#include "GameState.hpp"
#include "Level.hpp"
#include "theora/theoraplay.h"
#include "soundmanager.hpp"


// Audio queue structure for handling audio packets
typedef struct AudioQueue {
  const THEORAPLAY_AudioPacket *audio;
  int offset;
  struct AudioQueue *next;
} AudioQueue;

// Global audio queue variables
static volatile AudioQueue *audio_queue = NULL;
static volatile AudioQueue *audio_queue_tail = NULL;

// Audio callback function for SDL
static void SDLCALL audio_callback(void *userdata, Uint8 *stream, int len) {
  Sint16 *dst = (Sint16 *)stream;

  while (audio_queue && (len > 0)) {
    volatile AudioQueue *item = audio_queue;
    AudioQueue *next = item->next;
    const int channels = item->audio->channels;

    const float *src = item->audio->samples + (item->offset * channels);
    int cpy = (item->audio->frames - item->offset) * channels;
    int i;

    if (cpy > (len / sizeof(Sint16)))
      cpy = len / sizeof(Sint16);

    for (i = 0; i < cpy; i++) {
      const float val = *(src++);
      if (val < -1.0f)
        *(dst++) = -32768;
      else if (val > 1.0f)
        *(dst++) = 32767;
      else
        *(dst++) = (Sint16)(val * 32767.0f);
    }

    item->offset += (cpy / channels);
    len -= cpy * sizeof(Sint16);

    if (item->offset >= item->audio->frames) {
      THEORAPLAY_freeAudio(item->audio);
      SDL_free((void *)item);
      audio_queue = next;
    }
  }

  if (!audio_queue)
    audio_queue_tail = NULL;

  if (len > 0)
    memset(dst, '\0', len);
}

// Function to queue audio packets
static void queue_audio(const THEORAPLAY_AudioPacket *audio) {
  AudioQueue *item = (AudioQueue *)SDL_malloc(sizeof(AudioQueue));
  if (!item) {
    THEORAPLAY_freeAudio(audio);
    return;
  }

  item->audio = audio;
  item->offset = 0;
  item->next = NULL;

  SDL_LockAudio();
  if (audio_queue_tail)
    audio_queue_tail->next = item;
  else
    audio_queue = item;
  audio_queue_tail = item;
  SDL_UnlockAudio();
}

class LevelZero : public Level {
public:
  LevelZero(SDL_Renderer *renderer);
  ~LevelZero();
  void render(SDL_Renderer *renderer);
  void update();
  void handleEvents(SDL_Event *event, SDL_Renderer *renderer);

private:
  SDL_Texture *texture;
  THEORAPLAY_Decoder *decoder;
  const THEORAPLAY_VideoFrame *video;
  const THEORAPLAY_AudioPacket *audio;
  Uint32 baseticks;
  Uint32 framems;
  bool isOver;
  bool audioInitialized;
  SDL_AudioSpec spec;
};

void LevelZero::render(SDL_Renderer *renderer) {
  // Clear the renderer
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  if (isOver) {
    // If video is over, transition to menu
    GameState::setCurrentLevel(GameState::current_level + 1);
    return;
  }

  Uint32 now = SDL_GetTicks() - baseticks;

  // Get video frames when it's time
  if (!video)
    video = THEORAPLAY_getVideo(decoder);

  // Play video frames when it's time
  if (video && (video->playms <= now)) {
    // Skip frames to catch up if we're behind
    if (framems && ((now - video->playms) >= framems)) {
      const THEORAPLAY_VideoFrame *last = video;
      while ((video = THEORAPLAY_getVideo(decoder)) != NULL) {
        THEORAPLAY_freeVideo(last);
        last = video;
        if ((now - video->playms) < framems)
          break;
      }

      if (!video)
        video = last;
    }

    if (video) {
      // Create texture if needed
      if (!texture) {
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV,
                                    SDL_TEXTUREACCESS_STREAMING, video->width,
                                    video->height);
        if (!texture) {
          SDL_Log("Failed to create texture: %s", SDL_GetError());
          isOver = true;
          return;
        }
      }

      // Update texture with video frame data
      const int w = video->width;
      const int h = video->height;
      const Uint8 *y = (const Uint8 *)video->pixels;
      const Uint8 *u = y + (w * h);
      const Uint8 *v = u + ((w / 2) * (h / 2));

      SDL_UpdateYUVTexture(texture, NULL, y, w, u, w / 2, v, w / 2);

      // Free the video frame and get ready for the next one
      THEORAPLAY_freeVideo(video);
      video = NULL;
    }
  }

  // Always render the texture if it exists, even if we didn't update it this
  // frame
  if (texture) {
    SDL_RenderCopy(renderer, texture, NULL, NULL);
  }
}

void LevelZero::update() {
  // Check if decoder is still working
  if (!THEORAPLAY_isDecoding(decoder) && !video) {
    isOver = true;
  }

  // Pump the decoder to keep frames coming
  THEORAPLAY_pumpDecode(decoder, 5);

  // Process audio packets
  while ((audio = THEORAPLAY_getAudio(decoder)) != NULL) {
    queue_audio(audio);
  }
}

void LevelZero::handleEvents(SDL_Event *event, SDL_Renderer *r) {
  // Allow skipping the video with spacebar or escape
  if (event->type == SDL_KEYDOWN) {
    // make sdl break the game if Q was pressed
    if (event->key.keysym.sym == SDLK_SPACE) {
      isOver = true;
    }
  }
}

LevelZero::LevelZero(SDL_Renderer *renderer) : Level(renderer) {
  // Initialize variables
  texture = NULL;
  video = NULL;
  audio = NULL;
  isOver = false;
  audioInitialized = false;

  // Start decoding the video file
  decoder = THEORAPLAY_startDecodeFile("assets/video/video.ogv", 30,
                                       THEORAPLAY_VIDFMT_IYUV, NULL, 1);
  if (!decoder) {
    SDL_Log("Failed to start decoding video file");
    isOver = true;
    return;
  }

  // Stop playing music
  SOUND_MANAGER.stopMusic();
  SOUND_MANAGER.stopAllSoundEffects();

  // Wait for first video and audio frames
  while (!video || !audio) {
    THEORAPLAY_pumpDecode(decoder, 5);
    if (!video)
      video = THEORAPLAY_getVideo(decoder);
    if (!audio)
      audio = THEORAPLAY_getAudio(decoder);

    if (!THEORAPLAY_isDecoding(decoder) && (!video || !audio)) {
      SDL_Log("Failed to decode video or audio frames");
      isOver = true;
      return;
    }
    SDL_Delay(10);
  }

  // Set up audio
  memset(&spec, '\0', sizeof(SDL_AudioSpec));
  spec.freq = audio->freq;
  spec.format = AUDIO_S16SYS;
  spec.channels = audio->channels;
  spec.samples = 2048;
  spec.callback = audio_callback;

  // Close any existing audio device
  SDL_CloseAudio();

  if (SDL_OpenAudio(&spec, NULL) != 0) {
    SDL_Log("Failed to open audio: %s", SDL_GetError());
  } else {
    audioInitialized = true;
    SDL_PauseAudio(0); // Start audio playback
  }

  // Queue initial audio
  queue_audio(audio);
  audio = NULL;

  // Calculate frame duration in milliseconds
  framems = (video->fps == 0.0) ? 0 : ((Uint32)(1000.0 / video->fps));

  // Set base time for playback
  baseticks = SDL_GetTicks();
}

LevelZero::~LevelZero() {
  // Clean up resources
  if (video) {
    THEORAPLAY_freeVideo(video);
  }

  if (texture) {
    SDL_DestroyTexture(texture);
  }

  if (decoder) {
    THEORAPLAY_stopDecode(decoder);
  }

  // Clean up audio
  if (audioInitialized) {
    SDL_CloseAudio();
  }
}
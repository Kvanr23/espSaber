#include "Arduino.h"
#include "Sound.h"

Sound::Sound()
{
  // Only DAC0 for now. (Mono sound)
  out = new AudioOutputI2S(0, 1);
  wav = new AudioGeneratorWAV();

  wavDoneMsgSent = false;
}

void Sound::play(const char *wavFileName)
{
  playingSoundEffect = true;
  wavDoneMsgSent = false;

  if (wav->isRunning())
  {
    wav->stop();
    delete file;
  }
  file = new AudioFileSourceSD(wavFileName);
  wav->begin(file, out);
}

void Sound::audioLoop(int soundToPlay)
{
  if (wav->isRunning())
  {
    if (!wav->loop())
    {
      wav->loop();
    }
  }
  else
  {
    if (!wavDoneMsgSent)
    {
      wavDoneMsgSent = true;
      playingSoundEffect = false;
      delete file;
    }
  }
}

bool Sound::soundEffectPlaying(void)
{
    return playingSoundEffect;
}

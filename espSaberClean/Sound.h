#include "Arduino.h"
#include "AudioFileSourceSD.h"
#include "AudiogeneratorWAV.h"
#include "AudioOutputI2S.h"

#ifndef Sound_h
#define Sound_h

class Sound
{
  public:
    Sound(int dac);
    void play(const char *wavFileName);
    void audioLoop(int soundToPlay);
    void soundEffectPlaying(void);
  private:
    AudioGeneratorWAV *wav;
    AudioFileSourceSD *file;
    AudioOutputI2S *out;
    bool wavDoneMsgSent;
    bool playingSoundEffect;
}


#endif

/** Expected wav files are:
 * Sword On sound:
 *  - /ON.wav
 * 
 * Sword Off sound:
 *  - /OFF.wav
 * 
 * Hum sound:
 *  - /HUM.wav
 * 
 * Slow Swing 1 sound:
 *  - /SWL1.wav
 * Slow Swing 2 sound:
 *  - /SWL2.wav
 * Slow Swing 3 sound:
 *  - /SWL3.wav
 * Slow Swing 4 sound:
 *  - /SWL4.wav
 * Fast Swing 1 sound:
 *  - /SWS1.wav
 * Fast Swing 2 sound:
 *  - /SWS2.wav
 * Fast Swing 3 sound:
 *  - /SWS3.wav
 * Fast Swing 4 sound:
 *  - /SWS4.wav
 * Fast Swing 5 sound:
 *  - /SWS5.wav
 * */

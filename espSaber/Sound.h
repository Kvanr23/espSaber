#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

bool timeToHum(long period);
void setupSound(void);
void playSound(const char *wavFileName);
void hum(void);
void audioLoop(int soundToPlay);
bool soundEffectPlaying(void);

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

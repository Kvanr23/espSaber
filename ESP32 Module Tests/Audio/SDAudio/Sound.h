#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

bool timeToHum(long period);
void setupSound(void);
void playSound(const char *wavFileName);
void hum(void);
void audioLoop(int soundToPlay);
bool soundEffectPlaying(void);

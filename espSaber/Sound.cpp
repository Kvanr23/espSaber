#include "Sound.h"

int humCycle;
long lastHum;
bool wavDoneMsgSent;
bool playingSoundEffect;
bool humPlaying;

AudioGeneratorWAV *wav;
AudioFileSourceSD *file;
AudioOutputI2S *out;

void setupSound(void)
{
    out = new AudioOutputI2S(0, 1);
    wav = new AudioGeneratorWAV();

    wavDoneMsgSent = false;
    humCycle = millis() + 1000;
    Serial.printf("Sound system initiating\n");
    delay(500);
}

bool timeToHum(long period)
{
    if (millis() <= (lastHum + period))
    {
        return false;
    }
    else
    {
        lastHum = millis();
        return true;
    }
}

void playSound(const char *wavFileName)
{
    playingSoundEffect = true;
    wavDoneMsgSent = false;

    if ((wavFileName == "/HUM.wav"))
    {
        playingSoundEffect = false;
        humPlaying = true;
        Serial.printf("Hum\n");
    }
    else
    {
        if (wav->isRunning())
        {
            wav->stop();
            delete file;
            Serial.printf("Sound stopped\n");
        }
        Serial.printf("wavFileName");
    }
    file = new AudioFileSourceSD(wavFileName);
    wav->begin(file, out);
}

void hum(void)
{
    if (!playingSoundEffect)
    {
        if (wav->isRunning())
        {
            wav->stop();
            delete file;
            delay(1);
            lastHum = millis();
            playSound("/HUM.wav");
        }
    }
}

void audioLoop(int soundToPlay)
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
            Serial.printf("WAV done\n");
        }
    }
}

bool soundEffectPlaying(void)
{
    return playingSoundEffect;
}

#ifndef SoundFxManager_h
#define SoundFxManager_h

#include <DFPlayerMini.h>

class SoundFxManager
{
public:
    SoundFxManager();
    void initialize(DFPlayerMini *dFPlayerMini, int storedSounds, int defaultVolume);
    void playPowerUpSound();
    void playRandomSound();

private:
    DFPlayerMini _dFPlayerMini;
    int _storedSounds = 0;
};

#endif
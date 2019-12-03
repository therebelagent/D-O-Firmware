//
//  D-O SoundFxManager for DFPlayer Mini.
//  Created by Diego J. Arévalo.
//  D-O Builders group: https://www.facebook.com/groups/2468594199841880/
//  2019 v 1.0.
//

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
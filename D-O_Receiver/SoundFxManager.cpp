//
//  D-O SoundFxManager for DFPlayer Mini.
//  Created by Diego J. Arévalo.
//  D-O Builders group: https://www.facebook.com/groups/2468594199841880/
//  2019 v 1.0.
//

#include "SoundFxManager.h"

SoundFxManager::SoundFxManager() {}

void SoundFxManager::initialize(DFPlayerMini *dFPlayerMini, int storedSounds, int defaultVolume)
{
  _dFPlayerMini = *dFPlayerMini;
  _storedSounds = storedSounds;

  _dFPlayerMini.setVolume(map(defaultVolume, 1, 100, 1, 30));
}

void SoundFxManager::playPowerUpSound()
{
  if (!_dFPlayerMini.isBusy())
  {
    _dFPlayerMini.playFile(1);
  }
}

void SoundFxManager::playRandomSound()
{
  if (!_dFPlayerMini.isBusy())
  {
    int trackNumber = random(1, _storedSounds);
    _dFPlayerMini.playFile(trackNumber);
  }
}
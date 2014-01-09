#include "SoundPlayer.h"

SoundPlayer::SoundPlayer():
    m_soundBuffers(),
    m_sounds()
{
    m_soundBuffers.load(SoundEffect::AlliedGunfire, "alliedgunfire.wav");
    m_soundBuffers.load(SoundEffect::Button, "button.wav");
}

void SoundPlayer::play(SoundEffect::ID effect)
{
    m_sounds.push_back(sf::Sound(m_soundBuffers.get(effect)));
    m_sounds.back().play();
}

void SoundPlayer::removeStoppedSounds()
{
    m_sounds.remove_if([] (const sf::Sound& s)
                       {
                           return s.getStatus() == sf::Sound::Stopped;
                       });
}
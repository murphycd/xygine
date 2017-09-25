/*********************************************************************
(c) Matt Marchant 2017
http://trederia.blogspot.com

xygineXT - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#ifndef XY_AUDIO_SOURCE_IMPL_HPP_
#define XY_AUDIO_SOURCE_IMPL_HPP_

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/Music.hpp>

#include <SFML/System/Vector2.hpp>

#include <string>

namespace xy
{
    namespace Detail
    {
        class AudioSourceImpl
        {
        public:
            virtual ~AudioSourceImpl() = default;

            virtual void play() = 0;
            virtual void pause() = 0;
            virtual void stop() = 0;

            virtual void setPitch(float) = 0;
            virtual void setVolume(float) = 0;
            virtual void setPosition(sf::Vector2f) = 0;
            virtual void setRelativeTolistener(bool) = 0;
            virtual void setMinDistance(float) = 0;
            virtual void setAttenuation(float) = 0;
            virtual void setLooped(bool) = 0;

            virtual float getPitch() const = 0;
            virtual float getVolume() const = 0;
            virtual sf::Vector2f getPosition() const = 0;
            virtual bool isRelativeToListener() const = 0;
            virtual float getMinDistance() const = 0;
            virtual float getAttenuation() const = 0;
            virtual bool isLooped() const = 0;

            virtual sf::Int32 getStatus() const = 0;
        };

        class AudioSound final : public AudioSourceImpl
        {
        public:
            AudioSound(const sf::SoundBuffer&);

            void play() override;
            void pause() override;
            void stop() override;

            void setPitch(float) override;
            void setVolume(float) override;
            void setPosition(sf::Vector2f) override;
            void setRelativeTolistener(bool) override;
            void setMinDistance(float) override;
            void setAttenuation(float) override;
            void setLooped(bool) override;

            float getPitch() const override;
            float getVolume() const override;
            sf::Vector2f getPosition() const override;
            bool isRelativeToListener() const override;
            float getMinDistance() const override;
            float getAttenuation() const override;
            bool isLooped() const override;

            sf::Int32 getStatus() const override;

        private:
            sf::Sound m_sound;
        };

        class AudioMusic final : public AudioSourceImpl
        {
        public:
            AudioMusic(const std::string&);
            ~AudioMusic();

            void play() override;
            void pause() override;
            void stop() override;

            void setPitch(float) override;
            void setVolume(float) override;
            void setPosition(sf::Vector2f) override;
            void setRelativeTolistener(bool) override;
            void setMinDistance(float) override;
            void setAttenuation(float) override;
            void setLooped(bool) override;

            float getPitch() const override;
            float getVolume() const override;
            sf::Vector2f getPosition() const override;
            bool isRelativeToListener() const override;
            float getMinDistance() const override;
            float getAttenuation() const override;
            bool isLooped() const override;

            sf::Int32 getStatus() const override;

            bool isValid() const { return m_valid; }

        private:

            sf::Music m_music;
            bool m_valid;
        };
    }
}

#endif //XY_AUDIO_SOURCE_IMPL_HPP_
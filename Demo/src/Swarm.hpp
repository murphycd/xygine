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

#pragma once

#include <xyginext/ecs/System.hpp>

#include <SFML/System/Vector2.hpp>

#include <array>

struct Bug final
{
    float brightness = 1.f;
    sf::Vector2f velocity;
    sf::Vector2f position;
};

struct Swarm final
{
    std::array<Bug, 20u> bugs;
    
    Swarm();
};

class SwarmSystem final : public xy::System
{
public:
    explicit SwarmSystem(xy::MessageBus&);

    void process(float) override;
};

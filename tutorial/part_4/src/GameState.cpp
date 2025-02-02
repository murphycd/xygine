/*********************************************************************
(c) Matt Marchant 2019

This file is part of the xygine tutorial found at
https://github.com/fallahn/xygine

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

#include "GameState.hpp"
#include "States.hpp"
#include "CommandIDs.hpp"
#include "ResourceIDs.hpp"
#include "BallSystem.hpp"
#include "CollisionSystem.hpp"
#include "MessageIDs.hpp"
#include "ShapeUtils.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/components/BroadPhaseComponent.hpp>

#include <xyginext/ecs/systems/DynamicTreeSystem.hpp>
#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>

#include <xyginext/core/App.hpp>
#include <xyginext/util/Vector.hpp>

#include <SFML/Window/Event.hpp>

GameState::GameState(xy::StateStack& ss, xy::State::Context ctx) :
xy::State(ss,ctx),
m_gameScene(ctx.appInstance.getMessageBus())
{
    createScene();
    
    m_gameScene.getActiveCamera().getComponent<xy::Camera>().setView(ctx.defaultView.getSize());
    m_gameScene.getActiveCamera().getComponent<xy::Camera>().setViewport(ctx.defaultView.getViewport());

    ctx.appInstance.setMouseCursorVisible(false);
}

bool GameState::handleEvent(const sf::Event& evt)
{
    //look for mouse move events and send them to the paddle
    if(evt.type == sf::Event::MouseMoved)
    {
        auto worldMousePosition = xy::App::getRenderWindow()->mapPixelToCoords({evt.mouseMove.x, evt.mouseMove.y});
        xy::Command cmd;
        cmd.targetFlags = CommandID::Paddle;
        cmd.action = [worldMousePosition](xy::Entity entity, float)
        {
            //clamp the X position in the screen area minus the sprite width
            float posX = worldMousePosition.x;
            auto spriteWidth = entity.getComponent<xy::Sprite>().getTextureBounds().width / 2.f;
            posX = std::max(spriteWidth, worldMousePosition.x);
            posX = std::min(posX, xy::DefaultSceneSize.x - spriteWidth);

            auto& tx = entity.getComponent<xy::Transform>();
            auto currentPosition = tx.getPosition();
            currentPosition.x = posX;
            tx.setPosition(currentPosition);
        };
        m_gameScene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }
    else if(evt.type == sf::Event::MouseButtonReleased)
    {
        if(evt.mouseButton.button == sf::Mouse::Left)
        {
            //send a command to the paddle to launch the ball if it has one
            //else spawn a new ball
            xy::Command cmd;
            cmd.targetFlags = CommandID::Paddle;
            cmd.action = [&](xy::Entity entity, float)
            {
                auto& paddle = entity.getComponent<Paddle>();
                if(paddle.ball.isValid())
                {
                    paddle.ball.getComponent<Ball>().state = Ball::State::Active;
                    paddle.ball.getComponent<Collider>().dynamic = true;
                    auto ballBounds = paddle.ball.getComponent<xy::Sprite>().getTextureBounds();
                    paddle.ball.getComponent<xy::Transform>().setPosition(entity.getComponent<xy::Transform>().getPosition() + sf::Vector2f(0.f, -ballBounds.height / 2.f));
                    entity.getComponent<xy::Transform>().removeChild(paddle.ball.getComponent<xy::Transform>());
                    paddle.ball = {};
                }
            };
            m_gameScene.getSystem<xy::CommandSystem>().sendCommand(cmd);
        }
    }

    m_gameScene.forwardEvent(evt);
    return true;
}

void GameState::handleMessage(const xy::Message& msg)
{
    if (msg.id == MessageID::BallMessage)
    {
        const auto& data = msg.getData<BallEvent>();
        if (data.action == BallEvent::Despawned)
        {
            spawnBall();
        }
    }

    m_gameScene.forwardMessage(msg);
}

bool GameState::update(float dt)
{
    m_gameScene.update(dt);
    return true;
}

void GameState::draw()
{
    auto rw = getContext().appInstance.getRenderWindow();
    rw->draw(m_gameScene);
}

xy::StateID GameState::stateID() const
{
    return States::GameState;
}

void GameState::createScene()
{
    //add the systems
    auto& messageBus = getContext().appInstance.getMessageBus();
    m_gameScene.addSystem<BallSystem>(messageBus);
    m_gameScene.addSystem<xy::DynamicTreeSystem>(messageBus);
    m_gameScene.addSystem<CollisionSystem>(messageBus);
    m_gameScene.addSystem<xy::CommandSystem>(messageBus);
    m_gameScene.addSystem<xy::SpriteSystem>(messageBus);
    m_gameScene.addSystem<xy::RenderSystem>(messageBus);

    loadResources();

    //create the paddle
    auto entity = m_gameScene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize.x / 2.f, xy::DefaultSceneSize.y - 40.f);
    entity.addComponent<xy::Sprite>(m_resources.get<sf::Texture>(TextureID::handles[TextureID::Paddle]));
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::CommandTarget>().ID = CommandID::Paddle;
    entity.addComponent<Paddle>();

    auto paddleBounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Transform>().setOrigin(paddleBounds.width / 2.f, paddleBounds.height / 2.f);
    entity.addComponent<xy::BroadphaseComponent>(paddleBounds);
    entity.addComponent<Collider>();

    spawnBall();

    //create the colliders on 3 sides
    sf::FloatRect wallBounds = { 0.f, 0.f, xy::DefaultSceneSize.x, 20.f };
    entity = m_gameScene.createEntity();
    entity.addComponent<xy::Transform>();
    entity.addComponent<xy::BroadphaseComponent>(wallBounds);
    entity.addComponent<Collider>();
    Shape::setRectangle(entity.addComponent<xy::Drawable>(), { wallBounds.width, wallBounds.height });

    wallBounds.width = 20.f;
    wallBounds.height = xy::DefaultSceneSize.y;
    entity = m_gameScene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(0.f, 20.f);
    entity.addComponent<xy::BroadphaseComponent>(wallBounds);
    entity.addComponent<Collider>();
    Shape::setRectangle(entity.addComponent<xy::Drawable>(), { wallBounds.width, wallBounds.height });

    entity = m_gameScene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize.x - wallBounds.width, 20.f);
    entity.addComponent<xy::BroadphaseComponent>(wallBounds);
    entity.addComponent<Collider>();
    Shape::setRectangle(entity.addComponent<xy::Drawable>(), { wallBounds.width, wallBounds.height });
}

void GameState::loadResources()
{
    TextureID::handles[TextureID::Ball] = m_resources.load<sf::Texture>("assets/images/ball.png");
    TextureID::handles[TextureID::Paddle] = m_resources.load<sf::Texture>("assets/images/paddle.png");
}

void GameState::spawnBall()
{
    xy::Command cmd;
    cmd.targetFlags = CommandID::Paddle;
    cmd.action = [&](xy::Entity entity, float)
    {
        auto& paddle = entity.getComponent<Paddle>();
        paddle.ball = m_gameScene.createEntity();
        paddle.ball.addComponent<xy::Transform>();
        paddle.ball.addComponent<xy::Sprite>(m_resources.get<sf::Texture>(TextureID::handles[TextureID::Ball]));
        paddle.ball.addComponent<xy::Drawable>();
        paddle.ball.addComponent<Ball>();

        auto ballBounds = paddle.ball.getComponent<xy::Sprite>().getTextureBounds();
        auto paddleBounds = entity.getComponent<xy::Sprite>().getTextureBounds();
        paddle.ball.getComponent<xy::Transform>().setOrigin(ballBounds.width / 2.f, ballBounds.height / 2.f);
        paddle.ball.getComponent<xy::Transform>().setPosition(paddleBounds.width / 2.f, -ballBounds.height / 2.f);
        paddle.ball.addComponent<xy::BroadphaseComponent>(ballBounds);
        paddle.ball.addComponent<Collider>().callback = 
            [](xy::Entity e, xy::Entity other, Manifold man)
        {
            //if we hit the paddle change the velocity angle
            if (other.hasComponent<Paddle>())
            {
                auto newVel = e.getComponent<xy::Transform>().getPosition() - other.getComponent<xy::Transform>().getPosition();
                e.getComponent<Ball>().velocity = xy::Util::Vector::normalise(newVel);
            }
            else
            {
                //reflect the ball's velocity around the collision normal
                auto vel = e.getComponent<Ball>().velocity;
                vel = xy::Util::Vector::reflect(vel, man.normal);
                e.getComponent<Ball>().velocity = vel;
            }
        };

        entity.getComponent<xy::Transform>().addChild(paddle.ball.getComponent<xy::Transform>());
    };
    m_gameScene.getSystem<xy::CommandSystem>().sendCommand(cmd);
}

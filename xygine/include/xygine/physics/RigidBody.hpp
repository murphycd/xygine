/*********************************************************************
Matt Marchant 2014 - 2015
http://trederia.blogspot.com

xygine - Zlib license.

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

//creates a rigidbody component allowing the entity to be updated
//by the physics world. A valid physics world object must exist when
//these components are added to an entity, and require shape
//fixtures attached to them to have any affect. Entities with a rigid
//body attached will be unable to have their position set manually as 
//it will be automatically overriden by the physics simulation.

#ifndef XY_PHYSICS_RIGIDBODY_HPP_
#define XY_PHYSICS_RIGIDBODY_HPP_

#include <xygine/Component.hpp>
#include <xygine/physics/CollisionShape.hpp>
#include <xygine/physics/World.hpp>

#include <Box2D/Dynamics/b2Body.h>

#include <vector>
#include <memory>

namespace xy
{
    namespace Physics
    {
        enum class BodyType
        {
            Static = b2_staticBody,
            Dynamic = b2_dynamicBody,
            Kinematic = b2_kinematicBody
        };

        class RigidBody final : public Component
        {
        public:
            RigidBody(MessageBus&, BodyType);
            ~RigidBody() = default;

            RigidBody(const RigidBody&) = delete;
            const RigidBody& operator = (const RigidBody&) = delete;

            Component::Type type() const override
            {
                return Component::Type::Physics;
            }

            void entityUpdate(Entity&, float) override;
            void handleMessage(const Message&) override;
            void onStart(Entity&) override;
            void destroy() override;

            //linear velocity of the body in world coordinates
            void setLinearVelocity(const sf::Vector2f&);
            //set the angular velocity of the body
            void setAngularVelocity(float);
            //set the linear damping which will be applied to
            //this body
            void setLinearDamping(float);
            //set the angular damping which will be applied to
            //this body
            void setAngularDamping(float);
            //set allowing this body to sleep when inactive
            void allowSleep(bool);
            //set whether or not this body is awake initially
            void awakeOnSpawn(bool);
            //set whether or not this body can rotate
            void fixedRotation(bool);
            //set this to true if this body is expected to be
            //a fast moving object to reduce tunneling. can be
            //process intensive so should be used sparingly
            void isBullet(bool);
            //sets whether or not this body is active on spawn
            void activeOnSpawn(bool);
            //sets the scale of the gravity applied to this body
            //negative numbers can be used
            void setGravityScale(float);
            //adds a collision shape to this body. The original shape is
            //unmodified (so it can be attached to multiple bodies) and
            //a reference to the newly created shape is returned
            template <typename T>
            T* addCollisionShape(const T& cs)
            {
                static_assert(std::is_base_of<CollisionShape, T>::value, "Can only add shapes of collision type");
                m_collisionShapes.emplace_back(std::make_unique<T>(cs));
                auto& newShape = m_collisionShapes.back();
                if (m_body)
                {
                    newShape->m_fixture = m_body->CreateFixture(&newShape->m_fixtureDef);
                }
                else
                {
                    m_pendingShapes.push_back(newShape.get());
                }
                return dynamic_cast<T*>(newShape.get());
            }

        private:

            b2BodyDef m_bodyDef;
            b2Body* m_body;

            std::vector<std::unique_ptr<CollisionShape>> m_collisionShapes;
            std::vector<CollisionShape*> m_pendingShapes;
        };
    }
}

#endif //XY_PHYSICS_RIGIDBODY
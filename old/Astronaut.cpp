#include "Astronaut.h"
#include "Actor.h"
#include "ResourceHolder.h"
#include "Utility.h"
#include "TextNode.h"

#include <string>
#include <cmath>

// TODO Vitesse max

const float SCALE = 30.f; // Box2D works in a scale of 30 pixels = 1 meter

Astronaut::Astronaut(Type type, const TextureHolder& textures, const FontHolder& fonts, b2World& world):
    Actor(type, textures, fonts, world),
    m_power(100.f),
    m_powerRecoveryTime(sf::seconds(3.f)),
    m_isThrusting(false),
    m_isFiring(false),
    m_powerDisplay(nullptr),
    m_fireCommand(),
    m_targetPos()
{
    if(type != Actor::Hero)
    {
        std::unique_ptr<TextNode> powerDisplay(new TextNode(fonts, (std::to_string(m_power) + " PW")));
        powerDisplay->setPosition(0.f, -50.f);
        m_powerDisplay = powerDisplay.get();
        attachChild(std::move(powerDisplay));
    }

    m_powerRecovery.restart();

    m_fireCommand.category = Category::UpperScene;
    m_fireCommand.action =
    [this, &textures, &world] (SceneNode& node, sf::Time)
    {
        createBullets(node, textures, world);
    };
}

// TODO if in the check function
void Astronaut::thrusterUp()
{
    if (m_body->GetLinearVelocity().y * SCALE < 300 && m_power)
      m_body->ApplyForce(b2Vec2(0, m_body->GetMass()*25), m_body->GetWorldCenter(), true);

    m_isThrusting = true;
}

void Astronaut::thrusterDown()
{
    if (m_body->GetLinearVelocity().y * SCALE > -300 && m_power)
        m_body->ApplyForce(b2Vec2(0, -m_body->GetMass()*25), m_body->GetWorldCenter(), true);

    m_isThrusting = true;
}

void Astronaut::thrusterLeft()
{
    if (m_body->GetLinearVelocity().x * SCALE < 300 && m_power)
        m_body->ApplyForce(b2Vec2(m_body->GetMass()*10, 0), m_body->GetWorldCenter(), true);

    m_isThrusting = true;
}

void Astronaut::thrusterRight()
{
    if (m_body->GetLinearVelocity().x * SCALE > -300 && m_power)
      m_body->ApplyForce(b2Vec2(-m_body->GetMass()*10, 0), m_body->GetWorldCenter(), true);

    m_isThrusting = true;
}

float Astronaut::getPower() const
{
    return m_power;
}
// TODO prendre en compte le dt pour enlever le clock
// TODO � nettoyer
void Astronaut::checkThrusters(sf::Time dt, CommandQueue& commands)
{
    static bool isPlayed = false;
    if(m_isThrusting)
    {
        m_powerRecovery.restart();
        //TODO DEBUG m_power--;
        m_isThrusting = false;
        isPlayed = false;
        if(m_power < 0)
            m_power = 0;
    }
    else if(m_powerRecovery.getElapsedTime() > m_powerRecoveryTime)
    {
        if(!isPlayed && m_power < 100.f)
        {
            playLocalSound(commands, SoundEffect::Regenerate);
            isPlayed = true;
        }
        m_power++;

        if(m_power > 100)
            m_power = 100;
    }
}

// 1 prendre dans player la position de la souris avec Event (voir si c'est relatif � la fenetre )ensuite l'envoyer � fire et mettre a jour une targetDirection et calsuler la direction de la bullet
// 2 passer window � player
// limiter le radius de tir
//TODO Nettoyer
void Astronaut::fire(const sf::Vector2f& targetPos)
{
    m_isFiring = true;
    m_targetPos = targetPos;

    sf::Vector2f relativeUnitPos = Utility::unitVector((targetPos - getWorldPosition()));

    if(relativeUnitPos.x < 0)
        lookLeft();
    else if(relativeUnitPos.x > 0)
        lookRight();
}

void Astronaut::checkProjectileLaunch(sf::Time dt, CommandQueue& commands)
{
    if(m_isFiring && m_fireCountdown <= sf::Time::Zero)
    {
        playLocalSound(commands, SoundEffect::AlliedGunfire);
        commands.push(m_fireCommand);
        m_fireCountdown += sf::seconds(1.f/(4.f));
        m_isFiring = false;
    }
    else if(m_fireCountdown > sf::Time::Zero)
    {
        m_fireCountdown -= dt;
        m_isFiring = false;
    }
}
void Astronaut::createBullets(SceneNode& node, const TextureHolder& textures, b2World& world) const
{
    Projectile::Type type;
    if(m_type == Actor::Type::Hero || m_type == Actor::Type::Allied)
        type = Projectile::AlliedBullet;
    else
        type = Projectile::EnemyBullet;
    // Gun position
    if(m_lookingOrientation == LookingOrientation::Right)
        createProjectile(node, type, 1.2f, 0.5f, textures, world);
    else if(m_lookingOrientation == LookingOrientation::Left)
        createProjectile(node, type, -1.2f, 0.5f, textures, world);
}

// TODO � nettoyer
void Astronaut::createProjectile(SceneNode& node, Projectile::Type type, float xOffset, float yOffset, const TextureHolder& textures, b2World& world) const
{
    std::unique_ptr<Projectile> projectile(new Projectile(type, textures, world, 5));

    sf::Vector2f astronautPos(getWorldPosition()); // position de lasronaut
    sf::Vector2f offset(xOffset * m_walkAnim.getGlobalBounds().width, yOffset * m_walkAnim.getGlobalBounds().height); // offset
    // TODO � voir bug origin et offset douteux
    projectile->setPosition(getWorldPosition() + offset);

    sf::Vector2f projectileDirection(m_targetPos - astronautPos);
    projectileDirection = Utility::unitVector(projectileDirection);

    // 75 deg cone to shoot
    if(projectileDirection.y > 0.75f)
        projectileDirection.y = 0.75f;
    if(projectileDirection.y < -0.75f)
        projectileDirection.y = -0.75f;

    projectileDirection *= 100.f; // Pour avoir un angle en deg

    float vx;
    if(m_lookingOrientation == LookingOrientation::Left)
        vx = -20.f;
    else
        vx = 20.f;

    float vy = 20.f * std::sin((projectileDirection.y*Utility::pi())/180.f); // sin(radians)

    projectile->getBody()->SetLinearVelocity(b2Vec2(vx, vy));
    node.attachChild(std::move(projectile));
}

void Astronaut::updateCurrent(sf::Time dt, CommandQueue& commands)
{
    checkThrusters(dt, commands);
    checkProjectileLaunch(dt, commands);
    Actor::updateCurrent(dt, commands);
}

void Astronaut::updateText()
{
    if(m_powerDisplay)
        m_powerDisplay->setString(std::to_string(m_power) + " PW");
}

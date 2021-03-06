#include "Asteroid.h"

const float SCALE = 30.f; // Box2D works in a scale of 30 pixels = 1 meter

Asteroid::Asteroid(const TextureHolder& textures, b2World& world):
    Entity(createBody(world))
{
    AsteroidGenerator::makeRandomAsteroid(*this);
    m_shape.setTexture(&textures.get(Textures::Asteroid));

    int pointsCount = m_shape.getPointCount();

    b2Vec2 vertices[pointsCount];
    int j = pointsCount - 1;
    for(int i = 0; i < pointsCount; ++i) // CCW
    {
        vertices[j].Set(m_shape.getPoint(i).x / SCALE, m_shape.getPoint(i).y / SCALE);
        j--;
    }

    b2FixtureDef AsteroidFixtureDef;
    b2PolygonShape AsteroidShape;
    AsteroidShape.Set(vertices, pointsCount);
    AsteroidFixtureDef.shape = &AsteroidShape;
    AsteroidFixtureDef.density = 1.0f;
    AsteroidFixtureDef.userData = this;
    m_body->CreateFixture(&AsteroidFixtureDef);

    m_shape.setOrigin(m_body->GetWorldCenter().x, m_body->GetWorldCenter().y);
}

void Asteroid::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(m_shape, states);
}

b2Body* Asteroid::createBody(b2World& world)
{
    b2BodyDef AsteroidBodyDef;
    AsteroidBodyDef.type = b2_staticBody;
    b2Body* body = world.CreateBody(&AsteroidBodyDef);
    return body;
}

unsigned int Asteroid::getCategory() const
{
    return Category::LowerScene;
}

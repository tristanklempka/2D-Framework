#ifndef TEXTNODE_H
#define TEXTNODE_H

#include <string>

#include <SFML/Graphics.hpp>

#include <FRK2D/ResourceHolder.hpp>
#include <FRK2D/SceneNode.hpp>

class TextNode : public SceneNode
{
    public:
        explicit        TextNode(const FontHolder& fonts, const std::string& text);

        void            setString(const std::string& text);

    private:
        virtual void    drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

    private:
        sf::Text        m_text;
};

#endif // TEXTNODE_H

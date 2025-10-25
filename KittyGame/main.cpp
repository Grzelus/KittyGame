#include <SFML/Graphics.hpp>
#include <stdlib.h>
#include <time.h>

class Enemy {
private:
    float size = 20.f;
public:
    Enemy() {};
    float getSize() {
        return this->size;
    };
    sf::CircleShape spawn() {
        float x = rand() % (800 + 1);
        float y = rand() % (800 + 1);
        sf::CircleShape body(size);
        body.setFillColor(sf::Color::Red);
        body.setPosition(x, y);
        return body;
    };
};

int main()
{
    srand(time(nullptr));

    sf::RenderWindow window(sf::VideoMode(800, 600), "Movement");
    sf::CircleShape circle(100.f);
    circle.setPosition(100.f, 100.f);
    circle.setFillColor(sf::Color::Magenta);

    Enemy enemy = Enemy();
    sf::CircleShape enemy_body = enemy.spawn();

    sf::Clock clock;
    float speed = 300.f;
    while (window.isOpen()) {
        //Close window Handler
        sf::Event event;
        {
        while (window.pollEvent(event))
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        }

        float deltaTime = clock.restart().asSeconds();
        //Movement
        sf::Vector2f movement(0.f, 0.f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            movement.x -= speed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            movement.x += speed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            movement.y += speed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            movement.y -= speed;
        }
        if (circle.getPosition().x <= 0 && movement.x < 0) {
            movement.x = 0;
        }
        if (circle.getPosition().x + circle.getRadius()*2 >= 800 && movement.x > 0) {
            movement.x = 0;
        }
        if (circle.getPosition().y <= 0 && movement.y < 0) {
            movement.y = 0;
        }
        if (circle.getPosition().y + circle.getRadius() * 2 >= 600 && movement.y > 0) {
            movement.y = 0;
        }

        circle.move(movement * deltaTime);

        window.clear();
        window.draw(circle);
        window.draw(enemy_body);
        window.display();
    }
    return 0;
}

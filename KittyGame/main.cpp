#include <SFML/Graphics.hpp>
#include <stdlib.h>
#include <time.h>
#include <iostream>

//Classes
class Enemy : public sf::CircleShape{
private:
    float size = 20.f;
    float speed = 50.f;
 
public:
    Enemy() : sf::CircleShape(20.f) {
        this->setFillColor(sf::Color::Red);
    };
    float getSize() {
        return this->size;
    }
        float getSpeed() {
            return this->speed;
    };
    void spawn() {
        float x = rand() % (800 + 1);
        float y = rand() % (600 + 1);
        this->setPosition(x, y);
    };
};

//Functionality
bool checkColision(const sf::CircleShape & a, const sf::CircleShape & b) {
    sf::Vector2f aCenter = a.getPosition() + sf::Vector2f(a.getRadius(), a.getRadius());
    sf::Vector2f bCenter = b.getPosition() + sf::Vector2f(b.getRadius(), b.getRadius());
    float dist = std::hypot(aCenter.x - bCenter.x, aCenter.y - bCenter.y);
    return dist < (a.getRadius() + b.getRadius());
}

int main()
{
    srand(time(nullptr));

    sf::RenderWindow window(sf::VideoMode(800, 600), "Movement");
    sf::CircleShape player(100.f);
    player.setPosition(100.f, 100.f);
    player.setFillColor(sf::Color::Magenta);

    int counter = 0;

    //Spawning Enemies
    std::vector<Enemy> enemies;
    for (int i = 0; i < 10; i++) {
        Enemy enemy = Enemy();
        enemy.spawn();
        enemies.push_back(enemy);
    }

   

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
        sf::Vector2f playerCenter = player.getPosition() + sf::Vector2f(player.getRadius(), player.getRadius());
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
        if (player.getPosition().x <= 0 && movement.x < 0) {
            movement.x = 0;
        }
        if (player.getPosition().x + player.getRadius()*2 >= 800 && movement.x > 0) {
            movement.x = 0;
        }
        if (player.getPosition().y <= 0 && movement.y < 0) {
            movement.y = 0;
        }
        if (player.getPosition().y + player.getRadius() * 2 >= 600 && movement.y > 0) {
            movement.y = 0;
        }

        player.move(movement * deltaTime);


        //Scoring points 
        for (auto enemy = enemies.begin(); enemy != enemies.end();) {
            if (checkColision(player, *enemy)) {
                enemy = enemies.erase(enemy);
                counter++;
                std::cout << counter << std::endl;
            }
            else {
                enemy++;
            }
        }
        // Roaming Enemies
        for (auto& enemy : enemies) {
            sf::Vector2f enemyCenter = enemy.getPosition() + sf::Vector2f(enemy.getRadius(), enemy.getRadius());
            sf::Vector2f direction = playerCenter - enemyCenter;
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            if (length != 0) {
                direction /= length;
            }
            enemy.move(direction.x * deltaTime * enemy.getSpeed(), direction.y * deltaTime * enemy.getSpeed());
        }
        //Rendering
        window.clear();
        window.draw(player);
        for (auto& e : enemies) {
            window.draw(e);
        }
        window.display();
    }
    return 0;
}

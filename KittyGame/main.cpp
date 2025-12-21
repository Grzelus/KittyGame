#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>


//TODO

// balans attack speedu i dodanie jego wizualnej reprezentacji

// Classes

class GameObject {
protected:
    sf::Shape* body = nullptr;
    bool active = false;
public:
    GameObject() = default;
    virtual ~GameObject() = default;

    virtual void renderBody(sf::RenderWindow& window) {
        if (body) {
            window.draw(*body);
        }
    }

    void setBody(sf::Shape* b) { body = b; }
    sf::Shape* getBodyPtr() const { return body; }
};

class Item : public GameObject {
public:
    void PlayerEffect() {}
};

class Bullet : public GameObject {
private:
    float damage;
    float size = 15.f;
    float speed = 500.f;
    float x_position;
    float y_position;
    float angle;
    sf::CircleShape bulletShape;
public:
    Bullet(float o_damage, float o_angle, float new_x, float new_y)
        : damage(o_damage), angle(o_angle), x_position(new_x), y_position(new_y) {
        setBodyShape();
    };

    void update(float deltaTime) {
        x_position += std::cos(angle) * speed * deltaTime;
        y_position += std::sin(angle) * speed * deltaTime;
    }

    void setBodyShape() {
        bulletShape = sf::CircleShape(size);
        bulletShape.setFillColor(sf::Color::Green);
    }

    void renderBody(sf::RenderWindow& window) {
        bulletShape.setPosition(x_position, y_position);
        window.draw(bulletShape);
    }

    float getX() const { return x_position; }
    float getY() const { return y_position; }
    float getDamage() const { return damage; }

    bool operator==(const Bullet& other) const {
        return (this->x_position == other.x_position) && (this->y_position == other.y_position);
    }
};

class ExpCrystal : public GameObject {};

class Character : public GameObject {
private:
    float hp;
    float MAX_HP;
    float speed;
    float attack;
    float attack_speed;
public:
    Character(float o_hp, float o_MAX_HP, float o_speed, float o_attack, float attack_speed)
        : hp(o_hp), MAX_HP(o_MAX_HP), speed(o_speed), attack(o_attack), attack_speed(attack_speed) {
    }

    void takedamage(float dmg) { hp -= dmg; }
    void heal() { this->hp = MAX_HP; }
    void setMAX_HP(float new_MAX_HP) { this->MAX_HP = new_MAX_HP; }
    void setSpeed(float new_speed) { this->speed = new_speed; }
    void setAttack(float new_attack) { this->attack = new_attack; }
    void set_attack_speed(float new_attack_speed) { this->attack_speed = new_attack_speed; }

    float getHp() const { return hp; }
    float getAttack() const { return attack; }
    float getSpeed() const { return speed; }
    float get_attack_speed() const { return attack_speed; }
};

void spawnGameOver(sf::RenderWindow& window);

class Player : public Character {
private:
    float size = 0.f;
    sf::CircleShape bodyShape;
    int experience = 0;
    int level = 1;
public:
    Player(float o_hp = 100, float o_MAX_HP = 100, float o_speed = 300, float o_attack = 5, float attack_speed = 1.f)
        : Character(o_hp, o_MAX_HP, o_speed, o_attack, attack_speed),
        size(50.f),
        bodyShape(size)
    {
        bodyShape.setPosition(100.f, 100.f);
        bodyShape.setFillColor(sf::Color::Magenta);
        setBody(&bodyShape);
    }

    void createBody(float radius, const sf::Vector2f& position, const sf::Color& color) {
        size = radius;
        bodyShape = sf::CircleShape(radius);
        bodyShape.setPosition(position);
        bodyShape.setFillColor(color);
        setBody(&bodyShape);
    }

    bool gainExperience(int exp) {
        this->experience += exp;
        if (experience >= level * 10) {
            experience -= level * 10;
            levelUp();
            return true;
        }
        return false;
    }

    void levelUp() {
        this->level += 1;
        this->heal();
    }

    float shooting_angle(const sf::RenderWindow& window) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f playerCenter = bodyShape.getPosition() + sf::Vector2f(bodyShape.getRadius(), bodyShape.getRadius());
        return std::atan2(mousePos.y - playerCenter.y, mousePos.x - playerCenter.x);
    }

    void attacked(sf::RenderWindow& window, float dmg) {
        takedamage(dmg);
    }

    void update(float deltaTime, const sf::RenderWindow& window, bool canMove) {
        if (!canMove) return;

        sf::Vector2f movement(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  movement.x -= getSpeed();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) movement.x += getSpeed();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  movement.y += getSpeed();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    movement.y -= getSpeed();

        sf::Vector2u winSize = window.getSize();
        float diameter = bodyShape.getRadius() * 2.f;
        sf::Vector2f pos = bodyShape.getPosition();

        if (pos.x <= 0.f && movement.x < 0.f) movement.x = 0.f;
        if (pos.x + diameter >= static_cast<float>(winSize.x) && movement.x > 0.f) movement.x = 0.f;
        if (pos.y <= 0.f && movement.y < 0.f) movement.y = 0.f;
        if (pos.y + diameter >= static_cast<float>(winSize.y) && movement.y > 0.f) movement.y = 0.f;

        bodyShape.move(movement * deltaTime);
    }

    const sf::CircleShape& getBody() const { return bodyShape; }
    sf::CircleShape& getBody() { return bodyShape; }
};

class Enemy {
private:
    float size = 20.f;
public:
    float x, y;
    sf::CircleShape body;
    Enemy() {
        body = spawn();
    }
    float calculate_spawn_position() {
        
    };
    sf::CircleShape spawn() const {
        sf::CircleShape shape(size);
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(x, y);
        return shape;
    }
};

// Functionality

bool checkColision(const sf::CircleShape& a, const sf::CircleShape& b) {
    sf::Vector2f aCenter = a.getPosition() + sf::Vector2f(a.getRadius(), a.getRadius());
    sf::Vector2f bCenter = b.getPosition() + sf::Vector2f(b.getRadius(), b.getRadius());
    float dist = std::hypot(aCenter.x - bCenter.x, aCenter.y - bCenter.y);
    return dist < (a.getRadius() + b.getRadius());
}

void spawnWeaponChoice(sf::RenderWindow& window) {
    sf::RectangleShape table(sf::Vector2f(400.f, 200.f));
    table.setFillColor(sf::Color::Blue);
    table.setPosition(150.f, 150.f);

    sf::CircleShape f_up(40.f, 3); f_up.setFillColor(sf::Color::Magenta); f_up.setPosition(200.f, 200.f);
    sf::CircleShape s_up(40.f, 4); s_up.setFillColor(sf::Color::Yellow); s_up.setPosition(300.f, 200.f);
    sf::CircleShape t_up(40.f, 5); t_up.setFillColor(sf::Color::Cyan); t_up.setPosition(400.f, 200.f);

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) return;

    sf::Text choices("+5 Speed    +5 Attack   +5HP", font, 18);
    choices.setFillColor(sf::Color::White);
    choices.setPosition(200.f, 300.f);

    window.draw(table);
    window.draw(f_up);
    window.draw(s_up);
    window.draw(t_up);
    window.draw(choices);
}

void spawnGameOver(sf::RenderWindow& window) {
    sf::RectangleShape table(sf::Vector2f(400.f, 220.f));
    table.setFillColor(sf::Color(20, 20, 80, 220));
    table.setPosition(200.f, 150.f);

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) return;

    sf::Text go_text("GAME OVER!!", font, 36);
    go_text.setPosition(250.f, 180.f);
    sf::Text hint("R - Restart, Q - Quit", font, 18);
    hint.setPosition(280.f, 280.f);

    window.draw(table);
    window.draw(go_text);
    window.draw(hint);
}

void spawn_enemy_wave(float total_time, std::vector<Enemy>& enemies, int& wave) {
    int current_wave_count = static_cast<int>(total_time / 10);
    if (current_wave_count >= wave) {
        wave++;
        for (int i = 0; i < 5 + wave; i++) {
            enemies.push_back(Enemy());
        }
    }
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    sf::RenderWindow window(sf::VideoMode(800, 600), "Survival Game");
    window.setFramerateLimit(60);

    Player player;
    float total_time = 0.f;
    int score = 0;
    int wave = 0;
    bool weapon_spawned = false;
    bool game_over = false;
    float previous_shot_time = 0.f;

    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;

    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        float deltaTime = clock.restart().asSeconds();
        bool active = !weapon_spawned && !game_over;

        if (active) {
            total_time += deltaTime;
            spawn_enemy_wave(total_time, enemies, wave);
            player.update(deltaTime, window, active);

            previous_shot_time += deltaTime;
            if (previous_shot_time >= player.get_attack_speed()) {
                previous_shot_time = 0.f;
                float angle = player.shooting_angle(window);
                sf::Vector2f pPos = player.getBody().getPosition() + sf::Vector2f(player.getBody().getRadius(), player.getBody().getRadius());
                bullets.push_back(Bullet(player.getAttack(), angle, pPos.x, pPos.y));
            }

            for (auto& b : bullets) b.update(deltaTime);

            bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [&](const Bullet& b) {
                return b.getX() < 0 || b.getX() > 800 || b.getY() < 0 || b.getY() > 600;
                }), bullets.end());

            if (active) {
                // 1. Ruch przeciwników i kolizja z Graczem
                for (auto it = enemies.begin(); it != enemies.end();) {
                    // Obliczanie wektora ruchu w stronę gracza
                    sf::Vector2f ePos = it->body.getPosition() + sf::Vector2f(it->body.getRadius(), it->body.getRadius());
                    sf::Vector2f pPos = player.getBody().getPosition() + sf::Vector2f(player.getBody().getRadius(), player.getBody().getRadius());
                    sf::Vector2f dir = pPos - ePos;
                    float len = std::hypot(dir.x, dir.y);

                    if (len != 0) it->body.move((dir / len) * 70.f * deltaTime);

                    bool enemyRemoved = false;

                    // Kolizja: Przeciwnik -> Gracz
                    if (checkColision(player.getBody(), it->body)) {
                        player.attacked(window, 10.f); // Zadaj 10 obrażeń graczowi

                        if (player.getHp() <= 0) {
                            game_over = true;
                        }

                        it = enemies.erase(it); // Przeciwnik znika po dotknięciu gracza
                        enemyRemoved = true;
                    }

                    if (!enemyRemoved) {
                        ++it;
                    }
                }

                // 2. Kolizja: Pociski -> Przeciwnicy
                for (auto it_bullet = bullets.begin(); it_bullet != bullets.end();) {
                    bool bulletRemoved = false;

                    for (auto it_enemy = enemies.begin(); it_enemy != enemies.end();) {
                        // Tworzymy tymczasowe koło reprezentujące pocisk do sprawdzenia kolizji
                        sf::CircleShape bulletCirc(15.f);
                        bulletCirc.setPosition(it_bullet->getX(), it_bullet->getY());

                        if (checkColision(bulletCirc, it_enemy->body)) {
                            // Trafienie!
                            score++;
                            if (player.gainExperience(3)) weapon_spawned = true;

                            it_enemy = enemies.erase(it_enemy); // Usuń przeciwnika
                            bulletRemoved = true;
                            break; // Pocisk może zabić tylko jednego wroga, wychodzimy z pętli wrogów
                        }
                        else {
                            ++it_enemy;
                        }
                    }

                    if (bulletRemoved) {
                        it_bullet = bullets.erase(it_bullet); // Usuń pocisk
                    }
                    else {
                        ++it_bullet;
                    }
                }
            }
        }

        if (game_over) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
                game_over = false;
                score = 0; total_time = 0; wave = 0;
                enemies.clear(); bullets.clear();
                player.heal();
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) window.close();
        }

        if (weapon_spawned) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) { player.setSpeed(player.getSpeed() + 30); weapon_spawned = false; }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) { player.setAttack(player.getAttack() + 2); weapon_spawned = false; }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) { player.setMAX_HP(player.getHp() + 20); player.heal(); weapon_spawned = false; }
            //TODO
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) { player.set_attack_speed(player.get_attack_speed() - 0.5f); weapon_spawned = false; }
        }

        window.clear();
        player.renderBody(window);
        for (auto& e : enemies) window.draw(e.body);
        for (auto& b : bullets) b.renderBody(window);
        if (weapon_spawned) spawnWeaponChoice(window);
        if (game_over) spawnGameOver(window);
        window.display();
    }
    return 0;
}
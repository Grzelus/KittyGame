#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>

class Weapon;

int const WINDOW_WIDTH = 800;
int const WINDOW_HEIGHT = 600;

float const INITIAL_HP = 100.f;
float const INITIAL_MAX_HP = 100.f;
float const INITIAL_SPEED = 300.f;
float const INITIAL_ATTACK = 5.f;
float const INITIAL_ATTACK_SPEED = 2.f;


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
    float speed = 500.f;
    float x_position;
    float y_position;
    float angle;

    sf::Sprite sprite;
    static sf::Texture texture;
    static bool isTextureLoaded;

public:
    Bullet(float o_damage, float o_angle, float new_x, float new_y)
        : damage(o_damage), angle(o_angle), x_position(new_x), y_position(new_y) {

        if (!isTextureLoaded) {
            if (!texture.loadFromFile("assets/bullet.png")) {
                sf::Image img;
                img.create(10, 20, sf::Color::White);
                texture.loadFromImage(img);
            }
            texture.setSmooth(true);
            isTextureLoaded = true;
        }

        sprite.setTexture(texture);

        sf::Vector2u texSize = texture.getSize();
        // POPRAWKA HITBOXA Z POPRZEDNIEJ ODPOWIEDZI (Dzielenie zamiast mnożenia)
        sprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);

        // Opcjonalne skalowanie
        sprite.setScale(0.5f, 0.5f);

        sprite.setPosition(x_position, y_position);

        float degrees = angle * 180.f / 3.14159265f;
        sprite.setRotation(degrees + 90.f);
    };

    void update(float deltaTime) {
        x_position += std::cos(angle) * speed * deltaTime;
        y_position += std::sin(angle) * speed * deltaTime;

        sprite.setPosition(x_position, y_position);
    }

    void renderBody(sf::RenderWindow& window) override {
        window.draw(sprite);
    }

    float getX() const { return x_position; }
    float getY() const { return y_position; }
    float getDamage() const { return damage; }
    // Pomocniczy promień do kolizji
    float getRadius() const { return 8.f; }

    bool operator==(const Bullet& other) const {
        return (this->x_position == other.x_position) && (this->y_position == other.y_position);
    }
};

sf::Texture Bullet::texture;
bool Bullet::isTextureLoaded = false;

class ExpCrystal : public GameObject {};

class Character : public GameObject {
protected:
    float hp;
    float max_hp;
    float speed;
    float attack;
    float attack_speed;
public:
    Character(float o_hp, float o_max_hp, float o_speed, float o_attack, float attack_speed)
        : hp(o_hp), max_hp(o_max_hp), speed(o_speed), attack(o_attack), attack_speed(attack_speed) {
    }

    void takedamage(float dmg) { hp -= dmg; }
    void heal() { this->hp = max_hp; }
    void setMAX_HP(float new_MAX_HP) { this->max_hp = new_MAX_HP; }
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

    sf::Sprite sprite;
    sf::Texture texture;
    sf::Vector2i frameSize;
    int numFrames;
    int currentFrame;
    float animationTimer;
    float animationSpeed;
    bool isMoving;
public:
    std::vector<Weapon*> weapons;
    Player(float o_hp = 100, float o_MAX_HP = 100, float o_speed = 300, float o_attack = 5, float attack_speed = 1.f)
        : Character(o_hp, o_MAX_HP, o_speed, o_attack, attack_speed),
        size(50.f),
        bodyShape(size)
    {
        bodyShape.setPosition(WINDOW_WIDTH / 2 - size, WINDOW_HEIGHT / 2 - size);
        bodyShape.setFillColor(sf::Color::Transparent);

        bodyShape.setOutlineColor(sf::Color::Red);
        bodyShape.setOutlineThickness(2.f);

        bodyShape.setOrigin(size, size);
        setBody(&bodyShape);

        if (!texture.loadFromFile("assets/hero.png")) {
            sf::Image img;
            img.create(96, 32, sf::Color::Green);
            texture.loadFromImage(img);
        }
        sprite.setTexture(texture);

        frameSize = sf::Vector2i(32, 32);
        numFrames = 3;
        currentFrame = 0;
        animationTimer = 0.f;
        animationSpeed = 0.1f;
        isMoving = false;

        sprite.setTextureRect(sf::IntRect(0, 0, frameSize.x, frameSize.y));
        sprite.setOrigin(frameSize.x / 2.f, frameSize.y / 2.f);

        float scaleFactor = (size * 2.f) / frameSize.x;
        sprite.setScale(scaleFactor, scaleFactor);

        sprite.setPosition(bodyShape.getPosition());
    }

    void renderBody(sf::RenderWindow& window) override {
        if (body) {
            // window.draw(*body); // Odkomentuj do debugowania hitboxa
            window.draw(sprite);
        }
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
    void reset_stats() {
        attack = INITIAL_ATTACK;
        attack_speed = INITIAL_ATTACK_SPEED;
        hp = INITIAL_HP;
        max_hp = INITIAL_MAX_HP;
        speed = INITIAL_SPEED;

        experience = 0;
        level = 1;

        bodyShape.setPosition(WINDOW_WIDTH / 2 - size, WINDOW_HEIGHT / 2 - size);
    }

    float shooting_angle(const sf::RenderWindow& window) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f playerCenter = bodyShape.getPosition();
        return std::atan2(mousePos.y - playerCenter.y, mousePos.x - playerCenter.x);
    }

    void attacked(sf::RenderWindow& window, float dmg) {
        takedamage(dmg);
    }

    void update(float deltaTime, const sf::RenderWindow& window, bool canMove) {
        isMoving = false;
        if (!canMove) return;

        sf::Vector2f movement(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  movement.x -= getSpeed();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) movement.x += getSpeed();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  movement.y += getSpeed();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    movement.y -= getSpeed();

        if (movement.x != 0.f || movement.y != 0.f) {
            isMoving = true;
        }

        sf::Vector2u winSize = window.getSize();
        float diameter = bodyShape.getRadius() * 2.f;
        sf::Vector2f pos = bodyShape.getPosition();

        if (pos.x <= 0.f && movement.x < 0.f) movement.x = 0.f;
        if (pos.x + diameter >= static_cast<float>(winSize.x) && movement.x > 0.f) movement.x = 0.f;
        if (pos.y <= 0.f && movement.y < 0.f) movement.y = 0.f;
        if (pos.y + diameter >= static_cast<float>(winSize.y) && movement.y > 0.f) movement.y = 0.f;

        bodyShape.move(movement * deltaTime);

        sprite.setPosition(bodyShape.getPosition());

        if (isMoving) {
            animationTimer += deltaTime;
            if (animationTimer >= animationSpeed) {
                animationTimer = 0.f;
                currentFrame = (currentFrame + 1) % numFrames;
                int left = currentFrame * frameSize.x;
                sprite.setTextureRect(sf::IntRect(left, 0, frameSize.x, frameSize.y));
            }
        }
        else {
            // Opcjonalnie: Reset do klatki "idle"
        }

        if (movement.x > 0) sprite.setScale(std::abs(sprite.getScale().x), sprite.getScale().y);
        if (movement.x < 0) sprite.setScale(-std::abs(sprite.getScale().x), sprite.getScale().y);
    }


    const sf::CircleShape& getBody() const { return bodyShape; }
    sf::CircleShape& getBody() { return bodyShape; }
};


class Enemy {
private:
    float size = 20.f;
    sf::Vector2i frameSize;
    int numFrames;
    int currentFrame;
    float animationTimer;
    float animationSpeed;

public:
    float x, y;
    sf::CircleShape body;
    sf::Sprite sprite;

    static sf::Texture texture;
    static bool isTextureLoaded;

    Enemy() {
        if (!isTextureLoaded) {
            if (!texture.loadFromFile("assets/enemy.png")) {
                sf::Image img;
                img.create(128, 32, sf::Color::Blue);
                texture.loadFromImage(img);
            }
            isTextureLoaded = true;
        }

        frameSize = sf::Vector2i(32, 32);
        numFrames = 4;
        currentFrame = 0;
        animationTimer = 0.f;
        animationSpeed = 0.1f;

        calculate_spawn_position();
        body = spawn_hitbox();

        sprite.setTexture(texture);
        sprite.setTextureRect(sf::IntRect(0, 0, frameSize.x, frameSize.y));
        sprite.setOrigin(frameSize.x / 2.f, frameSize.y / 2.f);

        float scaleFactor = (size * 2.f) / frameSize.x;
        sprite.setScale(scaleFactor, scaleFactor);

        sprite.setPosition(body.getPosition());
    }

    void calculate_spawn_position() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis_x(100.f, WINDOW_WIDTH / 2.f);
        std::uniform_real_distribution<float> dis_y(100.f, WINDOW_HEIGHT / 2.f);
        std::uniform_int_distribution<int> dis_x_s(0, 1);
        std::uniform_int_distribution<int> dis_y_s(0, 1);

        float x_offset = dis_x(gen);
        float y_offset = dis_y(gen);

        if (dis_x_s(gen) == 1) x = WINDOW_WIDTH / 2.f - x_offset;
        else x = WINDOW_WIDTH / 2.f + x_offset;

        if (dis_y_s(gen) == 1) y = WINDOW_HEIGHT / 2.f - y_offset;
        else y = WINDOW_HEIGHT / 2.f + y_offset;
    };

    sf::CircleShape spawn_hitbox() const {
        sf::CircleShape shape(size);
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(sf::Color::Red);
        shape.setOutlineThickness(2.f);
        shape.setPosition(x, y);
        shape.setOrigin(size, size);
        return shape;
    }

    void update(float deltaTime) {
        sprite.setPosition(body.getPosition());

        animationTimer += deltaTime;
        if (animationTimer >= animationSpeed) {
            animationTimer = 0.f;
            currentFrame++;
            if (currentFrame >= numFrames) {
                currentFrame = 0;
            }
            int left = currentFrame * frameSize.x;
            sprite.setTextureRect(sf::IntRect(left, 0, frameSize.x, frameSize.y));
        }
    }
};

sf::Texture Enemy::texture;
bool Enemy::isTextureLoaded = false;
class Weapon {
protected:
    float damage_multiplier;
    float base_fire_delay;
    float current_cooldown = 0.f;

public:
    Weapon(float dmg_mult, float delay) : damage_multiplier(dmg_mult), base_fire_delay(delay) {}
    virtual ~Weapon() = default;
    float get_damage_multiplier() const { return damage_multiplier; };
    float get_base_fire_delay() const { return base_fire_delay; };
    void update(float deltaTime) {
        if (current_cooldown > 0.f) {
            current_cooldown -= deltaTime;
        }
    }
    bool can_fire() const {
        return current_cooldown <= 0.f;
    }
    virtual void fire(sf::Vector2f startPos, float angle, std::vector<Bullet>& bulletContainer, float player_atk, float player_atk_spd) = 0;
};

class Gun : public Weapon {
public:
    Gun(float damage_multiplier, float delay) : Weapon(damage_multiplier, delay) {}
    void fire(sf::Vector2f startPos, float angle, std::vector<Bullet>& bulletContainer, float player_atk, float player_atk_spd) override {
        if (can_fire()) {
            float actual_damage = player_atk * damage_multiplier;
            float actual_delay = (player_atk_spd > 0) ? (base_fire_delay / player_atk_spd) : base_fire_delay;
            bulletContainer.push_back(Bullet(actual_damage, angle, startPos.x, startPos.y));
            current_cooldown = actual_delay;
        }
    }
};


bool checkColision(const sf::CircleShape& a, const sf::CircleShape& b) {
    sf::Vector2f aPos = a.getPosition();
    sf::Vector2f bPos = b.getPosition();
    float dist = std::hypot(aPos.x - bPos.x, aPos.y - bPos.y);
    return dist < (a.getRadius() + b.getRadius());
}

void spawnWeaponChoice(sf::RenderWindow& window) {
    sf::RectangleShape table(sf::Vector2f(400.f, 200.f));
    table.setFillColor(sf::Color::Blue);
    table.setPosition(150.f, 150.f);
    // (Skróciłem kod rysowania wyboru broni dla czytelności, jest bez zmian)
    sf::CircleShape f_up(40.f, 3); f_up.setFillColor(sf::Color::Magenta); f_up.setPosition(200.f, 200.f);
    sf::CircleShape s_up(40.f, 4); s_up.setFillColor(sf::Color::Yellow); s_up.setPosition(300.f, 200.f);
    sf::CircleShape t_up(40.f, 5); t_up.setFillColor(sf::Color::Cyan); t_up.setPosition(400.f, 200.f);
    sf::Font font; if (!font.loadFromFile("arial.ttf")) return;
    sf::Text choices("+5 Speed    +5 Attack   +5HP    +0.5 AtkSpd", font, 18); choices.setFillColor(sf::Color::White); choices.setPosition(200.f, 300.f);
    sf::Text num1("1", font, 24); num1.setPosition(235.f, 230.f); num1.setFillColor(sf::Color::Black);
    sf::Text num2("2", font, 24); num2.setPosition(335.f, 230.f); num2.setFillColor(sf::Color::Black);
    sf::Text num3("3", font, 24); num3.setPosition(435.f, 230.f); num3.setFillColor(sf::Color::Black);
    sf::Text num4("4", font, 24); num4.setPosition(335.f, 160.f); num4.setFillColor(sf::Color::White);
    window.draw(table); window.draw(f_up); window.draw(s_up); window.draw(t_up); window.draw(choices);
    window.draw(num1); window.draw(num2); window.draw(num3); window.draw(num4);
}

void spawnGameOver(sf::RenderWindow& window) {
    sf::RectangleShape table(sf::Vector2f(400.f, 220.f));
    table.setFillColor(sf::Color(20, 20, 80, 220));
    table.setPosition(200.f, 150.f);
    sf::Font font; if (!font.loadFromFile("arial.ttf")) return;
    sf::Text go_text("GAME OVER!!", font, 36); go_text.setPosition(250.f, 180.f);
    sf::Text hint("R - Restart, Q - Quit", font, 18); hint.setPosition(280.f, 280.f);
    window.draw(table); window.draw(go_text); window.draw(hint);
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
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Survival Game");
    window.setFramerateLimit(60);

    // --- NOWY KOD: Ładowanie i konfiguracja tła ---
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("assets/background.png")) {
        std::cout << "Blad ladowania tla! Upewnij sie, ze plik assets/background.png istnieje." << std::endl;
        // Opcjonalnie: stwórz tymczasowe tło
        sf::Image img; img.create(WINDOW_WIDTH, WINDOW_HEIGHT, sf::Color(50, 100, 50));
        backgroundTexture.loadFromImage(img);
    }

    sf::Sprite backgroundSprite;
    backgroundSprite.setTexture(backgroundTexture);

    // Automatyczne skalowanie tła do rozmiaru okna
    sf::Vector2u textureSize = backgroundTexture.getSize();
    float scaleX = static_cast<float>(WINDOW_WIDTH) / textureSize.x;
    float scaleY = static_cast<float>(WINDOW_HEIGHT) / textureSize.y;
    backgroundSprite.setScale(scaleX, scaleY);
    // ----------------------------------------------


    Player player(INITIAL_HP, INITIAL_MAX_HP, INITIAL_SPEED, INITIAL_ATTACK, INITIAL_ATTACK_SPEED);

    Gun pistol(1.0f, 1.0f);
    player.weapons.push_back(&pistol);
    Gun pistol2(1.0f, 0.2f); // Zmieniłem delay na 0.2s dla szybszego strzelania
    player.weapons.push_back(&pistol2);

    float total_time = 0.f;
    int score = 0;
    int wave = 0;
    bool weapon_spawned = false;
    bool game_over = false;

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

            for (auto weapon = player.weapons.begin(); weapon != player.weapons.end();) {
                (*weapon)->update(deltaTime);
                if ((*weapon)->can_fire()) {
                    float angle = player.shooting_angle(window);
                    sf::Vector2f playerCenter = player.getBody().getPosition();
                    (*weapon)->fire(playerCenter, angle, bullets, player.getAttack(), player.get_attack_speed());
                }
                ++weapon;
            }

            for (auto& b : bullets) b.update(deltaTime);

            bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [&](const Bullet& b) {
                return b.getX() < 0 || b.getX() > WINDOW_WIDTH || b.getY() < 0 || b.getY() > WINDOW_HEIGHT;
                }), bullets.end());

            for (auto it = enemies.begin(); it != enemies.end();) {
                sf::Vector2f ePos = it->body.getPosition() + sf::Vector2f(it->body.getRadius(), it->body.getRadius());
                sf::Vector2f pPos = player.getBody().getPosition() + sf::Vector2f(player.getBody().getRadius(), player.getBody().getRadius());
                sf::Vector2f dir = pPos - ePos;
                float len = std::hypot(dir.x, dir.y);

                if (len != 0)
                {
                    it->body.move((dir / len) * 70.f * deltaTime);
                    it->update(deltaTime);
                };

                bool enemyRemoved = false;

                if (checkColision(player.getBody(), it->body)) {
                    player.attacked(window, 10.f);

                    if (player.getHp() <= 0) {
                        game_over = true;
                    }

                    it = enemies.erase(it);
                    enemyRemoved = true;
                }

                if (!enemyRemoved) {
                    ++it;
                }
            }

            for (auto it_bullet = bullets.begin(); it_bullet != bullets.end();) {
                bool bulletRemoved = false;

                for (auto it_enemy = enemies.begin(); it_enemy != enemies.end();) {

                    // --- POPRAWKA HITBOXA POCISKU Z POPRZEDNIEJ ODPOWIEDZI ---
                    float collisionRadius = 8.f;
                    sf::CircleShape bulletCirc(collisionRadius);
                    bulletCirc.setOrigin(collisionRadius, collisionRadius);
                    bulletCirc.setPosition(it_bullet->getX(), it_bullet->getY());
                    // --------------------------------------------------------

                    if (checkColision(bulletCirc, it_enemy->body)) {
                        score++;
                        if (player.gainExperience(3)) weapon_spawned = true;

                        it_enemy = enemies.erase(it_enemy);
                        bulletRemoved = true;
                        break;
                    }
                    else {
                        ++it_enemy;
                    }
                }

                if (bulletRemoved) {
                    it_bullet = bullets.erase(it_bullet);
                }
                else {
                    ++it_bullet;
                }
            }
        }

        if (game_over) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
                game_over = false;
                score = 0; total_time = 0; wave = 0;
                enemies.clear(); bullets.clear();
                player.reset_stats();
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) window.close();
        }

        if (weapon_spawned) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) { player.setSpeed(player.getSpeed() + 30); weapon_spawned = false; }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) { player.setAttack(player.getAttack() + 2); weapon_spawned = false; }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) { player.setMAX_HP(player.getHp() + 20); player.heal(); weapon_spawned = false; }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) { player.set_attack_speed(player.get_attack_speed() + 0.5f); weapon_spawned = false; }
        }

        window.clear();

        // --- NOWY KOD: Rysowanie tła (ZAWSZE PIERWSZE!) ---
        window.draw(backgroundSprite);
        // --------------------------------------------------

        player.renderBody(window);
        for (auto& e : enemies) {
            window.draw(e.sprite);
            // window.draw(e.body);   // Odkomentuj do debugowania hitboxa
        }
        for (auto& b : bullets) b.renderBody(window);
        if (weapon_spawned) spawnWeaponChoice(window);
        if (game_over) spawnGameOver(window);
        window.display();
    }
    return 0;
}
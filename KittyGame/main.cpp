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
        // DODANO: Ustawienie origin na środek, tak jak w Player i Enemy
        bulletShape.setOrigin(size, size);
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
        bodyShape.setFillColor(sf::Color::Transparent); // Ważne: Przezroczysty

        bodyShape.setOutlineColor(sf::Color::Red); // Czerwony obrys
        bodyShape.setOutlineThickness(2.f);        // Grubość obrysu

        bodyShape.setOrigin(size, size); // Origin na środku
        setBody(&bodyShape);

        if (!texture.loadFromFile("assets/hero.png")) {
            // Fallback: jeśli brak pliku, stwórzmy coś w pamięci
            sf::Image img;
            img.create(96, 32, sf::Color::Green);
            texture.loadFromImage(img);
        }
        sprite.setTexture(texture);

        // 3. Konfiguracja animacji
        frameSize = sf::Vector2i(32, 32); // Ustaw rozmiar jednej klatki (np. 32x32)
        numFrames = 3;                    // Liczba klatek w poziomie
        currentFrame = 0;
        animationTimer = 0.f;
        animationSpeed = 0.1f; // Szybkość animacji
        isMoving = false;

        // Ustawienie pierwszej klatki
        sprite.setTextureRect(sf::IntRect(0, 0, frameSize.x, frameSize.y));
        sprite.setOrigin(frameSize.x / 2.f, frameSize.y / 2.f);

        // Skalowanie sprite'a do wielkości hitboxa (średnica 100px)
        float scaleFactor = (size * 2.f) / frameSize.x;
        sprite.setScale(scaleFactor, scaleFactor);

        // Ustawienie pozycji startowej
        sprite.setPosition(bodyShape.getPosition());
    }

    void renderBody(sf::RenderWindow& window) override {
        if (body) {
            window.draw(*body); // Odkomentuj, żeby widzieć hitbox (debug)
            window.draw(sprite);   // Rysujemy sprite'a
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

        // Animujemy tylko, gdy gracz się porusza
        if (isMoving) {
            animationTimer += deltaTime;
            if (animationTimer >= animationSpeed) {
                animationTimer = 0.f;
                currentFrame = (currentFrame + 1) % numFrames; // Przejście do kolejnej klatki i zapętlenie
                int left = currentFrame * frameSize.x;
                sprite.setTextureRect(sf::IntRect(left, 0, frameSize.x, frameSize.y));
            }
        }
        else {
            // Opcjonalnie: Reset do klatki "idle" (stojącej), gdy się nie rusza
            // currentFrame = 0;
            // sprite.setTextureRect(sf::IntRect(0, 0, frameSize.x, frameSize.y));
        }

        // Opcjonalnie: Obracanie sprite'a w stronę ruchu (lewo/prawo)
        if (movement.x > 0) sprite.setScale(std::abs(sprite.getScale().x), sprite.getScale().y);
        if (movement.x < 0) sprite.setScale(-std::abs(sprite.getScale().x), sprite.getScale().y);
    }
    

    const sf::CircleShape& getBody() const { return bodyShape; }
    sf::CircleShape& getBody() { return bodyShape; }
};


    class Enemy {
    private:
        float size = 20.f; // Promień hitboxa

        // Zmienne do animacji
        sf::Vector2i frameSize;      // Rozmiar jednej klatki (np. 32x32)
        int numFrames;               // Liczba klatek (4)
        int currentFrame;            // Aktualna klatka (0-3)
        float animationTimer;        // Licznik czasu
        float animationSpeed;        // Co ile sekund zmieniać klatkę (np. 0.1s)

    public:
        float x, y;
        sf::CircleShape body;
        sf::Sprite sprite;

        static sf::Texture texture;
        static bool isTextureLoaded;

        Enemy() {
            // 1. Ładowanie tekstury (Arkusz duszka - 4 klatki w poziomie)
            if (!isTextureLoaded) {
                // Upewnij się, że masz plik z 4 klatkami obok siebie!
                if (!texture.loadFromFile("assets/enemy.png")) {
                    // Fallback: stwórz tymczasowy obrazek w pamięci
                    sf::Image img;
                    img.create(128, 32, sf::Color::Blue);
                    texture.loadFromImage(img);
                }
                isTextureLoaded = true;
            }

            // 2. Konfiguracja animacji
            frameSize = sf::Vector2i(32, 32); // Przyjmujemy, że jedna klatka ma 32x32px
            numFrames = 4;
            currentFrame = 0;
            animationTimer = 0.f;
            animationSpeed = 0.1f; // Zmiana klatki co 0.1 sekundy

            calculate_spawn_position();
            body = spawn_hitbox();

            sprite.setTexture(texture);

            // 3. Ustawienie pierwszego wycinka tekstury (IntRect)
            // (lewo, góra, szerokość, wysokość)
            sprite.setTextureRect(sf::IntRect(0, 0, frameSize.x, frameSize.y));

            // Ustawienie Origin na środek KLATKI, a nie całego paska
            sprite.setOrigin(frameSize.x / 2.f, frameSize.y / 2.f);

            // Dopasowanie skali, żeby pasowało do hitboxa (średnica 40px)
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

            shape.setOutlineColor(sf::Color::Red); // Czerwony obrys
            shape.setOutlineThickness(2.f);

            shape.setPosition(x, y);
            // Ważne: Origin hitboxa na środku
            shape.setOrigin(size, size);
            return shape;
        }

        // Nowa funkcja aktualizująca animację i pozycję
        void update(float deltaTime) {
            // 1. Aktualizacja pozycji wizualnej
            sprite.setPosition(body.getPosition());

            // 2. Logika animacji
            animationTimer += deltaTime;
            if (animationTimer >= animationSpeed) {
                animationTimer = 0.f;
                currentFrame++;

                // Zapętlenie animacji (0 -> 1 -> 2 -> 3 -> 0)
                if (currentFrame >= numFrames) {
                    currentFrame = 0;
                }

                // Przesunięcie okna wycinania (IntRect) na odpowiednią klatkę
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
    // Pobieramy pozycje. Zakładamy, że dla kół Origin jest ustawiony na środku.
    sf::Vector2f aPos = a.getPosition();
    sf::Vector2f bPos = b.getPosition();

    // Obliczamy odległość Euklidesową (Pitagoras)
    float dist = std::hypot(aPos.x - bPos.x, aPos.y - bPos.y);

    // Sprawdzamy czy odległość jest mniejsza niż suma promieni
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

    sf::Text choices("+5 Speed    +5 Attack   +5HP    +0.5 AtkSpd", font, 18);
    choices.setFillColor(sf::Color::White);
    choices.setPosition(200.f, 300.f);

    sf::Text num1("1", font, 24); num1.setPosition(235.f, 230.f); num1.setFillColor(sf::Color::Black);
    sf::Text num2("2", font, 24); num2.setPosition(335.f, 230.f); num2.setFillColor(sf::Color::Black);
    sf::Text num3("3", font, 24); num3.setPosition(435.f, 230.f); num3.setFillColor(sf::Color::Black);
    sf::Text num4("4", font, 24); num4.setPosition(335.f, 160.f); num4.setFillColor(sf::Color::White);

    window.draw(table);
    window.draw(f_up);
    window.draw(s_up);
    window.draw(t_up);
    window.draw(choices);
    window.draw(num1);
    window.draw(num2);
    window.draw(num3);
    window.draw(num4);
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
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Survival Game");
    window.setFramerateLimit(60);

    Player player(INITIAL_HP, INITIAL_MAX_HP, INITIAL_SPEED, INITIAL_ATTACK, INITIAL_ATTACK_SPEED);

    // First Gun: 1.0x damage, 1.0s base delay (slow but normal damage)
    Gun pistol(1.0f, 1.0f);
    player.weapons.push_back(&pistol);

    // Second Gun: 1.0x damage, 0.1s base delay (fast machine gun)
    Gun pistol2(1.0f, 2.f);
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
                    sf::CircleShape bulletCirc(15.f);
                    bulletCirc.setPosition(it_bullet->getX(), it_bullet->getY());

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
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                player.setSpeed(player.getSpeed() + 30);
                weapon_spawned = false;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                player.setAttack(player.getAttack() + 2);
                weapon_spawned = false;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
                player.setMAX_HP(player.getHp() + 20);
                player.heal();
                weapon_spawned = false;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) {
                player.set_attack_speed(player.get_attack_speed() + 0.5f);
                weapon_spawned = false;
            }
        }

        window.clear();
        player.renderBody(window);
        for (auto& e : enemies) {
            window.draw(e.sprite); // Rysuje grafikę
            window.draw(e.body);   // Rysuje hitbox (DODAJ TO)
        }
        for (auto& b : bullets) b.renderBody(window);
        if (weapon_spawned) spawnWeaponChoice(window);
        if (game_over) spawnGameOver(window);
        window.display();
    }
    return 0;
}
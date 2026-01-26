
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>

//graificzki do upgradów
//info o broniach
//label o killach + counter
//przeciwnicy z hp
//boss
//escape do menu startowego


// --- KONFIGURACJA ---
int const WINDOW_WIDTH = 800;
int const WINDOW_HEIGHT = 600;

float const INITIAL_HP = 100.f;
float const INITIAL_MAX_HP = 100.f;
float const INITIAL_SPEED = 300.f;
float const INITIAL_ATTACK = 5.f;
float const INITIAL_ATTACK_SPEED = 1.0f; // Zwiększona bazowa wartość dla lepszego odczucia

// --- KLASY BAZOWE ---

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
                img.create(10, 20, sf::Color::Yellow);
                texture.loadFromImage(img);
            }
            texture.setSmooth(true);
            isTextureLoaded = true;
        }

        sprite.setTexture(texture);
        sf::Vector2u texSize = texture.getSize();
        sprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
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
    float getRadius() const { return 8.f; }

    bool operator==(const Bullet& other) const {
        return (this->x_position == other.x_position) && (this->y_position == other.y_position);
    }
};

sf::Texture Bullet::texture;
bool Bullet::isTextureLoaded = false;

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

    void takeDamage(float dmg) { hp -= dmg; }
    void heal() { this->hp = max_hp; }
    void setMAX_HP(float new_MAX_HP) { this->max_hp = new_MAX_HP;}
	void setHp(float new_hp) { this->hp = new_hp; }
    void setSpeed(float new_speed) { this->speed = new_speed; }
    void setAttack(float new_attack) { this->attack = new_attack; }
    void set_attack_speed(float new_attack_speed) { this->attack_speed = new_attack_speed; }

    float getMaxHp()const { return max_hp; }
    float getHp() const { return hp; }
    float getAttack() const { return attack; }
    float getSpeed() const { return speed; }
    float get_attack_speed() const { return attack_speed; }
};

// Forward declaration
class Weapon;

class Player : public Character {
private:
    float size = 0.f;
    sf::CircleShape bodyShape;
    int experience = 0;
    int level = 1;
    int chosen_weapon = 0;
    int weaponsAvailable = 1;
	int killsCounter = 0;

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
        size(25.f), // Zmniejszyłem lekko hitbox
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

    bool gainExperience(int exp) {
        this->experience += exp;
        if (this->experience >= this->level * 10) {
            this->experience -= this->level * 10;
            this->levelUp();
            return true;
        }
        return false;
    }

    void levelUp() {
        this->level += 1;
        this->heal();
        if (this->level % 2 == 0 && weaponsAvailable < 4) { //co dwa poziomy
            weaponsAvailable++;
        }
    }

    void reset_stats() {
        attack = INITIAL_ATTACK;
        attack_speed = INITIAL_ATTACK_SPEED;
        hp = INITIAL_HP;
        max_hp = INITIAL_MAX_HP;
        speed = INITIAL_SPEED;
        weaponsAvailable = 1;
        chosen_weapon = 0;
        experience = 0;
        level = 1;
        bodyShape.setPosition(WINDOW_WIDTH / 2 - size, WINDOW_HEIGHT / 2 - size);
        killsCounter = 0;
    }

    int getWeaponIndex() const { return this->chosen_weapon; }

    void setWeapon(int weaponIndex) { this->chosen_weapon = weaponIndex; }

    float shooting_angle(const sf::RenderWindow& window) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f playerCenter = bodyShape.getPosition();
        return std::atan2(mousePos.y - playerCenter.y, mousePos.x - playerCenter.x);
    }

    void attacked(sf::RenderWindow& window, float dmg) {
        takeDamage(dmg);
    }
    int getKills() const { return this->killsCounter; }

    void mob_killed() { ++killsCounter; }

    void DrawLabelsWithKillsHealth(sf::RenderWindow &window) {
        // Ładuj textury i font raz
        static sf::Texture healthTex;
        static sf::Texture killsTex;
        static sf::Font font;
        static bool loaded = false;
        if (!loaded) {
            if (!healthTex.loadFromFile("assets/health.png")) {
                sf::Image img; img.create(64, 64, sf::Color::Red);
                healthTex.loadFromImage(img);
            }
            if (!killsTex.loadFromFile("assets/kill_counter.png")) {
                sf::Image img; img.create(64, 64, sf::Color::Yellow);
                killsTex.loadFromImage(img);
            }
            // próbuj załadować font, ale nie przerywaj jeśli nie ma
            font.loadFromFile("arial.ttf");
            healthTex.setSmooth(true);
            killsTex.setSmooth(true);
            loaded = true;
        }

        // Przygotuj sprite'y i skaluj proporcjonalnie do wysokości docelowej
        sf::Sprite health_sprite(healthTex);
        sf::Sprite kills_sprite(killsTex);
        const float iconDisplayHeight = 40.f;
        if (healthTex.getSize().y > 0) {
            float s = iconDisplayHeight / static_cast<float>(healthTex.getSize().y);
            health_sprite.setScale(s, s);
        }
        if (killsTex.getSize().y > 0) {
            float s = iconDisplayHeight / static_cast<float>(killsTex.getSize().y);
            kills_sprite.setScale(s, s);
        }

        // Wymiary po skalowaniu
        sf::FloatRect hb = health_sprite.getGlobalBounds();
        sf::FloatRect kb = kills_sprite.getGlobalBounds();
        sf::FloatRect killsBackroundRect;
        sf::FloatRect healthBackroundRect;

        const float padding = 140.f;
        const float spacing = 40.f;

        // Pozycjonowanie w prawym górnym rogu
        float healthX = static_cast<float>(WINDOW_WIDTH) - padding - hb.width;
        float killsX = healthX - spacing - kb.width;
        float topY = 5.f;

        health_sprite.setPosition(healthX, topY);
        kills_sprite.setPosition(killsX, topY);

        // Rysuj ikony
        window.draw(health_sprite);
        window.draw(kills_sprite);

        // Rysuj tekst (jeśli font załadowany)
        if (font.getInfo().family != "") {
            // HP — po lewej od ikony health
            std::string hpStr = std::to_string(static_cast<int>(this->hp));
            sf::Text hpText(hpStr, font, 16);
            hpText.setFillColor(sf::Color::White);
            hpText.setOutlineColor(sf::Color::Black);
            hpText.setOutlineThickness(1.f);
            sf::FloatRect th = hpText.getLocalBounds();
            hpText.setOrigin(th.width, 0.f);
            hpText.setPosition(health_sprite.getPosition().x + 80.f, topY + (hb.height - th.height) / 2.f - th.top);
            window.draw(hpText);

            // Kills — po prawej stronie ikony kills
            std::string killsStr = std::to_string(this->killsCounter);
            sf::Text killsText(killsStr, font, 16);
            killsText.setFillColor(sf::Color::White);
            killsText.setOutlineColor(sf::Color::Black);
            killsText.setOutlineThickness(1.f);
            sf::FloatRect tk = killsText.getLocalBounds();
            killsText.setOrigin(0.f, 0.f);
            killsText.setPosition(kills_sprite.getPosition().x + 55.f, topY + (kb.height - tk.height) / 2.f - tk.top);
            window.draw(killsText);
        }
    }

    void update(float deltaTime, const sf::RenderWindow& window, bool canMove) {
        isMoving = false;
        if (!canMove) return;

        sf::Vector2f movement(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))  movement.x -= getSpeed();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += getSpeed();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))  movement.y += getSpeed();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))    movement.y -= getSpeed();

        if (movement.x != 0.f || movement.y != 0.f) {
            isMoving = true;
        }

        sf::Vector2u winSize = window.getSize();
        float diameter = bodyShape.getRadius() * 2.f;
        sf::Vector2f pos = bodyShape.getPosition();

        // Kolizja ze ścianami (uwzględniając origin na środku)
        float radius = bodyShape.getRadius();
        if (pos.x - radius <= 0.f && movement.x < 0.f) movement.x = 0.f;
        if (pos.x + radius >= static_cast<float>(winSize.x) && movement.x > 0.f) movement.x = 0.f;
        if (pos.y - radius <= 0.f && movement.y < 0.f) movement.y = 0.f;
        if (pos.y + radius >= static_cast<float>(winSize.y) && movement.y > 0.f) movement.y = 0.f;

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


        // Zmiana broni

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z) && weaponsAvailable >= 1) setWeapon(0);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::X) && weaponsAvailable >= 2) setWeapon(1);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::C) && weaponsAvailable >= 3) setWeapon(2);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::V) && weaponsAvailable >= 4) setWeapon(3);

        // Obracanie sprite'a
        if (movement.x > 0) sprite.setScale(std::abs(sprite.getScale().x), sprite.getScale().y);
        if (movement.x < 0) sprite.setScale(-std::abs(sprite.getScale().x), sprite.getScale().y);
        


    }

    const sf::CircleShape& getBody() const { return bodyShape; }
    sf::CircleShape& getBody() { return bodyShape; }
};

class Enemy : public Character {
protected:
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

    std::string spriteName;


    Enemy(float hp, float attack, sf::Texture& textureRef)
        : Character(hp, hp, 1.f, attack, 1.f), spriteName(spriteName)
    {
       sprite.setTexture(textureRef);

        // Animation setup
        frameSize = sf::Vector2i(32, 32);
        numFrames = 4;
        currentFrame = 0;
        animationTimer = 0.f;
        animationSpeed = 0.1f;

        calculate_spawn_position();
        body = spawn_hitbox();

        sprite.setTextureRect(sf::IntRect(0, 0, frameSize.x, frameSize.y));
        sprite.setOrigin(frameSize.x / 2.f, frameSize.y / 2.f);

        float scaleFactor = (size * 2.f) / frameSize.x;
        sprite.setScale(scaleFactor, scaleFactor);

        sprite.setPosition(body.getPosition());

        this->setAttack(attack);
        this->setMAX_HP(hp);
        this->setHp(hp);
    }

    // ... (Keep calculate_spawn_position, spawn_hitbox, and update exactly as they were) ...
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

class Boss : public Enemy {
public:
    Boss(float hp, float attack, sf::Texture& textureRef)
        : Enemy(hp, attack, textureRef)
    {
        this->size = 60.f;
        this->numFrames = 5;
        this->speed = 100.f;
        this->animationSpeed = 0.15f;


        body.setRadius(size);
        body.setOrigin(size, size);

        this->frameSize = sf::Vector2i(32, 32);

        float scaleFactor = (size * 2.f) / frameSize.x;
        sprite.setScale(scaleFactor, scaleFactor);

        sprite.setTextureRect(sf::IntRect(0, 0, frameSize.x, frameSize.y));
        sprite.setOrigin(frameSize.x / 2.f, frameSize.y / 2.f);
    }
};


// --- BRONIE ---

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

// --- FUNKCJE POMOCNICZE ---

bool checkColision(const sf::CircleShape& a, const sf::CircleShape& b) {
    sf::Vector2f aPos = a.getPosition();
    sf::Vector2f bPos = b.getPosition();
    float dist = std::hypot(aPos.x - bPos.x, aPos.y - bPos.y);
    return dist < (a.getRadius() + b.getRadius());
};

void spawnUpgradeChoice(sf::RenderWindow& window) {
	sf::Texture swordTexture;
    if(!swordTexture.loadFromFile("assets/attack.png")){
        sf::Image img; 
        img.create(150, 150, sf::Color::White);
        swordTexture.loadFromImage(img);
	}
    sf::Sprite attack_up(swordTexture);
    attack_up.setScale(5.f, 5.f);
    sf::Texture healthTexture;


    if (!healthTexture.loadFromFile("assets/health.png")) {
        sf::Image img;
        img.create(150, 150, sf::Color::White);
        healthTexture.loadFromImage(img);
    }
    sf::Sprite health_up(healthTexture);
    health_up.setScale(5.f, 5.f);


    sf::Texture speedTexture;
    if (!speedTexture.loadFromFile("assets/speed.png")) {
        sf::Image img;
        img.create(150, 150, sf::Color::White);
        speedTexture.loadFromImage(img);
    }
    sf::Sprite speed_up(speedTexture);
	speed_up.setScale(5.f, 5.f);

    sf::Texture bowTexture;
    if(!bowTexture.loadFromFile("assets/attack_speed.png")){
        sf::Image img;
        img.create(150, 150, sf::Color::White);
        bowTexture.loadFromImage(img);
    }
    sf::Sprite attack_speed_up(bowTexture);
    attack_speed_up.setScale(5.f, 5.f);


    sf::RectangleShape table(sf::Vector2f(560.f, 220.f));
    table.setFillColor(sf::Color(0, 0, 200, 200));
    table.setPosition(120.f, 140.f);

    const float radius = 40.f;
    speed_up.setPosition(170.f, 200.f);
    attack_up.setPosition(280.f, 200.f);
    health_up.setPosition(390.f, 200.f);
    attack_speed_up.setPosition(500.f, 200.f);

    sf::Font font;
    // Ładowanie fontu - jeśli nie ma, użyjemy domyślnego mechanizmu SFML (brak tekstu)
    bool fontLoaded = font.loadFromFile("arial.ttf");

    window.draw(table);
    window.draw(attack_up);
    window.draw(speed_up);
    window.draw(health_up);
    window.draw(attack_speed_up);

    if (fontLoaded) {
        sf::Text choices("  + Speed         + Attack            + HP        + Atk Spd", font, 18);
        choices.setFillColor(sf::Color::White);
        choices.setPosition(160.f, 300.f);

        auto createText = [&](std::string str, sf::Vector2f pos) {
            sf::Text t(str, font, 24);
            t.setFillColor(sf::Color::Black);
            sf::FloatRect b = t.getLocalBounds();
            t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
            t.setPosition(pos + sf::Vector2f(radius, radius));
            return t;
            };

       // window.draw(createText("1", speed_up.getPosition()));
        //window.draw(createText("2", attack_up.getPosition()));
        //window.draw(createText("3", health_up.getPosition()));
        ///window.draw(createText("4", fo_up.getPosition()));
        window.draw(choices);
    }
}




void spawnGameOver(sf::RenderWindow& window) {
    sf::RectangleShape table(sf::Vector2f(400.f, 220.f));
    table.setFillColor(sf::Color(20, 20, 80, 220));
    table.setPosition(200.f, 150.f);

    window.draw(table);

    sf::Font font;
    if (font.loadFromFile("arial.ttf")) {
        sf::Text go_text("GAME OVER!!", font, 36);
        go_text.setPosition(250.f, 180.f);
        sf::Text hint("R - Restart, Q - Quit", font, 18);
        hint.setPosition(280.f, 280.f);
        window.draw(go_text);
        window.draw(hint);
    }
}

void spawn_enemy_wave(float total_time, std::vector<Enemy>& enemies, int& wave, std::map<std::string, sf::Texture>& textures) {
    int current_wave_count = static_cast<int>(total_time / 10);
    if (current_wave_count >= wave) {
        wave++;
        for (int i = 0; i < 5 + wave; i++) {
            int randomSpriteNumber = rand() % 3;
            std::string key;
            if (randomSpriteNumber == 0) {
                key = "enemy";
            }
            else if (randomSpriteNumber == 1) {
                key = "enemy_green";
            }
            else {
                key = "enemy_red";
            }
            enemies.push_back(Enemy(10+(current_wave_count-1),current_wave_count, textures[key]));
        }
    }
}

// --- MAIN ---

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Survival Game");
    window.setFramerateLimit(60);

    // TEXTURES
    std::map<std::string, sf::Texture> enemyTextures;
    if (!enemyTextures["enemy"].loadFromFile("assets/enemy.png")) {
        enemyTextures["enemy"].create(32, 32);
    }
    if (!enemyTextures["enemy_green"].loadFromFile("assets/enemy_green.png")) {
        enemyTextures["enemy_green"].create(32, 32);
    }
    if (!enemyTextures["enemy_red"].loadFromFile("assets/enemy_red.png")) {
        enemyTextures["enemy_red"].create(32, 32);
    }

    sf::Texture bossTexture;
    if (!bossTexture.loadFromFile("assets/boss.png")) {
        bossTexture.create(64, 64);
    }

    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("assets/background.png")) {
        sf::Image img; img.create(WINDOW_WIDTH, WINDOW_HEIGHT, sf::Color(50, 100, 50));
        backgroundTexture.loadFromImage(img);
    }
    sf::Sprite backgroundSprite;
    backgroundSprite.setTexture(backgroundTexture);
    sf::Vector2u textureSize = backgroundTexture.getSize();
    float scaleX = static_cast<float>(WINDOW_WIDTH) / textureSize.x;
    float scaleY = static_cast<float>(WINDOW_HEIGHT) / textureSize.y;
    backgroundSprite.setScale(scaleX, scaleY);

    Player player(INITIAL_HP, INITIAL_MAX_HP, INITIAL_SPEED, INITIAL_ATTACK, INITIAL_ATTACK_SPEED);

    Gun pistol(1.0f, 1.0f);
    player.weapons.push_back(&pistol);

    float total_time = 0.f;
    int score = 0;
    int wave = 0;
    bool weapon_spawned = false;
    bool game_over = false;

    float fire_timer = 0.f;
    int burst_shots_fired = 0;
    float burst_delay_timer = 0.f;
    bool is_bursting = false;

    // BOSS CONFIGURATION
    bool spawnBossAtStart = true;
    Boss* boss = nullptr;

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

            // SPAWN BOSS
            if (spawnBossAtStart && boss == nullptr) {
                boss = new Boss(500.f, 20.f, bossTexture);
            }

            spawn_enemy_wave(total_time, enemies, wave, enemyTextures);
            player.update(deltaTime, window, active);

            fire_timer += deltaTime;
            int weapon = player.getWeaponIndex();
            sf::Vector2f pPos = player.getBody().getPosition();

            switch (weapon) {
                case 0: {
                    float delay = 1.0f / player.get_attack_speed();
                    if (fire_timer >= delay) {
                        fire_timer = 0.f;
                        float angle = player.shooting_angle(window);
                        bullets.push_back(Bullet(player.getAttack(), angle, pPos.x, pPos.y));
                    }
                    break;
                }
                case 1: {
                    float delay = 0.2f / player.get_attack_speed();
                    if (fire_timer >= delay) {
                        fire_timer = 0.f;
                        float angle = player.shooting_angle(window);
                        float spread = (std::rand() % 20 - 10) * 0.01f;
                        bullets.push_back(Bullet(player.getAttack() * 0.5f, angle + spread, pPos.x, pPos.y));
                    }
                    break;
                }
                case 2: {
                    float main_delay = 1.5f / player.get_attack_speed();
                    if (!is_bursting && fire_timer >= main_delay) {
                        is_bursting = true;
                        burst_shots_fired = 0;
                        fire_timer = 0.f;
                    }
                    if (is_bursting) {
                        burst_delay_timer += deltaTime;
                        if (burst_delay_timer >= 0.1f) {
                            burst_delay_timer = 0.f;
                            float angle = player.shooting_angle(window);
                            bullets.push_back(Bullet(player.getAttack(), angle, pPos.x, pPos.y));
                            burst_shots_fired++;
                            if (burst_shots_fired >= 5) {
                                is_bursting = false;
                                fire_timer = 0.f;
                            }
                        }
                    }
                    break;
                }
                case 3: {
                    float delay = 1.2f / player.get_attack_speed();
                    if (fire_timer >= delay) {
                        fire_timer = 0.f;
                        float angle = player.shooting_angle(window);
                        bullets.push_back(Bullet(player.getAttack(), angle, pPos.x, pPos.y));
                        bullets.push_back(Bullet(player.getAttack(), angle - 0.3f, pPos.x, pPos.y));
                        bullets.push_back(Bullet(player.getAttack(), angle + 0.3f, pPos.x, pPos.y));
                    }
                    break;
                }
            }

            for (auto& b : bullets) b.update(deltaTime);
            bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [&](const Bullet& b) {
                return b.getX() < -50 || b.getX() > WINDOW_WIDTH + 50 || b.getY() < -50 || b.getY() > WINDOW_HEIGHT + 50;
            }), bullets.end());

            if (boss) {

                sf::Vector2f bPos = boss->body.getPosition();

                sf::Vector2f dir = pPos - bPos;
                float len = std::hypot(dir.x, dir.y);
                if (len > 0) {
                    dir /= len;


                    boss->body.move(dir * boss->getSpeed() * deltaTime);

                    boss->update(deltaTime);
                }
                if (checkColision(player.getBody(), boss->body)) {
                    player.attacked(window, boss->getAttack());
                    if (player.getHp() <= 0) game_over = true;
                }
            }

            for (auto it = enemies.begin(); it != enemies.end();) {
                sf::Vector2f ePos = it->body.getPosition();
                sf::Vector2f dir = pPos - ePos;
                float len = std::hypot(dir.x, dir.y);

                if (len != 0) {
                    it->body.move((dir / len) * 70.f * deltaTime);
                    it->update(deltaTime);
                }

                if (checkColision(player.getBody(), it->body)) {
                    player.attacked(window, it->getAttack());
                    if (player.getHp() <= 0) game_over = true;
                    it = enemies.erase(it);
                } else {
                    ++it;
                }
            }

            for (auto it_bullet = bullets.begin(); it_bullet != bullets.end();) {
                bool bulletRemoved = false;
                sf::CircleShape bulletHitbox(8.f);
                bulletHitbox.setOrigin(8.f, 8.f);
                bulletHitbox.setPosition(it_bullet->getX(), it_bullet->getY());

                // Bullet vs Boss
                if (boss && checkColision(bulletHitbox, boss->body)) {
                    boss->takeDamage(it_bullet->getDamage());
                    if (boss->getHp() <= 0) {
                        delete boss;
                        boss = nullptr;
                        score += 100;
                        player.gainExperience(50);
                    }
                    bulletRemoved = true;
                }

                // Bullet vs Enemies
                if (!bulletRemoved) {
                    for (auto it_enemy = enemies.begin(); it_enemy != enemies.end();) {
                        if (checkColision(bulletHitbox, it_enemy->body)) {
                            score++;
                            if (player.gainExperience(3)) weapon_spawned = true;
                            it_enemy->takeDamage(it_bullet->getDamage());
                            if (it_enemy->getHp() <= 0) {
                                it_enemy = enemies.erase(it_enemy);
                                player.mob_killed();
                            }
                            bulletRemoved = true;
                            break;
                        } else {
                            ++it_enemy;
                        }
                    }
                }

                if (bulletRemoved) {
                    it_bullet = bullets.erase(it_bullet);
                } else {
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
                fire_timer = 0.f;
                is_bursting = false;

                if (boss) {
                    delete boss;
                    boss = nullptr;
                }
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) window.close();
        }

        if (weapon_spawned) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) { player.setSpeed(player.getSpeed() + 30); weapon_spawned = false; }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) { player.setAttack(player.getAttack() + 2); weapon_spawned = false; }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) { player.setMAX_HP(player.getMaxHp()+20.f); player.heal(); weapon_spawned = false; }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) { player.set_attack_speed(player.get_attack_speed() + 0.2f); weapon_spawned = false; }
        }

        window.clear();
        window.draw(backgroundSprite);

        for (auto& e : enemies) window.draw(e.sprite);

        if (boss) {
            boss->renderBody(window);
            window.draw(boss->sprite);
        }

        for (auto& b : bullets) b.renderBody(window);
        player.renderBody(window);
        player.DrawLabelsWithKillsHealth(window);
        if (weapon_spawned) spawnUpgradeChoice(window);
        if (game_over) spawnGameOver(window);

        window.display();
    }
    return 0;
}
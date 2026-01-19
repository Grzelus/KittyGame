#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <memory>
#include <iostream>
#include <cstdlib>
#include <ctime>

// --- KLASA BAZOWA ---

class Entity {
protected:
    sf::Shape* shape; 
    bool active = true; 
    std::string tag; // Np. "Player", "Enemy", "Wall"

public:
    virtual ~Entity() { delete shape; }

    virtual void update(float deltaTime) = 0; 

    virtual void render(sf::RenderWindow& window) {
        if (active && shape) window.draw(*shape);
    }

    sf::Vector2f getPosition() const { return shape->getPosition(); }
    void setPosition(float x, float y) { shape->setPosition(x, y); }
    bool isActive() const { return active; }
    void destroy() { active = false; }
    std::string getTag() const { return tag; }
    sf::FloatRect getBounds() const { return shape->getGlobalBounds(); }

    // Prosta kolizja AABB
    bool checkCollision(const Entity& other) const {
        if (!active || !other.active || !shape || !other.shape) return false;
        return getBounds().intersects(other.getBounds());
    }
};

// --- ŚCIANY I PRZESZKODY ---

class Wall : public Entity {
public:
    Wall(float x, float y, float w, float h) {
        tag = "Wall";
        shape = new sf::RectangleShape(sf::Vector2f(w, h));
        shape->setFillColor(sf::Color(100, 100, 100)); // Szary
        shape->setPosition(x, y);
    }
    void update(float deltaTime) override {} // Ściana nic nie robi
};

// --- POSTACIE ---

class Character : public Entity {
protected:
    float speed;
    float hp;
    float maxHp;

public:
    Character(float spd, float health) : speed(spd), hp(health), maxHp(health) {}
    
    virtual void takeDamage(float dmg) { 
        hp -= dmg; 
        if (hp <= 0) destroy(); 
    }
    
    // Metoda pomocnicza do cofania ruchu przy kolizji ze ścianą
    void undoMove(sf::Vector2f velocity) {
        shape->move(-velocity);
    }
};

// --- GRACZ ---

class Player : public Character {
private:
    int experience;
    int level;

public:
    Player() : Character(250.f, 100.f), experience(0), level(1) {
        tag = "Player";
        shape = new sf::CircleShape(15.f);
        shape->setFillColor(sf::Color::Magenta);
        shape->setPosition(400, 300);
    }

    // Zmieniamy update, aby zwracał wektor ruchu (potrzebne do kolizji w GameMap)
    sf::Vector2f calculateMovement(float deltaTime) {
        sf::Vector2f movement(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) movement.x -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) movement.x += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) movement.y -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) movement.y += speed;
        return movement * deltaTime;
    }

    void update(float deltaTime) override {
        // Logika ruchu przeniesiona do GameMap dla obsługi kolizji
    }
    
    void move(sf::Vector2f offset) {
        shape->move(offset);
    }

    void gainExp(int amount) {
        experience += amount;
        if (experience >= 100 * level) {
            level++;
            experience = 0;
            hp = maxHp; // Uleczenie przy awansie
            std::cout << "LEVEL UP! Poziom postac: " << level << std::endl;
        }
    }
    
    void resetStats() {
        hp = maxHp;
        level = 1;
        experience = 0;
        setPosition(400, 300);
    }
    
    int getLevel() const { return level; }
};

// --- PRZECIWNICY ---

class Enemy : public Character {
protected:
    Player* target; 

public:
    Enemy(Player* p, float spd, float hp) : Character(spd, hp), target(p) { tag = "Enemy"; }
};

class Minion : public Enemy {
public:
    Minion(Player* p, float difficultyMultiplier) : Enemy(p, 100.f + (difficultyMultiplier * 10), 20.f * difficultyMultiplier) {
        shape = new sf::CircleShape(15.f);
        shape->setFillColor(sf::Color::Red);
        // Losowa pozycja startowa
        shape->setPosition(rand() % 750 + 25, rand() % 550 + 25);
    }

    void update(float deltaTime) override {
        sf::Vector2f dir = target->getPosition() - getPosition();
        float len = std::hypot(dir.x, dir.y);
        if (len > 0) dir /= len;
        shape->move(dir * speed * deltaTime);
    }
};

// --- PORTAL (Przejście do następnego poziomu) ---

class Portal : public Entity {
public:
    Portal(float x, float y) {
        tag = "Portal";
        shape = new sf::RectangleShape(sf::Vector2f(40.f, 60.f));
        shape->setFillColor(sf::Color::Blue);
        shape->setOutlineColor(sf::Color::Cyan);
        shape->setOutlineThickness(2.f);
        shape->setPosition(x, y);
    }
    void update(float deltaTime) override { /* Portal może pulsować */ }
};

// --- MANAGERY ---

class GameMap {
private:
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Wall>> walls; // Przeszkody
    std::unique_ptr<Portal> portal;
    
    int dungeonLevel = 1;

public:
    GameMap() {
        player = std::make_unique<Player>();
        generateLevel();
    }

    // GENEROWANIE PROCEDURALNE
    void generateLevel() {
        enemies.clear();
        walls.clear();
        portal = nullptr;
        
        player->setPosition(100, 100); // Start gracza

        // 1. Generuj ściany (Losowe bloki)
        for (int i = 0; i < 10; i++) {
            float w = rand() % 100 + 50;
            float h = rand() % 100 + 50;
            float x = rand() % 600 + 100;
            float y = rand() % 400 + 100;
            
            // Sprawdź czy ściana nie pojawia się na graczu
            sf::FloatRect wallRect(x, y, w, h);
            if (!wallRect.intersects(player->getBounds())) {
                 walls.push_back(std::make_unique<Wall>(x, y, w, h));
            }
        }

        // 2. Generuj wrogów (ilość zależy od poziomu lochu)
        int enemyCount = 3 + dungeonLevel;
        for (int i = 0; i < enemyCount; i++) {
            enemies.push_back(std::make_unique<Minion>(player.get(), (float)dungeonLevel)); // Wrogowie silniejsi co poziom
        }
        
        // 3. Stwórz portal (zawsze daleko od startu)
        portal = std::make_unique<Portal>(700, 500);
        
        std::cout << "Wygenerowano POZIOM LOCHU: " << dungeonLevel << std::endl;
    }

    void nextLevel() {
        dungeonLevel++;
        player->gainExp(50); // Nagroda za przejście poziomu
        generateLevel();
    }

    void gameOver() {
        std::cout << "GAME OVER! Zginales na poziomie " << dungeonLevel << std::endl;
        dungeonLevel = 1;
        player->resetStats();
        generateLevel();
    }

    void update(float deltaTime) {
        // 1. Logika ruchu gracza z kolizjami ścian
        sf::Vector2f moveVec = player->calculateMovement(deltaTime);
        player->move(moveVec);
        
        // Sprawdź kolizję ze ścianami (jeśli wejdzie w ścianę, cofnij ruch)
        for(auto& wall : walls) {
            if(player->checkCollision(*wall)) {
                player->undoMove(moveVec);
                break;
            }
        }

        // 2. Aktualizacja wrogów
        for (auto& enemy : enemies) {
            if (!enemy->isActive()) continue;
            
            // Zapamiętaj pozycję przed ruchem
            sf::Vector2f oldPos = enemy->getPosition();
            enemy->update(deltaTime);
            
            // Wrogowie też nie przenikają przez ściany (prosta wersja)
            for(auto& wall : walls) {
                if(enemy->checkCollision(*wall)) {
                    enemy->setPosition(oldPos.x, oldPos.y); // Cofnij
                }
            }

            // Kolizja z graczem
            if (player->checkCollision(*enemy)) {
                player->takeDamage(10.f); // Obrażenia
                std::cout << "Otrzymano obrazenia!" << std::endl;
                enemy->destroy(); // Minion ginie przy ataku (kamikadze) dla uproszczenia
                
                if (!player->isActive()) {
                    gameOver();
                    return; // Przerwij update
                }
            }
        }
        
        // 3. Sprawdzenie Portalu
        if (portal && player->checkCollision(*portal) && enemies.empty()) {
             // Warunek: musisz zabić wszystkich wrogów, żeby przejść
             nextLevel();
        } else if (portal && player->checkCollision(*portal) && !enemies.empty()) {
             // Opcjonalnie: komunikat "Zabij wrogów!"
        }

        // Czyszczenie martwych wrogów z pamięci
        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), 
            [](const std::unique_ptr<Enemy>& e) { return !e->isActive(); }), enemies.end());
    }

    void render(sf::RenderWindow& window) {
        if(portal) portal->render(window);
        for (const auto& wall : walls) wall->render(window);
        for (const auto& enemy : enemies) enemy->render(window);
        player->render(window);
    }
};

int main() {
    srand(time(nullptr));
    sf::RenderWindow window(sf::VideoMode(800, 600), "Roguelike SFML");
    window.setFramerateLimit(60);

    GameMap dungeon;
    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        float deltaTime = clock.restart().asSeconds();
        dungeon.update(deltaTime);

        window.clear();
        dungeon.render(window);
        window.display();
    }
    return 0;
}
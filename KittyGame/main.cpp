#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <time.h>
#include <iostream>
#include <vector>
#include <cmath>

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
    float size=2;
	float speed;
	float x_position;
	float y_position;
	sf::CircleShape bulletShape;
public:    
    Bullet(float o_damage, float o_size, float o_speed)
        : damage(o_damage), size(o_size), speed(o_speed) {
	}
	void move(float deltaTime, float angle) {
		x_position = x_position + std::cos(angle) * speed * deltaTime;
		y_position = y_position + std::sin(angle) * speed * deltaTime;
    }
    void setBodyShape() {
        bulletShape = sf::CircleShape(size);
        bulletShape.setFillColor(sf::Color::Green);
	}

    void setPosition(float x, float y) {
        x_position = x;
		y_position = y;
	    void renderBody(sf::RenderWindow& window) {
            bulletShape.setPosition(x_position, y_position);
            window.draw(bulletShape);
        }
    }

    float getDamage() const {
        return this->damage;
    }
};


class ExpCrystal : public GameObject {};

class Character : public GameObject {
private:
    float hp;
    float MAX_HP;
    float speed;
    float attack;
    float x_position = 0.f;
    float y_position = 0.f;
public:
    Character(float o_hp, float o_MAX_HP, float o_speed, float o_attack)
        : hp(o_hp), MAX_HP(o_MAX_HP), speed(o_speed), attack(o_attack) {
    }

    virtual void takeDamage(float dmg) { hp -= dmg; }
    void heal() { this->hp = MAX_HP; }
    void setMAX_HP(float new_MAX_HP) { this->MAX_HP = new_MAX_HP; }
    void setSpeed(float new_speed) { this->speed = new_speed; }
    void setAttack(float new_attack) { this->attack = new_attack; }

    float getHp() const { return this->hp; }
    float getAttack() const { return this->attack; }
    float getSpeed() const { return this->speed; }
    float getpostionX() const { return this->x_position; }
    float getpostionY() const { return this->y_position; }
};

void renderBullets(sf::RenderWindow& window, float angle) {};

class Player : public Character {
private:
    float size = 0.f;
    sf::CircleShape bodyShape;
    int experience = 0;
    int level = 1;
public:
    Player(float o_hp = 100, float o_MAX_HP = 100, float o_speed = 300, float o_attack = 5)
        : Character(o_hp, o_MAX_HP, o_speed, o_attack),
        size(50.f),
        bodyShape(size)
    {
        bodyShape.setPosition(100.f, 100.f);
        bodyShape.setFillColor(sf::Color::Magenta);
        setBody(&bodyShape);
    }

    // Funkcja tworz¹ca/konfigruj¹ca body gracza
    void createBody(float radius, const sf::Vector2f& position, const sf::Color& color) {
        size = radius;
        bodyShape = sf::CircleShape(radius);
        bodyShape.setPosition(position);
        bodyShape.setFillColor(color);
        setBody(&bodyShape);
    }

    bool gainExperience(int exp) {
        this->experience += exp;
        if(experience >= level * 10) {
            experience -= level * 10;
            levelUp();
            return 1; 
		}
        return 0;
    }
    void levelUp() {
        this->level += 1;
        this->heal();
    }
    float shooting_angle(sf::RenderWindow &window,bool canMove) {
        if (!canMove) return;
        float mouse_to_player_angle = std::atan2(
            sf::Mouse::getPosition(window).y - (bodyShape.getPosition().y + bodyShape.getRadius()),
            sf::Mouse::getPosition(window).x - (bodyShape.getPosition().x + bodyShape.getRadius())
		);
		return mouse_to_player_angle;
    }


    void update(float deltaTime, const sf::RenderWindow& window, bool canMove) {
        if (!canMove) return;

        sf::Vector2f movement(0.f, 0.f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  movement.x -= getSpeed();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) movement.x += getSpeed();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  movement.y += getSpeed();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    movement.y -= getSpeed();

        // ograniczenie do granic okna
        sf::Vector2u winSize = window.getSize();
        float diameter = bodyShape.getRadius() * 2.f;
        sf::Vector2f pos = bodyShape.getPosition();

        if (pos.x <= 0.f && movement.x < 0.f) movement.x = 0.f;
        if (pos.x + diameter >= static_cast<float>(winSize.x) && movement.x > 0.f) movement.x = 0.f;
        if (pos.y <= 0.f && movement.y < 0.f) movement.y = 0.f;
        if (pos.y + diameter >= static_cast<float>(winSize.y) && movement.y > 0.f) movement.y = 0.f;

        bodyShape.move(movement * deltaTime);

        //shooting

    }

    // wygodny dostêp do kszta³tu
    const sf::CircleShape& getBody() const { return bodyShape; }
    sf::CircleShape& getBody() { return bodyShape; }

    // opcjonalna metoda rysuj¹ca (nie wymagana, bo renderBody robi to przez body pointer)
    void draw(sf::RenderWindow& window) const { window.draw(bodyShape); }
};

class Enemy {
private:
    float size = 20.f;
public:
    Enemy() {};
    float getSize() const { return this->size; }
    sf::CircleShape spawn() const {
        float x = static_cast<float>(std::rand() % 801);
        float y = static_cast<float>(std::rand() % 601);
        sf::CircleShape body(size);
        body.setFillColor(sf::Color::Red);
        body.setPosition(x, y);
        return body;
    }
};

class Boss : public Enemy {};
class Minion : public Enemy {};

// Functionality
bool checkColision(const sf::CircleShape& a, const sf::CircleShape& b) {
    sf::Vector2f aCenter = a.getPosition() + sf::Vector2f(a.getRadius(), a.getRadius());
    sf::Vector2f bCenter = b.getPosition() + sf::Vector2f(b.getRadius(), b.getRadius());
    float dist = std::hypot(aCenter.x - bCenter.x, aCenter.y - bCenter.y);
    return dist < (a.getRadius() + b.getRadius());
}


// Spawning upgrade choice UI
void spawnWeaponChoice(sf::RenderWindow& window) {
    sf::RectangleShape table(sf::Vector2f(400.f, 200.f));
    table.setFillColor(sf::Color::Blue);
    table.setPosition(150.f, 150.f);

    sf::CircleShape first_upgrade(40.f, 3);
    first_upgrade.setFillColor(sf::Color::Magenta);
    first_upgrade.setPosition(200.f, 200.f);

    sf::CircleShape second_upgrade(40.f, 4);
    second_upgrade.setFillColor(sf::Color::Yellow);
    second_upgrade.setPosition(300.f, 200.f);

    sf::CircleShape third_upgrade(40.f, 5);
    third_upgrade.setFillColor(sf::Color::Cyan);
    third_upgrade.setPosition(400.f, 200.f);


    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        std::cerr << "Blad ladowania czcionki!" << std::endl;
    }

    sf::Text below_choices;
    below_choices.setFont(font);
    below_choices.setString("+5 Speed    +5 Attack   +5HP");
    below_choices.setCharacterSize(18);
    below_choices.setFillColor(sf::Color::White);
    below_choices.setStyle(sf::Text::Bold);
    below_choices.setPosition(200.f, 300.f);

    // tworzymy trzy oddzielne teksty, po jednym na figurê, i centrujemy je wzglêdem œrodka ko³a
    sf::Text num1, num2, num3;
    num1.setFont(font); num1.setString("1"); num1.setCharacterSize(24); num1.setFillColor(sf::Color::Black);
    num2.setFont(font); num2.setString("2"); num2.setCharacterSize(24); num2.setFillColor(sf::Color::Black);
    num3.setFont(font); num3.setString("3"); num3.setCharacterSize(24); num3.setFillColor(sf::Color::Black);

    // obliczamy œrodki kó³ (pozycja + promieñ)
    sf::Vector2f center1 = first_upgrade.getPosition() + sf::Vector2f(first_upgrade.getRadius(), first_upgrade.getRadius());
    sf::Vector2f center2 = second_upgrade.getPosition() + sf::Vector2f(second_upgrade.getRadius(), second_upgrade.getRadius());
    sf::Vector2f center3 = third_upgrade.getPosition() + sf::Vector2f(third_upgrade.getRadius(), third_upgrade.getRadius());

    // ustawiamy origin ka¿dego tekstu na œrodek jego bounding box, by poprawnie siê wycentrowa³
    auto centerText = [](sf::Text& t, const sf::Vector2f& pos) {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        t.setPosition(pos);
    };

    centerText(num1, center1);
    centerText(num2, center2);
    centerText(num3, center3);

    window.draw(table);
    window.draw(first_upgrade);
    window.draw(second_upgrade);
    window.draw(third_upgrade);
    window.draw(num1);
    window.draw(num2);
    window.draw(num3);
    window.draw(below_choices);
}

void renderBullets(sf::RenderWindow& window, float angle) {


}


int main()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    sf::RenderWindow window(sf::VideoMode(800, 600), "Movement (C++17)");
    window.setFramerateLimit(60);

    // utwórz gracza i skonfiguruj jego body
    Player player;
    player.createBody(50.f, sf::Vector2f(100.f, 100.f), sf::Color::Magenta);

    int counter = 0;
    bool weapon_spawned = false;

    // Spawning Enemies
    std::vector<sf::CircleShape> enemies;
    enemies.reserve(16);
    for (int i = 0; i < 10; ++i) {
        Enemy enemy;
        enemies.push_back(enemy.spawn());
    }

    sf::Clock clock;
    while (window.isOpen()) {
        // Close window Handler
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        float deltaTime = clock.restart().asSeconds();

        // Update player (ruch)
        player.update(deltaTime, window, !weapon_spawned);

        // compute player center for enemy AI / kolizje
        sf::Vector2f playerCenter = player.getBody().getPosition() + sf::Vector2f(player.getBody().getRadius(), player.getBody().getRadius());

        // Scoring points
        for (auto it = enemies.begin(); it != enemies.end();) {
            if (checkColision(player.getBody(), *it)) {
                it = enemies.erase(it);
                counter++;
                if(player.gainExperience(1))
                {
                    weapon_spawned = true;
                }
                std::cout << "Score: " << counter << '\n';
            }
            else {
                ++it;
            }
        }

		// handling choice of weapon and upgrades
        if (weapon_spawned) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
            {   
				player.setSpeed(player.getSpeed() + 30);
				counter = 0;
				weapon_spawned = false;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
            {
				player.setAttack(player.getAttack() + 1);
                counter = 0;
                weapon_spawned = false;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
            {
				player.setMAX_HP(player.getHp() + 10);
                counter = 0;
                weapon_spawned = false;
            }
        }

        // Enemy Movement (goni¹ gracza)
        if (!weapon_spawned) {
            for (auto& enemy : enemies) {
                sf::Vector2f enemyCenter = enemy.getPosition() + sf::Vector2f(enemy.getRadius(), enemy.getRadius());
                sf::Vector2f direction = playerCenter - enemyCenter;
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length != 0.f) direction /= length;
                float enemySpeed = 50.f;
                enemy.move(direction.x * deltaTime * enemySpeed, direction.y * deltaTime * enemySpeed);
            }
        }

        // Rendering
        window.clear();
        player.renderBody(window);
        for (auto& e : enemies) window.draw(e);

        if (weapon_spawned) {
            spawnWeaponChoice(window); 
        }

        window.display();
    }

    return 0;
}
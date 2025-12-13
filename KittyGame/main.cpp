#include <SFML/Graphics.hpp>
#include <stdlib.h>
#include <time.h>
#include <iostream>

//Classes

class GameObject {
private:
    sf::Shape* body;
	bool active=0;
public:
    GameObject() = default;




};
class Item : public GameObject {
private:
public:
    void PlayerEffect();

};

class Weapon : public GameObject {};

class ExpCrystal : public GameObject {};


class Character: public GameObject{
private:
    float hp;
	float MAX_HP;
    float speed;
	float attack;
	float x_position;
	float y_position;
public:
    Character(float o_hp,float o_MAX_HP,float o_speed,float o_attack): hp(o_hp), MAX_HP(o_MAX_HP),speed(o_speed), attack(o_attack) {};
    
    virtual void takeDamage(float dmg) {};
    void heal(float new_hp) {
        this->hp = MAX_HP;
    }
    void setMAX_HP(float new_MAX_HP) {
        this->MAX_HP = new_MAX_HP;
	}
    void setSpeed(float new_speed)
    {
        this->speed = new_speed;
	}
    void setAttack(float new_attack)
    {
        this->attack = new_attack;
	}
};

class Player : public Character {
private:
    float size;
    int experience;
    int level;
public:
    Player(float o_hp=100, float o_MAX_HP=100, float o_speed=5, float o_attack=5)
        : Character(o_hp, o_MAX_HP, o_speed, o_attack), experience(0), level(1) {};

    void gainExperience(int exp) {
        this->experience += exp;
        //level up to set
    }
    void levelUp(){
        this->level += 1;
    }
    //void Heal() {
      //  hp = MAX_HP;
    //}
};




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
        float y = rand() % (600 + 1);
        sf::CircleShape body(size);
        body.setFillColor(sf::Color::Red);
        body.setPosition(x, y);
        return body;
    };
};

class Boss : public Enemy {

};
class Minion : public Enemy {
};

//Functionality
bool checkColision(const sf::CircleShape& a, const sf::CircleShape& b) {
    sf::Vector2f aCenter = a.getPosition() + sf::Vector2f(a.getRadius(), a.getRadius());
    sf::Vector2f bCenter = b.getPosition() + sf::Vector2f(b.getRadius(), b.getRadius());
    float dist = std::hypot(aCenter.x - bCenter.x, aCenter.y - bCenter.y);
    return dist < (a.getRadius() + b.getRadius());
};



void spawnWeaponChoice(int level){
	sf::RectangleShape table(sf::Vector2f(200.f, 100.f));
	table.setFillColor(sf::Color::Blue);
	table.setPosition(300.f, 250.f);
    
    sf::CircleShape first_weapon(80.f, 3);
    table.setFillColor(sf::Color::Magenta);
    table.setPosition(200.f, 200.f);

    sf::CircleShape second_weapon(80.f, 4);
    table.setFillColor(sf::Color::Yellow);
    table.setPosition(150.f, 150.f);

    sf::CircleShape third_weapon(80.f, 8);
    table.setFillColor(sf::Color::Cyan);
    table.setPosition(100.f, 100.f);

}

int main()
{
    srand(time(nullptr));

    sf::RenderWindow window(sf::VideoMode(800, 600), "Movement");
    sf::CircleShape player(100.f);
    player.setPosition(100.f, 100.f);
    player.setFillColor(sf::Color::Magenta);

    int counter = 0;
	bool weapon_spawned = false;
    //Spawning Enemies
    std::vector<sf::CircleShape> enemies;
    for (int i = 0; i < 10; i++) {
        Enemy enemy = Enemy();
        sf::CircleShape enemy_body = enemy.spawn();
        enemies.push_back(enemy_body);
    }

   

    sf::Clock clock;
    float speed = 300.f;
    while (window.isOpen()) {
        //Close window Handler
        sf::Event event;
       
        while (window.pollEvent(event)){
            if (event.type == sf::Event::Closed) {
                window.close();
             }
        }
        sf::Vector2f playerCenter = player.getPosition() + sf::Vector2f(player.getRadius(), player.getRadius());
        float deltaTime = clock.restart().asSeconds();
        //Movement
        
        sf::Vector2f movement(0.f, 0.f);
        if (!weapon_spawned) {

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
            if (player.getPosition().x + player.getRadius() * 2 >= 800 && movement.x > 0) {
                movement.x = 0;
            }
            if (player.getPosition().y <= 0 && movement.y < 0) {
                movement.y = 0;
            }
            if (player.getPosition().y + player.getRadius() * 2 >= 600 && movement.y > 0) {
                movement.y = 0;
            }

            player.move(movement * deltaTime);
        }

        //Scoring points 
        for (auto enemy = enemies.begin(); enemy != enemies.end();) {
            if (checkColision(player, *enemy)) {
                enemy = enemies.erase(enemy);
                counter++;
                std::cout << counter << std::endl;
                if(counter==10){
                    std::cout<<"New weapon!"<<std::endl;
                    
					//table to choose weapon
 
					//Player.levelUp();
                    //auto table_with_weapons= spawnWeaponChoice(Player.level);
                    //window.draw(table_with_weapons.first);
                    //window.draw(table_with_weapons.second);
                    //window.draw(table_with_weapons.third);
                    //window.draw(table_with_weapons.fourth);
					//window.display();
					//weapon_spawned=true;

				}
            }
            else {
                enemy++;
            }
        }
		//handling choice of weapon       
        if (weapon_spawned) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                weapon_spawned = false;
                counter = 0;
            }
        }

		//Enemy Movement
        if (!weapon_spawned) {

            for (auto& enemy : enemies) {
                sf::Vector2f enemyCenter = enemy.getPosition() + sf::Vector2f(enemy.getRadius(), enemy.getRadius());
                sf::Vector2f direction = playerCenter - enemyCenter;
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length != 0) {
                    direction /= length;
                }
                float enemySpeed = 50.f;
                enemy.move(direction.x * deltaTime * enemySpeed, direction.y * deltaTime * enemySpeed);
            }
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

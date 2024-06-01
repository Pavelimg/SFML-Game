#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>
#include <string>
#include <cmath>

#define PI 3.14159265
float window_size_x = 2000, window_size_y = 1000;
int n = 5;

class SpaceObject {
public:
    float x = 0, y = 0, x_v = 0, y_v = 0, sphere_range = std::rand() % 100 + 10;
    bool repulse = false;

    SpaceObject(int x_cord, int y_cord, bool is_repulse) {
        x = x_cord;
        y = y_cord;
        repulse = is_repulse;
    }

    SpaceObject(int x_cord, int y_cord, int x_speed, int y_speed, bool is_repulse) {
        x = x_cord;
        y = y_cord;
        x_v = x_speed;
        y_v = y_speed;
        repulse = is_repulse;
    }

    SpaceObject(int x_cord, int y_cord, int x_speed, int y_speed, bool is_repulse, int sphere_r) {
        x = x_cord;
        y = y_cord;
        x_v = x_speed;
        y_v = y_speed;
        repulse = is_repulse;
        sphere_range = sphere_r;
    }

    void move(float seconds_from_last_flip) {
        x += x_v * seconds_from_last_flip;
        y += y_v * seconds_from_last_flip;

        if (!repulse) { return; }
        if (x + sphere_range > window_size_x || 0 > x - sphere_range) {
            x_v = -x_v;
            x = std::min(window_size_x - sphere_range, std::max(0.0f + sphere_range, x));
        }
        if (y + sphere_range > window_size_y || 0 > y - sphere_range) {
            y_v = -y_v;
            y = std::min(window_size_y - sphere_range, std::max(0.0f + sphere_range, y));
        }
    }
};

class Bullet : public SpaceObject {
public:
    sf::RectangleShape sprite;
    bool exist = false;
    int speed = 400;

    Bullet() : SpaceObject(0, 0, 0, 0, false) {
    }

    void flip(float dt) {
        if (exist) {
            move(dt);
            if (x > window_size_x || x < 0 || y > window_size_y || y < 0) {
                exist = false;
                sprite.setFillColor(sf::Color(255, 255, 255, 255));
            }
            sprite.setPosition(x, y);
        }
    }


    void setup(int x_pos, int y_pos, float rotate) {
        sprite.setFillColor(sf::Color(255, 255, 255));
        sprite.setSize(sf::Vector2f(5, 30));
        sprite.setRotation(rotate);
        exist = true;
        x = x_pos;
        y = y_pos;
        x_v = speed * cos((180 - rotate + 90) * PI / 180);
        y_v = -speed * sin((180 - rotate + 90) * PI / 180);
        std::cout << x_v << " " << y_v << " " << 180 - rotate + 90 << "\n";
        sprite.setPosition(x, y);
    }
};


class Controllable : public SpaceObject {
public:
    sf::Sprite sprite;
    float fric = 2;
    float accel = 90;
    float rotate = 0;
    int rotation_speed = 80;
    int time_from_last_shoot = 0;
    int cooldown = 1;
    static const int max_bullets = 10;
    Bullet bullets[max_bullets];

    int last_bullet = 0;

    Controllable() : SpaceObject(100, 100, false) {
    }

    void flip(float dt) {
        x += x_v * dt;
        y += y_v * dt;
        sprite.setRotation(rotate);
        x_v -= sin((360 - rotate) * PI / 180) * fric * dt;
        y_v -= cos((360 - rotate) * PI / 180) * fric * dt;
        sprite.setPosition(sf::Vector2(x, y));

        for (int i = 0; i < max_bullets; i++) {
            bullets[i].flip(dt);
        }
    }

    void clockwise_rotate(float dt) {
        rotate += rotation_speed * dt;
    }

    void counterclockwise_rotate(float dt) {
        rotate -= rotation_speed * dt;
    }

    void acceleration(float dt) {
        x_v += sin((360 - rotate) * PI / 180) * accel * dt;
        y_v += cos((360 - rotate) * PI / 180) * accel * dt;

    }

    void shoot(float global_timer_seconds) {
        if (global_timer_seconds - time_from_last_shoot < cooldown) {
            return;
        }
        time_from_last_shoot = global_timer_seconds;
        bullets[last_bullet].setup(x, y, sprite.getRotation());
        last_bullet++;
        if (last_bullet > n - 1) {
            last_bullet = 0;
        }
    }
};

class Destroyer : public Controllable {
    sf::Texture texture;
    float scale = 0.2;
public:
    Destroyer() : Controllable() {
        texture.loadFromFile("textures/ship1.png");
        sprite.setTexture(texture);
        sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
        sprite.setScale(sf::Vector2(scale, scale));
    }
};

class Enemy : public SpaceObject {
public:
    sf::CircleShape hitbox;
    sf::Texture texture;
    bool exist = true;

    Enemy(int size, int x_cord, int y_cord, int x_speed, int y_speed) : SpaceObject(x_cord, y_cord, x_speed, y_speed,
        true, size) {
        hitbox_setup();
    }

    Enemy(int size, int x_cord, int y_cord) : SpaceObject(x_cord, y_cord, std::rand() % 100, std::rand() % 100, true,
        size) {
        hitbox_setup();
    }

    Enemy(int size, int x_cord, int y_cord, std::string texture_path) : SpaceObject(x_cord, y_cord, std::rand() % 100,
        std::rand() % 100, true, size) {
        hitbox_setup();
        texture_setup(texture_path);
    }

    Enemy() : SpaceObject(std::rand() % (int)window_size_x, std::rand() % (int)window_size_x, std::rand() % 500,
        std::rand() % 500, true) {
        hitbox_setup();
    }

    Enemy(std::string texture_path) : SpaceObject(std::rand() % (int)window_size_x, std::rand() % (int)window_size_x,
        std::rand() % 500, std::rand() % 500, true) {
        hitbox_setup();
        texture_setup(texture_path);
    }

    virtual ~Enemy() {
    }

private:
    void texture_setup(std::string texture_path) {
        texture.loadFromFile(texture_path);
        hitbox.setTexture(&texture);
    }

    void hitbox_setup() {
        hitbox.setRadius(sphere_range);
        hitbox.setFillColor(sf::Color(255, 255, 255));
        hitbox.setOrigin(sphere_range, sphere_range);
    }

public:
    void flip(float seconds_from_last_flip) {
        move(seconds_from_last_flip);
        hitbox.setPosition(x, y);
    }

    virtual void on_hit() {

    }
};

class Asteroid : public Enemy {
public:
    Asteroid() : Enemy("textures/stone-texture.jpg") {}

    void on_hit() override {
        //std::cout << "Астероид стукнулся!\n";
    }
};

class Moon : public Enemy {
public:
    Moon() : Enemy(150, (int)(window_size_x / 4), (int)(window_size_y / 2), "textures/moon.jpg") {
    }

    void on_hit() override {
        //std::cout << "Астероид стукнулся!\n";
    }
};

class Sun : public Enemy {
public:
    Sun() : Enemy(250, (int)(window_size_x / 4 * 3), (int)(window_size_y / 2), "textures/sun.jpeg") {}

    void on_hit() override {
        //std::cout << "Солнце столкнулось!!\n";
    }
};

Asteroid* asteroids;
sf::Sprite* background_puzzle;

void check_enemy_collision(Enemy& first, Enemy& second) {
    if (!first.exist || !second.exist) { return; }
    if (std::pow(first.sphere_range + second.sphere_range, 2) >
        std::pow(first.x - second.x, 2) + std::pow(first.y - second.y, 2)) {
        float a_x = first.x_v;
        first.x_v = second.x_v;
        second.x_v = a_x;

        float a_y = first.y_v;
        first.y_v = second.y_v;
        second.y_v = a_y;

        while (sqrt(pow(first.x - second.x, 2) + pow(first.y - second.y, 2)) <
            first.sphere_range + second.sphere_range) {
            first.flip(0.001);
            second.flip(0.001);
        }

        first.on_hit();
        second.on_hit();
    }
};

void check_collisions() {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            check_enemy_collision(asteroids[i], asteroids[j]);
        }
    }

}

void load_background(sf::Texture& back_texture) {
    int counter = 0;
    for (int x = 0; x < window_size_x / 256; x++) {
        for (int y = 0; y < window_size_y / 256; y++) {
            background_puzzle[counter].setTexture(back_texture);
            background_puzzle[counter].setPosition(x * 256, y * 256);
            counter++;
        }
    }
}

void draw_background(sf::RenderWindow& window) {
    for (int i = 0; i < (((int)window_size_x / 256 + 1) * ((int)window_size_y / 256 + 1)); i++) {
        window.draw(background_puzzle[i]);
    }
}

void check_events(sf::RenderWindow& window, Destroyer& battleship, float& dt, sf::Clock& global_clock) {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        battleship.acceleration(dt);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        battleship.acceleration(-dt);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        battleship.counterclockwise_rotate(dt);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        battleship.clockwise_rotate(dt);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        battleship.shoot(global_clock.getElapsedTime().asSeconds());
    }
}

void enemy_update(sf::RenderWindow& window, float dt, Sun& sun, Moon& moon) {
    for (int i = 0; i < n; i++) {
        if (asteroids[i].exist) {
            asteroids[i].flip(dt);
            window.draw(asteroids[i].hitbox);
        }
    }

    if (moon.exist) {
        moon.flip(dt);
        window.draw(moon.hitbox);
    }
    if (sun.exist) {
        sun.flip(dt);
        window.draw(sun.hitbox);
    }
    for (int i = 0; i < n; i++) {
        if (asteroids[i].exist) {
            check_enemy_collision(asteroids[i], moon);
            check_enemy_collision(asteroids[i], sun);
        }
    }
    check_enemy_collision(moon, sun);
    check_collisions();
}

float timer(sf::Clock& clock) {
    float dt = clock.getElapsedTime().asSeconds();
    clock.restart();
    return dt;
}

void check_bullets(sf::RenderWindow& window, Destroyer& battleship, Sun& sun, Moon& moon) {
    for (int i = 0; i < battleship.max_bullets; i++) {
        if (battleship.bullets[i].exist) {
            window.draw(battleship.bullets[i].sprite);
            for (int j = 0; j < n; j++) {
                if (pow(asteroids[j].x - battleship.bullets[i].x, 2) +
                    pow(asteroids[j].y - battleship.bullets[i].y, 2) < pow(asteroids[j].sphere_range, 2) &&
                    asteroids[j].exist) {
                    battleship.bullets[i].exist = false;
                    asteroids[j].exist = false;
                }
            }
            if (pow(sun.x - battleship.bullets[i].x, 2) + pow(sun.y - battleship.bullets[i].y, 2) <
                pow(sun.sphere_range, 2) && sun.exist) {
                battleship.bullets[i].exist = false;
                sun.exist = false;
            }
            if (pow(moon.x - battleship.bullets[i].x, 2) + pow(moon.y - battleship.bullets[i].y, 2) <
                pow(moon.sphere_range, 2) && moon.exist) {
                battleship.bullets[i].exist = false;
                moon.exist = false;
            }
        }
    }
}

void battleship_update(sf::RenderWindow& window, Destroyer& battleship, float& dt) {
    battleship.flip(dt);
    window.draw(battleship.sprite);
}

int main() {
    sf::Texture background_texture;
    background_texture.loadFromFile("textures/end_portal.png");
    background_puzzle = new sf::Sprite[((int)window_size_x / 256 + 1) * ((int)window_size_y / 256 + 1)];
    load_background(background_texture);
    Destroyer battleship;
    std::srand(std::time(0));
    sf::RenderWindow window(sf::VideoMode(window_size_x, window_size_y), "SFML works!");
    asteroids = new Asteroid[n];
    Moon moon = Moon();
    Sun sun = Sun();
    sf::Clock clock;
    sf::Clock global_clock;

    while (window.isOpen()) {
        window.clear();
        draw_background(window);
        float dt = timer(clock);
        enemy_update(window, dt, sun, moon);
        battleship_update(window, battleship, dt);
        check_bullets(window, battleship, sun, moon);
        check_events(window, battleship, dt, global_clock);
        window.display();
    }
    return 0;
}

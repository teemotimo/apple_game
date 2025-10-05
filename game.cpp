#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>

// Constants
const int WIDTH = 800;
const int HEIGHT = 600;
const float PLAYER_SPEED = 5.0f;
const float APPLE_FALL_SPEED = 2.0f;
const int MIN_DESIRE = 30;
const int MAX_DESIRE = 80;
const int GAME_DURATION = 60; // seconds

enum class GameState {
    INTRO,
    PLAYING,
    GAME_OVER,
    VICTORY
};

enum class AppleType {
    RED,
    GOLDEN,
    ROTTEN
};

struct Apple {
    sf::CircleShape shape;
    AppleType type;
    float speed;
    bool active;

    Apple(float x, float y, AppleType t) : type(t), speed(APPLE_FALL_SPEED), active(true) {
        shape.setRadius(15.0f);
        shape.setPosition(sf::Vector2f(x, y));
        
        switch(type) {
            case AppleType::RED:
                shape.setFillColor(sf::Color(220, 20, 60));
                break;
            case AppleType::GOLDEN:
                shape.setFillColor(sf::Color(255, 215, 0));
                break;
            case AppleType::ROTTEN:
                shape.setFillColor(sf::Color(101, 67, 33));
                break;
        }
    }

    void update() {
        if (active) {
            shape.move(sf::Vector2f(0.f, speed));
        }
    }

    bool isOffScreen() {
        return shape.getPosition().y > HEIGHT;
    }

    sf::FloatRect getBounds() {
        return shape.getGlobalBounds();
    }
};

class Game {
private:
    sf::RenderWindow window;
    GameState state;
    sf::Font font;
    
    // Player
    sf::RectangleShape player;
    float playerX;
    
    // Game stats
    int score;
    int desireGauge;
    float gameTime;
    float spawnTimer;
    float introTimer;
    int introScene;
    
    // Apples
    std::vector<Apple> apples;
    
    // UI Elements
    sf::Text scoreText;
    sf::Text desireText;
    sf::Text timerText;
    sf::RectangleShape desireBar;
    sf::RectangleShape desireBarBg;
    
    std::string gameOverReason;

public:
    Game() : window(sf::VideoMode({WIDTH, HEIGHT}), "Balance of Desire"),
             font(),
             scoreText(font, "", 24),
             desireText(font, "", 20),
             timerText(font, "", 20),
             state(GameState::INTRO), playerX(WIDTH / 2.0f), 
             score(0), desireGauge(50), gameTime(0), spawnTimer(0),
             introTimer(0), introScene(0) {
        
        window.setFramerateLimit(60);
        srand(static_cast<unsigned>(time(0)));
        
        // Load font
        if (!font.openFromFile("/System/Library/Fonts/Helvetica.ttc")) {
            // Try Windows font path
            if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
                // Try Linux font path
                if (!font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
                    // Font loading failed - using default
                }
            }
        }
        
        // Setup player
        player.setSize(sf::Vector2f(50.f, 20.f));
        player.setFillColor(sf::Color::White);
        player.setOrigin(sf::Vector2f(25.f, 10.f));
        player.setPosition(sf::Vector2f(playerX, static_cast<float>(HEIGHT) - 50.f));
        
        // Setup UI
        setupUI();
    }

    void setupUI() {
        scoreText = sf::Text(font, "", 24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(sf::Vector2f(10.f, 10.f));
        
        desireText = sf::Text(font, "", 20);
        desireText.setFillColor(sf::Color::White);
        desireText.setPosition(sf::Vector2f(10.f, 50.f));
        
        timerText = sf::Text(font, "", 20);
        timerText.setFillColor(sf::Color::White);
        timerText.setPosition(sf::Vector2f(WIDTH - 150.f, 10.f));
        
        // Desire bar background
        desireBarBg.setSize(sf::Vector2f(200.f, 20.f));
        desireBarBg.setFillColor(sf::Color(50, 50, 50));
        desireBarBg.setPosition(sf::Vector2f(10.f, 80.f));
        
        // Desire bar
        desireBar.setSize(sf::Vector2f(200.f, 20.f));
        desireBar.setFillColor(sf::Color::Green);
        desireBar.setPosition(sf::Vector2f(10.f, 80.f));
    }

    void run() {
        sf::Clock clock;
        
        while (window.isOpen()) {
            float deltaTime = clock.restart().asSeconds();
            
            handleEvents();
            update(deltaTime);
            render();
        }
    }

    void handleEvents() {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (state == GameState::INTRO) {
                    if (keyPressed->code == sf::Keyboard::Key::Space) {
                        state = GameState::PLAYING;
                        resetGame();
                    }
                }
                if (state == GameState::GAME_OVER || state == GameState::VICTORY) {
                    if (keyPressed->code == sf::Keyboard::Key::R) {
                        state = GameState::INTRO;
                        introScene = 0;
                        introTimer = 0;
                    }
                }
            }
        }
    }

    void update(float deltaTime) {
        switch(state) {
            case GameState::INTRO:
                updateIntro(deltaTime);
                break;
            case GameState::PLAYING:
                updatePlaying(deltaTime);
                break;
            case GameState::GAME_OVER:
            case GameState::VICTORY:
                break;
        }
    }

    void updateIntro(float deltaTime) {
        introTimer += deltaTime;
        
        // Cycle through intro scenes every 3 seconds
        if (introTimer > 3.0f) {
            introTimer = 0;
            introScene = (introScene + 1) % 6;
        }
    }

    void updatePlaying(float deltaTime) {
        gameTime += deltaTime;
        
        // Check win condition
        if (gameTime >= GAME_DURATION) {
            if (desireGauge >= MIN_DESIRE && desireGauge <= MAX_DESIRE) {
                state = GameState::VICTORY;
            } else {
                state = GameState::GAME_OVER;
                gameOverReason = "Time's up!";
            }
            return;
        }
        
        // Player movement - Arrow keys or A/D
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            playerX -= PLAYER_SPEED;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            playerX += PLAYER_SPEED;
        }
        
        // Clamp player position
        playerX = std::max(25.0f, std::min(playerX, static_cast<float>(WIDTH) - 25.0f));
        player.setPosition(sf::Vector2f(playerX, static_cast<float>(HEIGHT) - 50.f));
        
        // Spawn apples
        spawnTimer += deltaTime;
        if (spawnTimer > 1.0f) {
            spawnApple();
            spawnTimer = 0;
        }
        
        // Update apples
        for (auto it = apples.begin(); it != apples.end();) {
            it->update();
            
            // Check collision with player
            if (it->active && it->getBounds().findIntersection(player.getGlobalBounds()).has_value()) {
                collectApple(*it);
                it = apples.erase(it);
            }
            // Remove off-screen apples
            else if (it->isOffScreen()) {
                missApple();
                it = apples.erase(it);
            }
            else {
                ++it;
            }
        }
        
        // Slowly decrease desire over time
        if (static_cast<int>(gameTime * 2) % 3 == 0) {
            desireGauge = std::max(0, desireGauge - 1);
        }
        
        // Check game over conditions
        if (desireGauge < MIN_DESIRE) {
            state = GameState::GAME_OVER;
            gameOverReason = "Apathy - You lost the will to live";
        }
        else if (desireGauge > MAX_DESIRE) {
            state = GameState::GAME_OVER;
            gameOverReason = "Obsession - Consumed by greed";
        }
        
        // Update desire bar
        float desirePercent = desireGauge / 100.0f;
        desireBar.setSize(sf::Vector2f(200.f * desirePercent, 20.f));
        
        // Change bar color based on danger
        if (desireGauge < MIN_DESIRE || desireGauge > MAX_DESIRE) {
            desireBar.setFillColor(sf::Color::Red);
        } else if (desireGauge < 40 || desireGauge > 70) {
            desireBar.setFillColor(sf::Color::Yellow);
        } else {
            desireBar.setFillColor(sf::Color::Green);
        }
    }

    void spawnApple() {
        float x = static_cast<float>(rand() % (WIDTH - 60) + 30);
        int chance = rand() % 100;
        
        AppleType type;
        if (chance < 60) {
            type = AppleType::RED;
        } else if (chance < 80) {
            type = AppleType::GOLDEN;
        } else {
            type = AppleType::ROTTEN;
        }
        
        apples.emplace_back(x, -30.f, type);
    }

    void collectApple(Apple& apple) {
        switch(apple.type) {
            case AppleType::RED:
                score += 20;
                desireGauge = std::min(100, desireGauge + 20);
                break;
            case AppleType::GOLDEN:
                score += 80;
                desireGauge = std::max(0, desireGauge - 10);
                break;
            case AppleType::ROTTEN:
                score += 5;
                desireGauge = std::min(100, desireGauge + 40);
                break;
        }
    }

    void missApple() {
        desireGauge = std::max(0, desireGauge - 20);
    }

    void resetGame() {
        score = 0;
        desireGauge = 50;
        gameTime = 0;
        spawnTimer = 0;
        playerX = WIDTH / 2.0f;
        apples.clear();
    }

    void render() {
        window.clear(sf::Color::Black);
        
        switch(state) {
            case GameState::INTRO:
                renderIntro();
                break;
            case GameState::PLAYING:
                renderPlaying();
                break;
            case GameState::GAME_OVER:
                renderGameOver();
                break;
            case GameState::VICTORY:
                renderVictory();
                break;
        }
        
        window.display();
    }

    void renderIntro() {
        std::string scenes[] = {
            "A single red apple falls from the sky...\n\n\"The apple reflects the desire of mankind.\"",
            "A golden apple descends slowly...\n\n\"Some desires shine brighter than others...\"\n\"...tempting, precious, yet fleeting.\"",
            "The light fades. A rotten apple drops...\n\n\"But every desire carries danger within.\"\n\"Corruption follows those who crave too much.\"",
            "Golden and rotten apples fall together...\n\n\"We must choose...\"\n\"Which desire will we fulfill?\"",
            "Hundreds of apples fall from the sky...\n\n\"At times, choice is not a gift...\"\n\"...but a necessity.\"",
            "\"How will you endure your own desire?\"\n\"Find the balance... or be devoured by it.\"\n\n\nPress SPACE to begin\nUse Arrow Keys or A/D to move"
        };
        
        sf::Text introText(font, scenes[introScene], 22);
        introText.setFillColor(sf::Color::White);
        introText.setPosition(sf::Vector2f(WIDTH / 2.f - 300.f, HEIGHT / 2.f - 100.f));
        window.draw(introText);
    }

    void renderPlaying() {
        // Draw apples
        for (auto& apple : apples) {
            window.draw(apple.shape);
        }
        
        // Draw player
        window.draw(player);
        
        // Draw UI
        scoreText.setString("Score: " + std::to_string(score));
        window.draw(scoreText);
        
        desireText.setString("Desire: " + std::to_string(desireGauge) + " [30-80]");
        window.draw(desireText);
        
        int timeLeft = GAME_DURATION - static_cast<int>(gameTime);
        timerText.setString("Time: " + std::to_string(timeLeft) + "s");
        window.draw(timerText);
        
        window.draw(desireBarBg);
        window.draw(desireBar);
        
        // Draw legend
        sf::Text legend(font);
        legend.setCharacterSize(14);
        legend.setFillColor(sf::Color::White);
        legend.setPosition(sf::Vector2f(WIDTH - 280.f, HEIGHT - 80.f));
        legend.setString("Red: +20 score, +20 desire\nGold: +80 score, -10 desire\nBrown: +5 score, +40 desire\nMissed: -20 desire");
        window.draw(legend);
    }

    void renderGameOver() {
        std::string message = "GAME OVER\n\n" + gameOverReason + 
                             "\n\nFinal Score: " + std::to_string(score) +
                             "\n\nPress R to restart";
        sf::Text gameOverText(font, message, 28);
        gameOverText.setFillColor(sf::Color::White);
        gameOverText.setStyle(sf::Text::Bold);
        gameOverText.setPosition(sf::Vector2f(WIDTH / 2.f - 250.f, HEIGHT / 2.f - 100.f));
        window.draw(gameOverText);
    }

    void renderVictory() {
        std::string message = "VICTORY!\n\nYou maintained balance\n\nFinal Score: " + 
                             std::to_string(score) +
                             "\nDesire: " + std::to_string(desireGauge) +
                             "\n\nPress R to restart";
        sf::Text victoryText(font, message, 28);
        victoryText.setFillColor(sf::Color(255, 215, 0));
        victoryText.setStyle(sf::Text::Bold);
        victoryText.setPosition(sf::Vector2f(WIDTH / 2.f - 200.f, HEIGHT / 2.f - 120.f));
        window.draw(victoryText);
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}
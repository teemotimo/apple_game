#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>

// Constants
const int WIDTH = 1000;
const int HEIGHT = 700;
const float PLAYER_SPEED = 8.0f;
const float APPLE_FALL_SPEED = 3.375f;
const int MIN_DESIRE = 30;
const int MAX_DESIRE = 80;
const int GAME_DURATION = 180;

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
    float alpha; // For fade effects

    Apple(float x, float y, AppleType t, float customSpeed = 0) : type(t), speed(customSpeed > 0 ? customSpeed : APPLE_FALL_SPEED), active(true), alpha(255.0f) {
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

    void updateAlpha(float newAlpha) {
        alpha = newAlpha;
        sf::Color color = shape.getFillColor();
        color.a = static_cast<std::uint8_t>(alpha);
        shape.setFillColor(color);
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
    
    // Intro animation
    std::vector<Apple> introApples;
    float introTimer;
    int introScene;
    float sceneTimer;
    
    // Player
    sf::RectangleShape player;
    float playerX;
    
    // Game stats
    int score;
    int desireGauge;
    float gameTime;
    float spawnTimer;
    
    // Apples
    std::vector<Apple> apples;
    
    // UI Elements
    sf::Text titleText;
    sf::Text scoreText;
    sf::Text desireText;
    sf::Text timerText;
    sf::RectangleShape desireBar;
    sf::RectangleShape desireBarBg;
    sf::RectangleShape desireBarBorder;
    sf::RectangleShape uiPanel;
    sf::RectangleShape legendPanel;
    
    std::string gameOverReason;

public:
    Game() : window(sf::VideoMode({WIDTH, HEIGHT}), "Balance of Desire"),
             font(),
             titleText(font, "", 32),
             scoreText(font, "", 28),
             desireText(font, "", 24),
             timerText(font, "", 28),
             state(GameState::INTRO), playerX(WIDTH / 2.0f), 
             score(0), desireGauge(50), gameTime(0), spawnTimer(0),
             introTimer(0), introScene(0), sceneTimer(0) {
        
        window.setFramerateLimit(60);
        srand(static_cast<unsigned>(time(0)));
        
        // Load font
        if (!font.openFromFile("/System/Library/Fonts/Helvetica.ttc")) {
            if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
                if (!font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
                    // Font loading failed - using default
                }
            }
        }
        
        // Setup player
        player.setSize(sf::Vector2f(70.f, 15.f));
        player.setFillColor(sf::Color(139, 69, 19));
        player.setOutlineThickness(2.f);
        player.setOutlineColor(sf::Color(101, 50, 15));
        player.setOrigin(sf::Vector2f(35.f, 7.5f));
        player.setPosition(sf::Vector2f(playerX, static_cast<float>(HEIGHT) - 80.f));
        
        // Setup UI
        setupUI();
    }

    void setupUI() {
        uiPanel.setSize(sf::Vector2f(WIDTH, 120.f));
        uiPanel.setFillColor(sf::Color(20, 20, 30, 230));
        uiPanel.setPosition(sf::Vector2f(0.f, 0.f));
        
        titleText = sf::Text(font, "BALANCE OF DESIRE", 32);
        titleText.setFillColor(sf::Color(255, 215, 0));
        titleText.setStyle(sf::Text::Bold);
        titleText.setPosition(sf::Vector2f(WIDTH / 2.f - 180.f, 10.f));
        
        scoreText = sf::Text(font, "Score: 0", 28);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setStyle(sf::Text::Bold);
        scoreText.setPosition(sf::Vector2f(30.f, 55.f));
        
        timerText = sf::Text(font, "Time: 180s", 28);
        timerText.setFillColor(sf::Color(100, 200, 255));
        timerText.setStyle(sf::Text::Bold);
        timerText.setPosition(sf::Vector2f(WIDTH - 180.f, 55.f));
        
        desireText = sf::Text(font, "Desire Gauge", 24);
        desireText.setFillColor(sf::Color(255, 255, 200));
        desireText.setPosition(sf::Vector2f(WIDTH / 2.f - 80.f, 55.f));
        
        desireBarBorder.setSize(sf::Vector2f(304.f, 24.f));
        desireBarBorder.setFillColor(sf::Color::Transparent);
        desireBarBorder.setOutlineThickness(3.f);
        desireBarBorder.setOutlineColor(sf::Color(200, 200, 200));
        desireBarBorder.setPosition(sf::Vector2f(WIDTH / 2.f - 150.f, 85.f));
        
        desireBarBg.setSize(sf::Vector2f(300.f, 20.f));
        desireBarBg.setFillColor(sf::Color(40, 40, 50));
        desireBarBg.setPosition(sf::Vector2f(WIDTH / 2.f - 148.f, 87.f));
        
        desireBar.setSize(sf::Vector2f(300.f, 20.f));
        desireBar.setFillColor(sf::Color::Green);
        desireBar.setPosition(sf::Vector2f(WIDTH / 2.f - 148.f, 87.f));
        
        legendPanel.setSize(sf::Vector2f(WIDTH, 60.f));
        legendPanel.setFillColor(sf::Color(20, 20, 30, 230));
        legendPanel.setPosition(sf::Vector2f(0.f, HEIGHT - 60.f));
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
                        sceneTimer = 0;
                        introApples.clear();
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
        sceneTimer += deltaTime;
        
        // Scene 0: Single red apple falling slowly
        if (introScene == 0) {
            if (sceneTimer < 0.5f && introApples.empty()) {
                introApples.emplace_back(WIDTH / 2.f, -30.f, AppleType::RED, 1.5f);
            }
        }
        // Scene 1: Golden apple descending slowly with spotlight
        else if (introScene == 1) {
            if (sceneTimer < 0.5f && introApples.empty()) {
                introApples.emplace_back(WIDTH / 2.f, -30.f, AppleType::GOLDEN, 1.0f);
            }
        }
        // Scene 2: Rotten apple drops quickly
        else if (introScene == 2) {
            if (sceneTimer < 0.5f && introApples.empty()) {
                introApples.emplace_back(WIDTH / 2.f, -30.f, AppleType::ROTTEN, 2.5f);
            }
        }
        // Scene 3: Golden and rotten falling together
        else if (introScene == 3) {
            if (sceneTimer < 0.5f) {
                if (introApples.empty()) {
                    introApples.emplace_back(WIDTH / 2.f - 100.f, -30.f, AppleType::GOLDEN, 1.8f);
                    introApples.emplace_back(WIDTH / 2.f + 100.f, -30.f, AppleType::ROTTEN, 1.8f);
                }
            }
        }
        // Scene 4: Rain of apples
        else if (introScene == 4) {
            static float rainTimer = 0;
            rainTimer += deltaTime;
            if (rainTimer > 0.3f) {
                float x = static_cast<float>(rand() % WIDTH);
                int typeRand = rand() % 3;
                AppleType type = typeRand == 0 ? AppleType::RED : (typeRand == 1 ? AppleType::GOLDEN : AppleType::ROTTEN);
                introApples.emplace_back(x, -30.f, type, 2.0f + (rand() % 100) / 100.0f);
                rainTimer = 0;
            }
        }
        
        // Update intro apples
        for (auto it = introApples.begin(); it != introApples.end();) {
            it->update();
            if (it->isOffScreen()) {
                it = introApples.erase(it);
            } else {
                ++it;
            }
        }
        
        // Advance scenes
        if (sceneTimer > 5.0f) {
            sceneTimer = 0;
            introScene++;
            introApples.clear();
            
            if (introScene > 5) {
                introScene = 0;
                introTimer = 0;
            }
        }
    }

    void updatePlaying(float deltaTime) {
        gameTime += deltaTime;
        
        if (gameTime >= GAME_DURATION) {
            if (desireGauge >= MIN_DESIRE && desireGauge <= MAX_DESIRE) {
                state = GameState::VICTORY;
            } else {
                state = GameState::GAME_OVER;
                gameOverReason = "Time's up!";
            }
            return;
        }
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            playerX -= PLAYER_SPEED;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            playerX += PLAYER_SPEED;
        }
        
        playerX = std::max(35.0f, std::min(playerX, static_cast<float>(WIDTH) - 35.0f));
        player.setPosition(sf::Vector2f(playerX, static_cast<float>(HEIGHT) - 80.f));
        
        spawnTimer += deltaTime;
        if (spawnTimer > 2.0f) {
            spawnApple();
            spawnTimer = 0;
        }
        
        for (auto it = apples.begin(); it != apples.end();) {
            it->update();
            
            if (it->active && it->getBounds().findIntersection(player.getGlobalBounds()).has_value()) {
                collectApple(*it);
                it = apples.erase(it);
            }
            else if (it->isOffScreen()) {
                missApple();
                it = apples.erase(it);
            }
            else {
                ++it;
            }
        }
        
        static float desireDecayTimer = 0;
        desireDecayTimer += deltaTime;
        if (desireDecayTimer > 10.0f) {
            desireGauge = std::max(0, desireGauge - 1);
            desireDecayTimer = 0;
        }
        
        if (desireGauge < MIN_DESIRE) {
            state = GameState::GAME_OVER;
            gameOverReason = "Apathy - You lost the will to live";
        }
        else if (desireGauge > MAX_DESIRE) {
            state = GameState::GAME_OVER;
            gameOverReason = "Obsession - Consumed by greed";
        }
        
        float desirePercent = desireGauge / 100.0f;
        desireBar.setSize(sf::Vector2f(300.f * desirePercent, 20.f));
        
        if (desireGauge < MIN_DESIRE) {
            desireBar.setFillColor(sf::Color(200, 50, 50));
        } else if (desireGauge > MAX_DESIRE) {
            desireBar.setFillColor(sf::Color(255, 50, 0));
        } else if (desireGauge < 40) {
            desireBar.setFillColor(sf::Color(255, 200, 0));
        } else if (desireGauge > 70) {
            desireBar.setFillColor(sf::Color(255, 165, 0));
        } else {
            desireBar.setFillColor(sf::Color(50, 205, 50));
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
        desireGauge = std::max(0, desireGauge - 10);
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
        // Dark gradient background
        sf::RectangleShape background(sf::Vector2f(WIDTH, HEIGHT));
        background.setFillColor(sf::Color(10, 10, 20));
        window.draw(background);
        
        // Draw animated apples
        for (auto& apple : introApples) {
            window.draw(apple.shape);
            
            // Add glow effect for golden apples
            if (apple.type == AppleType::GOLDEN) {
                sf::CircleShape glow(25.f);
                glow.setPosition(apple.shape.getPosition() - sf::Vector2f(10.f, 10.f));
                glow.setFillColor(sf::Color(255, 215, 0, 50));
                window.draw(glow);
            }
        }
        
        // Scene-specific text
        std::string scenes[] = {
            "A single red apple falls from the sky...\n\n\"The apple reflects the desire of mankind.\"",
            "A golden apple descends slowly...\n\n\"Some desires shine brighter than others...\"\n\"...tempting, precious, yet fleeting.\"",
            "The light fades. A rotten apple drops...\n\n\"But every desire carries danger within.\"\n\"Corruption follows those who crave too much.\"",
            "Golden and rotten apples fall together...\n\n\"We must choose...\"\n\"Which desire will we fulfill?\"",
            "Hundreds of apples fall from the sky...\n\n\"At times, choice is not a gift...\"\n\"...but a necessity.\"",
            "\"How will you endure your own desire?\"\n\"Find the balance... or be devoured by it.\"\n\n\nPress SPACE to begin\nUse Arrow Keys or A/D to move"
        };
        
        // Semi-transparent subtitle background
        sf::RectangleShape subtitleBg(sf::Vector2f(WIDTH, 200.f));
        subtitleBg.setFillColor(sf::Color(0, 0, 0, 180));
        subtitleBg.setPosition(sf::Vector2f(0.f, HEIGHT - 220.f));
        window.draw(subtitleBg);
        
        // Subtitle text
        sf::Text introText(font, scenes[introScene], 24);
        introText.setFillColor(sf::Color::White);
        introText.setStyle(sf::Text::Bold);
        introText.setLineSpacing(1.5f);
        
        sf::FloatRect textBounds = introText.getLocalBounds();
        introText.setPosition(sf::Vector2f(WIDTH / 2.f - textBounds.size.x / 2.f, HEIGHT - 190.f));
        window.draw(introText);
        
        // Title at top
        sf::Text title(font, "BALANCE OF DESIRE", 48);
        title.setFillColor(sf::Color(255, 215, 0));
        title.setStyle(sf::Text::Bold);
        sf::FloatRect titleBounds = title.getLocalBounds();
        title.setPosition(sf::Vector2f(WIDTH / 2.f - titleBounds.size.x / 2.f, 50.f));
        window.draw(title);
    }

    void renderPlaying() {
        for (auto& apple : apples) {
            window.draw(apple.shape);
        }
        
        window.draw(player);
        window.draw(uiPanel);
        window.draw(legendPanel);
        window.draw(titleText);
        
        scoreText.setString("Score: " + std::to_string(score));
        window.draw(scoreText);
        
        desireText.setString("Desire: " + std::to_string(desireGauge) + "%");
        window.draw(desireText);
        
        int timeLeft = GAME_DURATION - static_cast<int>(gameTime);
        int minutes = timeLeft / 60;
        int seconds = timeLeft % 60;
        timerText.setString("Time: " + std::to_string(minutes) + ":" + 
                           (seconds < 10 ? "0" : "") + std::to_string(seconds));
        window.draw(timerText);
        
        window.draw(desireBarBg);
        window.draw(desireBar);
        window.draw(desireBarBorder);
        
        float safeStartX = WIDTH / 2.f - 148.f + (MIN_DESIRE * 3.f);
        float safeEndX = WIDTH / 2.f - 148.f + (MAX_DESIRE * 3.f);
        
        sf::RectangleShape safeMarkerLeft(sf::Vector2f(2.f, 26.f));
        safeMarkerLeft.setFillColor(sf::Color::White);
        safeMarkerLeft.setPosition(sf::Vector2f(safeStartX, 86.f));
        window.draw(safeMarkerLeft);
        
        sf::RectangleShape safeMarkerRight(sf::Vector2f(2.f, 26.f));
        safeMarkerRight.setFillColor(sf::Color::White);
        safeMarkerRight.setPosition(sf::Vector2f(safeEndX, 86.f));
        window.draw(safeMarkerRight);
        
        float legendY = HEIGHT - 35.f;
        float legendStartX = 30.f;
        float spacing = 235.f;
        
        sf::CircleShape redCircle(10.f);
        redCircle.setFillColor(sf::Color(220, 20, 60));
        redCircle.setPosition(sf::Vector2f(legendStartX, legendY));
        window.draw(redCircle);
        sf::Text redText(font, "Red: +20 score, +20 desire", 16);
        redText.setFillColor(sf::Color::White);
        redText.setPosition(sf::Vector2f(legendStartX + 25.f, legendY - 2.f));
        window.draw(redText);
        
        sf::CircleShape goldCircle(10.f);
        goldCircle.setFillColor(sf::Color(255, 215, 0));
        goldCircle.setPosition(sf::Vector2f(legendStartX + spacing, legendY));
        window.draw(goldCircle);
        sf::Text goldText(font, "Gold: +80 score, -10 desire", 16);
        goldText.setFillColor(sf::Color::White);
        goldText.setPosition(sf::Vector2f(legendStartX + spacing + 25.f, legendY - 2.f));
        window.draw(goldText);
        
        sf::CircleShape rottenCircle(10.f);
        rottenCircle.setFillColor(sf::Color(101, 67, 33));
        rottenCircle.setPosition(sf::Vector2f(legendStartX + spacing * 2, legendY));
        window.draw(rottenCircle);
        sf::Text rottenText(font, "Rotten: +5 score, +40 desire", 16);
        rottenText.setFillColor(sf::Color::White);
        rottenText.setPosition(sf::Vector2f(legendStartX + spacing * 2 + 25.f, legendY - 2.f));
        window.draw(rottenText);
        
        sf::Text missedText(font, "Missed: -10 desire", 16);
        missedText.setFillColor(sf::Color(255, 100, 100));
        missedText.setPosition(sf::Vector2f(legendStartX + spacing * 3 + 5.f, legendY - 2.f));
        window.draw(missedText);
    }

    void renderGameOver() {
        sf::RectangleShape overlay(sf::Vector2f(WIDTH, HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window.draw(overlay);
        
        sf::RectangleShape messageBox(sf::Vector2f(600.f, 350.f));
        messageBox.setFillColor(sf::Color(30, 30, 40, 250));
        messageBox.setOutlineThickness(5.f);
        messageBox.setOutlineColor(sf::Color(200, 50, 50));
        messageBox.setPosition(sf::Vector2f(WIDTH / 2.f - 300.f, HEIGHT / 2.f - 175.f));
        window.draw(messageBox);
        
        sf::Text gameOverText(font, "GAME OVER", 48);
        gameOverText.setFillColor(sf::Color(255, 100, 100));
        gameOverText.setStyle(sf::Text::Bold);
        sf::FloatRect bounds = gameOverText.getLocalBounds();
        gameOverText.setPosition(sf::Vector2f(WIDTH / 2.f - bounds.size.x / 2.f, HEIGHT / 2.f - 140.f));
        window.draw(gameOverText);
        
        sf::Text reasonText(font, gameOverReason, 24);
        reasonText.setFillColor(sf::Color::White);
        sf::FloatRect reasonBounds = reasonText.getLocalBounds();
        reasonText.setPosition(sf::Vector2f(WIDTH / 2.f - reasonBounds.size.x / 2.f, HEIGHT / 2.f - 60.f));
        window.draw(reasonText);
        
        sf::Text scoreDisplay(font, "Final Score: " + std::to_string(score), 32);
        scoreDisplay.setFillColor(sf::Color(255, 215, 0));
        scoreDisplay.setStyle(sf::Text::Bold);
        sf::FloatRect scoreBounds = scoreDisplay.getLocalBounds();
        scoreDisplay.setPosition(sf::Vector2f(WIDTH / 2.f - scoreBounds.size.x / 2.f, HEIGHT / 2.f + 10.f));
        window.draw(scoreDisplay);
        
        sf::Text restartText(font, "Press R to restart", 22);
        restartText.setFillColor(sf::Color(150, 150, 150));
        sf::FloatRect restartBounds = restartText.getLocalBounds();
        restartText.setPosition(sf::Vector2f(WIDTH / 2.f - restartBounds.size.x / 2.f, HEIGHT / 2.f + 100.f));
        window.draw(restartText);
    }

    void renderVictory() {
        sf::RectangleShape overlay(sf::Vector2f(WIDTH, HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 0, 200));
        window.draw(overlay);
        
        sf::RectangleShape messageBox(sf::Vector2f(600.f, 400.f));
        messageBox.setFillColor(sf::Color(30, 40, 30, 250));
        messageBox.setOutlineThickness(5.f);
        messageBox.setOutlineColor(sf::Color(255, 215, 0));
        messageBox.setPosition(sf::Vector2f(WIDTH / 2.f - 300.f, HEIGHT / 2.f - 200.f));
        window.draw(messageBox);
        
        sf::Text victoryTitle(font, "VICTORY!", 52);
        victoryTitle.setFillColor(sf::Color(255, 215, 0));
        victoryTitle.setStyle(sf::Text::Bold);
        sf::FloatRect titleBounds = victoryTitle.getLocalBounds();
        victoryTitle.setPosition(sf::Vector2f(WIDTH / 2.f - titleBounds.size.x / 2.f, HEIGHT / 2.f - 160.f));
        window.draw(victoryTitle);
        
        sf::Text balanceText(font, "You maintained balance!", 28);
        balanceText.setFillColor(sf::Color(100, 255, 100));
        sf::FloatRect balanceBounds = balanceText.getLocalBounds();
        balanceText.setPosition(sf::Vector2f(WIDTH / 2.f - balanceBounds.size.x / 2.f, HEIGHT / 2.f - 80.f));
        window.draw(balanceText);
        
        sf::Text scoreDisplay(font, "Final Score: " + std::to_string(score), 36);
        scoreDisplay.setFillColor(sf::Color::White);
        scoreDisplay.setStyle(sf::Text::Bold);
        sf::FloatRect scoreBounds = scoreDisplay.getLocalBounds();
        scoreDisplay.setPosition(sf::Vector2f(WIDTH / 2.f - scoreBounds.size.x / 2.f, HEIGHT / 2.f - 10.f));
        window.draw(scoreDisplay);
        
        sf::Text desireDisplay(font, "Final Desire: " + std::to_string(desireGauge) + "%", 28);
        desireDisplay.setFillColor(sf::Color(150, 255, 150));
        sf::FloatRect desireBounds = desireDisplay.getLocalBounds();
        desireDisplay.setPosition(sf::Vector2f(WIDTH / 2.f - desireBounds.size.x / 2.f, HEIGHT / 2.f + 50.f));
        window.draw(desireDisplay);
        
        sf::Text restartText(font, "Press R to restart", 22);
        restartText.setFillColor(sf::Color(150, 150, 150));
        sf::FloatRect restartBounds = restartText.getLocalBounds();
        restartText.setPosition(sf::Vector2f(WIDTH / 2.f - restartBounds.size.x / 2.f, HEIGHT / 2.f + 130.f));
        window.draw(restartText);
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}
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
float PLAYER_SPEED = 8.0f;
const float APPLE_FALL_SPEED = 3.375f;
const int MIN_DESIRE = 20;
const int MAX_DESIRE = 80;
const int GAME_DURATION = 180;

enum class GameState {
    INTRO,
    PLAYING,
    PAUSED,
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

    bool isOffScreen() const {
        return shape.getPosition().y > HEIGHT;
    }

    sf::FloatRect getBounds() const {
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
    int lastSpeedIncreaseScore;
    int lastRangeDecreaseScore;
    float currentAppleSpeed;
    int currentMinDesire;
    int currentMaxDesire;
    
    // Intro animation apples
    std::vector<Apple> introApples;
    
    // Apples
    std::vector<Apple> apples;
    
    // UI Elements
    sf::Text titleText;
    sf::Text scoreText;
    sf::Text desireText;
    sf::Text timerText;
    sf::Text minDesireLabel;
    sf::Text maxDesireLabel;
    sf::RectangleShape desireBar;
    sf::RectangleShape desireBarBg;
    sf::RectangleShape desireBarBorder;
    sf::RectangleShape uiPanel;
    sf::RectangleShape legendPanel;
    
    // Notifications
    float rangeChangeNotificationTimer;
    std::string rangeChangeMessage;
    float speedIncreaseNotificationTimer;
    std::string speedIncreaseMessage;
    
    // Pause menu elements
    sf::RectangleShape pauseOverlay;
    sf::RectangleShape pauseMenu;
    sf::RectangleShape resumeButton;
    sf::RectangleShape restartButton;
    sf::RectangleShape quitButton;
    sf::Text pauseTitle;
    sf::Text resumeText;
    sf::Text restartText;
    sf::Text quitText;
    int hoveredButton;
    
    std::string gameOverReason;

public:
    Game() : window(sf::VideoMode({WIDTH, HEIGHT}), "Balance of Desire"),
             font(),
             titleText(font, "", 32),
             scoreText(font, "", 28),
             desireText(font, "", 24),
             timerText(font, "", 28),
             minDesireLabel(font, "", 18),
             maxDesireLabel(font, "", 18),
             pauseTitle(font, "", 48),
             resumeText(font, "", 28),
             restartText(font, "", 28),
             quitText(font, "", 28),
             state(GameState::INTRO), playerX(WIDTH / 2.0f), 
             score(0), desireGauge(50), gameTime(0), spawnTimer(0),
             introTimer(0), introScene(0), hoveredButton(0),
             lastSpeedIncreaseScore(0), lastRangeDecreaseScore(0),
             currentAppleSpeed(APPLE_FALL_SPEED), 
             currentMinDesire(MIN_DESIRE), currentMaxDesire(MAX_DESIRE),
             rangeChangeNotificationTimer(0), rangeChangeMessage(""),
             speedIncreaseNotificationTimer(0), speedIncreaseMessage("") {
        
        window.setFramerateLimit(60);
        srand(static_cast<unsigned>(time(0)));
        
        // Load font
        if (!font.openFromFile("/System/Library/Fonts/Helvetica.ttc")) {
            if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
                if (!font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
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
        
        setupUI();
        setupPauseMenu();
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
        
        minDesireLabel = sf::Text(font, std::to_string(MIN_DESIRE), 18);
        minDesireLabel.setFillColor(sf::Color(255, 255, 100));
        minDesireLabel.setStyle(sf::Text::Bold);
        
        maxDesireLabel = sf::Text(font, std::to_string(MAX_DESIRE), 18);
        maxDesireLabel.setFillColor(sf::Color(255, 255, 100));
        maxDesireLabel.setStyle(sf::Text::Bold);
        
        legendPanel.setSize(sf::Vector2f(WIDTH, 60.f));
        legendPanel.setFillColor(sf::Color(20, 20, 30, 230));
        legendPanel.setPosition(sf::Vector2f(0.f, HEIGHT - 60.f));
    }

    void setupPauseMenu() {
        pauseOverlay.setSize(sf::Vector2f(WIDTH, HEIGHT));
        pauseOverlay.setFillColor(sf::Color(0, 0, 0, 180));
        
        pauseMenu.setSize(sf::Vector2f(500.f, 450.f));
        pauseMenu.setFillColor(sf::Color(30, 30, 40, 250));
        pauseMenu.setOutlineThickness(5.f);
        pauseMenu.setOutlineColor(sf::Color(100, 100, 150));
        pauseMenu.setPosition(sf::Vector2f(WIDTH / 2.f - 250.f, HEIGHT / 2.f - 225.f));
        
        pauseTitle = sf::Text(font, "PAUSED", 48);
        pauseTitle.setFillColor(sf::Color(150, 200, 255));
        pauseTitle.setStyle(sf::Text::Bold);
        sf::FloatRect pauseBounds = pauseTitle.getLocalBounds();
        pauseTitle.setPosition(sf::Vector2f(WIDTH / 2.f - pauseBounds.size.x / 2.f, HEIGHT / 2.f - 180.f));
        
        resumeButton.setSize(sf::Vector2f(350.f, 70.f));
        resumeButton.setFillColor(sf::Color(220, 20, 60)); // Red apple color
        resumeButton.setOutlineThickness(3.f);
        resumeButton.setOutlineColor(sf::Color::White);
        resumeButton.setPosition(sf::Vector2f(WIDTH / 2.f - 175.f, HEIGHT / 2.f - 80.f));
        
        resumeText = sf::Text(font, "Resume Game", 28);
        resumeText.setFillColor(sf::Color::White);
        resumeText.setStyle(sf::Text::Bold);
        sf::FloatRect resumeBounds = resumeText.getLocalBounds();
        resumeText.setPosition(sf::Vector2f(WIDTH / 2.f - resumeBounds.size.x / 2.f, HEIGHT / 2.f - 65.f));
        
        restartButton.setSize(sf::Vector2f(350.f, 70.f));
        restartButton.setFillColor(sf::Color(255, 215, 0)); // Golden apple color
        restartButton.setOutlineThickness(3.f);
        restartButton.setOutlineColor(sf::Color::White);
        restartButton.setPosition(sf::Vector2f(WIDTH / 2.f - 175.f, HEIGHT / 2.f + 20.f));
        
        restartText = sf::Text(font, "Restart Game", 28);
        restartText.setFillColor(sf::Color::White);
        restartText.setStyle(sf::Text::Bold);
        sf::FloatRect restartBounds = restartText.getLocalBounds();
        restartText.setPosition(sf::Vector2f(WIDTH / 2.f - restartBounds.size.x / 2.f, HEIGHT / 2.f + 35.f));
        
        quitButton.setSize(sf::Vector2f(350.f, 70.f));
        quitButton.setFillColor(sf::Color(101, 67, 33)); // Rotten apple color
        quitButton.setOutlineThickness(3.f);
        quitButton.setOutlineColor(sf::Color::White);
        quitButton.setPosition(sf::Vector2f(WIDTH / 2.f - 175.f, HEIGHT / 2.f + 120.f));
        
        quitText = sf::Text(font, "Quit to Menu", 28);
        quitText.setFillColor(sf::Color::White);
        quitText.setStyle(sf::Text::Bold);
        sf::FloatRect quitBounds = quitText.getLocalBounds();
        quitText.setPosition(sf::Vector2f(WIDTH / 2.f - quitBounds.size.x / 2.f, HEIGHT / 2.f + 135.f));
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
                else if (state == GameState::PLAYING) {
                    if (keyPressed->code == sf::Keyboard::Key::Escape || 
                        keyPressed->code == sf::Keyboard::Key::P) {
                        state = GameState::PAUSED;
                        hoveredButton = 0;
                    }
                }
                else if (state == GameState::PAUSED) {
                    if (keyPressed->code == sf::Keyboard::Key::Escape) {
                        state = GameState::PLAYING;
                    }
                }
                else if (state == GameState::GAME_OVER || state == GameState::VICTORY) {
                    if (keyPressed->code == sf::Keyboard::Key::R) {
                        state = GameState::INTRO;
                        introScene = 0;
                        introTimer = 0;
                    }
                }
            }
            
            if (state == GameState::PAUSED) {
                if (const auto* mouseMove = event->getIf<sf::Event::MouseMoved>()) {
                    sf::Vector2f mousePos(static_cast<float>(mouseMove->position.x), 
                                         static_cast<float>(mouseMove->position.y));
                    
                    if (resumeButton.getGlobalBounds().contains(mousePos)) {
                        hoveredButton = 1;
                    } else if (restartButton.getGlobalBounds().contains(mousePos)) {
                        hoveredButton = 2;
                    } else if (quitButton.getGlobalBounds().contains(mousePos)) {
                        hoveredButton = 3;
                    } else {
                        hoveredButton = 0;
                    }
                }
                
                if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseButton->button == sf::Mouse::Button::Left) {
                        sf::Vector2f mousePos(static_cast<float>(mouseButton->position.x), 
                                             static_cast<float>(mouseButton->position.y));
                        
                        if (resumeButton.getGlobalBounds().contains(mousePos)) {
                            state = GameState::PLAYING;
                        } else if (restartButton.getGlobalBounds().contains(mousePos)) {
                            state = GameState::PLAYING;
                            resetGame();
                        } else if (quitButton.getGlobalBounds().contains(mousePos)) {
                            state = GameState::INTRO;
                            introScene = 0;
                            introTimer = 0;
                        }
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
            case GameState::PAUSED:
                break;
            case GameState::GAME_OVER:
            case GameState::VICTORY:
                break;
        }
    }

    void updateIntro(float deltaTime) {
        introTimer += deltaTime;
        
        for (auto& apple : introApples) {
            apple.update();
        }
        
        introApples.erase(
            std::remove_if(introApples.begin(), introApples.end(),
                [](const Apple& a) { return a.isOffScreen(); }),
            introApples.end()
        );
        
        float sceneTime = introTimer;
        float sceneDuration = 7.0f;
        float fadeOutStart = 6.5f;
        
        switch(introScene) {
            case 0:
                if (sceneTime > 1.0f && sceneTime < 1.1f && introApples.empty()) {
                    introApples.emplace_back(WIDTH / 2.f, -30.f, AppleType::RED);
                    introApples.back().speed = 2.0f;
                }
                if (sceneTime > sceneDuration) {
                    introTimer = 0;
                    introScene++;
                    introApples.clear();
                }
                break;
                
            case 1:
                if (sceneTime > 1.5f && sceneTime < 1.6f && introApples.empty()) {
                    introApples.emplace_back(WIDTH / 2.f, -30.f, AppleType::GOLDEN);
                    introApples.back().speed = 1.0f;
                }
                if (sceneTime > sceneDuration) {
                    introTimer = 0;
                    introScene++;
                    introApples.clear();
                }
                break;
                
            case 2:
                if (sceneTime > 1.5f && sceneTime < 1.6f && introApples.empty()) {
                    introApples.emplace_back(WIDTH / 2.f, -30.f, AppleType::ROTTEN);
                    introApples.back().speed = 3.0f;
                }
                if (sceneTime > sceneDuration) {
                    introTimer = 0;
                    introScene++;
                    introApples.clear();
                }
                break;
                
            case 3:
                if (sceneTime > 1.0f && sceneTime < 1.1f && introApples.empty()) {
                    introApples.emplace_back(WIDTH / 2.f - 100.f, -30.f, AppleType::GOLDEN);
                    introApples.back().speed = 1.2f;
                    introApples.emplace_back(WIDTH / 2.f + 100.f, -30.f, AppleType::ROTTEN);
                    introApples.back().speed = 1.2f;
                }
                if (sceneTime > sceneDuration) {
                    introTimer = 0;
                    introScene++;
                    introApples.clear();
                }
                break;
                
            case 4:
                if (sceneTime < 5.5f && static_cast<int>(sceneTime * 10) % 3 == 0) {
                    float randomX = 100.f + static_cast<float>(rand() % (WIDTH - 200));
                    int appleChoice = rand() % 3;
                    AppleType type = (appleChoice == 0) ? AppleType::RED : 
                                   (appleChoice == 1) ? AppleType::GOLDEN : AppleType::ROTTEN;
                    
                    if (introApples.size() < 15) {
                        introApples.emplace_back(randomX, -30.f, type);
                        introApples.back().speed = 1.5f + static_cast<float>(rand() % 100) / 100.f;
                    }
                }
                if (sceneTime > sceneDuration) {
                    introTimer = 0;
                    introScene++;
                    introApples.clear();
                }
                break;
                
            case 5:
                break;
        }
    }

    void updatePlaying(float deltaTime) {
        gameTime += deltaTime;
        
        // Check for speed increases at 200, 600, 1000, 1400, etc.
        if (score >= 200) {
            int currentMilestone = (score / 200) * 200;
            // Trigger on odd multiples: 200, 600, 1000, 1400...
            if (currentMilestone > lastSpeedIncreaseScore && currentMilestone % 400 == 200) {
                currentAppleSpeed *= 1.5f;
                lastSpeedIncreaseScore = currentMilestone;
                speedIncreaseNotificationTimer = 3.0f;
                PLAYER_SPEED += 4;
                speedIncreaseMessage = "Apples Falling Faster!";
            }
        }
        
        // Check for range decreases at 400, 800, 1200, 1600, etc.
        if (score >= 400) {
            int currentMilestone = (score / 200) * 200;
            // Trigger on even multiples: 400, 800, 1200, 1600...
            if (currentMilestone > lastRangeDecreaseScore && currentMilestone % 400 == 0) {
                currentMinDesire = std::min(45, currentMinDesire + 5);
                currentMaxDesire = std::max(55, currentMaxDesire - 5);
                lastRangeDecreaseScore = currentMilestone;
                rangeChangeNotificationTimer = 3.0f;
                rangeChangeMessage = "Safe Zone Narrowed! " + std::to_string(currentMinDesire) + "-" + std::to_string(currentMaxDesire) + "%";
            }
        }
        
        // Check win condition
        if (gameTime >= GAME_DURATION) {
            if (desireGauge >= currentMinDesire && desireGauge <= currentMaxDesire) {
                state = GameState::VICTORY;
            } else {
                state = GameState::GAME_OVER;
                PLAYER_SPEED = 8;
                gameOverReason = "Time's up!";
            }
            return;
        }
        
        // Player movement
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
        
        // Spawn apples
        spawnTimer += deltaTime;
        if (spawnTimer > 1.0f) {
            spawnApple();
            spawnTimer = 0;
        }
        
        // Update apples
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
        
        // Desire decay
        static float desireDecayTimer = 0;
        desireDecayTimer += deltaTime;
        if (desireDecayTimer > 10.0f) {
            desireGauge = std::max(0, desireGauge - 1);
            desireDecayTimer = 0;
        }
        
        // Update notification timers
        if (rangeChangeNotificationTimer > 0) {
            rangeChangeNotificationTimer -= deltaTime;
        }
        if (speedIncreaseNotificationTimer > 0) {
            speedIncreaseNotificationTimer -= deltaTime;
        }
        
        // Check game over conditions
        if (desireGauge < currentMinDesire) {
            state = GameState::GAME_OVER;
            gameOverReason = "Apathy - You lost the will to live";
        }
        else if (desireGauge > currentMaxDesire) {
            state = GameState::GAME_OVER;
            gameOverReason = "Obsession - Consumed by greed";
        }
        
        // Update desire bar
        float desirePercent = desireGauge / 100.0f;
        desireBar.setSize(sf::Vector2f(300.f * desirePercent, 20.f));
        
        // Change bar color based on danger
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
        apples.back().speed = currentAppleSpeed;
    }

    void collectApple(Apple& apple) {
        switch(apple.type) {
            case AppleType::RED:
                score += 20;
                desireGauge = std::min(100, desireGauge + 20);
                break;
            case AppleType::GOLDEN:
                score += 80;
                desireGauge = std::max(0, desireGauge - 15);
                break;
            case AppleType::ROTTEN:
                score += 5;
                desireGauge = std::min(100, desireGauge + 30);
                break;
        }
    }

    void missApple() {
        desireGauge = std::max(0, desireGauge - 5);
    }

    void resetGame() {
        score = 0;
        desireGauge = 50;
        gameTime = 0;
        spawnTimer = 0;
        playerX = WIDTH / 2.0f;
        apples.clear();
        introApples.clear();
        lastSpeedIncreaseScore = 0;
        lastRangeDecreaseScore = 0;
        currentAppleSpeed = APPLE_FALL_SPEED;
        currentMinDesire = MIN_DESIRE;
        currentMaxDesire = MAX_DESIRE;
        rangeChangeNotificationTimer = 0;
        rangeChangeMessage = "";
        speedIncreaseNotificationTimer = 0;
        speedIncreaseMessage = "";
        PLAYER_SPEED = 8;
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
            case GameState::PAUSED:
                renderPlaying();
                renderPauseMenu();
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
        float fadeAlpha = 255.f;
        float sceneDuration = 7.0f;
        float fadeOutStart = 6.0f;
        
        if (introTimer > fadeOutStart && introScene < 5) {
            float fadeProgress = (introTimer - fadeOutStart) / (sceneDuration - fadeOutStart);
            fadeAlpha = 255.f * (1.0f - fadeProgress);
        }
        
        if (introTimer < 0.5f) {
            fadeAlpha = 255.f * (introTimer / 0.5f);
        }
        
        fadeAlpha = std::max(0.f, std::min(255.f, fadeAlpha));
        
        sf::RectangleShape bgGradient(sf::Vector2f(WIDTH, HEIGHT));
        int bgAlpha = static_cast<int>(220.f * (fadeAlpha / 255.f));
        
        switch(introScene) {
            case 0:
                bgGradient.setFillColor(sf::Color(20, 0, 0, bgAlpha));
                break;
            case 1:
                bgGradient.setFillColor(sf::Color(30, 20, 0, bgAlpha));
                break;
            case 2:
                bgGradient.setFillColor(sf::Color(10, 5, 0, std::min(230, bgAlpha + 10)));
                break;
            case 3:
                bgGradient.setFillColor(sf::Color(20, 10, 5, bgAlpha));
                break;
            case 4:
                bgGradient.setFillColor(sf::Color(5, 0, 5, std::max(180, bgAlpha - 20)));
                break;
            default:
                bgGradient.setFillColor(sf::Color(0, 0, 0, 200));
                break;
        }
        window.draw(bgGradient);
        
        int appleAlpha = static_cast<int>(fadeAlpha);
        for (auto& apple : introApples) {
            sf::CircleShape fadedApple = apple.shape;
            sf::Color appleColor = fadedApple.getFillColor();
            appleColor.a = appleAlpha;
            fadedApple.setFillColor(appleColor);
            
            if (apple.type == AppleType::GOLDEN) {
                sf::CircleShape glow(25.f);
                glow.setFillColor(sf::Color(255, 215, 0, std::min(50, appleAlpha / 5)));
                glow.setPosition(apple.shape.getPosition() - sf::Vector2f(10.f, 10.f));
                window.draw(glow);
            }
            
            if (apple.type == AppleType::ROTTEN) {
                sf::CircleShape aura(20.f);
                aura.setFillColor(sf::Color(50, 30, 20, std::min(80, appleAlpha / 3)));
                aura.setPosition(apple.shape.getPosition() - sf::Vector2f(5.f, 5.f));
                window.draw(aura);
            }
            
            window.draw(fadedApple);
        }
        
        float pulseScale = 1.0f + 0.05f * sin(introTimer * 2.0f);
        sf::Text title(font, "BALANCE OF DESIRE", 48);
        title.setFillColor(sf::Color(255, 215, 0, static_cast<int>(fadeAlpha)));
        title.setStyle(sf::Text::Bold);
        sf::FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin(sf::Vector2f(titleBounds.size.x / 2.f, titleBounds.size.y / 2.f));
        title.setPosition(sf::Vector2f(WIDTH / 2.f, 80.f));
        title.setScale(sf::Vector2f(pulseScale, pulseScale));
        window.draw(title);
        
        std::string dialogues[] = {
            "A single red apple falls from the sky...\n\n\"The apple reflects the desire of mankind.\"",
            "A golden apple descends slowly...\n\n\"Some desires shine brighter than others...\"\n\"...tempting, precious, yet fleeting.\"",
            "The light fades. A rotten apple drops...\n\n\"But every desire carries danger within.\"\n\"Corruption follows those who crave too much.\"",
            "Golden and rotten apples fall together...\n\n\"We must choose...\"\n\"Which desire will we fulfill?\"",
            "Hundreds of apples fall from the sky...\n\n\"At times, choice is not a gift...\"\n\"...but a necessity.\"",
            "\"How will you endure your own desire?\"\n\"Find the balance... or be devoured by it.\"\n\n\nPress SPACE to begin\nUse Arrow Keys or A/D to move"
        };
        
        int textAlpha = static_cast<int>(fadeAlpha);
        if (introTimer < 0.8f) {
            textAlpha = static_cast<int>(255.f * (introTimer / 0.8f));
        }
        
        sf::Text subtitleText(font, dialogues[introScene], 22);
        sf::Text shadowText(font, dialogues[introScene], 22);
        shadowText.setFillColor(sf::Color(0, 0, 0, std::min(200, textAlpha)));
        shadowText.setLineSpacing(1.4f);
        
        subtitleText.setFillColor(sf::Color(255, 255, 255, textAlpha));
        subtitleText.setLineSpacing(1.4f);
        sf::FloatRect textBounds = subtitleText.getLocalBounds();
        
        if (introScene == 5) {
            shadowText.setPosition(sf::Vector2f(WIDTH / 2.f - textBounds.size.x / 2.f + 2.f, HEIGHT / 2.f - textBounds.size.y / 2.f + 2.f));
            subtitleText.setPosition(sf::Vector2f(WIDTH / 2.f - textBounds.size.x / 2.f, HEIGHT / 2.f - textBounds.size.y / 2.f));
        } else {
            shadowText.setPosition(sf::Vector2f(WIDTH / 2.f - textBounds.size.x / 2.f + 2.f, HEIGHT - 178.f));
            subtitleText.setPosition(sf::Vector2f(WIDTH / 2.f - textBounds.size.x / 2.f, HEIGHT - 180.f));
        }
        
        window.draw(shadowText);
        window.draw(subtitleText);
        
        if (introScene < 5) {
            sf::Text sceneIndicator(font, "Scene " + std::to_string(introScene + 1) + " / 6", 16);
            sceneIndicator.setFillColor(sf::Color(150, 150, 150, std::min(150, static_cast<int>(fadeAlpha * 0.6f))));
            sceneIndicator.setPosition(sf::Vector2f(WIDTH - 120.f, HEIGHT - 25.f));
            window.draw(sceneIndicator);
        }
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
        
        float safeStartX = WIDTH / 2.f - 148.f + (currentMinDesire * 3.f);
        float safeEndX = WIDTH / 2.f - 148.f + (currentMaxDesire * 3.f);
        
        sf::RectangleShape safeMarkerLeft(sf::Vector2f(2.f, 26.f));
        safeMarkerLeft.setFillColor(sf::Color::White);
        safeMarkerLeft.setPosition(sf::Vector2f(safeStartX, 86.f));
        window.draw(safeMarkerLeft);
        
        sf::RectangleShape safeMarkerRight(sf::Vector2f(2.f, 26.f));
        safeMarkerRight.setFillColor(sf::Color::White);
        safeMarkerRight.setPosition(sf::Vector2f(safeEndX, 86.f));
        window.draw(safeMarkerRight);
        
        minDesireLabel.setString(std::to_string(currentMinDesire));
        maxDesireLabel.setString(std::to_string(currentMaxDesire));
        
        sf::FloatRect minBounds = minDesireLabel.getLocalBounds();
        sf::FloatRect maxBounds = maxDesireLabel.getLocalBounds();
        
        minDesireLabel.setPosition(sf::Vector2f(safeStartX - minBounds.size.x - 8.f, 85.f));
        maxDesireLabel.setPosition(sf::Vector2f(safeEndX + 8.f, 85.f));
        
        window.draw(minDesireLabel);
        window.draw(maxDesireLabel);
        
        // Draw speed increase notification
        if (speedIncreaseNotificationTimer > 0) {
            float alpha = 255.f;
            if (speedIncreaseNotificationTimer > 2.5f) {
                alpha = 255.f * (3.0f - speedIncreaseNotificationTimer) / 0.5f;
            } else if (speedIncreaseNotificationTimer < 0.5f) {
                alpha = 255.f * (speedIncreaseNotificationTimer / 0.5f);
            }
            
            float yOffset = rangeChangeNotificationTimer > 0 ? -50.f : 0.f;
            
            sf::Text notification(font, speedIncreaseMessage, 24);
            notification.setFillColor(sf::Color(255, 100, 100, static_cast<int>(alpha)));
            notification.setStyle(sf::Text::Bold);
            sf::FloatRect notifBounds = notification.getLocalBounds();
            notification.setPosition(sf::Vector2f(WIDTH / 2.f - notifBounds.size.x / 2.f, HEIGHT / 2.f - 100.f + yOffset));
            
            sf::Text notifShadow(font, speedIncreaseMessage, 24);
            notifShadow.setFillColor(sf::Color(0, 0, 0, static_cast<int>(alpha * 0.8f)));
            notifShadow.setStyle(sf::Text::Bold);
            notifShadow.setPosition(sf::Vector2f(WIDTH / 2.f - notifBounds.size.x / 2.f + 2.f, HEIGHT / 2.f - 98.f + yOffset));
            
            window.draw(notifShadow);
            window.draw(notification);
        }
        
        // Draw range change notification
        if (rangeChangeNotificationTimer > 0) {
            float alpha = 255.f;
            if (rangeChangeNotificationTimer > 2.5f) {
                alpha = 255.f * (3.0f - rangeChangeNotificationTimer) / 0.5f;
            } else if (rangeChangeNotificationTimer < 0.5f) {
                alpha = 255.f * (rangeChangeNotificationTimer / 0.5f);
            }
            
            sf::Text notification(font, rangeChangeMessage, 24);
            notification.setFillColor(sf::Color(255, 200, 0, static_cast<int>(alpha)));
            notification.setStyle(sf::Text::Bold);
            sf::FloatRect notifBounds = notification.getLocalBounds();
            notification.setPosition(sf::Vector2f(WIDTH / 2.f - notifBounds.size.x / 2.f, HEIGHT / 2.f - 100.f));
            
            sf::Text notifShadow(font, rangeChangeMessage, 24);
            notifShadow.setFillColor(sf::Color(0, 0, 0, static_cast<int>(alpha * 0.8f)));
            notifShadow.setStyle(sf::Text::Bold);
            notifShadow.setPosition(sf::Vector2f(WIDTH / 2.f - notifBounds.size.x / 2.f + 2.f, HEIGHT / 2.f - 98.f));
            
            window.draw(notifShadow);
            window.draw(notification);
        }
        
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
        sf::Text goldText(font, "Gold: +80 score, -15 desire", 16);
        goldText.setFillColor(sf::Color::White);
        goldText.setPosition(sf::Vector2f(legendStartX + spacing + 25.f, legendY - 2.f));
        window.draw(goldText);
        
        sf::CircleShape rottenCircle(10.f);
        rottenCircle.setFillColor(sf::Color(101, 67, 33));
        rottenCircle.setPosition(sf::Vector2f(legendStartX + spacing * 2, legendY));
        window.draw(rottenCircle);
        sf::Text rottenText(font, "Rotten: +5 score, +30 desire", 16);
        rottenText.setFillColor(sf::Color::White);
        rottenText.setPosition(sf::Vector2f(legendStartX + spacing * 2 + 25.f, legendY - 2.f));
        window.draw(rottenText);
        
        sf::Text missedText(font, "Missed: -5 desire", 16);
        missedText.setFillColor(sf::Color(255, 100, 100));
        missedText.setPosition(sf::Vector2f(legendStartX + spacing * 3 + 5.f, legendY - 2.f));
        window.draw(missedText);
    }

    void renderPauseMenu() {
        window.draw(pauseOverlay);
        window.draw(pauseMenu);
        window.draw(pauseTitle);
        
        if (hoveredButton == 1) {
            resumeButton.setFillColor(sf::Color(255, 60, 100)); // Brighter red apple
        } else {
            resumeButton.setFillColor(sf::Color(220, 20, 60)); // Red apple color
        }
        
        if (hoveredButton == 2) {
            restartButton.setFillColor(sf::Color(255, 235, 50)); // Brighter golden apple
        } else {
            restartButton.setFillColor(sf::Color(255, 215, 0)); // Golden apple color
        }
        
        if (hoveredButton == 3) {
            quitButton.setFillColor(sf::Color(131, 87, 53)); // Brighter rotten apple
        } else {
            quitButton.setFillColor(sf::Color(101, 67, 33)); // Rotten apple color
        }
        
        window.draw(resumeButton);
        window.draw(resumeText);
        window.draw(restartButton);
        window.draw(restartText);
        window.draw(quitButton);
        window.draw(quitText);
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
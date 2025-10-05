// File: src/main.cpp
#include <SFML/Graphics.hpp>

int main() {
    // Create render window
    sf::RenderWindow window(sf::VideoMode({800u, 600u}), "Apple of Balance");
    
    // Load textures
    sf::Texture appleTexture;
    if (!appleTexture.loadFromFile("assets/apple.png")) return -1;
    
    sf::Sprite apple(appleTexture);
    sf::Vector2f applePos{400.f, 0.f};
    float appleSpeed = 3.f;
    
    sf::Texture basketTexture;
    if (!basketTexture.loadFromFile("assets/basket.png")) return -1;
    
    sf::Sprite basket(basketTexture);
    basket.setPosition({400.f, 550.f});
    float basketSpeed = 5.f;
    
    // Main loop
    while (window.isOpen()) {
        // Poll events
        while (const std::optional event = window.pollEvent()) {
            if (event.has_value()) {
                if (const auto* closed = event->getIf<sf::Event::Closed>()) {
                    window.close();
                }
            }
        }
        
        // Basket movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            basket.move({-basketSpeed, 0.f});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            basket.move({basketSpeed, 0.f});
        
        // Apple movement
        applePos.y += appleSpeed;
        if (applePos.y > 600.f) applePos.y = 0.f;
        apple.setPosition(applePos);
        
        // Draw everything
        window.clear(sf::Color(240, 240, 240));
        window.draw(apple);
        window.draw(basket);
        window.display();
    }
    
    return 0;
}
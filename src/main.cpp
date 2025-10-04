#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

int main() {
    // Create window
    sf::RenderWindow window(sf::VideoMode(800, 600), "Apple of Balance 🍎");
    window.setFramerateLimit(60);

    // Load textures
    sf::Texture appleTexture;
    appleTexture.loadFromFile("assets/apple.png");
    sf::Sprite apple(appleTexture);
    float appleX = 400.f;
    float appleY = 0.f;
    float appleSpeed = 3.f;

    sf::Texture basketTexture;
    basketTexture.loadFromFile("assets/basket.png");
    sf::Sprite basket(basketTexture);
    basket.setPosition(400.f, 550.f);
    float basketSpeed = 5.f;

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Move basket
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            basket.move(-basketSpeed, 0.f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            basket.move(basketSpeed, 0.f);
        }

        // Move apple
        appleY += appleSpeed;
        if (appleY > 600) appleY = 0.f; // reset at bottom
        apple.setPosition(appleX, appleY);

        // Draw everything
        window.clear(sf::Color(240, 240, 240));
        window.draw(apple);
        window.draw(basket);
        window.display();
    }

    return 0;
}

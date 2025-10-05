#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

int main() {
    sf::RenderWindow window({800, 600}, "Apple of Balance 🍎");
    window.setFramerateLimit(60);

    sf::Texture appleTexture;
    if (!appleTexture.loadFromFile("assets/apple.png")) return -1;
    sf::Sprite apple(appleTexture);
    float appleX = 400.f, appleY = 0.f, appleSpeed = 3.f;

    sf::Texture basketTexture;
    if (!basketTexture.loadFromFile("assets/basket.png")) return -1;
    sf::Sprite basket(basketTexture);
    basket.setPosition(400.f, 550.f);
    float basketSpeed = 5.f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.is<sf::Event::Closed>())  // new SFML 3 event syntax
                window.close();
        }

        // Basket movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            basket.move(-basketSpeed, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            basket.move(basketSpeed, 0.f);

        // Apple movement
        appleY += appleSpeed;
        if (appleY > 600) appleY = 0.f;
        apple.setPosition(appleX, appleY);

        // Draw
        window.clear(sf::Color(240, 240, 240));
        window.draw(apple);
        window.draw(basket);
        window.display();
    }
}

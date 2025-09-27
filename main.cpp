#include "Hive.hpp"

int main() {
    const float radius = 40.f;
    const int rows = 5;
    const int cols = 5;

    sf::RenderWindow window(sf::VideoMode(800, 600), "Hive Grid");

    // Precompute spacing
    const float horizSpacing = 1.5f * radius;
    const float vertSpacing  = std::sqrt(3.f) * radius;

    // Store all hexagons in a vector
    std::vector<sf::CircleShape> hexes;
    hexes.reserve(rows * cols);

    for (int col = 0; col < cols; ++col) {
        for (int row = 0; row < rows; ++row) {
            sf::CircleShape hex(radius, 6);
            hex.setRotation(30);                  // flat-topped
            hex.setFillColor(sf::Color(200, 200, 255));
            hex.setOutlineThickness(-2);
            hex.setOutlineColor(sf::Color::Black);

            float x = col * horizSpacing + 100.f;
            float y = row * vertSpacing + 100.f;

            if (col % 2 == 1)
                y += vertSpacing / 2.f;  // stagger every other column

            hex.setPosition(x, y);
            hexes.push_back(hex);
        }
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);
        for (const auto& hex : hexes)
            window.draw(hex);
        window.display();
    }

    return 0;
}

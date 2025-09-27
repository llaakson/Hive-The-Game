#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>

int main() {
    const float radius = 40.f;
    const int rows = 5;
    const int cols = 5;
    float x = 0;
    float y = 0;

    sf::RenderWindow window(sf::VideoMode(800, 600), "Hive Grid");
     // Load font
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font.\n";
        return -1;
    }


    // Precompute spacing
    const float horizSpacing = 1.5f * radius;
    const float vertSpacing  = std::sqrt(3.f) * radius;

    // Store all hexagons in a vector
    std::vector<sf::CircleShape> hexes;
    std::vector<sf::Text> texts;
    hexes.reserve(rows * cols);

    for (int col = 0; col < cols; ++col) {
        for (int row = 0; row < rows; ++row) {
            sf::CircleShape hex(radius, 6);
            hex.setRotation(30);                  // flat-topped
            hex.setFillColor(sf::Color(200, 200, 255));
            hex.setOutlineThickness(-2);
            hex.setOutlineColor(sf::Color::Black);
	    
            x = col * horizSpacing + 100.f;
            y = row * vertSpacing + 100.f;

            if (col % 2 == 1)
                y += vertSpacing / 2.f;  // stagger every other column

            hex.setPosition(x, y);
            hexes.push_back(hex);
	     //text
	    sf::Text text;
	    text.setFont(font);
	    text.setString("E");
	    text.setCharacterSize(24);
	    text.setFillColor(sf::Color::Red);

	    // Center the text on the hex
            sf::FloatRect textBounds = text.getLocalBounds();
            text.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
            text.setPosition(x + radius, y + radius); // center of
	    texts.push_back(text);
        }
    }

    //Game pieces
    sf::RectangleShape rectangle({800.f, 800.f});
    rectangle.setSize({800.f, 800.f});
    rectangle.setPosition(x,y);
    rectangle.setFillColor(sf::Color(200,2,255));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        
        window.clear(sf::Color::White);
        for (size_t i = 0; i < hexes.size(); ++i) {
            window.draw(hexes[i]);
            window.draw(texts[i]);
        }
	window.draw(rectangle);
        window.display();
    }
				
    return 0;
}

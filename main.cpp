#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

int main() {
	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "SFML window", sf::Style::Fullscreen);
	auto windowWidth = window.getSize().x, windowHeight = window.getSize().y;
	int maxDepth = 1000;
	double x0 = 0., y0 = 0.;
	int imageHeight = 3., imageWidth = imageHeight * windowWidth / windowHeight;
	double imageCenterX = -0.75, imageCenterY = 0.;
	sf::Uint8* pixels = new sf::Uint8[windowWidth * windowHeight * 4];
	for (int px = 0; px < windowWidth; px++)
		for (int py = 0; py < windowHeight; py++) {
			double cx = ((double)px - 0.5 * windowWidth) / windowWidth * imageWidth + imageCenterX;
			double cy = ((double)py - 0.5 * windowHeight) / windowHeight * imageHeight + imageCenterY;
			int exitDepth = maxDepth;
			double xn = x0, yn = y0;
			for (int i = 0; i < maxDepth; i++) {
				double xtemp = xn*xn - yn*yn + cx;
				yn = 2*xn*yn + cy;
				xn = xtemp;
				if (xn*xn + yn*yn > 4) {
					exitDepth = i;
					break;
				}
			}
			int i = px + py * windowWidth;
			double r = 0., g = 0., b = 0.;
			if (exitDepth != maxDepth) {
				double hue = 360 * (double)exitDepth / maxDepth;
				double sat = 1., val = 1.;
				double c = val * sat;
				double x = c * (1 - abs(fmod(hue / 60, 2) - 1));
				double m = val - c;
				if (hue >= 0 && hue < 60) {r = c; g = x;}
				else if (hue >= 60 && hue < 120) {r = x; g = c;}
				else if (hue >= 120 && hue < 180) {g = c; b = x;}
				else if (hue >= 180 && hue < 240) {g = x; b = c;}
				else if (hue >= 240 && hue < 300) {r = x; b = c;}
				else if (hue >= 300 && hue < 360) {r = c; b = x;}
				r = 255 * (r + m);
				g = 255 * (g + m);
				b = 255 * (b + m);
			} 
			pixels[4 * i] = r;
			pixels[4 * i + 1] = g;
			pixels[4 * i + 2] = b;
			pixels[4 * i + 3] = 255;
		}
	sf::Image image;
	image.create(windowWidth, windowHeight, pixels);
	sf::Texture texture;
	texture.loadFromImage(image);
	sf::Sprite sprite(texture);
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) window.close();
		}
		window.clear();
		window.draw(sprite);
		window.display();
	}
	return 0;
}
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <complex>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

void hsv_rgb(double h, double s, double v, double& r, double& g, double& b) {
	double c = v * s;
	double x = c * (1 - abs(fmod(h / 60, 2) - 1));
	double m = v - c;
	r = g = b = 0;
	if (h >= 0 && h < 60) {r = c; g = x;}
	else if (h >= 60 && h < 120) {r = x; g = c;}
	else if (h >= 120 && h < 180) {g = c; b = x;}
	else if (h >= 180 && h < 240) {g = x; b = c;}
	else if (h >= 240 && h < 300) {r = x; b = c;}
	else if (h >= 300 && h < 360) {r = c; b = x;}
	r = 255 * (r + m);
	g = 255 * (g + m);
	b = 255 * (b + m);
}

int main() {
	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "SFML window", sf::Style::Fullscreen);
	window.setFramerateLimit(60);
	int windowWidth = window.getSize().x, windowHeight = window.getSize().y;
	int maxDepth = 1000;
	std::complex<double> z0(0., 0.);
	double imageHeight = 3., imageWidth = imageHeight * windowWidth / windowHeight;
	double imageCenterX = -0.75, imageCenterY = 0.;
	sf::Uint8* pixels = new sf::Uint8[windowWidth * windowHeight * 4];
	// multithreading
	int nThreads = std::thread::hardware_concurrency();
	std::vector<std::thread> threads(nThreads);
	for (int n = 0; n < nThreads; n++) threads[n] = std::thread([=, &pixels] {
		for (int px = n; px < windowWidth; px += nThreads)
			for (int py = 0; py < windowHeight; py++) {
				std::complex<double> c(((double)px - 0.5 * windowWidth) / windowWidth * imageWidth + imageCenterX,
									   ((double)py - 0.5 * windowHeight) / windowHeight * imageHeight + imageCenterY);
				int exitDepth = maxDepth;
				std::complex<double> zn = z0;
				for (int i = 0; i < maxDepth; i++) {
					zn = std::pow(zn, 2) + c;
					if (abs(zn) > 2) {
						exitDepth = i;
						break;
					}
				}
				int i = px + py * windowWidth;
				double h = powf(360 * (double)exitDepth / maxDepth, 2);
				double r = 0, g = 0, b = 0;
				if (exitDepth != maxDepth) hsv_rgb(h, 1., 1., r, g, b);
				pixels[4 * i] = (sf::Uint8)r;
				pixels[4 * i + 1] = (sf::Uint8)g;
				pixels[4 * i + 2] = (sf::Uint8)b;
				pixels[4 * i + 3] = 255;
			}
		});
	for (int n = 0; n < nThreads; n++) threads[n].detach();
	sf::Texture texture;
	texture.create(windowWidth, windowHeight);
	texture.update(pixels);
	sf::Sprite sprite(texture);
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) window.close();
		}
		texture.update(pixels);
		sf::Sprite sprite(texture);
		window.clear();
		window.draw(sprite);
		window.display();
	}
	return 0;
}
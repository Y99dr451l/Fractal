#include <vector>
#include <thread>
#include <complex>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

void hsv_rgb(double h, double s, double v, double& r, double& g, double& b) {
	double c = v * s;
	double x = c * (1 - abs(fmod(h / 60, 2) - 1));
	double m = v - c;
	r = g = b = 0;
	if (h < 60) {r = c; g = x;}
	else if (h < 120) {r = x; g = c;}
	else if (h < 180) {g = c; b = x;}
	else if (h < 240) {g = x; b = c;}
	else if (h < 300) {r = x; b = c;}
	else {r = c; b = x;}
	r = 255 * (r + m);
	g = 255 * (g + m);
	b = 255 * (b + m);
}

std::complex<double> power(std::complex<double> x, int n) {
	std::complex<double> ans;
	if (n == 0) return std::complex<double>(1., 0.);
	if (n == 1) return x;
	std::complex<double> sq = power(x, n / 2);
	if (n % 2 == 0) return sq * sq;
	return x * sq * sq;
}

struct windowInfo {
	windowInfo(sf::RenderWindow* window) {
		windowWidth = window->getSize().x;
		windowHeight = window->getSize().y;
		imageHeight = 3.; imageWidth = imageHeight * windowWidth / windowHeight;
		imageCenterX = -0.75; imageCenterY = 0.;
	}
	int windowWidth, windowHeight;
	double imageHeight, imageWidth, imageCenterX, imageCenterY;
};

struct drawInfo {
	drawInfo() {
		maxDepth = 1000; power = 2;
		z0 = std::complex<double>(0., 0.);
	}
	int maxDepth, power;
	std::complex<double> z0;
};

void draw(sf::Uint8* &pixels, windowInfo wi, drawInfo di) {
	int nThreads = std::thread::hardware_concurrency();
	std::vector<std::thread> threads(nThreads);
	for (int n = 0; n < nThreads; n++) threads[n] = std::thread([=, &pixels] {
		for (int px = n; px < wi.windowWidth; px += nThreads)
			for (int py = 0; py < wi.windowHeight; py++) {
				std::complex<double> c(((double)px - 0.5 * wi.windowWidth) / wi.windowWidth * wi.imageWidth + wi.imageCenterX,
					((double)py - 0.5 * wi.windowHeight) / wi.windowHeight * wi.imageHeight + wi.imageCenterY);
				int exitDepth = di.maxDepth;
				std::complex<double> zn = di.z0;
				for (int i = 0; i < di.maxDepth; i++) {
					zn = power(zn, di.power) + c;
					if (zn.real() * zn.real() + zn.imag() * zn.imag() > 4) {
						exitDepth = i; break;
					}
				}
				int i = px + py * wi.windowWidth;
				double h = powf(360 * (double)exitDepth / di.maxDepth, 2);
				double r = 0, g = 0, b = 0;
				if (exitDepth != di.maxDepth) hsv_rgb(h, 1., 1., r, g, b);
				pixels[4 * i] = (sf::Uint8)r;
				pixels[4 * i + 1] = (sf::Uint8)g;
				pixels[4 * i + 2] = (sf::Uint8)b;
				pixels[4 * i + 3] = 255;
			}
		});
	for (int n = 0; n < nThreads; n++) threads[n].detach();
}

int main() {
	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "SFML window");
	window.setFramerateLimit(60);
	windowInfo wi(&window); drawInfo di;
	sf::Uint8* pixels = new sf::Uint8[wi.windowWidth * wi.windowHeight * 4];
	draw(pixels, wi, di);
	sf::Texture texture;
	texture.create(wi.windowWidth, wi.windowHeight);

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
				window.close();
				return 0;
			}
			// zoom
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Add) {
				wi.imageWidth /= 1.12; wi.imageHeight /= 1.12;
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Subtract) {
				wi.imageWidth *= 1.12; wi.imageHeight *= 1.12;
			}
			// move
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Down) wi.imageCenterY += 0.1 * wi.imageHeight;
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Up) wi.imageCenterY -= 0.1 * wi.imageHeight;
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Left) wi.imageCenterX -= 0.1 * wi.imageWidth;
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Right) wi.imageCenterX += 0.1 * wi.imageWidth;
			// maxdepth
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::M) di.maxDepth *= 1.2;
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::N) di.maxDepth /= 1.2;
			// power
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P) di.power++;
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::O) di.power--;
			// reset
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
				wi = windowInfo(&window); di = drawInfo();
			}
			if (event.type == sf::Event::KeyPressed) draw(pixels, wi, di);
		}
		texture.update(pixels);
		sf::Sprite sprite(texture);
		window.clear();
		window.draw(sprite);
		window.display();
	}
	return 0;
}
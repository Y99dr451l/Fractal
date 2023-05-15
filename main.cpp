#include <vector>
#include <thread>
#include <complex>
#include <math.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#define M_PI 3.14159265358979323846

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

double power(double& x, int n) {
	if (n == 0) return 1.;
	if (n == 1) return x;
	double sq = power(x, n / 2);
	if (n % 2 == 0) return sq * sq;
	return x * sq * sq;
}

std::complex<double> power(std::complex<double>& x, int n) {
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
		maxDepth = 100; power = 2;
		z0 = std::complex<double>(0., 0.);
		julia = false;
	}
	int maxDepth, power;
	std::complex<double> z0;
	bool julia;
};

void draw(sf::Uint8* &pixels, windowInfo wi, drawInfo di) {
	int nThreads = std::thread::hardware_concurrency();
	static std::vector<std::thread> threads(nThreads);
	for (int i = 0; i < threads.size(); i++) threads[i].~thread();
	for (int n = 0; n < nThreads; n++) threads[n] = std::thread([=, &pixels] {
		for (int px = n; px < wi.windowWidth; px += nThreads)
			for (int py = 0; py < wi.windowHeight; py++) {
				std::complex<double> temp(((double)px - 0.5 * wi.windowWidth) / wi.windowWidth * wi.imageWidth + wi.imageCenterX,
					((double)py - 0.5 * wi.windowHeight) / wi.windowHeight * wi.imageHeight + wi.imageCenterY);
				std::complex<double> c, zn;
				if (di.julia) { c = di.z0; zn = temp; }
				else { c = temp; zn = di.z0; }
				int depth = di.maxDepth;
				for (depth = 0; depth < di.maxDepth; depth++) {
					zn = power(zn, di.power) + c;
					if (zn.real() * zn.real() + zn.imag() * zn.imag() > 4) break;
				}
				int i = px + py * wi.windowWidth;
				double h = 360 * (double)depth / di.maxDepth;
				double r = 0, g = 0, b = 0;
				if (depth != di.maxDepth) hsv_rgb(h, 1., 1., r, g, b);
				pixels[4 * i] = (sf::Uint8)r;
				pixels[4 * i + 1] = (sf::Uint8)g;
				pixels[4 * i + 2] = (sf::Uint8)b;
				pixels[4 * i + 3] = 255;
			}
		});
	for (int n = 0; n < nThreads; n++) threads[n].detach();
}

int main() {
	sf::ContextSettings settings; // settings.antialiasingLevel = 8;
	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "SFML window", sf::Style::Close, settings);
	window.setFramerateLimit(60);
	windowInfo wi(&window); drawInfo di;
	
	sf::Font font; sf::Text text, text2;
	font.loadFromFile("Lavinia.otf");
	text.setFont(font); text2.setFont(font);
	text.setCharacterSize(20); text2.setCharacterSize(20);
	text.setPosition(5, 5);
	text2.setPosition(5, 800);
	std::stringstream ss2;
	ss2 << "Move: Up Left Down Right\nMove z0: W A S D\nZoom: + -\nPower +/-: P O\nMaxDepth +/-: M N\nToggle julia: J\nReset: R" << "Coarseness +/-: LCtrl LShift (hold)";
	text2.setString(ss2.str());

	sf::Uint8* pixels = new sf::Uint8[wi.windowWidth * wi.windowHeight * 4];
	draw(pixels, wi, di);
	sf::Texture texture;
	texture.create(wi.windowWidth, wi.windowHeight);

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) { window.close(); return 0; }
			// zoom
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Add) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) { wi.imageWidth /= 1.05; wi.imageHeight /= 1.05; }
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) { wi.imageWidth /= 2.; wi.imageHeight /= 2.; }
				else { wi.imageWidth /= 1.12; wi.imageHeight /= 1.12; }
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Subtract) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) { wi.imageWidth *= 1.05; wi.imageHeight *= 1.05; }
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) { wi.imageWidth *= 2.; wi.imageHeight *= 2.; }
				else { wi.imageWidth *= 1.12; wi.imageHeight *= 1.12; }
			}
			// move
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Down) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) wi.imageCenterY += 0.001 * wi.imageHeight;
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) wi.imageCenterY += 0.1 * wi.imageHeight;
				else wi.imageCenterY += 0.01 * wi.imageHeight;
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Up) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) wi.imageCenterY -= 0.001 * wi.imageHeight;
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) wi.imageCenterY -= 0.1 * wi.imageHeight;
				else wi.imageCenterY -= 0.01 * wi.imageHeight;
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Left) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) wi.imageCenterX -= 0.001 * wi.imageHeight;
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) wi.imageCenterX -= 0.1 * wi.imageHeight;
				else wi.imageCenterX -= 0.01 * wi.imageHeight;
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Right) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) wi.imageCenterX += 0.001 * wi.imageHeight;
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) wi.imageCenterX += 0.1 * wi.imageHeight;
				else wi.imageCenterX += 0.01 * wi.imageHeight;
			}
			// move z0
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) di.z0 += std::complex<double>(0., -0.001);
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) di.z0 += std::complex<double>(0., -0.1);
				else di.z0 += std::complex<double>(0., -0.01);
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::W) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) di.z0 += std::complex<double>(0., 0.001);
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) di.z0 += std::complex<double>(0., 0.1);
				else di.z0 += std::complex<double>(0., 0.01);
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::A) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) di.z0 -= 0.001;
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) di.z0 -= 0.1;
				else di.z0 -= 0.01;
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::D) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) di.z0 += 0.001;
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) di.z0 += 0.1;
				else di.z0 += 0.01;
			}
			// maxdepth
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::M) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) di.maxDepth *= 1.2;
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) di.maxDepth *= 2;
				else di.maxDepth *= 1.05;
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::N) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) di.maxDepth /= 1.2;
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) di.maxDepth /= 2;
				else di.maxDepth /= 1.05;
			}
			// power
			/*if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) di.power += 1.1;
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) di.power += 5;
				else di.power++;
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::O) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) di.power -= 1.1;
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) di.power -= 5;
				else di.power--;
			}*/
			// reset
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) { wi = windowInfo(&window); di = drawInfo(); }
			// toggle mandelbrot/julia
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::J) di.julia = !di.julia;
			// resize
			/*if (event.type == sf::Event::Resized) {
				wi.windowWidth = event.size.width; wi.windowHeight = event.size.height;
				delete[] pixels; pixels = new sf::Uint8[wi.windowWidth * wi.windowHeight * 4];
				texture.create(wi.windowWidth, wi.windowHeight);
			}*/

			if (event.type == sf::Event::KeyPressed || event.type == sf::Event::Resized) draw(pixels, wi, di);
		}
		texture.update(pixels);
		sf::Sprite sprite(texture);
		window.clear();
		window.draw(sprite);
		// write parameters
		std::stringstream ss;
		ss << "Image: (" << wi.imageWidth << ", " << wi.imageHeight << ")"
			<< "\nImageCenter: (" << wi.imageCenterX << ", " << wi.imageCenterY << ")"
			<< "\nz0: " << di.z0 << "\nMaxDepth: " << di.maxDepth << "\nPower: " << di.power << "\njulia: " << di.julia;
		text.setString(ss.str());
		window.draw(text);
		window.draw(text2);
		window.display();
	}
	return 0;
}
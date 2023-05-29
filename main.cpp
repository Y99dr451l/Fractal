#include <vector>
#include <thread>
#include <complex>
#include <math.h>
#include <atomic>
#include <chrono>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "threadpool.h"

#define M_PI 3.14159265358979323846

static std::atomic<int> nWorkerComplete;

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
		julia = false, ui = true, help = true;
	}
	int maxDepth, power;
	std::complex<double> z0;
	bool julia, ui, help;
};


void draw(sf::Uint8* pixels, windowInfo* wi, drawInfo* di, int nThreads, int id) {
	for (int px = id; px < wi->windowWidth; px += nThreads)
		for (int py = 0; py < wi->windowHeight; py++) {
			std::complex<double> temp(((double)px - 0.5 * wi->windowWidth) / wi->windowWidth * wi->imageWidth + wi->imageCenterX,
				((double)py - 0.5 * wi->windowHeight) / wi->windowHeight * wi->imageHeight + wi->imageCenterY);
			std::complex<double> c, zn;
			if (di->julia) { c = di->z0; zn = temp; }
			else { c = temp; zn = di->z0; }
			int depth = di->maxDepth;
			for (depth = 0; depth < di->maxDepth; depth++) {
				zn = power(zn, di->power) + c;
				if (zn.real() * zn.real() + zn.imag() * zn.imag() > 4) break;
			}
			int i = px + py * wi->windowWidth;
			//pixels[i] = depth;
			double h = 360 * (double)depth / di->maxDepth;
			double r = 0, g = 0, b = 0;
			if (depth != di->maxDepth) hsv_rgb(h, 1., 1., r, g, b);
			pixels[4 * i] = (sf::Uint8)r;
			pixels[4 * i + 1] = (sf::Uint8)g;
			pixels[4 * i + 2] = (sf::Uint8)b;
			pixels[4 * i + 3] = 255;
		}
	nWorkerComplete++;
}

int main() {

	// Window
	sf::ContextSettings settings;
	settings.depthBits = 24; settings.stencilBits = 8; settings.antialiasingLevel = 4;
	sf::RenderWindow window(sf::VideoMode(1000, 1000), "Mandelbrot", sf::Style::Titlebar, settings);
	
	// Text
	sf::Font font;
	font.loadFromFile("Lavinia.otf");
	sf::Text text, text2;
	text.setFont(font); text2.setFont(font);
	text.setCharacterSize(20); text2.setCharacterSize(20);
	std::stringstream ss2;
	ss2 << "Move: Up Left Down Right\nMove z0: W A S D\nZoom: + -\nPower +/-: P O\nMaxDepth +/-: M N\nToggle julia: J\nReset: R\nCoarseness +/-: LCtrl LShift (hold)"
		<< "\nToggle UI: U\nToggle Help: H";
	text2.setString(ss2.str());
	text.setPosition(5, 5);
	text2.setPosition(5, window.getSize().y - text2.getGlobalBounds().height - 5);

	// Fractal
	windowInfo wi(&window); drawInfo di;
	sf::Uint8* pixels = new sf::Uint8[wi.windowWidth * wi.windowHeight * 4];
	sf::Texture texture; texture.create(wi.windowWidth, wi.windowHeight);

	// Threading
	int nThreads = std::thread::hardware_concurrency();
	ThreadPool pool(nThreads);
	
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
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P) {
				//if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) di.power += 1.1;
				//else 
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) di.power += 5;
				else di.power++;
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::O) {
				//if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) di.power -= 1.1;
				//else 
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) di.power -= 5;
				else di.power--;
			}
			// reset
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) { wi = windowInfo(&window); di = drawInfo(); }
			// toggle mandelbrot/julia
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::J) di.julia = !di.julia;
			// toggle ui
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::I) di.ui = !di.ui;
			// toggle help
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::H) di.help = !di.help;
			// resize
			/*if (event.type == sf::Event::Resized) {
				wi.windowWidth = event.size.width; wi.windowHeight = event.size.height;
				delete[] pixels; pixels = new sf::Uint8[wi.windowWidth * wi.windowHeight * 4];
				texture.create(wi.windowWidth, wi.windowHeight);
			}*/

			if (event.type == sf::Event::KeyPressed || event.type == sf::Event::Resized) {
				nWorkerComplete = 0;
				auto start = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < nThreads; ++i) pool.enqueue([&, i] {draw(pixels, &wi, &di, nThreads, i); });
				while (nWorkerComplete < nThreads) {}
				auto end = std::chrono::high_resolution_clock::now();
				texture.update(pixels);
				sf::Sprite sprite(texture);
				window.clear();
				window.draw(sprite);

				if (di.ui) {
					std::stringstream ss;
					ss << "Image: (" << wi.imageWidth << ", " << wi.imageHeight << ")"
						<< "\nImageCenter: (" << wi.imageCenterX << ", " << wi.imageCenterY << ")"
						<< "\nz0: " << di.z0 << "\nMaxDepth: " << di.maxDepth << "\nPower: " << di.power << "\njulia: " << di.julia
						<< "\nFrametime: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.;
					text.setString(ss.str());
					sf::RectangleShape rect(sf::Vector2f(text.getLocalBounds().width + 20, text.getLocalBounds().height + 20));
					rect.setFillColor(sf::Color(0, 0, 0, 128));
					rect.setPosition(text.getPosition() - sf::Vector2f(10, 10));
					window.draw(rect);
					window.draw(text);
				}

				if (di.help) {
					sf::RectangleShape rect2(sf::Vector2f(text2.getLocalBounds().width + 20, text2.getLocalBounds().height + 20));
					rect2.setFillColor(sf::Color(0, 0, 0, 128));
					rect2.setPosition(text2.getPosition() - sf::Vector2f(10, 10));
					window.draw(rect2);
					window.draw(text2);
				}
				window.display();
			}
		}
		
	}
	return 0;
}
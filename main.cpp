#include <vector>
#include <thread>
#include <complex>
#include <math.h>
#include <atomic>
#include <chrono>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "utils.h"
#include "threadpool.h"
#include "fractal.h"
#include "colour.h"

struct Renderer {
	Renderer(int nThreads = std::thread::hardware_concurrency()) : nThreads(nThreads) {
		sf::VideoMode desktop = sf::VideoMode::getDesktopMode(); desktop.width *= 3. / 4; desktop.height *= 3. / 4;
		sf::ContextSettings settings; settings.depthBits = 24; settings.stencilBits = 8; settings.antialiasingLevel = 0;
		window.create(desktop, "FractalRenderer", sf::Style::Titlebar, settings); reset();
		font.loadFromFile("Lavinia.otf");
		infoText.setFont(font); helpText.setFont(font); titleText.setFont(font); colourText.setFont(font);
		infoText.setCharacterSize(15); helpText.setCharacterSize(15); titleText.setCharacterSize(30); colourText.setCharacterSize(15);
		std::stringstream ss2;
		ss2 << "Move: Up Left Down Right\nMove z0 : W A S D" << "\nZoom : + -"
			<< "\nPower: P O\nMaxDepth: M N" << "\nCoarseness (hold): LCtrl LShift(hold)"
			<< "\nToggle Info: I\nToggle Help: H" << "\nReset : R" << "\nChange fractal: 1 2 3 (x2 julia)\nChange colour: 4 5";
		helpText.setString(ss2.str());
		infoText.setPosition(5, 5);
		helpText.setPosition(5, window.getSize().y - helpText.getGlobalBounds().height - 5);

		infoRect.setFillColor(sf::Color(0, 0, 0, 128));
		helpRect.setFillColor(sf::Color(0, 0, 0, 128));
		titleRect.setFillColor(sf::Color(0, 0, 0, 128));
		infoRect.setPosition(infoText.getPosition() - sf::Vector2f(10, 10));
		helpRect.setPosition(helpText.getPosition() - sf::Vector2f(10, 10));
		helpRect.setSize(sf::Vector2f(helpText.getLocalBounds().width + 20, helpText.getLocalBounds().height + 20));

		pixels = new sf::Uint8[windowWidth * windowHeight * 4];
		texture.create(windowWidth, windowHeight);
		pool = new ThreadPool(nThreads);
	}

	sf::RenderWindow window;
	sf::Uint8* pixels;
	sf::Texture texture;
	sf::Text infoText, helpText, titleText, colourText;
	sf::RectangleShape infoRect, helpRect, titleRect;
	sf::Font font;

	int windowWidth, windowHeight;
	double imageHeight, imageWidth, imageCenterX, imageCenterY;
	int maxDepth, power;
	std::complex<double> z0;
	bool ui, help;

	Fractal* fractal = new Mandelbrot();
	Colour* colour = new Linear();

	int nThreads;
	ThreadPool* pool;
	std::atomic<int> nWorkerComplete;

	double moveModifier, zoomModifier, depthModifier;
	int powerModifier;

	void reset() {
		windowWidth = window.getSize().x;
		windowHeight = window.getSize().y;
		if (windowWidth < windowHeight) { imageWidth = 3.; imageHeight = imageWidth * windowHeight / windowWidth; }
		else { imageHeight = 3.; imageWidth = imageHeight * windowWidth / windowHeight; }
		imageCenterX = -0.75; imageCenterY = 0.;
		maxDepth = 100; power = 2;
		z0 = std::complex<double>(0., 0.);
		ui = true, help = true;
	}

	void zoom(double zoomModifier) { imageHeight *= zoomModifier; imageWidth *= zoomModifier; };
	void moveX(double moveModifier) { imageCenterX += moveModifier * imageWidth; };
	void moveY(double moveModifier) { imageCenterY += moveModifier * imageHeight; };
	void move(double moveModifierX, double moveModifierY) { moveX(moveModifierX); moveY(moveModifierY); };
	void moveZ0(double moveModifierX, double moveModifierY) { z0 += std::complex<double>(moveModifierX, moveModifierY); };

	void draw(int nThreads, int id) {
		for (int px = id; px < windowWidth; px += nThreads)
			for (int py = 0; py < windowHeight; py++) {
				std::complex<double> temp(((double)px - 0.5 * windowWidth) / windowWidth * imageWidth + imageCenterX,
										  ((double)py - 0.5 * windowHeight) / windowHeight * imageHeight + imageCenterY);
				int depth = (*fractal)(temp, z0, maxDepth, power);
				int i = px + py * windowWidth;
				double r = 0, g = 0, b = 0;
				(*colour)(depth, maxDepth, r, g, b);
				pixels[4 * i] = (sf::Uint8)r;
				pixels[4 * i + 1] = (sf::Uint8)g;
				pixels[4 * i + 2] = (sf::Uint8)b;
				pixels[4 * i + 3] = 255;
			}
		nWorkerComplete++;
	}

	void run() {
		while (window.isOpen()) {
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::Closed || (event.key.code == sf::Keyboard::Escape)) { window.close(); return; }

				if (event.key.shift) { moveModifier = 0.001; zoomModifier = 1.05; depthModifier = 1.05; powerModifier = 1; }
				else if (event.key.control) { moveModifier = 0.1; zoomModifier = 2.; depthModifier = 2.; powerModifier = 5; }
				else { moveModifier = 0.01, zoomModifier = 1.12, depthModifier = 1.2; int powerModifier = 1; }

				KEY_ACTION(Add, zoom(1. / zoomModifier);) KEY_ACTION(Subtract, zoom(zoomModifier);) // zoom
				KEY_ACTION(Left, move(-moveModifier, 0.);) KEY_ACTION(Right, move(moveModifier, 0.);) // move
				KEY_ACTION(Up, move(0., -moveModifier);) KEY_ACTION(Down, move(0., moveModifier);)
				KEY_ACTION(S, moveZ0(0., -moveModifier);) KEY_ACTION(W, moveZ0(0., moveModifier);) // move z0
				KEY_ACTION(A, moveZ0(-moveModifier, 0.);) KEY_ACTION(D, moveZ0(moveModifier, 0.);)
				KEY_ACTION(M, maxDepth *= depthModifier;) KEY_ACTION(N, if (int(maxDepth / depthModifier < 1)) break; maxDepth /= depthModifier;) // maxdepth
				KEY_ACTION(P, power += powerModifier;) KEY_ACTION(O, power -= powerModifier;) // power
				KEY_ACTION(I, ui = !ui;) KEY_ACTION(H, help = !help;) // UI
				KEY_ACTION(Num1, if (fractal->str() == "Mandelbrot") fractal = new MandelbrotJulia(); else fractal = new Mandelbrot();) // fractals
				KEY_ACTION(Num2, if (fractal->str() == "Burning Ship") fractal = new BurningShipJulia(); else fractal = new BurningShip();)
				KEY_ACTION(Num3, if (fractal->str() == "Tricorn") fractal = new TricornJulia(); else fractal = new Tricorn();)
				KEY_ACTION(Num4, colour = new Linear();) KEY_ACTION(Num5, colour = new Sine();) // colours
				KEY_ACTION(R, reset();) // reset

				if (event.type == sf::Event::KeyPressed || event.type == sf::Event::Resized) {
					nWorkerComplete = 0;
					auto start = std::chrono::high_resolution_clock::now();
					for (int i = 0; i < nThreads; ++i) pool->enqueue([&, i] {draw(nThreads, i); });
					while (nWorkerComplete < nThreads) {}
					auto end = std::chrono::high_resolution_clock::now();
					texture.update(pixels); sf::Sprite sprite(texture);
					window.clear(); window.draw(sprite);
					
					if (ui) {
						std::stringstream ss;
						ss << "Image: (" << imageWidth << ", " << imageHeight << ")"
							<< "\nImageCenter: (" << imageCenterX << ", " << imageCenterY << ")"
							<< "\nz0: " << z0 << "\nMaxDepth: " << maxDepth << "\nPower: " << power
							<< "\nFrametime: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1e6;
						infoText.setString(ss.str());
						titleText.setString(fractal->str());
						colourText.setString(colour->str());
						titleText.setPosition(window.getSize().x - titleText.getGlobalBounds().width - 5, 5);
						colourText.setPosition(window.getSize().x - colourText.getGlobalBounds().width - 5, 5 + titleText.getGlobalBounds().height + 5);
						titleRect.setPosition(titleText.getPosition() - sf::Vector2f(10, 10));
						infoRect.setSize(sf::Vector2f(infoText.getLocalBounds().width + 20, infoText.getLocalBounds().height + 20));
						// titleRect.setSize(sf::Vector2f(titleText.getLocalBounds().width + 20, titleText.getLocalBounds().height + 20));
						// titleRect covers titleText and ColourText
						titleRect.setSize(sf::Vector2f(std::max(titleText.getLocalBounds().width, colourText.getLocalBounds().width) + 20,
																				  titleText.getLocalBounds().height + colourText.getLocalBounds().height + 30));
						window.draw(infoRect); window.draw(titleRect);
						window.draw(infoText); window.draw(titleText); window.draw(colourText);
					}

					if (help) { window.draw(helpRect); window.draw(helpText); }
					window.display();
				}
			}
		}
	}
};

int main() {
	Renderer fractal;
	fractal.run();
	return 0;
}
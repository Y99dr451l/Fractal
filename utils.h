#pragma once

#define KEY_ACTION(KEY, CODE) if ( event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::KEY) { CODE }

void hsv_rgb(double h, double s, double v, double& r, double& g, double& b) {
	double c = v * s;
	double x = c * (1 - abs(fmod(h / 60, 2) - 1));
	double m = v - c;
	r = g = b = 0;
	if (h < 60) { r = c; g = x; }
	else if (h < 120) { r = x; g = c; }
	else if (h < 180) { g = c; b = x; }
	else if (h < 240) { g = x; b = c; }
	else if (h < 300) { r = x; b = c; }
	else { r = c; b = x; }
	r = 255 * (r + m);
	g = 255 * (g + m);
	b = 255 * (b + m);
}

double pow(double& x, int n) {
	if (n == 0) return 1.;
	if (n == 1) return x;
	double sq = pow(x, n / 2);
	if (n % 2 == 0) return sq * sq;
	return x * sq * sq;
}

std::complex<double> pow(std::complex<double>& x, int n) {
	if (n == 0) return std::complex<double>(1., 0.);
	if (n == 1) return x;
	std::complex<double> sq = pow(x, n / 2);
	if (n % 2 == 0) return sq * sq;
	return x * sq * sq;
}
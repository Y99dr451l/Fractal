#pragma once

#include <complex>
#include "utils.h"

struct Fractal {
    virtual int operator()(const std::complex<double>& xy, const std::complex<double>& z0, int maxDepth, int power) const = 0;
	virtual std::string str() const = 0;
};

struct Mandelbrot : public Fractal {
	int operator()(const std::complex<double>& xy, const std::complex<double>& z0, int maxDepth, int power) const override {
		std::complex<double> c = xy, zn = z0;
		int depth = maxDepth;
		for (depth = 0; depth < maxDepth; depth++) {
			zn = pow(zn, power) + c;
			if (zn.real() * zn.real() + zn.imag() * zn.imag() > 4) break;
		}
		return depth;
	}
	std::string str() const override { return "Mandelbrot"; }
};

struct MandelbrotJulia : public Fractal {
	std::complex<double> c;
	int operator()(const std::complex<double>& xy, const std::complex<double>& z0, int maxDepth, int power) const override {
		std::complex<double> c = z0, zn = xy;
		int depth = maxDepth;
		for (depth = 0; depth < maxDepth; depth++) {
			zn = pow(zn, power) + c;
			if (zn.real() * zn.real() + zn.imag() * zn.imag() > 4) break;
		}
		return depth;
	}
	std::string str() const override { return "Mandelbrot (Julia)"; }
};

struct BurningShip : public Fractal {
	int operator()(const std::complex<double>& xy, const std::complex<double>& z0, int maxDepth, int power) const override {
		std::complex<double> c = xy, zn = z0;
		int depth = maxDepth;
		for (depth = 0; depth < maxDepth; depth++) {
			zn = std::complex<double>(abs(zn.real()), abs(zn.imag()));
			zn = pow(zn, power) + c;
			if (zn.real() * zn.real() + zn.imag() * zn.imag() > 4) break;
		}
		return depth;
	}
	std::string str() const override { return "Burning Ship"; }
};

struct BurningShipJulia : public Fractal {
	std::complex<double> c;
	int operator()(const std::complex<double>& xy, const std::complex<double>& z0, int maxDepth, int power) const override {
		std::complex<double> c = z0, zn = xy;
		int depth = maxDepth;
		for (depth = 0; depth < maxDepth; depth++) {
			zn = std::complex<double>(abs(zn.real()), abs(zn.imag()));
			zn = pow(zn, power) + c;
			if (zn.real() * zn.real() + zn.imag() * zn.imag() > 4) break;
		}
		return depth;
	}
	std::string str() const override { return "Burning Ship (Julia)"; }
};

struct Tricorn : public Fractal {
	int operator()(const std::complex<double>& xy, const std::complex<double>& z0, int maxDepth, int power) const override {
		std::complex<double> c = xy, zn = z0;
		int depth = maxDepth;
		for (depth = 0; depth < maxDepth; depth++) {
			zn = std::complex<double>(zn.real(), -zn.imag());
			zn = pow(zn, power) + c;
			if (zn.real() * zn.real() + zn.imag() * zn.imag() > 4) break;
		}
		return depth;
	}
	std::string str() const override { return "Tricorn"; }
};

struct TricornJulia : public Fractal {
	std::complex<double> c;
	int operator()(const std::complex<double>& xy, const std::complex<double>& z0, int maxDepth, int power) const override {
		std::complex<double> c = z0, zn = xy;
		int depth = maxDepth;
		for (depth = 0; depth < maxDepth; depth++) {
			zn = std::complex<double>(zn.real(), -zn.imag());
			zn = pow(zn, power) + c;
			if (zn.real() * zn.real() + zn.imag() * zn.imag() > 4) break;
		}
		return depth;
	}
	std::string str() const override { return "Tricorn (Julia)"; }
};
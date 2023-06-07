#pragma once

#include<math.h>

#include "utils.h"

#define M_PI 3.14159265358979323846f

struct Colour {
    virtual void operator()(int depth, int maxDepth, double& r, double& g, double& b) const = 0;
	virtual std::string str() const = 0;
};

struct Linear : public Colour {
	void operator()(int depth, int maxDepth, double& r, double& g, double& b) const override {
		double h = 360. * depth / maxDepth;
		double s = 1;
		double v = depth < maxDepth ? 1 : 0;
		hsv_rgb(h, s, v, r, g, b);
	}
	std::string str() const override { return "Linear"; }
};

struct Sine : public Colour {
	void operator()(int depth, int maxDepth, double& r, double& g, double& b) const override {
		double h = 360. * sinf(float(depth) / maxDepth * M_PI / 2);
		double s = 1;
		double v = depth < maxDepth ? 1 : 0;
		hsv_rgb(h, s, v, r, g, b);
	}
	std::string str() const override { return "Sine"; }
};
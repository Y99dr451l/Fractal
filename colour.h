#pragma once

#include<math.h>

#include "utils.h"

struct Colour {
    virtual void operator()(int depth, int maxDepth, double& r, double& g, double& b) const = 0;
};

struct HSV : public Colour {
	void operator()(int depth, int maxDepth, double& r, double& g, double& b) const override {
		double h = 360. * depth / maxDepth;
		double s = 1;
		double v = depth < maxDepth ? 1 : 0;
		hsv_rgb(h, s, v, r, g, b);
	}
};

struct Smooth : public Colour {
	void operator()(int depth, int maxDepth, double& r, double& g, double& b) const override {
		double h = 360. * depth / maxDepth;
		double s = 1;
		double v = depth < maxDepth ? 1 : 0;
		if (depth < maxDepth) {
			double zn = sqrt(depth);
			double nu = log(log(zn) / log(2)) / log(2);
			double mu = depth + 1 - nu;
			double r0, g0, b0;
			hsv_rgb(h, s, v, r0, g0, b0);
			hsv_rgb(h + 1, s, v, r, g, b);
			r = r0 + (r - r0) * mu;
			g = g0 + (g - g0) * mu;
			b = b0 + (b - b0) * mu;
		}
	}
};
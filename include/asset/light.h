#pragma once

#include "material.h"

class diffuse_light : public material{
public:
    diffuse_light(shared_ptr<texture> a) : emit(a) {}
    diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

    color emitted(double u, double v, const point3 &p) const override {
        return emit->value(u, v, p);
    }

    bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const override {
        return false;
    }

public:
    shared_ptr<texture> emit;
};
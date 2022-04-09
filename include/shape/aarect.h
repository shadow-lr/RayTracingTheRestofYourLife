#pragma once

#include "geometry/hittable.h"
#include "rtweekend.h"

class xy_rect : public hittable {
public:
    xy_rect() {}

    xy_rect(double x0_, double x1_, double y0_, double y1_, double k_, shared_ptr<material> mat)
            : x0(x0_), x1(x1_), y0(y0_), y1(y1_), k(k_), mp(mat) {}

    virtual bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const override {
        auto t = (k - r.orig.z()) / r.direction().z();

        if (t < t_min || t > t_max)
            return false;

        // hit point
        double hit_x = r.origin().x() + t * r.direction().x();
        double hit_y = r.origin().y() + t * r.direction().y();

        if (hit_x < x0 || hit_x > x1 || hit_y < y0 || hit_y > y1)
            return false;

        rec.u = (hit_x - x0) / (x1 - x0);
        rec.v = (hit_y - y0) / (y1 - y0);
        rec.t = t;
        // todo: may cause some error(z )
        auto outward_normal = vec3(0, 0, 1);
        rec.set_face_normal(r, outward_normal);
        rec.mat_ptr = mp;
        rec.p = r.at(t);
        return true;
    }

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override {
        // The bounding box must have non-zero width in each dimension, so pad the Z
        // dimension a small amount.

        output_box = aabb(point3(x0, y0, k - 0.0001), point3(x1, y1, k + 0.0001));
        return true;
    }

public:
    shared_ptr<material> mp;
    // z = k
    double x0{}, x1{}, y0{}, y1{}, k{};
};

class xz_rect : public hittable {
public:
    xz_rect() {}

    xz_rect(double x0_, double x1_, double z0_, double z1_, double k_, shared_ptr<material> mat)
            : x0(x0_), x1(x1_), z0(z0_), z1(z1_), k(k_), mp(mat) {}

    virtual bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const override {
        auto t = (k - r.origin().y()) / r.direction().y();
        if (t < t_min || t > t_max)
            return false;
        auto x = r.origin().x() + t * r.direction().x();
        auto z = r.origin().z() + t * r.direction().z();
        if (x < x0 || x > x1 || z < z0 || z > z1)
            return false;
        rec.u = (x - x0) / (x1 - x0);
        rec.v = (z - z0) / (z1 - z0);
        rec.t = t;
        auto outward_normal = vec3(0, 1, 0);
        rec.set_face_normal(r, outward_normal);
        rec.mat_ptr = mp;
        rec.p = r.at(t);
        return true;
    }

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override {
        // The bounding box must have non-zero width in each dimension, so pad the Y
        // dimension a small amount.
        output_box = aabb(point3(x0, k - 0.0001, z0), point3(x1, k + 0.0001, z1));
        return true;
    }

public:
    shared_ptr<material> mp;
    double x0, x1, z0, z1, k;
};

class yz_rect : public hittable {
public:
    yz_rect() {}

    yz_rect(double y0_, double y1_, double z0_, double z1_, double k_, shared_ptr<material> mat)
            : y0(y0_), y1(y1_), z0(z0_), z1(z1_), k(k_), mp(mat) {}

    virtual bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const override {
        auto t = (k - r.origin().x()) / r.direction().x();
        if (t < t_min || t > t_max)
            return false;
        auto y = r.origin().y() + t * r.direction().y();
        auto z = r.origin().z() + t * r.direction().z();
        if (y < y0 || y > y1 || z < z0 || z > z1)
            return false;
        rec.u = (y - y0) / (y1 - y0);
        rec.v = (z - z0) / (z1 - z0);
        rec.t = t;
        auto outward_normal = vec3(1, 0, 0);
        rec.set_face_normal(r, outward_normal);
        rec.mat_ptr = mp;
        rec.p = r.at(t);
        return true;
    }

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override {
        // The bounding box must have non-zero width in each dimension, so pad the X
        // dimension a small amount.
        output_box = aabb(point3(k - 0.0001, y0, z0), point3(k + 0.0001, y1, z1));
        return true;
    }

public:
    shared_ptr<material> mp;
    double y0, y1, z0, z1, k;
};

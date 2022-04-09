#pragma once

#include "geometry/hittable.h"
#include "math/vec3.h"
#include "geometry/aabb.h"

class sphere : public hittable {
public:
    sphere() = default;

    sphere(point3 cen, double r, shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) {};

    virtual bool hit(
            const ray &r, double t_min, double t_max, hit_record &rec) const override;

    // 正方体AABB包围盒 包围 球
    virtual bool bounding_box(double time0, double time1, aabb &output_box) const;

    // 更新 u v的值
    static void get_sphere_uv(const point3 &p, double &u, double &v) {
        double theta = acos(-p.y());
        double phi = atan2(-p.z(), p.x()) + PI;

        u = phi / (2 * PI);
        v = theta / PI;
    }

public:
    point3 center;
    double radius;
    shared_ptr<material> mat_ptr;
};



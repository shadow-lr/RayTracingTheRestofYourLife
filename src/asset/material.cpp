#include "asset/material.h"

// 漫反射
bool lambertian::scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const {
	// 散射的向量
	vec3 scatter_direction = rec.normal + random_unit_vector();
	//        vec3 scatter_direction = rec.normal + random_in_unit_sphere();
	//        vec3 scatter_direction = rec.normal + random_in_hemisphere(rec.normal);

	// Catch degenerate scatter direction
	if (scatter_direction.near_zero())
		scatter_direction = rec.normal;

	scattered = ray(rec.p, scatter_direction, r_in.time());
	attenuation = albedo->value(rec.u, rec.v, rec.p);
	return true;
}

// 金属
bool metal::scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const {
    vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
//        scattered = ray(rec.p, reflected);
// 扰动 fuzz * random_unit_sphere
    scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
    attenuation = albedo;
    return (dot(scattered.direction(), rec.normal) > 0);
}

// 电介质
bool dielectric::scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const {
    attenuation = color(1.0, 1.0, 1.0);
    double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

    // 先根据法线和入射向量计算出角度的三角函数值 然后再根据公式 n1 / n2 * sin(θ) > 1 大于1就全反射，反之 散射
    vec3 unit_direction = unit_vector(r_in.direction());
    vec3 refracted = refract(unit_direction, rec.normal, refraction_ratio);

    // new content
    // dot(v, n) = cos(θ)
    double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
    double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    vec3 direction;

    // 真正的玻璃具有与角度不同的反射率————从陡峭的角度看窗户，它就会变成一面镜子
    if (refraction_ratio * sin_theta > 1.0 || reflectance(cos_theta, refraction_ratio) > random_double()) {
        // Must Reflect 必定反射 直接执行反射公式
        direction = reflect(unit_direction, rec.normal);
    } else {
        // Can refract 可以散射 走折射公式
        direction = refract(unit_direction, rec.normal, refraction_ratio);
    }

    scattered = ray(rec.p, direction, r_in.time());
    return true;
}
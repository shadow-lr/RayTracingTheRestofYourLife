#include "geometry/ray.h"
#include "math/vec3.h"
#include "rtweekend.h"
#include "color.h"

#include "geometry/hittable_list.h"
#include "geometry/translate.h"
#include "geometry/rotate.h"
#include "geometry/bvh.h"
#include "geometry/pdf.h"

#include "shape/sphere.h"
#include "shape/aarect.h"
#include "shape/box.h"
#include "shape/moving_sphere.h"
#include "shape/constant_medium.h"

#include "asset/material.h"
#include "asset/camera.h"
#include "asset/noise_texture.h"
#include "asset/image_texture.h"
#include "asset/light.h"

#include <omp.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <chrono>

// Image
double aspect_ratio = 16.0 / 9.0;
int image_width = 400;
int image_height = static_cast<int>(image_width / aspect_ratio);
const int max_depth = 50;

// World
hittable_list world;

camera cam;

// 225 600
std::vector<std::vector<color>> color_table(image_height + 1, std::vector<color>(image_width + 1));

// ray recursion
color ray_color(const ray &r, const color &background, const hittable &world, shared_ptr<hittable> &lights, int depth) {
	hit_record rec;

	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return color(0, 0, 0);

	if (!world.hit(r, 0.001, infinity, rec))
		return background;

//	double light_area = (343 - 213) * (332 - 227);
//	// TODO: hard code
//	auto light_cosine = std::fabs(to_light.y());
//	if (light_cosine < 0.000001)
//		return emitted;
//
//	pdf = distance_squared / (light_cosine * light_area);
//	scattered = ray(rec.p, to_light, r.time());

//	hittable_pdf light_pdf(lights, rec.p);
//	scattered = ray(rec.p, light_pdf.generate(), r.time());
//	pdf = light_pdf.value(scattered.direction());

//	cosine_pdf p(rec.normal);
//	scattered = ray(rec.p, p.generate(), r.time());
//	pdf = p.value(scattered.direction());

//	auto p0 = make_shared<hittable_pdf>(lights, rec.p);
//	auto p1 = make_shared<cosine_pdf>(rec.normal);
//
//	mixture_pdf mix_pdf(p0, p1);
//	scattered = ray(rec.p, mix_pdf.generate(), r.time());
//	pdf = mix_pdf.value(scattered.direction());
//
	scatter_record srec;
	color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
	if (!rec.mat_ptr->scatter(r, rec, srec))
		return emitted;

	if (srec.is_specular) {
		return srec.attenuation
			* ray_color(srec.specular_ray, background, world, lights, depth-1);
	}

	auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
	mixture_pdf p(light_ptr, srec.pdf_ptr);

	ray scattered = ray(rec.p, p.generate(), r.time());
	auto pdf_val = p.value(scattered.direction());

	// Monte-Carlo BRDF
	return emitted
		+ srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered)
			* ray_color(scattered, background, world, lights, depth - 1) / pdf_val;
}

void scan_calculate_color(int height,
						  int width,
						  color &background,
						  int samples_per_pixel,
						  shared_ptr<hittable> lights) {
	int i = width, j = height;
	color pixel_color(0, 0, 0);
	for (int s = 0; s < samples_per_pixel; ++s) {
		double u = (i + random_double()) / (image_width - 1.0);
		double v = (j + random_double()) / (image_height - 1.0);
		ray r = cam.get_ray(u, v);
		pixel_color += ray_color(r, background, world, lights, max_depth);
	}
	//    write_color(std::cout, pixel_color, samples_per_pixel);
	write_color_table(pixel_color, samples_per_pixel, color_table, j, i);
}

hittable_list cornell_box() {
	hittable_list objects;

	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));

//    auto baseColor = vec3(.34299999, .54779997, .22700010);
	auto baseColor = vec3(1.0);
	auto light = make_shared<diffuse_light>(baseColor * color(15, 15, 15));

	// box
	shared_ptr<material> aluminum = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
	shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), aluminum);
	box1 = make_shared<rotate_y>(box1, 15);
	box1 = make_shared<translate>(box1, vec3(265, 0, 295));
	objects.add(box1);

	auto glass = make_shared<dielectric>(1.5);
	objects.add(make_shared<sphere>(point3(190,90,190), 90 , glass));

	// wall
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
	objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light)));
//    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
	objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

	return objects;
}

int main() {
	// default Camera
	point3 lookfrom;
	point3 lookat;
	vec3 vup(0, 1, 0);
	double dist_to_focus = 10.0;
	double vfov = 40.0;
	double aperture = 0.0;

	int samples_per_pixel = 10;

	color background(0, 0, 0);

	int option = 7;

	auto lights = make_shared<hittable_list>();
//	lights->add(make_shared<xz_rect>(213, 343, 227, 332, 554, shared_ptr<material>()));
	lights->add(make_shared<sphere>(point3(190, 90, 190), 90, shared_ptr<material>()));

	switch (option) {
		case 7:world = cornell_box();
			aspect_ratio = 1.0;
			image_width = 300;
			samples_per_pixel = 100;
			background = color(0, 0, 0);
			lookfrom = point3(278, 278, -800);
			lookat = point3(278, 278, 0);
			vfov = 40.0;
			break;
	}

	cam.reset(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);
	image_height = static_cast<int>(image_width / aspect_ratio);
	color_table.resize(image_height + 1);
	for (auto &col : color_table) {
		col.resize(image_width + 1);
	}

	// Render
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	omp_set_num_threads(32);

	int finish = image_width - 1;
	static omp_lock_t lock;
	omp_init_lock(&lock);

	const auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel for
	for (int j = image_height - 1; j >= 0; --j) {
		for (int i = 0; i < image_width; ++i) {
			scan_calculate_color(j, i, background, samples_per_pixel, lights);
		}

		omp_set_lock(&lock);
		std::cerr << "\routput remaining: " << finish << ' ' << std::flush;
		finish--;
		omp_unset_lock(&lock);
	}

	for (int j = image_height - 1; j >= 0; --j) {
		std::cerr << "\routput remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i) {
			out_color_table(std::cout, color_table, j, i);
		}
	}

	const auto stop = std::chrono::high_resolution_clock::now();
	const auto elapsed = std::chrono::duration<float, std::chrono::seconds::period>(stop - start).count();

	std::cerr << "\n" << "duration : " << elapsed << "s\tDone.\n";
}

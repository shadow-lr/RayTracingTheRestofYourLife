#include "geometry/ray.h"
#include "math/vec3.h"
#include "rtweekend.h"
#include "color.h"

#include "geometry/hittable_list.h"
#include "geometry/translate.h"
#include "geometry/rotate.h"
#include "geometry/bvh.h"

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

// function
hittable_list random_scene();

// Image
double aspect_ratio = 16.0 / 9.0;
int image_width = 400;
int image_height = static_cast<int>(image_width / aspect_ratio);
const int max_depth = 50;

// World
hittable_list world = random_scene();

camera cam;

// 225 600
std::vector<std::vector<color>> color_table(image_height + 1, std::vector<color>(image_width + 1));

// ray recursion
color ray_color(const ray &r, const color &background, const hittable &world, int depth) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0, 0, 0);

    if (!world.hit(r, 0.001, infinity, rec))
        return background;

    ray scattered;
    color attenuation;
    color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return emitted;

    return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
}

void scan_calculate_color(int height, int width, color &background, int samples_per_pixel) {
    int i = width, j = height;
    color pixel_color(0, 0, 0);
    for (int s = 0; s < samples_per_pixel; ++s) {
        double u = (i + random_double()) / (image_width - 1.0);
        double v = (j + random_double()) / (image_height - 1.0);
        ray r = cam.get_ray(u, v);
        pixel_color += ray_color(r, background, world, max_depth);
    }
    //    write_color(std::cout, pixel_color, samples_per_pixel);
    write_color_table(pixel_color, samples_per_pixel, color_table, j, i);
}

hittable_list random_scene() {
    hittable_list world;

    // 原来的地面
//    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
//    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto center2 = center + vec3(0, random_double(0, 0.5), 0);
                    world.add(make_shared<moving_sphere>(center, center2, 0.0, 1.0, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return world;
}

hittable_list two_spheres() {
    hittable_list objects;

    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

    objects.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    return objects;
}

hittable_list two_perlin_spheres() {
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    return objects;
}

hittable_list earth() {
    hittable_list objects;

    auto earth_texture = make_shared<image_texture>("../assets/earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);

//    auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);
    objects.add(make_shared<sphere>(point3(0, 0, 0), 2, earth_surface));

    return objects;
}

hittable_list simple_light() {
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(color(4, 4, 4));
    objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

    return objects;
}

hittable_list cornell_box() {
    hittable_list objects;

    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    // box
    shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265, 0, 295));
    objects.add(box1);

    shared_ptr<hittable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130, 0, 65));
    objects.add(box2);

    // wall
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    return objects;
}

hittable_list cornell_smoke() {
    hittable_list objects;

    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265, 0, 295));

    shared_ptr<hittable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130, 0, 65));

    objects.add(make_shared<constant_medium>(box1, 0.01, color(0, 0, 0)));
    objects.add(make_shared<constant_medium>(box2, 0.01, color(1, 1, 1)));

    return objects;
}

hittable_list final_scene() {
    hittable_list boxes1;
    auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

    const int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i * w;
            auto z0 = -1000.0 + j * w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1, 101);
            auto z1 = z0 + w;

            boxes1.add(make_shared<box>(point3(x0, y0, z0), point3(x1, y1, z1), ground));
        }
    }

    hittable_list objects;

    objects.add(make_shared<bvh_node>(boxes1, 0, 1));

    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

    auto center1 = point3(400, 400, 200);
    auto center2 = center1 + vec3(30, 0, 0);
    auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
    objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

    objects.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
    objects.add(make_shared<sphere>(
            point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)
    ));

    auto boundary = make_shared<sphere>(point3(360, 150, 145), 70, make_shared<dielectric>(1.5));
    objects.add(boundary);
    objects.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
    boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
    objects.add(make_shared<constant_medium>(boundary, .0001, color(1, 1, 1)));

    auto emat = make_shared<lambertian>(make_shared<image_texture>("../assets/earthmap.jpg"));
    objects.add(make_shared<sphere>(point3(400, 200, 400), 100, emat));
    auto pertext = make_shared<noise_texture>(0.1);
    objects.add(make_shared<sphere>(point3(220, 280, 300), 80, make_shared<lambertian>(pertext)));

    hittable_list boxes2;
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(make_shared<sphere>(point3::random(0, 165), 10, white));
    }

    objects.add(make_shared<translate>(
            make_shared<rotate_y>(
                    make_shared<bvh_node>(boxes2, 0.0, 1.0), 15),
            vec3(-100, 270, 395)
                )
    );

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

    int option = 9;

    switch (option) {
        case 1:
            world = random_scene();
            lookfrom = point3(13, 2, 3);
            lookat = point3(0, 0, 0);
            vfov = 20.0;
            aperture = 0.1;
            background = color(0.70, 0.80, 1.00);
            break;
        case 2:
            world = two_spheres();
            lookfrom = point3(13, 2, 3);
            lookat = point3(0, 0, 0);
            vfov = 20.0;
            background = color(0.70, 0.80, 1.00);
            break;
        case 3:
            world = two_perlin_spheres();
            lookfrom = point3(13, 2, 3);
            lookat = point3(0, 0, 0);
            vfov = 20.0;
            background = color(0.70, 0.80, 1.00);
            break;
        case 4:
            world = earth();
            lookfrom = point3(13, 2, 3);
            lookat = point3(0, 0, 0);
            vfov = 20.0;
            background = color(0.70, 0.80, 1.00);
            break;

        case 5:
            background = color(0.0, 0.0, 0.0);
            break;
        case 6:
            world = simple_light();
            samples_per_pixel = 400;
            background = color(0, 0, 0);
            lookfrom = point3(26, 3, 6);
            lookat = point3(0, 2, 0);
            vfov = 20.0;
            break;
        case 7:
            world = cornell_box();
            aspect_ratio = 1.0;
            image_width = 300;
            samples_per_pixel = 200;
            background = color(0, 0, 0);
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
            break;
        case 8:
            world = cornell_smoke();
            aspect_ratio = 1.0;
            image_width = 2540;
            samples_per_pixel = 300;
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
            break;
        case 9:
            world = final_scene();
            aspect_ratio = 1.0;
            image_width = 800;
            samples_per_pixel = 300;
            background = color(0, 0, 0);
            lookfrom = point3(478, 278, -600);
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
            scan_calculate_color(j, i, background, samples_per_pixel);
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

# RayTracingTheRestofYourLife
Learning ray tracing the rest of your life from https://raytracing.github.io/books/RayTracingTheRestOfYourLife.html#lightscattering/thescatteringpdf

## build
Run build_window.bat to generate visual studio 2019 project files
```cpp
build_window.bat
```

## FrameWork
include:
```txt
│  color.h
│  rtweekend.h
│
├─asset
│      camera.h
│      image_texture.h
│      light.h
│      material.h
│      noise_texture.h
│      texture.h
│
├─external
│      stb_image.h
│
├─geometry
│      aabb.h
│      bvh.h
│      hittable.h
│      hittable_list.h
│      obn.h
│      pdf.h
│      ray.h
│      rotate.h
│      translate.h
│
├─math
│      vec3.h
│
├─sample
│      perlin.h
│
├─shape
│      aarect.h
│      box.h
│      constant_medium.h
│      cube.h
│      moving_sphere.h
│      sphere.h
│
├─thread
│      render_thread.h
│
└─utility
        rtw_stb_image.h
```
src:
```txt
src:
│  main.cpp
│
├─asset
│      material.cpp
│
├─geometry
│      aabb.cpp
│      bvh.cpp
│      hittable_list.cpp
│
├─math
│      pi.cpp
│
├─sample
│      perlin.cpp
│
└─shape
        aarect.cpp
        moving_sphere.cpp
        sphere.cpp
```

## Imporve
- Multi-threads support
- Random sampling points using trigonometry instead of rejection
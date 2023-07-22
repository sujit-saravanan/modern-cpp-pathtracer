cmake_minimum_required(VERSION 3.16)
project(raytracer)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-std=c++20 -O3 -flto -march=native -fomit-frame-pointer -ffast-math -fno-math-errno -DSOA")
set(CMAKE_EXPORT_COMPILE_COMMANDS )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../output)

set(VENDOR_HEADER_FILES
                ../vendor/stb_image_write/stb_image_write.h
                ../vendor/thread_pool/thread_pool.h
                ../vendor/stb_image/stb_image.h
                )

set(INTERNAL_HEADER_FILES
                ../internal/raytracer_random/raytracer_random.h
                ../internal/ray/ray.h
                ../internal/scene/scene.h
                ../internal/shape_soa/shape_soa.h
                ../internal/shape/shape.h
                ../internal/image/image.h
                ../internal/camera/camera.h
                )

set(VENDOR_SOURCE_FILES
                ../vendor/stb_image_write/stb_image_write.cpp
                ../vendor/stb_image/stb_image.cpp
                )

set(INTERNAL_SOURCE_FILES
                ../internal/raytracer_random/raytracer_random.cpp
                ../internal/ray/ray.cpp
                ../internal/scene/scene.cpp
                ../internal/shape_soa/shape_soa.cpp
                ../internal/shape/shape.cpp
                ../internal/image/image.cpp
                ../internal/camera/camera.cpp
                )

set(SOURCE_FILES ../src/main.cpp
                ${INTERNAL_SOURCE_FILES}
                ${VENDOR_SOURCE_FILES}
                ${INTERNAL_HEADER_FILES}
                ${VENDOR_HEADER_FILES}
                )

include_directories(../vendor
                ../vendor/stb_image_write
                ../vendor/thread_pool
                ../vendor/stb_image
                ../internal/raytracer_random
                ../internal/ray
                ../internal/scene
                ../internal/shape_soa
                ../internal/shape
                ../internal/image
                ../internal/camera
                )

add_executable(raytracer ${SOURCE_FILES})
target_precompile_headers(raytracer PRIVATE ${VENDOR_HEADER_FILES})

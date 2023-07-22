# modern-cpp-pathtracer
## Summary
A C++ raytracer to experiment with std::variant. All shapes are stored in a Shape variant, and follow the CRTP pattern to avoid vtable lookups. This approach also allows surprising levels of flexibility as a result of giving the type system deeper knowledge of your codebase. Examine the files located in `internal/shape` for the relevant code.

Exploration through godbolt indicates this pattern compiles to roughly the same assembly as a C-like approach to static polymorphism using enums and switch statements(Tested using Clang 15.0.7 with -O3 and flto).

The raytracer also implements **N**ext **E**vent **E**stimation(NEE) in order to converge to higher quality results using less samples. This is especially important in scenes where the light source is difficult to intersect with reliably.

## Optimizations
Significant performance can be gained by splitting the `Shape` class into `LargeShape` and `SmallShape`, as right now, shapes that take less storage like spheres and planes are expanded to match the size of the largest shape, triangles. This results in massive amounts of waste(triangles are 12 floats, circles and planes are 4) in both memory as well as cache-line usage. (**In order to improve cache locality, a struct of arrays pipeline has been implemented.**)

Another simple optimization would be to give each thread a "tile" from the image to trace rather than arbitrary pixels. This would result in better cache locality as it's likely neighboring rays will traverse the same path through the scene. (**A similar optimization has been implemented, where each thread gets a row of pixels rather than a tile. This resulted in a 20% performance gain over the original idea.**)

As of right now, there is no acceleration structure, resulting in every single shape needing an intersection test. A BVH would be relatively straight forward to implement. An interesting optimization might be to store nodes in a contiguous buffer and use indices to jump around rather than chasing pointers, this would improve spatial locality, resulting in it being more likely relevant nodes are stored in the cache.

## Showcase
![test](https://github.com/sujit-saravanan/modern-cpp-pathtracer/assets/105571100/6c1a0080-a1b1-403a-ba55-fa01e2fae853)

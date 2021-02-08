# KHRay
This is a on-going project. The goal is to write a simple ray tracer that produces realistic images with unique effects.

# Features
- Ambient occlusion integrator
- Intel's embree3 library for accelerated ray tracing
- Multi-threaded rendering
- DXR-like acceleration structure (BLAS and TLAS)

# Goals
- Implement path tracing
- Implement spectral path tracing
- Implement importance sampling
- Implement multiple importance sampling
- Implement materials
- Implement various samplers

# Build
- Visual Studio 2019
- C++ 20

All of the required libraries should be included in the repository, the only thing needs to be done is to initialize the submodules

# Bibliography
- Physically Based Rendering: From Theory to Implementation by Matt Pharr, Wenzel Jakob, and Greg Humphreys
- Ray Tracing book series (In One Weekend, The Next Week, The Rest of Your Life) by Peter Shirley
- Real-Time Rendering, Fourth Edition by Eric Haines, Naty Hoffman, and Tomas Möller
- Advanced Global Illumination by Kavita Bala, Philip Dutre, and Philippe Bekaert
- Nori an educational ray tracer by Wenzel Jakob
- Robust Monte Carlo Methods for Light Transport Simulation by Eric Veach

# Acknowledgements
- [assimp][0]
- [embree][1]
- [pcg32][2]
- [stb][3]

# Progress
![0](/Progress/FirstTriangle.png?raw=true "FirstTriangle")
![1](/Progress/InterpolatedNormal.png?raw=true "InterpolatedNormal")
![2](/Progress/GeometricNormal.png?raw=true "GeometricNormal")
![3](/Progress/ShadingNormal.png?raw=true "ShadingNormal")
![4](/Progress/AmbientOcclusion.png?raw=true "AmbientOcclusion")
![5](/Progress/LambertianBuggy.png?raw=true "LambertianBuggy")
Buggy lambertian, thanks to Trolljanhorse#1596 from GP discord for helping me figure out the problem, the issue was that I wasnt clamping my lighting results before writing to png, I need to add proper tone mapping in the future.

![4](/Progress/Lambertian.png?raw=true "Lambertian")

[0]: <https://github.com/assimp/assimp> "assimp"
[1]: <https://github.com/embree/embree> "embree"
[2]: <https://github.com/wjakob/pcg32.git> "pcg32"
[3]: <https://github.com/nothings/stb> "stb"
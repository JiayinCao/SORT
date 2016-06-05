# SORT

[![Build status](https://ci.appveyor.com/api/projects/status/6kiio0dak0wc3ics?svg=true)](https://ci.appveyor.com/project/JerryCao1985/sort)
[![Build status](https://travis-ci.org/JerryCao1985/SORT.svg?branch=master)](https://travis-ci.org/JerryCao1985/SORT)


SORT is short for Simple Open-source Ray Tracing. It is a physically based renderer that I initiated a couple of years ago, which is quite simlilar to PBRT, because I wrote the whole renderer after reading pbrt several times and I'd like to enhance my understanding on offline rendering with this small project.
SORT is cross-platform, it works well on Windows, Mac and Ubuntu.

Here are the features implemented so far:
  - Integrator. (Whitted ray tracing, direct lighting, path tracing, light tracing, bidirectional path tracing, instant radiosity, ambient occlusion)
  - Spatial acceleration structure. (KD-Tree, BVH, Uniform grid, OcTree)
  - Sampler. (Regular, random, stratified)
  - BXDF. (Lambert, Oran Nayar, MicroFacet, MERL)
  - Node graph based material system.
  - Camera. (DOF)
  - Multi-thread rendering.
  - Blender plugin. (Working on it these days.)

Here is a screen shot of SORT working in Blender
![Image](https://agraphicsguy.files.wordpress.com/2015/09/ss.png)

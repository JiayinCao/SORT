# SORT

SORT is short for Simple Open-source Ray Tracing. It is a project that I initiated a couple of years ago, which is a project quite simlilar to PBRT, because I wrote the whole renderer after reading pbrt several times.
SORT is cross-platform, it works well on Windows, Mac and Ubuntu (There should be some issue on Ubuntu because I don't have an ubuntu machine right now, I'll fix those issues when I have one)

Here are the features implemented so far:
  - Integrator. (Whitted ray tracing, direct lighting, path tracing)
  - Spatial acceleration structure. (KD-Tree, BVH, Uniform grid)
  - Sampler. (Regular, random, stratified)
  - BXDF. (Lambert, Oran Nayar, MicroFacet, MERL)
  - Node graph based material system.
  - Camera. (DOF)
  - Multi-thread rendering.
  - Blender plugin. (Working on it these days.)

Here is a screen shot of SORT working in Blender
![Image](https://agraphicsguy.files.wordpress.com/2015/09/ss.png)

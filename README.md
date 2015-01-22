Deferred-Shading
================

Implementation of the Deferred Shading and the Forward+ rendering technique.

Render
-------

- Forwad: Render using the standard forward rendering. We use texture buffer objects to send the lights information to the shader.
- Forward(blend): Previous implementation of forward rendering. We render the scene in intervals of a 100 lights and blend the results.
- Deferred: Render using deferred shading. Expect notable increments on framerate w/ high number of small lights (the smaller the better this is).
- Forward+: Render using Forward+. We used the implementation exposed on Eurographics 2012 by AMD. Link to the slide: http://www.slideshare.net/takahiroharada/forward-34779335.
- Forward+(CUDA): We tried to use CUDA to improve the light culling of Forward+ by parallelizing it. The results were not as great as we expected probably because of lack of knowledge on how to eficiently parallelize processes with CUDA.
 
- Position/Diffuse/Normal/All: G-buffers used for deferred shading.

- Depth: Depth texture.
- Lighting Grid: Grid used on Forward+ to ease the light calculation. Each square is colored in fuction of how many lights affect the tile.

Links
-----

- Informe previ: https://docs.google.com/document/d/1WdmQJXsaLGKol9I29WGMWySgJ58TuFWXiA42b2zQVWY/edit?usp=sharing
- Memoria: https://docs.google.com/document/d/1v11-xn4V8qD2v0Qlmg4sUC3VWC_EHTtSf-8XuuO3Ol8/edit?usp=sharing

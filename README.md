Deferred-Shading
================

Implementation of the Deferred Shading Rendering Technique.

Functionality
-------------

- Load 3d models with textures from various formats. (only .obj format tested)
- Move through the scene using a first person camera. (WASD + left mouse, sensitivity and speed adjustable)
- Genereate random lights (initially 20, up to a 100).
- Adjust lighting parameters (attenuation, intensity and bounding box scale).
- 2 Render modes:
    * Forward Render: standard OpenGL render.
    * Deferred Render: Draws the scene in 2 passes: one to get the info of the geometry and another to apply the lighting, drawing light as spheres.
- Debug Render Mode where the texture from the gbuffers are scaled to rgb colours.

TODO
----

- Linux support.

Links
-----

- Drive: https://docs.google.com/document/d/18DZZu6uij1NknfQejhovRVg8y3756xz0BbT67DwLoBc/edit?usp=sharing
- Imgur: http://imgur.com/a/MBGr2
- Informe previ: https://docs.google.com/document/d/1WdmQJXsaLGKol9I29WGMWySgJ58TuFWXiA42b2zQVWY/edit?usp=sharing

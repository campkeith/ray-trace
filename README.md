# ray-trace
Basic ray tracing program designed as a class project for teaching C programming.

Supports four kinds of surface primitives:
* Sphere defined by a center and radius
* Frustum defined by two centers and two radii
* Circle defined by a center, radius, and normal vector
* Parallelogram "quad" defined by three vertices

Surfaces support three kinds of ray manipulations:
* A diffuse color for light-source based cosine shading
* A specular color for reflection
* A transmission color and refaction index for refraction

Scenes are defined by input files with a hierarchically parsed format, containing:
* Camera position, resolution, and field of view
* Lights with positions and colors
* Surface primitives with their ray manipulation characteristics

Additional documentation is in the "doc" directory.

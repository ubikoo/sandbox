## fluid-sph

Smooth particle hydrodynamics fluid solver.

- **python** Python scripts for different interpolation kernel functions.

- **sph-gpu-full** SPH/OpenCL using full N^2 neighbour search and collision
boundary conditions.

- **sph-gpu-full-pbc** SPH/OpenCL using full N^2 neighbour search and periodic
boundary conditions.

- **sph-gpu-grid** SPH/OpenCL using a hashmap representation of a uniform
grid and collision boundary conditions.

- **sph-gpu-render-surface** SPH/OpenCL using a hashmap representation of a uniform
grid and collision boundary conditions.
Render the fluid surface by rendering the particles as spheres using point
sprites with depth replacement. Implement a naive smoothing of the depth texture
in a second pass.

- **sph-gpu-render-surface-raymarch** SPH/OpenCL using a hashmap representation of a uniform
grid and collision boundary conditions.
Render the fluid surface using a sphere tracing algorithm to compute an
isosurface with a specified density.

<!--
## References
## Acknowlegements
-->

## License

Distributed under the terms of the [MIT](https://choosealicense.com/licenses/mit/) license. See  accompanying `LICENSE.md` for more information.

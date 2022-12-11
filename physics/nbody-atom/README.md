## particle-md

Molecular dynamics particle solvers. The solver design is intentionally basic,
using simple structures as much as possible, to help translation onto the gpu.

- **md-cpu-full** molecular dynamics on the CPU using full N^2 neighbour
search.

- **md-cpu-graph** molecular dynamics on the CPU using a graph of neighbours
in adjacency list form.

- **md-cpu-grid** molecular dynamics on the CPU using a uniform grid as a
spatial data structure.

- **md-cpu-render** molecular dynamics on the CPU using a uniform grid as a
spatial data structure with real time rendering.

- **md-gpu-full** molecular dynamics on the GPU with OpenCL using full N^2
neighbour search.

- **md-gpu-grid** molecular dynamics on the GPU with OpenCL using a uniform
grid as a spatial data structure.

- **md-gpu-neig** molecular dynamics on the GPU with OpenCL using a Verlet
neighbour list with full N^2 list update.

- **md-gpu-neig-grid** molecular dynamics on the GPU with OpenCL using a Verlet
neighbour list with update using a uniform grid spatial data structure.

<!--
## References
## Acknowlegements
-->

## License
Distributed under the terms of the [MIT](https://choosealicense.com/licenses/mit/) license.
See  accompanying `LICENSE.md` for more information.


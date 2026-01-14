# D-Cubed Engine (D³)

[![CI](https://github.com/cbryce996/d-cubed/actions/workflows/ci.yml/badge.svg)](
https://github.com/cbryce996/d-cubed/actions/workflows/ci.yml
)

https://github.com/user-attachments/assets/62ead7ad-d64f-4199-aec5-69004e1431ea

**D-Cubed (D³)** is an open-source simulation engine focused on exploring low-level interactions between the CPU and GPU.  
It is built with as few dependencies as possible to encourage experimentation with mathematics, linear algebra, and first principles in real-time rendering.

---

## Features

- **Deferred rendering pipeline** with explicit render passes and a simple render graph
- **Instanced mesh rendering** for efficiently drawing large numbers of identical meshes
- **Material system** for binding shaders, pipelines, and render state independently of geometry
- **Asynchronous CPU-side task scheduling** for simulation workloads
- **Shader compilation and reflection** to automate uniform and vertex binding

---

## Overview

D³ is organized into a small set of focused systems, with a clear separation between engine code and the sandbox simulation used to exercise it.

```
.
├── src/
│   ├── engine/                 # Core engine runtime and systems
│   │   ├── render/             # Rendering systems and CPU↔GPU interaction
│   │   ├── simulation/         # Simulation core and execution interfaces
│   │   ├── input/              # User input handling and device abstraction
│   │   ├── maths/              # Common math types, operations, and geometry
│   │   ├── cameras/            # Camera, view, and projection management
│   │   ├── assets/             # Asset loading, management, and lifetime
│   │   └── utils/              # Shared utilities and helper code
│   └── simulation/             # Simulation demo / sandbox application
├── assets/                     # Shaders, materials, meshes, and loadable content
├── media/                      # Screenshots, gifs, and demo media
├── scripts/                    # Scripts for builds and shader compilation
├── tests/                      # Engine and integration tests
└── CMakeLists.txt
```

The engine is still in an **early stage of development** and is intended primarily as a learning and experimentation platform rather than a production-ready framework.

---

## Building & Running

> **Note:** The engine is currently **macOS-only**. Other platforms are not yet supported.

### Requirements

- macOS
- C++20-compatible compiler (Clang recommended)
- CMake
- **SDL3**
- **glm**
- **tinyobjloader**
- **nlohmann_json**
- A GPU with support for modern graphics APIs

Third-party dependencies are expected to be available under an `external/` directory or otherwise accessible to the build system.

### Build

Given the early stage of the engine, a small amount of manual setup is required.

Clone the repository:

```bash
git clone https://github.com/cbryce996/d-cubed.git
cd d-cubed
````

Compile shaders and build the executable:

```bash
make shaders
make build
```

### Tests (Optional)

To verify the project builds correctly, you can run:

```bash
make tests
```
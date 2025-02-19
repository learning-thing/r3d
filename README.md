# R3D - 3D Rendering Library for raylib

<img  align="left" style="width:152px" src="https://github.com/Bigfoot71/r3d/blob/master/logo.png" width="124px">

R3D is a 3D rendering library designed to work with [raylib](https://www.raylib.com/). It offers features for lighting, shadows, materials, post effects, and more.

R3D is ideal for developers who want to add 3D rendering to their raylib projects without building a full rendering engine from scratch.

---

## Features

- **Material**: Uses raylib’s material system, just load a model and set its material maps, R3D handles the rest.  
- **Lighting**: Supports deferred lighting with directional, spot, and omni-directional lights.  
- **Shadow Mapping**: Real-time shadows with adjustable resolution and support for multiple light types.  
- **Skyboxes**: Loads and renders HDR/non-HDR skyboxes, with IBL support for scene lighting.  
- **Post-processing**: Includes SSAO, bloom, fog, tonemapping, color adjustment, FXAA, and more.  
- **Instanced Rendering**: Supports instance rendering with matrix arrays, an optional global matrix, and per-instance colors.  
- **Frustum Culling**: Provides easy shape tests (bounding boxes, spheres, points) for visibility in the scene frustum.  
- **Blit Management**: Renders at an internal resolution and blits the result to the main framebuffer or a render texture, with aspect ratio options.  

---

## Getting Started

To use R3D, you must have [raylib](https://www.raylib.com/) installed, or if you don’t have it, clone the repository with the command:
```
git clone --recurse-submodules https://github.com/Bigfoot71/r3d
```

Si vous avez déjà cloné le dépot sans cloner raylib avec vous pouvez faire:
```
git submodule update --init --recursive
```

### Prerequisites

- **raylib**: The library is provided as a submodule, which is optional. If **raylib 5.5** is already installed on your system, you can use R3D without cloning the submodule.
- **CMake**: For building the library.
- **C Compiler**: A C99 or higher compatible compiler.

### Installation

1. **Clone the repository**:

   ```bash
   git clone https://github.com/Bigfoot71/r3d
   cd R3D
   ```

2. **Optional: Clone raylib submodule**:

   ```bash
   git submodule update --init --recursive
   ```

3. **Build the library**:

   Use CMake to configure and build the library.

   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

4. **Link the library to your project**:

   - R3D is a CMake project, and you can include it in your own CMake-based project via `add_subdirectory()` or by linking directly to the built library.
   - If you're using it as the main project, you can build the examples using the option `R3D_BUILD_EXAMPLES` in CMake.

---

## Usage

### Initialize R3D

To initialize R3D, you need to specify the internal resolution in which it will render, as well as the flags that determine how it should operate. You can simply set them to '0' to start.  

Here is a basic example:

```c
#include <r3d.h>

int main()
{
    // Initialize raylib window
    InitWindow(800, 600, "R3D Example");

    // Initialize R3D Renderer with default settings
    R3D_Init(800, 600, 0);

    // Load a model to render
    Model model = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));

    // Setup material with default values
    R3D_SetMaterialOcclusion(&model.materials[0], NULL, 1.0f);
    R3D_SetMaterialRoughness(&model.materials[0], NULL, 1.0f);
    R3D_SetMaterialMetalness(&model.materials[0], NULL, 0.0f);

    // Create a directional light
    R3D_Light light = R3D_CreateLight(R3D_DIRLIGHT);
    R3D_SetLightDirection(light, Vector3Normalize((vector3) { -1, -1, -1 }));

    // Init a Camera3D
    Camera3D camera = {
        .position = (Vector3) { 0, 0, 5 },
        .target = (Vector3) { 0, 0, 0 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    // Main rendering loop
    while (!WindowShouldClose()) {
        BeginDrawing();
            R3D_Begin(camera);
                R3D_DrawModel(model, (Vector3) { 0 }, 1.0f);
            R3D_End();
        EndDrawing();
    }

    // Close R3D renderer and raylib
    R3D_Close();
    CloseWindow();

    return 0;
}
```

This example demonstrates how to set up a basic 3D scene using R3D:

1. Initializes a raylib window (800x600 pixels).  
2. Calls `R3D_Init()` to set up the R3D renderer with the default internal resolution (same as the raylib window).  
3. Loads and renders a simple cube model using `LoadModelFromMesh()`.  
4. Sets some material values, as raylib doesn't initialize them by default. `NULL` is used for the texture, meaning the current (default) texture is kept, then we assign the material map factor.  
5. Creates a directional light to illuminate the scene. R3D automatically normalizes the direction.  
6. Initializes a `Camera3D` to view the scene.  
7. Runs the main loop, rendering the cube and light until the window is closed.  
8. Closes the R3D renderer and raylib window properly.  

### Adding Lights

R3D supports several types of lights, each with its own behavior and characteristics. You can create and manage lights as shown below:

```c
R3D_Light light = R3D_CreateLight(R3D_SPOTLIGHT, 0);                    // Adds a spotlight, without shadows (zero parameter)
R3D_SetLightPositionTarget(light, (Vector3){0, 10, 0}, (Vector3){0});   // Set light position and target
```

The `R3D_CreateLight()` function takes one parameter: the light type, which remains constant for the light's lifetime.

R3D supports the following light types:

1. **R3D_DIRLIGHT (Directional Light)**:
   - Simulates sunlight, casting parallel rays of light in a specific direction across the entire scene.
   - Useful for outdoor environments where consistent lighting over large areas is required.

2. **R3D_SPOTLIGHT (Spotlight)**:
   - Emits a cone-shaped beam of light from a specific position, pointing toward a target.
   - Requires defining both the light's **position** and its **target** to determine the direction of the beam.
   - Spotlights include the following configurable parameters:
     - **Range**: Defines how far the spotlight reaches before fading out completely.
     - **Inner Cutoff**: The angle of the cone where the light is at full intensity.
     - **Outer Cutoff**: The angle where the light fades out to darkness.
     - **Attenuation**: Controls how the light intensity decreases with distance, enabling realistic falloff effects.

3. **R3D_OMNILIGHT (Omni Light)**:
   - A point light that radiates uniformly in all directions, similar to a light bulb.
   - Requires a **position** but does not use a direction or target.
     - **Range**: Determines the maximum distance the light affects objects before fading out.
     - **Attenuation**: Controls how the light intensity decreases with distance, enabling realistic falloff effects.

### Drawing a Model

To draw a model in the scene, use the `R3D_DrawModel()` methods. There are three possible variants:

- `void R3D_DrawModel(const R3D_Model* model)`
- `void R3D_DrawModelEx(const R3D_Model* model, Vector3 position, float scale)`
- `void R3D_DrawModelPro(const R3D_Model* model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale)`

These functions work like raylib’s but differ internally. The only visible difference is that they don’t take a tint. This is due to implementation details, but you can still set the color in your material’s albedo map.

Otherwise, the types used are the same as in raylib, so you can load them directly with raylib, here’s an example:

```c
Model model = LoadModel("model.obj");
R3D_DrawModel(model, (Vector3) { 0 }, 1.0f);
```

---

## Additional Notes

- **Shadow Mapping**: Supports shadows for point, spot, and directional lights. When creating a light, you can specify a shadow map resolution. Shadows can still be disabled later using the `R3D_DisableLightShadow` function:  

```c
R3D_EnableLightShadow(light, 2048);     // Enable shadow mapping with a 2048x2048 shadow map resolution
R3D_DisableLightShadow(light, false);   // Disables the light; `false` keeps the allocated shadow map, while `true` destroys it
```

- **Material System**: R3D uses raylib’s materials but provides helper functions to simplify setup, as raylib doesn’t always initialize all values and configuring a model’s material can be repetitive.  

```c
Material material = LoadMaterialDefault();  // Provided by raylib  

// Sets the material's albedo with a texture (copied) and a tint.
R3D_SetMaterialAlbedo(&material, &myTexture, WHITE);

// Sets roughness; `NULL` keeps the existing texture.
// If none was set, R3D assigns a default one.
R3D_SetMaterialRoughness(&material, NULL, 1.0f);

// Metalness and roughness should be set together.
// By default, both are 0, similar to smooth plastic.
R3D_SetMaterialMetalness(&material, NULL, 0.0f);

// It’s recommended to set occlusion to `1` by default.
// Otherwise, shadowed areas may appear completely black.
// While it may seem logical for the value to be inverted,
// for standard formats, like GLB, `1` should be interpreted as no occlusion.  
R3D_SetMaterialOcclusion(&material, NULL, 1.0f);

// If no emission texture is assigned, R3D defaults it to black.
// Calling this function with `NULL` changes it to white, assuming you want emission.
// But don't panic! If the factor is `0`, the material won’t emit light.  
R3D_SetMaterialEmission(&material, NULL, RED, 3.0f);
```  

- **Post-processing**: Post-processing effects like fog, bloom, tonemapping or color correction can be added at the end of the rendering pipeline using R3D's built-in shaders.

---

## Contributing

If you'd like to contribute, feel free to open an issue or submit a pull request.

---

## License

This project is licensed under the **Zlib License** - see the [LICENSE](LICENSE) file for details.

---

## Acknowledgements

Thanks to [raylib](https://www.raylib.com/) for providing an easy-to-use framework for 3D development!

## Screenshots

![](examples/screenshots/sponza.png)
![](examples/screenshots/pbr.png)
![](examples/screenshots/bloom.png)

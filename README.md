# Raytracer
## Overview
This project is a basic raytracer that implements backtracing at a very basic level. It is designed to render simple 3D scenes by simulating the way light interacts with objects.

## Features
- Basic ray-object intersection
- Simple shading model
- Support for spheres and planes

## Getting Started
### Prerequisites
- A C++ compiler (e.g., g++)
- CMake

### Building
1. Clone the repository:
    ```sh
    git clone https://github.com/yourusername/raytracer.git
    cd raytracer
    ```
2. Build the project using CMake:
    ```sh
    mkdir build
    cd build
    cmake ..
    make
    ```

### Running
After building the project, you can run the raytracer:
```sh
./raytracer
```

## Usage
The raytracer will generate an image file (e.g., `output.ppm`) in the project directory. You can open this file with an image viewer that supports the PPM format.

## Contributing
Contributions are welcome! Please fork the repository and submit a pull request.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
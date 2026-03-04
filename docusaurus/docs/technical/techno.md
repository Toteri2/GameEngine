---
sidebar_position: 1
---

# Techno

This section descibe our choice in multiple domains. Please found some reasons in these tables.

## Package manager

| Feature                  | **Conan**                        | **vcpkg**                            | **CPM**                      |
| ------------------------ | -------------------------------- | ------------------------------------ | ----------------------------------------- |
| **Integration**          | Works with CMake and other tools | Native CMake integration             | Pure CMake integration (no external tool) |
| **Ease of Use**          | Requires setup/configuration     | Easier for Microsoft ecosystem       | Extremely lightweight and simple          |
| **Package Availability** | Large community-contributed repo | Large set of libraries, esp. Windows | Depends on what’s available via Git       |
| **Custom Packages**      | Requires recipe creation         | Uses portfiles                       | Just add Git URLs, very flexible          |
| **Binary Caching**       | Built-in, configurable           | Microsoft’s binary cache system      | No caching; builds from source            |
| **Cross-Platform**       | Yes                              | Yes                                  | Yes (as long as CMake works)              |
| **Maintenance Overhead** | Medium to high                   | Medium                               | Very low (since it's just CMake logic)    |

### Why we chose CPM:

- Seamless integration with CMake, no need for additional tooling or configuration.
- Minimal overhead and fits well in small to mid-size projects.
- Encourages developers to understand and control their dependencies.
- Ideal for source-based dependency management and Git-based versioning.

## Graphical lib

| Feature                | **SFML**                   | **SDL2**                         | **SDL3**                                | **Raylib**                            |
| ---------------------- | -------------------------- | -------------------------------- | -------------------------------------------- | ------------------------------------- |
| **Language Support**   | C++                        | C (with C++ bindings)            | C (with C++ bindings)                        | C (some C++ support)                  |
| **Ease of Use**        | High-level, easy API       | Low-level, more verbose          | Improved API, cleaner than SDL2              | Very beginner-friendly                |
| **2D/3D Capabilities** | Strong 2D, weak 3D         | Mostly 2D, 3D via add-ons        | Excellent 2D, 3D via OpenGL/Vulkan           | Strong 2D and basic 3D support        |
| **Performance**        | Good, but with overhead    | High performance, very low-level | High performance, modern architecture        | Decent, suitable for prototypes/games |
| **Community & Docs**   | Mature, good documentation | Very mature, lots of resources   | Growing, modernized API documentation        | Growing community, friendly docs      |
| **Extensibility**      | Moderate                   | High (barebones architecture)    | High, with better modern C++ support         | Limited by design simplicity          |

### Why we chose SDL3:

- Modern cross-platform multimedia library with excellent performance.
- Provides low-level access to graphics, audio, and input handling.
- Active development and community support with improved API over SDL2.
- Native CMake integration and CPM compatibility.
- Excellent for 2D games with OpenGL/Vulkan support for advanced rendering.

## Network library

| Feature               | **boost::asio**                          | **ENet**                          | **libcurl**                           |
| --------------------- | ---------------------------------------- | --------------------------------- | ------------------------------------- |
| **Protocol Support**  | TCP, UDP, SSL/TLS, serial ports, more    | UDP only                          | HTTP/HTTPS, FTP, SMTP, etc.           |
| **Abstraction Level** | Low-level, powerful                      | Medium-level, focused on games    | High-level (HTTP-focused)             |
| **Async Support**     | Full async with coroutines or handlers   | Yes (non-blocking I/O)            | Basic async via callbacks or threads  |
| **Performance**       | Very high                                | Very high (optimized for games)   | Good for web protocols, not real-time |
| **Thread Safety**     | Thread-safe with io_context per thread   | Limited                           | Thread-safe in multi handle mode      |
| **Ease of Use**       | Steep learning curve                     | Simple API for UDP communication  | Very simple for basic use             |
| **C++ Integration**   | Excellent (header-only in Boost 1.70+)   | C library (can be wrapped in C++) | C API, usable from C++                |
| **Use Case Fit**      | General-purpose networking               | Real-time games over UDP          | Web APIs, downloads, HTTP comms       |

### Why we chose boost::asio:

- Offers low-level, fine-grained control over networking, essential for real-time systems like multiplayer games.
- Supports both TCP and UDP, giving flexibility for different types of connections.
- Fully asynchronous with coroutine support, ideal for scalable, non-blocking architectures.
- Well-maintained, widely used in production systems, and integrates smoothly with modern C++.
- Portable and cross-platform, with support for SSL/TLS when needed.

## Developement language

| Feature                             | **C**                          | **C++**                     | **JavaScript**                                    |
| ----------------------------------- | ------------------------------ | --------------------------------- | ------------------------------------------------- |
| **Performance**                     | Very high                      | Very high                         | Lower (interpreted or JIT compiled)               |
| **Memory Management**               | Manual                         | Manual or RAII (smart pointers)   | Garbage-collected                                 |
| **Complexity**                      | Simple syntax, low abstraction | Complex but powerful              | Simple, but async-heavy logic                     |
| **Tooling & Ecosystem**             | Mature, low-level tools        | Mature, wide tooling              | Vast web tooling, less suited to native dev       |
| **Object-Oriented Support**         | None (emulated via structs)    | Full OO support                   | Prototype-based OO                                |
| **Multiplatform**                   | Yes                            | Yes                               | Yes, mostly browser/server environments           |
| **Suitability for Game/Engine Dev** | Core systems, embedded logic   | Full-featured engines and systems | Not suitable for performance-intensive native dev |

### Why we chose C++:

- Combines high performance with modern language features (RAII, templates, smart pointers, etc.).
- Excellent for game engine development, allowing both low-level control and high-level abstractions.
- Interoperates easily with C (for system-level calls) and supports modern build systems like CMake + CPM.
- Better type safety and modularity compared to C, and far more suited for native code than JavaScript.

## Summary of our choices

| Domain              | Our Choice | Why                                                                   |
| ------------------- | ---------- | --------------------------------------------------------------------- |
| **Package Manager** | **CPM**    | Lightweight, simple, native CMake, zero setup, Git-based dependencies |
| **Graphical Lib**   | **SDL3**   | Modern, high-performance, cross-platform, excellent 2D support        |
| **Language**        | **C++**    | Best mix of performance, features, and system-level access            |

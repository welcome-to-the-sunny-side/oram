# Path ORAM Implementation

An implementation of (non-recursive) path-oram (https://eprint.iacr.org/2013/280.pdf).

This library provides client-side and server-side components to enable oblivious access to data stored on an untrusted server using the Path ORAM scheme.

## Project Structure

The core components are organized as follows:

*   `include/`: Contains header files for the ORAM library (`oram_lib.hpp` and specific components like `oram/`, `net/`, `util/`).
*   `src/`: Contains source code for the client (`src/client/`) and server (`src/server/`) executables, as well as potentially shared library components.
*   `misc/`: Contains miscellaneous files, like diagrams.
*   `CMakeLists.txt`: Defines the build process using CMake.

![ORAM Structure](misc/structure.png)

## Dependencies

This project requires the following libraries:

*   **CMake:** (Version 3.10 or higher) For building the project.
*   **A C++17 compliant compiler:** (e.g., GCC, Clang)
*   **Boost:** Specifically `system` and `serialization` components.
*   **OpenSSL:** For secure communication (specifically libssl and libcrypto).

Installation methods vary by operating system. On macOS with Homebrew, you might install them like this:

```bash
brew install cmake boost openssl@3
```

On Debian/Ubuntu based systems:

```bash
sudo apt-get update
sudo apt-get install build-essential cmake libboost-system-dev libboost-serialization-dev libssl-dev
```

Make sure the necessary headers and libraries can be found by CMake. The `CMakeLists.txt` might need adjustments based on your installation paths (e.g., `OPENSSL_ROOT_DIR`).

## Building

1.  **Clone the repository:**
    ```bash
    git clone <repository-url>
    cd path-oram-project # Or your project directory name
    ```

2.  **Configure with CMake:**
    ```bash
    cmake -S . -B build
    ```
    *Note: If dependencies are installed in non-standard locations, you might need to provide hints to CMake, e.g., `-DCMAKE_PREFIX_PATH=/path/to/openssl`.*

3.  **Build the project:**
    ```bash
    cmake --build build
    ```
    This will create the `client` and `server` executables inside the `build/` directory (or subdirectories depending on CMake configuration).

## Running

1.  **Start the Server:**
    Open a terminal, navigate to the build directory, and run the server:
    ```bash
    cd build
    ./server # Or ./src/server/server depending on build structure
    ```
    The server will likely listen on a specific port (this detail might need to be added if known).

2.  **Run the Client:**
    Open another terminal, navigate to the build directory, and run the client:
    ```bash
    cd build
    ./client # Or ./src/client/client
    ```
    The client example provided will connect to the server, perform some ORAM operations, and print results.

## Example client usage

```cpp
#include <iostream>
#include <vector>

#include "include/oram_lib.hpp"

using namespace oram_lib;
std::vector<unsigned char> Encryptor::key;
client_network_communicator cnc;
void o_init()
{
    Encryptor::initialize();
    cnc.connect_to_server(); // Ensure server address/port are correctly configured
}

int main()
{
    o_init();

    int n;
    std::cout << "Enter array size: ";
    std::cin >> n;

    o_array a(n, cnc);
    std::cout << "Enter " << n << " array elements:" << std::endl;
    for(int i = 0; i < n; i ++)
        std::cin >> a[i];

    o_array prefix_sum_a(n, cnc);
    prefix_sum_a[0] = a[0];
    for(int i = 1; i < n; i ++)
        prefix_sum_a[i] = prefix_sum_a[i - 1] + a[i];

    std::cout << "Prefix sums:" << std::endl;
    for(int i = 0; i < n; i ++)
        std::cout << prefix_sum_a[i] << " ";
    std::cout << std::endl;

    return 0;
}
```

## TODO

See `TODO.md` for planned features or improvements.

## Contributing

(Optional: Add guidelines for contributing if desired).
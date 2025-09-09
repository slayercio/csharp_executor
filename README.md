# FiveM C# Executor
A lightweight **C# executor for FiveM**.

This project allows you to load and execute managed assemblies `(.NET / C#)` directly inside the FiveM environment.

It is a working executor, but not yet fully featured – currently limited to executing assemblies stored as raw byte arrays in headers.

### Currently executed script is a simple `sanchez` spawner (clicking F6 spawns and places player into it) (source is in ExecutedResource.cs)

### Loaded scripts should be created just like you would make FiveM c# scripts, see: [FiveM docs](https://docs.fivem.net/docs/scripting-manual/runtimes/csharp/)

# Disclaimer
### This project is provided for educational purposes only.
- I do **not** condone or take responsibility for using this software in violation of FiveM's Terms of Service, copyright laws, or any other legal restrictions.
- Use at your own risk.
- The author is not responsible for any consequences, bans, or damages caused by misuse.

By using this software, you acknowledge and accept that it is your responsibility to comply with all applicable laws and regulations.

## Features
- Execute managed C# assemblies from raw uint8_t[] blobs.
- Minimal API with clear extension points.

## Example Usage
Currently, execution works by embedding your assembly as raw bytes inside a header file:

```cpp
// Convert your C# assembly to a static uint8_t array
static std::vector<uint8_t> scriptData = std::vector<uint8_t>(
    std::begin(executed), 
    std::end(executed)
);

// Run the script
executor.Execute("test_script", scriptData);
```

## Building
The project uses **CMake** for cross-platform builds.
```sh
# Create a build directory
mkdir build
cd build

# Configure
cmake ..
# or if using single-configuration generator
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
cmake --build . 
# or if using something like MSBuild/MSVC
cmake --build . --config Release
```

## Generating header from assembly
Since execution requires embedding assemblies into headers, you can use the included `generate_header.py` script:
```sh
./generate_header.py <input_file> <output_header>
```
### Example:
```sh
./generate_header.py your_assembly.dll include/cse/executed.h
```

#### This will generate a C++ header containing
```cpp
const unsigned char executed[] = {
    0x4D, 0x5A, 0x90, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, ...
};
```

## API reference
### Execute a script
```cpp
bool Executor::Execute(
    const std::string& scriptName,
    const std::vector<uint8_t>& scriptData,
    std::optional<std::reference_wrapper<std::vector<uint8_t>>> pdbData,
    std::optional<std::reference_wrapper<RuntimeInfo>> runtime
);
```
- **scriptName** -> doesn't really matter that much, used just internally
- **scriptData** -> vector containing raw bytes of the assembly
- **pdbData** (optional) -> you can include raw bytes of debug symbols if you want
- **runtime** (optional) -> if you want to execute in a specific runtime otherwise executes in the first available

Returns `true` on success `false` on failure

### Get available runtimes
```cpp
const std::vector<RuntimeInfo>& Executor::GetRuntimes();
```
Provides access to all registered runtimes.

### RuntimeInfo reference
```cpp
std::string RuntimeInfo::GetResourceName() const;
```
Returns a string containing name of the resource owning the runtime

## Extending
Right now, the executor only runs a single hardcoded script blob. (F6 to spawn a sanchez)

To extend functionality:
1. **Load assemblies from disk**
    - replace `scriptData` in `executor.Execute("test_script", scriptData);` with your loaded assembly
2. **Runtime selection**
    - add a selector based on resource name
    - pass the selected runtime into `executor.Execute("test_script", scriptData, std::nullopt, std::cref(runtime));`


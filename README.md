# Custom VFS - source code

This is a source code for my bachelor thesis.  
The goal of this project is to create a custom VFS with added versioning and encryption features.

For more information about the project, see the [thesis repository](https://gitlab.mff.cuni.cz/teaching/theses/yaghob/vesely-milan/thesis).

## Usage

CMake project... blah blah

### Requirements

* C++17
* CMake
* Fuse - So far version 2.9.9 has been tested (TODO - test with 3.x)
* Crypto++

### Build

```bash
mkdir build
cd build
cmake .. 
````

### Run

Simply pass a directory as a mountpoint to the executable.

**Note:** The directory must exist and be empty - TODO solve empty directory problem

```bash
CustomVFS <mountpoint>
```

## Project TODO

- [X] Project setup
  - [X] Empty CMake project
  - [X] Empty unit tests
  - [X] Setup pipelines (build, test, clang-format, clang-tidy, ...)
  - [X] Look into [vcpkg](https://github.com/microsoft/vcpkg) - will not be used
- [X] Functioning VFS
  - [X] Hello world project
  - [ ] ...
- [ ] Versioning features
  - [ ] Logging file changes 
  - [ ] Creating snapshots
  - [ ] Saving incremental updates
- [ ] Encryption features
  - [ ] Encrypting files
  - [ ] Decrypting files
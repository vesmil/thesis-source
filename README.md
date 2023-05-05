# Custom VFS - source code

This is a source code for my bachelor thesis.  
The goal of this project is to create a custom VFS with added versioning and encryption features.

For more information about the project, see
the [thesis repository](https://gitlab.mff.cuni.cz/teaching/theses/yaghob/vesely-milan/thesis).

## Usage

CMake project... blah blah

### Requirements

* C++17
* CMake
* Fuse - So far version 2.9.9 has been tested
* Libsodium
* Google test

### Build

```bash
mkdir build
cd build
cmake .. 
````

### Run

Simply pass a Directory as a mountpoint to the executable.

**Note:** The Directory must exist and be empty - TODO solve empty Directory problem

```bash
CustomVFS <mountpoint>
```

## Project TODO

- [X] Rewrite the base VFS
- [] Hardcode default backing dir
- [] Current versions in versioning
- [] File protection
    - [] Distinguish encrypted 
    - [] Open decrypt and close encrypt
    - [] Add support for on disk key
- [] Recursive folder protection
- [] Metadata?

Current constraits



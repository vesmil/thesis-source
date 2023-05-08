# Custom VFS - source code

This is a source code for my bachelor thesis.  
The goal of this project is to create a custom VFS with added versioning and encryption features.

For more information about the project, see
the [thesis repository](https://gitlab.mff.cuni.cz/teaching/theses/yaghob/vesely-milan/thesis).

### Requirements

* C++17
* CMake
* Fuse - So far version 2.1 and higher was tested
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

```bash
CustomVFS <mountpoint>
```

## Project TODO

- [ ] Add support for on disk key
- [ ] Improve directory encryption
- [ ] Rewrite commands

Current constraits



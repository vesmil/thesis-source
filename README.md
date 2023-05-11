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
* Boost program options

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

### Usage

The VFS is controlled by tools from the `tools` directory.

The usage is as follows:

```bash
cvfs_encrypt --lock <file>
cvfs_encrypt --lock <file> --key <key>
cvfs_encrypt --default-lock <file>         # Requires set-key-path to be run first

cvfs_encrypt --unlock <file>
cvfs_encrypt --unlock <file> --key <key>

cvfs_encrypt --generate <file>             # Generates a new key
cvfs_encrypt --set-key-path <vfs> <file>   # Sets default key path for the VFS
```

And for the versioning

```bash
cvfs_version --list <file>
cvfs_version --restore <version> <file> 
cvfs_version --delete <version> <file>
cvfs_version --delete-all <file>  
```


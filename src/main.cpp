#include "custom_vfs.h"

int main(int argc, char* argv[]) {
    CustomVfs fuseWrapper;

    fuseWrapper.main(argc, argv);
    fuseWrapper.test_files();

    return 0;
}

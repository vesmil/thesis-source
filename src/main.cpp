#include "CustomVFS.h"

int main(int argc, char* argv[]) {
    CustomVFS fuseWrapper;
    fuseWrapper.main(argc, argv);

    return 0;
}

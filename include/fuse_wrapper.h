#ifndef FUSEWRAPPER_H
#define FUSEWRAPPER_H

#include <fuse.h>
#include <vector>

class FuseWrapper {
   public:
    FuseWrapper(const FuseWrapper&) = delete;
    FuseWrapper& operator=(const FuseWrapper&) = delete;

    static FuseWrapper& instance();
    int run(int argc, char *argv[]);

   private:
    FuseWrapper() = default;
    ~FuseWrapper() = default;

    static int getattr(const char *path, struct stat *stbuf);
    static int mkdir(const char *path, mode_t mode);
    static int read(const char* path, char* buf, size_t size, off_t offset);
    static int write(const char* path, const char* buf, size_t size, off_t offset);

    /*
I need to implement

struct fuse_operations {
    int (getattr) (const char , struct stat );
    int (readlink) (const char , char , size_t);
    int (getdir) (const char , fuse_dirh_t, fuse_dirfil_t);
    int (mknod) (const char , mode_t, dev_t);
    int (mkdir) (const char , mode_t);
    int (unlink) (const char );
    int (rmdir) (const char );
    int (symlink) (const char , const char );
    int (rename) (const char , const char );
    int (link) (const char , const char );
    int (chmod) (const char , mode_t);
    int (chown) (const char , uid_t, gid_t);
    int (truncate) (const char , off_t);
    int (utime) (const char , struct utimbuf );
    int (open) (const char , struct fuse_file_info );
    int (read) (const char , char , size_t, off_t, struct fuse_file_info );
    int (write) (const char , const char , size_t, off_t,struct fuse_file_info );
    int (statfs) (const char , struct statfs );
    int (flush) (const char , struct fuse_file_info );
    int (release) (const char , struct fuse_file_info );
    int (fsync) (const char , int, struct fuse_file_info );
    int (setxattr) (const char , const char , const char , size_t, int);
    int (getxattr) (const char , const char , char , size_t);
    int (listxattr) (const char , char , size_t);
    int (removexattr) (const char , const char *);
};
     */

    static const fuse_operations* getOperations();
    static fuse_operations ops;
};

#endif // FUSEWRAPPER_H

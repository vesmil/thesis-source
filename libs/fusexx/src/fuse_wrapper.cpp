#include <fuse_wrapper.h>

#include <cerrno>
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
#define thread_local _declspec(thread)
#else
#define thread_local __thread
#endif

#if FUSE_VERSION <= 22
// #error "Fuse version too old"
#endif

class FuseWrapper::detail {
public:
    detail() = delete;

    static class FuseWrapper &fuse() {
        auto *ctx = fuse_get_context();
        auto *fuseptr = static_cast<class FuseWrapper *>(ctx->private_data);

        if (fuseptr->pid == 0) {
            fuseptr->uid = ctx->uid;
            fuseptr->gid = ctx->gid;
            fuseptr->pid = ctx->pid;
#if FUSE_VERSION >= 28
            fuseptr->umask = ctx->umask;
#endif
        }
        return *fuseptr;
    }

    static int getattr(const char *pathname, struct stat *buf) {
        return fuse().getattr(pathname, buf);
    }

    static int readlink(const char *pathname, char *buffer, size_t size) {
        return fuse().readlink(pathname, buffer, size);
    }

    static int mknod(const char *pathname, mode_t mode, dev_t dev) {
        return fuse().mknod(pathname, mode, dev);
    }

    static int mkdir(const char *pathname, mode_t mode) {
        return fuse().mkdir(pathname, mode);
    }

    static int unlink(const char *pathname) {
        return fuse().unlink(pathname);
    }

    static int rmdir(const char *pathname) {
        return fuse().rmdir(pathname);
    }

    static int symlink(const char *target, const char *linkpath) {
        return fuse().symlink(target, linkpath);
    }
#if FUSE_VERSION < 30
    static int rename(const char *oldpath, const char *newpath) {
        return fuse().rename(oldpath, newpath, 0);
    }
#else  // FUSE_VERSION >= 30
    static int rename(const char *oldpath, const char *newpath, unsigned int flags) {
        return fuse().rename(oldpath, newpath, flags);
    }
#endif

    static int link(const char *oldpath, const char *newpath) {
        return fuse().link(oldpath, newpath);
    }

#if FUSE_VERSION < 30
    static int chmod(const char *pathname, mode_t mode) {
        return fuse().chmod(pathname, mode);
    }

    static int chown(const char *pathname, uid_t uid, gid_t gid) {
        return fuse().chown(pathname, uid, gid);
    }

    static int truncate(const char *path, off_t length) {
        return fuse().truncate(path, length);
    }
#else  // FUSE_VERSION >= 30
    static int chmod(const char *pathname, mode_t mode, struct fuse_file_info *fi) {
#warning chmod fuse_file_info unimplemented
        (void)fi;
        return fuse().chmod(pathname, mode);
    }
    static int chown(const char *pathname, uid_t uid, gid_t gid, struct fuse_file_info *fi) {
#warning chown fuse_file_info unimplemented
        (void)fi;
        return fuse().chown(pathname, uid, gid);
    }
    static int truncate(const char *path, off_t length, struct fuse_file_info *fi) {
#warning truncate fuse_file_info unimplemented
        (void)fi;
        return fuse().truncate(path, length);
    }
#endif
    static int open(const char *pathname, struct fuse_file_info *fi) {
        return fuse().open(pathname, fi);
    }

    static int read(const char *pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
        return fuse().read(pathname, buf, count, offset, fi);
    }

    static int write(const char *pathname, const char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
        return fuse().write(pathname, buf, count, offset, fi);
    }

    static int statfs(const char *path, struct statvfs *buf) {
        return fuse().statfs(path, buf);
    }

    static int flush(const char *pathname, struct fuse_file_info *fi) {
        return fuse().flush(pathname, fi);
    }

    static int release(const char *pathname, struct fuse_file_info *fi) {
        return fuse().release(pathname, fi);
    }

    static int fsync(const char *pathname, int datasync, struct fuse_file_info *fi) {
        return fuse().fsync(pathname, datasync, fi);
    }

    static int setxattr(const char *path, const char *name, const char *value, size_t size, int flags) {
        return fuse().setxattr(path, name, value, size, flags);
    }

    static int getxattr(const char *path, const char *name, char *value, size_t size) {
        return fuse().getxattr(path, name, value, size);
    }

    static int listxattr(const char *path, char *list, size_t size) {
        return fuse().listxattr(path, list, size);
    }

    static int removexattr(const char *path, const char *name) {
        return fuse().removexattr(path, name);
    }

    static thread_local void *filler_handle;
    static thread_local fuse_fill_dir_t filler;

    static int opendir(const char *opendir, struct fuse_file_info *fi) {
        return fuse().opendir(opendir, fi);
    }

    static int readdir(const char *pathname, void *buf, fuse_fill_dir_t fillerDir, off_t off,
                       struct fuse_file_info *fi) {
        int flags = 0;
        detail::filler_handle = buf;
        detail::filler = fillerDir;
        return fuse().readdir(pathname, off, fi, static_cast<FuseWrapper::readdir_flags>(flags));
    }

    static int releasedir(const char *pathname, struct fuse_file_info *fi) {
        return fuse().releasedir(pathname, fi);
    }

    static int fsyncdir(const char *pathname, int datasync, struct fuse_file_info *fi) {
        return fuse().fsyncdir(pathname, datasync, fi);
    }

    static void *init(struct fuse_conn_info *conn) {
        struct {
        } cfg;

        class FuseWrapper *fuseptr = &fuse();
        (void)conn;
        (void)cfg;
        fuseptr->init();
        return fuseptr;
    }
    static void destroy(void *private_data) {
        auto *fuseptr = static_cast<class FuseWrapper *>(private_data);
        fuseptr->destroy();
    }

#if FUSE_VERSION >= 25
    static int access(const char *pathname, int mode) {
        return fuse().access(pathname, mode);
    }
    static int create(const char *pathname, mode_t mode, struct fuse_file_info *fi) {
        return fuse().create(pathname, mode, fi);
    }
#endif  // FUSE_VERSION < 25

#if FUSE_VERSION >= 26
    static int lock(const char *pathname, struct fuse_file_info *fi, int cmd, struct flock *lock) {
        return fuse().lock(pathname, fi, cmd, lock);
    }
#if FUSE_VERSION < 30
    static int utimens(const char *pathname, const struct timespec tv[2]) {
        return fuse().utimens(pathname, tv);
    }
#else  // FUSE_VERSION >= 30
    static int utimens(const char *pathname, const struct timespec tv[2], struct fuse_file_info *fi) {
#warning utimens fuse_file_info unimplemented
        (void)fi;
        return fuse().utimens(pathname, tv);
    }
#endif
    static int bmap(const char *pathname, size_t blocksize, uint64_t *idx) {
        return fuse().bmap(pathname, blocksize, idx);
    }

    static int ioctl(const char *pathname, int cmd, void *arg, struct fuse_file_info *fi, unsigned int flags,
                     void *data) {
        return fuse().ioctl(pathname, cmd, arg, fi, flags, data);
    }

    static int poll(const char *pathname, struct fuse_file_info *fi, struct fuse_pollhandle *ph, unsigned *reventsp) {
        return fuse().poll(pathname, fi, ph, reventsp);
    }

    [[maybe_unused]] static int write_buf(const char *pathname, struct fuse_bufvec *buf, off_t off,
                                          struct fuse_file_info *fi) {
        return fuse().write_buf(pathname, buf, off, fi);
    }

    [[maybe_unused]] static int read_buf(const char *pathname, struct fuse_bufvec **bufp, size_t size, off_t off,
                                         struct fuse_file_info *fi) {
        return fuse().read_buf(pathname, bufp, size, off, fi);
    }

    static int flock(const char *pathname, struct fuse_file_info *fi, int op) {
        return fuse().flock(pathname, fi, op);
    }

    static int fallocate(const char *pathname, int mode, off_t offset, off_t len, struct fuse_file_info *fi) {
        return fuse().fallocate(pathname, mode, offset, len, fi);
    }
#else  // FUSE_VERSION < 26
    static int utime(const char *pathname, struct utimbuf *times) {
        struct timespec tv[2] = {{.tv_sec = times->actime, .tv_nsec = 0}, {.tv_sec = times->modtime, .tv_nsec = 0}};

        return fuse().utimens(pathname, tv);
    }
#endif
};

thread_local void *FuseWrapper::detail::filler_handle;
thread_local fuse_fill_dir_t FuseWrapper::detail::filler;

int FuseWrapper::getattr(const std::string &, struct stat *) {
    return -ENOSYS;
}
int FuseWrapper::readlink(const std::string &, char *, size_t) {
    return -ENOSYS;
}
int FuseWrapper::mknod(const std::string &, mode_t, dev_t) {
    return -ENOSYS;
}
int FuseWrapper::mkdir(const std::string &, mode_t) {
    return -ENOSYS;
}
int FuseWrapper::unlink(const std::string &) {
    return -ENOSYS;
}
int FuseWrapper::rmdir(const std::string &) {
    return -ENOSYS;
}
int FuseWrapper::symlink(const std::string &, const std::string &) {
    return -ENOSYS;
}
int FuseWrapper::rename(const std::string &, const std::string &, unsigned int) {
    return -ENOSYS;
}
int FuseWrapper::link(const std::string &, const std::string &) {
    return -ENOSYS;
}
int FuseWrapper::chmod(const std::string &, mode_t) {
    return -ENOSYS;
}
int FuseWrapper::chown(const std::string &, uid_t, gid_t) {
    return -ENOSYS;
}
int FuseWrapper::truncate(const std::string &, off_t) {
    return -ENOSYS;
}
int FuseWrapper::open(const std::string &, struct fuse_file_info *) {
    return 0;
}
int FuseWrapper::read(const std::string &, char *, size_t, off_t, struct fuse_file_info *) {
    return -ENOSYS;
}
int FuseWrapper::write(const std::string &, const char *, size_t, off_t, struct fuse_file_info *) {
    return -ENOSYS;
}
int FuseWrapper::statfs(const std::string &, struct statvfs *) {
    return 0;
}
int FuseWrapper::flush(const std::string &, struct fuse_file_info *) {
    return -ENOSYS;
}
int FuseWrapper::release(const std::string &, struct fuse_file_info *) {
    return 0;
}
int FuseWrapper::fsync(const std::string &, int, struct fuse_file_info *) {
    return -ENOSYS;
}
int FuseWrapper::setxattr(const std::string &, const std::string &, const std::string &, size_t, int) {
    return -ENOSYS;
}
int FuseWrapper::getxattr(const std::string &, const std::string &, char *, size_t) {
    return -ENOSYS;
}

int FuseWrapper::listxattr(const std::string &, char *, size_t) {
    return -ENOSYS;
}
int FuseWrapper::removexattr(const std::string &, const std::string &) {
    return -ENOSYS;
}
int FuseWrapper::opendir(const std::string &, struct fuse_file_info *) {
    return 0;
}
int FuseWrapper::readdir(const std::string &, off_t, struct fuse_file_info *, FuseWrapper::readdir_flags) {
    return -ENOSYS;
}

int FuseWrapper::fill_dir(const std::string &name, const struct stat *stbuf, off_t off,
                          [[maybe_unused]] FuseWrapper::fill_dir_flags flags) {
#if FUSE_VERSION < 30
    return detail::filler(detail::filler_handle, name.c_str(), stbuf, off);
#else  // FUSE_VERSION >= 30
    return detail::filler(detail::filler_handle, name.c_str(), stbuf, off, (::fuse_fill_dir_flags)flags);
#endif
}

int FuseWrapper::releasedir(const std::string &, struct fuse_file_info *) {
    return 0;
}
int FuseWrapper::fsyncdir(const std::string &, int, struct fuse_file_info *) {
    return -ENOSYS;
}
void FuseWrapper::init() {}
void FuseWrapper::destroy() {}

#if FUSE_VERSION >= 25
int FuseWrapper::access(const std::string &, int) {
    return -ENOSYS;
}
int FuseWrapper::create(const std::string &, mode_t, struct fuse_file_info *) {
    return -ENOSYS;
}
#endif  // FUSE_VERSION >= 25

#if FUSE_VERSION >= 26
int FuseWrapper::lock(const std::string &, struct fuse_file_info *, int, struct flock *) {
    return -ENOSYS;
}
int FuseWrapper::utimens(const std::string &, const struct timespec[2]) {
    return -ENOSYS;
}
int FuseWrapper::bmap(const std::string &, size_t, uint64_t *) {
    return -ENOSYS;
}
int FuseWrapper::ioctl(const std::string &, int, void *, struct fuse_file_info *, unsigned int, void *) {
    return -ENOSYS;
}
int FuseWrapper::poll(const std::string &, struct fuse_file_info *, struct fuse_pollhandle *, unsigned *) {
    return -ENOSYS;
}

int FuseWrapper::write_buf(const std::string &pathname, struct fuse_bufvec *bufvec, off_t off,
                           struct fuse_file_info *fi) {
    int total = 0;

    for (size_t idx = 0; idx < bufvec->count; ++idx) {
        struct fuse_buf &buf = bufvec->buf[idx];

        if (buf.flags & FUSE_BUF_IS_FD) {
            return -ENOSYS;
        }

        size_t bufoff = 0;
        while (bufoff < buf.size) {
            int subsize = static_cast<int>(buf.size - bufoff);
            int subtotal = write(pathname, static_cast<char *>(buf.mem) + bufoff, subsize, off, fi);
            if (subtotal < 0) {
                return subtotal;
            }

            bufoff += subtotal;
            off += subtotal;
            total += subtotal;

            if (subtotal < subsize) {
                return total;
            }
        }
    }
    return total;
}

int FuseWrapper::read_buf(const std::string &pathname, struct fuse_bufvec **bufp, size_t size, off_t off,
                          struct fuse_file_info *fi) {
    *bufp = static_cast<struct fuse_bufvec *>(malloc(sizeof(**bufp)));
    auto &bufvec = **bufp;

    bufvec = FUSE_BUFVEC_INIT(size);
    bufvec.buf[0].mem = malloc(size);
    ssize_t amount = read(pathname, static_cast<char *>(bufvec.buf[0].mem), size, off, fi);

    bufvec.buf[0].size = amount;
    bufvec.off = amount;

    return static_cast<int>(amount);
}

int FuseWrapper::flock(const std::string &, struct fuse_file_info *, int) {
    return -ENOSYS;
}
int FuseWrapper::fallocate(const std::string &, int, off_t, off_t, struct fuse_file_info *) {
    return -ENOSYS;
}
#endif  // FUSE_VERSION >= 26

FuseWrapper::FuseWrapper() : flag_nopath(0), flag_nullpath_ok(0), flag_reserved(0), flag_utime_omit_ok(0) {}

struct fuse_operations FuseWrapper::ops = {
    .getattr = FuseWrapper::detail::getattr,
    .readlink = FuseWrapper::detail::readlink,
#if FUSE_VERSION < 30
    .getdir = nullptr,
#endif  // FUSE_VERSION >= 30
    .mknod = FuseWrapper::detail::mknod,
    .mkdir = FuseWrapper::detail::mkdir,
    .unlink = FuseWrapper::detail::unlink,
    .rmdir = FuseWrapper::detail::rmdir,
    .symlink = FuseWrapper::detail::symlink,
    .rename = FuseWrapper::detail::rename,
    .link = FuseWrapper::detail::link,
    .chmod = FuseWrapper::detail::chmod,
    .chown = FuseWrapper::detail::chown,
    .truncate = FuseWrapper::detail::truncate,

#if FUSE_VERSION < 30
#if FUSE_VERSION >= 26
    .utime = nullptr,
#else
    .utime = FuseWrapper::detail::utime,
#endif
#endif
    .open = FuseWrapper::detail::open,
    .read = FuseWrapper::detail::read,
    .write = FuseWrapper::detail::write,
    .statfs = FuseWrapper::detail::statfs,
    .flush = FuseWrapper::detail::flush,
    .release = FuseWrapper::detail::release,

#if FUSE_VERSION > 21
    .fsync = FuseWrapper::detail::fsync,
    // .setxattr = FuseWrapper::detail::setxattr,
    // .getxattr = FuseWrapper::detail::getxattr,
    .listxattr = FuseWrapper::detail::listxattr,
    .removexattr = FuseWrapper::detail::removexattr,
#endif

    .opendir = FuseWrapper::detail::opendir,
    .readdir = FuseWrapper::detail::readdir,
    .releasedir = FuseWrapper::detail::releasedir,
    .fsyncdir = FuseWrapper::detail::fsyncdir,
    .init = FuseWrapper::detail::init,
    .destroy = FuseWrapper::detail::destroy,

#if FUSE_VERSION >= 25
    .access = FuseWrapper::detail::access,
    .create = FuseWrapper::detail::create,
#if FUSE_VERSION < 30
    .ftruncate = nullptr,
    .fgetattr = nullptr,
#endif
#endif

#if FUSE_VERSION >= 26
    .lock = FuseWrapper::detail::lock,
    .utimens = FuseWrapper::detail::utimens,
    .bmap = FuseWrapper::detail::bmap,

#if FUSE_VERSION < 30
    /* flag_nullpath_ok and flag_nopath apply to:
     * read, write, flush, release, fsync, readdir, releasedir, fsyncdir, ftruncate, fgetattr, lock, ioctl and poll */

    .flag_nullpath_ok = false,  // lets -ohard_remove work on unlinked
    .flag_nopath = false,

    .flag_utime_omit_ok = false,
    .flag_reserved = 0,
#endif

    .ioctl = FuseWrapper::detail::ioctl,
    .poll = FuseWrapper::detail::poll,
    .write_buf = nullptr,  // fuse::detail::write_buf,
    .read_buf = nullptr,   // fuse::detail::read_buf,
    .flock = FuseWrapper::detail::flock,
    .fallocate = FuseWrapper::detail::fallocate,
#endif
};

int FuseWrapper::main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &ops, this);
}

FuseWrapper::~FuseWrapper() = default;

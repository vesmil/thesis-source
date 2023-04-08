#ifndef FUSE_WRAPPER_H
#define FUSE_WRAPPER_H

#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 30
#endif

#include <fuse.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstddef>
#include <cstdint>
#include <string>

class FuseWrapper {
public:
    FuseWrapper();
    ~FuseWrapper();

    /**
     * Main function of FUSE.
     *
     * This function does the following:
     *   - parses command line options (-d -s and -h)
     *   - passes relevant mount options to fuse_mount()
     *   - installs signal handlers for INT, HUP, TERM and PIPE
     *   - registers an exit handler to unmount the filesystem on program exit
     *   - calls either the single-threaded or the multi-threaded event loop
     *
     * @param argc the argument counter passed to the main() function
     * @param argv the argument vector passed to the main() function
     * @return 0 on success, nonzero on failure
     */
    int main(int argc, char *argv[]);

protected:
    /** User ID of the calling process */
    uid_t uid = 0;

    /** Group ID of the calling process */
    gid_t gid = 0;

    /** Thread ID of the calling process */
    pid_t pid = 0;

    /** Umask of the calling process (introduced in version 2.8) */
    mode_t umask = 022;

    /** Get file attributes.
     *
     * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
     * ignored.	 The 'st_ino' field is ignored except if the 'use_ino'
     * mount option is given.  Return -ENOENT if the file does not exist.
     */
    virtual int getattr(const std::string &pathname, struct stat *buf);

    /** Read the target of a symbolic link
     *
     * The buffer should be filled with a null terminated string.  The
     * buffer size argument includes the space for the terminating
     * null character.	If the linkname is too long to fit in the
     * buffer, it should be truncated.	The return value should be 0
     * for success.
     */
    virtual int readlink(const std::string &pathname, char *buffer, size_t size);

    /**
     * Readdir flags, passed to readdir()
     */
    enum readdir_flags {
        READDIR_PLUS [[maybe_unused]] = (1 << 0),
    };

    /** Read directory
     *
     * The filesystem may choose between two modes of operation:
     *
     * 1) The readdir implementation ignores the offset parameter, and
     * passes zero to fill_dir's offset.  fill_dir will not return '1'
     * (unless an error happens), so the whole directory is read in a
     * single readdir operation.
     *
     * 2) The readdir implementation keeps track of the offsets of the
     * directory entries.  It uses the offset parameter and always
     * passes non-zero offset to fill_dir.  When the buffer is full
     * (or an error happens), fill_dir will return '1'.
     */
    virtual int readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi, readdir_flags flags);

    enum fill_dir_flags {
        /**
         * "Plus" mode: all file attributes are valid
         *
         * The attributes are used by the kernel to prefill the inode cache
         * during a readdir.
         *
         * It is okay to set FILL_DIR_PLUS if READDIR_PLUS is not set
         * and vice versa.
         */
        FILL_DIR_PLUS [[maybe_unused]] = (1 << 1),
    };

    /** Function to add an entry in a readdir() operation
     *
     * @param name the file name of the directory entry
     * @param stbuf file attributes, can be NULL
     * @param off offset of the next entry or zero
     * @param flags fill flags
     * @return 1 if buffer is full, zero otherwise
     */
    static int fill_dir(const std::string &name, const struct stat *stbuf, off_t off = 0,
                        [[maybe_unused]] fill_dir_flags flags = (fill_dir_flags)0);

    /** Create a file node
     *
     * This is called for creation of all non-directory, non-symlink
     * nodes.  If the filesystem defines a create() method, then for
     * regular files that will be called instead.
     */
    virtual int mknod(const std::string &pathname, mode_t mode, dev_t dev);

    /** Create a directory
     *
     * Note that the mode argument may not have the type specification
     * bits set, i.e. S_ISDIR(mode) can be false.  To obtain the
     * correct directory type bits use  mode|S_IFDIR
     * */
    virtual int mkdir(const std::string &pathname, mode_t mode);

    /** Remove a file */
    virtual int unlink(const std::string &pathname);

    /** Remove a directory */
    virtual int rmdir(const std::string &pathname);

    /** Create a symbolic link */
    virtual int symlink(const std::string &target, const std::string &linkpath);

    /** Rename a file */
    virtual int rename(const std::string &oldpath, const std::string &newpath, unsigned int flags);

    /** Create a hard link to a file */
    virtual int link(const std::string &oldpath, const std::string &newpath);

    /** Change the permission bits of a file */
    virtual int chmod(const std::string &pathname, mode_t mode);

    /** Change the owner and group of a file */
    virtual int chown(const std::string &pathname, uid_t uid, gid_t gid);

    /** Change the size of a file */
    virtual int truncate(const std::string &path, off_t length);

    /** File open operation
     *
     * No creation (O_CREAT, O_EXCL) and by default also no
     * truncation (O_TRUNC) flags will be passed to open(). If an
     * application specifies O_TRUNC, fuse first calls truncate()
     * and then open(). Only if 'atomic_o_trunc' has been
     * specified and kernel version is 2.6.24 or later, O_TRUNC is
     * passed on to open.
     *
     * Unless the 'default_permissions' mount option is given,
     * open should check if the operation is permitted for the
     * given flags. Optionally open may also return an arbitrary
     * filehandle in the fuse_file_info structure, which will be
     * passed to all file operations.
     */
    virtual int open(const std::string &pathname, struct fuse_file_info *fi);

    /** Read data from an open file
     *
     * Read should return exactly the number of bytes requested except
     * on EOF or error, otherwise the rest of the data will be
     * substituted with zeroes.	 An exception to this is when the
     * 'direct_io' mount option is specified, in which case the return
     * value of the read system call will reflect the return value of
     * this operation.
     */
    virtual int read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi);

    /** Write data to an open file
     *
     * Write should return exactly the number of bytes requested
     * except on error.	 An exception to this is when the 'direct_io'
     * mount option is specified (see read operation).
     */
    virtual int write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                      struct fuse_file_info *fi);

    /** Get file system statistics
     *
     * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
     */
    virtual int statfs(const std::string &path, struct statvfs *buf);

    /** Possibly flush cached data
     *
     * BIG NOTE: This is not equivalent to fsync().  It's not a
     * request to sync dirty data.
     *
     * Flush is called on each close() of a file descriptor.  So if a
     * filesystem wants to return write errors in close() and the file
     * has cached dirty data, this is a good place to write back data
     * and return any errors.  Since many applications ignore close()
     * errors this is not always useful.
     *
     * NOTE: The flush() method may be called more than once for each
     * open().	This happens if more than one file descriptor refers
     * to an opened file due to dup(), dup2() or fork() calls.	It is
     * not possible to determine if a flush is final, so each flush
     * should be treated equally.  Multiple write-flush sequences are
     * relatively rare, so this shouldn't be a problem.
     *
     * Filesystems shouldn't assume that flush will always be called
     * after some writes, or that if will be called at all.
     */
    virtual int flush(const std::string &pathname, struct fuse_file_info *fi);

    /** Release an open file
     *
     * Release is called when there are no more references to an open
     * file: all file descriptors are closed and all memory mappings
     * are unmapped.
     *
     * For every open() call there will be exactly one release() call
     * with the same flags and file descriptor.	 It is possible to
     * have a file opened more than once, in which case only the last
     * release will mean, that no more reads/writes will happen on the
     * file.  The return value of release is ignored.
     */
    virtual int release(const std::string &pathname, struct fuse_file_info *fi);

    /** Synchronize file contents
     *
     * If the datasync parameter is non-zero, then only the user data
     * should be flushed, not the meta data.
     */
    virtual int fsync(const std::string &pathname, int datasync, struct fuse_file_info *fi);

    /** Set extended attributes */
    virtual int setxattr(const std::string &path, const std::string &name, const std::string &value, size_t size,
                         int flags);

    /** Get extended attributes */
    virtual int getxattr(const std::string &path, const std::string &name, char *value, size_t size);

    /** List extended attributes */
    virtual int listxattr(const std::string &path, char *list, size_t size);

    /** Remove extended attributes */
    virtual int removexattr(const std::string &path, const std::string &name);

    /** Open directory
     *
     * Unless the 'default_permissions' mount option is given,
     * this method should check if opendir is permitted for this
     * directory. Optionally opendir may also return an arbitrary
     * filehandle in the fuse_file_info structure, which will be
     * passed to readdir, closedir and fsyncdir.
     */
    virtual int opendir(const std::string &name, struct fuse_file_info *fi);

    /** Release directory
     */
    virtual int releasedir(const std::string &pathname, struct fuse_file_info *fi);

    /** Synchronize directory contents
     *
     * If the datasync parameter is non-zero, then only the user data
     * should be flushed, not the meta data
     */
    virtual int fsyncdir(const std::string &pathname, int datasync, struct fuse_file_info *fi);

    /**
     * Initialize filesystem
     */
    virtual void init();

    /**
     * Clean up filesystem
     *
     * Called on filesystem exit.
     */
    virtual void destroy();

    /**
     * Check file access permissions
     *
     * This will be called for the access() system call.  If the
     * 'default_permissions' mount option is given, this method is not
     * called.
     *
     * This method is not called under Linux kernel versions 2.4.x
     */
    virtual int access(const std::string &pathname, int mode);

    /**
     * Create and open a file
     *
     * If the file does not exist, first create it with the specified
     * mode, and then open it.
     *
     * If this method is not implemented or under Linux kernel
     * versions earlier than 2.6.15, the mknod() and open() methods
     * will be called instead.
     */
    virtual int create(const std::string &pathname, mode_t mode, struct fuse_file_info *fi);

    /**
     * Perform POSIX file locking operation
     *
     * The cmd argument will be either F_GETLK, F_SETLK or F_SETLKW.
     *
     * For the meaning of fields in 'struct flock' see the man page
     * for fcntl(2).  The l_whence field will always be set to
     * SEEK_SET.
     *
     * For checking lock ownership, the 'fuse_file_info->owner'
     * argument must be used.
     *
     * For F_GETLK operation, the library will first check currently
     * held locks, and if a conflicting lock is found it will return
     * information without calling this method.	 This ensures, that
     * for local locks the l_pid field is correctly filled in.	The
     * results may not be accurate in case of race conditions and in
     * the presence of hard links, but it's unlikely that an
     * application would rely on accurate GETLK results in these
     * cases.  If a conflicting lock is not found, this method will be
     * called, and the filesystem may fill out l_pid by a meaningful
     * value, or it may leave this field zero.
     *
     * For F_SETLK and F_SETLKW the l_pid field will be set to the pid
     * of the process performing the locking operation.
     *
     * Note: if this method is not implemented, the kernel will still
     * allow file locking to work locally.  Hence it is only
     * interesting for network filesystems and similar.
     */
    virtual int lock(const std::string &pathname, struct fuse_file_info *fi, int cmd, struct flock *lock);

    /**
     * Change the access and modification times of a file with
     * nanosecond resolution
     *
     * This supersedes the old utime() interface.  New applications
     * should use this.
     *
     * See the utimensat(2) man page for details.
     */
    virtual int utimens(const std::string &pathname, const struct timespec tv[2]);

    /**
     * Map block index within file to block index within device
     *
     * Note: This makes sense only for block device backed filesystems
     * mounted with the 'blkdev' option
     */
    virtual int bmap(const std::string &pathname, size_t blocksize, uint64_t *idx);

    /**
     * Flag indicating that the filesystem can accept a NULL path
     * as the first argument for the following operations:
     *
     * read, write, flush, release, fsync, readdir, releasedir,
     * fsyncdir, ftruncate, fgetattr, lock, ioctl and poll
     *
     * If this flag is set these operations continue to work on
     * unlinked files even if "-ohard_remove" option was specified.
     */
    unsigned int flag_nullpath_ok [[maybe_unused]] : 1;

    /**
     * Flag indicating that the path need not be calculated for
     * the following operations:
     *
     * read, write, flush, release, fsync, readdir, releasedir,
     * fsyncdir, ftruncate, fgetattr, lock, ioctl and poll
     *
     * Closely related to flag_nullpath_ok, but if this flag is
     * set then the path will not be calculaged even if the file
     * wasn't unlinked.  However the path can still be non-NULL if
     * it needs to be calculated for some other reason.
     */
    [[maybe_unused]] unsigned int flag_nopath [[maybe_unused]] : 1;

    /**
     * Flag indicating that the filesystem accepts special
     * UTIME_NOW and UTIME_OMIT values in its utimens operation.
     */
    unsigned int flag_utime_omit_ok [[maybe_unused]] : 1;

    /**
     * Reserved flags, don't set
     */
    unsigned int flag_reserved [[maybe_unused]] : 29;

    /**
     * Ioctl
     *
     * flags will have FUSE_IOCTL_COMPAT set for 32bit ioctls in
     * 64bit environment.  The size and direction of data is
     * determined by _IOC_*() decoding of cmd.  For _IOC_NONE,
     * data will be NULL, for _IOC_WRITE data is out area, for
     * _IOC_READ in area and if both are set in/out area.  In all
     * non-NULL cases, the area is of _IOC_SIZE(cmd) bytes.
     *
     * If flags has FUSE_IOCTL_DIR then the fuse_file_info refers to a
     * directory file handle.
     */
    virtual int ioctl(const std::string &pathname, int cmd, void *arg, struct fuse_file_info *fi, unsigned int flags,
                      void *data);

    /**
     * Poll for IO readiness events
     *
     * Note: If ph is non-NULL, the client should notify
     * when IO readiness events occur by calling
     * fuse_notify_poll() with the specified ph.
     *
     * Regardless of the number of times poll with a non-NULL ph
     * is received, single notification is enough to clear all.
     * Notifying more times incurs overhead but doesn't harm
     * correctness.
     *
     * The callee is responsible for destroying ph with
     * fuse_pollhandle_destroy() when no longer in use.
     */
    virtual int poll(const std::string &pathname, struct fuse_file_info *fi, struct fuse_pollhandle *ph,
                     unsigned *reventsp);

    /** Write contents of buffer to an open file
     *
     * Similar to the write() method, but data is supplied in a
     * generic buffer.  Use fuse_buf_copy() to transfer data to
     * the destination.
     */
    virtual int write_buf(const std::string &pathname, struct fuse_bufvec *buf, off_t off, struct fuse_file_info *fi);

    /** Store data from an open file in a buffer
     *
     * Similar to the read() method, but data is stored and
     * returned in a generic buffer.
     *
     * No actual copying of data has to take place, the source
     * file descriptor may simply be stored in the buffer for
     * later data transfer.
     *
     * The buffer must be allocated dynamically and stored at the
     * location pointed to by bufp.  If the buffer contains memory
     * regions, they too must be allocated using malloc().  The
     * allocated memory will be freed by the caller.
     */
    virtual int read_buf(const std::string &pathname, struct fuse_bufvec **bufp, size_t size, off_t off,
                         struct fuse_file_info *fi);

    /**
     * Perform BSD file locking operation
     *
     * The op argument will be either LOCK_SH, LOCK_EX or LOCK_UN
     *
     * Nonblocking requests will be indicated by ORing LOCK_NB to
     * the above operations
     *
     * For more information see the flock(2) manual page.
     *
     * Additionally fi->owner will be set to a value unique to
     * this open file.  This same value will be supplied to
     * ->release() when the file is released.
     *
     * Note: if this method is not implemented, the kernel will still
     * allow file locking to work locally.  Hence it is only
     * interesting for network filesystems and similar.
     */
    virtual int flock(const std::string &pathname, struct fuse_file_info *fi, int op);

    /**
     * Allocates space for an open file
     *
     * This function ensures that required space is allocated for specified
     * file.  If this function returns success then any subsequent write
     * request to specified range is guaranteed not to fail because of lack
     * of space on the file system media.
     */
    virtual int fallocate(const std::string &pathname, int mode, off_t offset, off_t len, struct fuse_file_info *fi);

private:
    static struct fuse_operations ops;

    class detail;
    friend class detail;
};

#endif
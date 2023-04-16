#ifndef SRC_ENCRYPTION_DECORATOR_H
#define SRC_ENCRYPTION_DECORATOR_H

#include "cryptopp/aes.h"
#include "vfs_decorator.h"

class EncryptionVfs : public VfsDecorator {
public:
    explicit EncryptionVfs(std::shared_ptr<CustomVfs> wrapped_vfs);

    int read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) override;
    int write(const std::string &pathname, const char *buf, size_t count, off_t offset,
              struct fuse_file_info *fi) override;

private:
    CryptoPP::byte key_[CryptoPP::AES::DEFAULT_KEYLENGTH];
    CryptoPP::byte iv_[CryptoPP::AES::BLOCKSIZE];
};

#endif  // SRC_ENCRYPTION_DECORATOR_H
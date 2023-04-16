#include "encryption_decorator.h"

#include <iostream>

#include "cryptopp/cryptlib.h"
#include "cryptopp/filters.h"
#include "cryptopp/modes.h"
#include "cryptopp/osrng.h"
#include "custom_vfs.h"

EncryptionVfs::EncryptionVfs(std::shared_ptr<CustomVfs> wrapped_vfs)
    : VfsDecorator(std::move(wrapped_vfs)) {  // NOLINT(cppcoreguidelines-pro-type-member-init)
    CryptoPP::AutoSeededRandomPool prng;

    // TODO the key would be stored in a file prepared by some tool

    prng.GenerateBlock(key_, sizeof(key_));
    prng.GenerateBlock(iv_, sizeof(iv_));
}

int EncryptionVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    int result = wrapped_vfs_->read(pathname, buf, count, offset, fi);

    if (result >= 0) {
        try {
            CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryption(key_, sizeof(key_), iv_);
            CryptoPP::StreamTransformationFilter decryptor(decryption,
                                                           new CryptoPP::ArraySink((unsigned char *)buf, count));
            decryptor.Put((unsigned char *)buf, static_cast<size_t>(result));
            decryptor.MessageEnd();

        } catch (const CryptoPP::Exception &ex) {
            std::cerr << "Error decrypting data: " << ex.what() << std::endl;
            return -EIO;
        }
    }

    return result;
}

int EncryptionVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    std::vector<char> encrypted_buf(count);

    try {
        CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption encryption(key_, sizeof(key_), iv_);
        CryptoPP::StreamTransformationFilter encryptor(
            encryption, new CryptoPP::ArraySink((unsigned char *)encrypted_buf.data(), count));
        encryptor.Put((unsigned char *)buf, count);
        encryptor.MessageEnd();
    } catch (const CryptoPP::Exception &ex) {
        std::cerr << "Error encrypting data: " << ex.what() << std::endl;
        return -EIO;
    }

    return wrapped_vfs_->write(pathname, encrypted_buf.data(), count, offset, fi);
}
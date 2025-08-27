#ifndef ENCRYPTION_MANAGER_H
#define ENCRYPTION_MANAGER_H

#include <string>
#include <vector>
// Use extern "C" for C library headers in C++
#ifdef __cplusplus
extern "C" {
#endif
#include <sodium.h>
#ifdef __cplusplus
}
#endif

class EncryptionManager {
public:
    // Derives a key from a passphrase and salt using Argon2
    static std::vector<unsigned char> deriveKey(const std::string& passphrase, const std::string& salt, uint32_t derived_key_len);

public:
    // Size of the nonce (number once) for secretbox
    static const size_t NONCE_SIZE = crypto_secretbox_NONCEBYTES;
    static const size_t HMAC_KEY_SIZE = crypto_auth_hmacsha256_KEYBYTES;
    static const size_t HMAC_TAG_SIZE = crypto_auth_hmacsha256_BYTES;

    // Generates a random key for encryption
    std::vector<unsigned char> generateKey();

    // Generates a random key for HMAC
    std::vector<unsigned char> generateHmacKey();

    // Encrypts data using a key and returns ciphertext with prepended nonce and appended HMAC tag
    std::vector<unsigned char> encrypt(const std::vector<unsigned char>& data, const std::vector<unsigned char>& key, const std::vector<unsigned char>& hmac_key);

    // Decrypts data using a key, expecting nonce to be prepended and HMAC tag appended
    std::vector<unsigned char> decrypt(const std::vector<unsigned char>& encrypted_data_with_nonce_and_hmac, const std::vector<unsigned char>& key, const std::vector<unsigned char>& hmac_key);
};

#endif // ENCRYPTION_MANAGER_H
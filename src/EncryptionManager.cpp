#include "EncryptionManager.h"
#include <iostream>
#include <algorithm> // For std::copy

// Use extern "C" for C library headers in C++
#ifdef __cplusplus
extern "C" {
#endif
#include <sodium.h>
#ifdef __cplusplus
}
#endif

std::vector<unsigned char> EncryptionManager::generateKey() {
    std::vector<unsigned char> key(crypto_secretbox_KEYBYTES);
    randombytes_buf(key.data(), key.size());
    return key;
}

std::vector<unsigned char> EncryptionManager::generateHmacKey() {
    std::vector<unsigned char> hmac_key(HMAC_KEY_SIZE);
    crypto_auth_hmacsha256_keygen(hmac_key.data());
    return hmac_key;
}

std::vector<unsigned char> EncryptionManager::encrypt(const std::vector<unsigned char>& data, const std::vector<unsigned char>& key, const std::vector<unsigned char>& hmac_key) {
    if (key.size() != crypto_secretbox_KEYBYTES) {
        std::cerr << "Error: Invalid encryption key size." << std::endl;
        return {};
    }
    if (hmac_key.size() != HMAC_KEY_SIZE) {
        std::cerr << "Error: Invalid HMAC key size." << std::endl;
        return {};
    }

    std::vector<unsigned char> nonce(NONCE_SIZE);
    randombytes_buf(nonce.data(), nonce.size());

    std::vector<unsigned char> ciphertext(data.size() + crypto_secretbox_MACBYTES);

    if (crypto_secretbox_easy(ciphertext.data(), data.data(), data.size(), nonce.data(), key.data()) != 0) {
        std::cerr << "Error: libsodium encryption failed." << std::endl;
        return {};
    }

    // Compute HMAC of the ciphertext
    std::vector<unsigned char> hmac_tag(HMAC_TAG_SIZE);
    if (crypto_auth_hmacsha256(hmac_tag.data(), ciphertext.data(), ciphertext.size(), hmac_key.data()) != 0) {
        std::cerr << "Error: libsodium HMAC computation failed." << std::endl;
        return {};
    }

    // Prepend nonce to ciphertext and append HMAC tag
    std::vector<unsigned char> encrypted_data_with_nonce_and_hmac;
    encrypted_data_with_nonce_and_hmac.reserve(nonce.size() + ciphertext.size() + hmac_tag.size());
    encrypted_data_with_nonce_and_hmac.insert(encrypted_data_with_nonce_and_hmac.end(), nonce.begin(), nonce.end());
    encrypted_data_with_nonce_and_hmac.insert(encrypted_data_with_nonce_and_hmac.end(), ciphertext.begin(), ciphertext.end());
    encrypted_data_with_nonce_and_hmac.insert(encrypted_data_with_nonce_and_hmac.end(), hmac_tag.begin(), hmac_tag.end());

    return encrypted_data_with_nonce_and_hmac;
}

std::vector<unsigned char> EncryptionManager::decrypt(const std::vector<unsigned char>& encrypted_data_with_nonce_and_hmac, const std::vector<unsigned char>& key, const std::vector<unsigned char>& hmac_key) {
    if (key.size() != crypto_secretbox_KEYBYTES) {
        std::cerr << "Error: Invalid encryption key size." << std::endl;
        return {};
    }
    if (hmac_key.size() != HMAC_KEY_SIZE) {
        std::cerr << "Error: Invalid HMAC key size." << std::endl;
        return {};
    }
    if (encrypted_data_with_nonce_and_hmac.size() < NONCE_SIZE + crypto_secretbox_MACBYTES + HMAC_TAG_SIZE) {
        std::cerr << "Error: Encrypted data too short for decryption and HMAC verification." << std::endl;
        return {};
    }

    // Extract nonce, ciphertext, and HMAC tag
    std::vector<unsigned char> nonce(NONCE_SIZE);
    std::copy(encrypted_data_with_nonce_and_hmac.begin(), encrypted_data_with_nonce_and_hmac.begin() + NONCE_SIZE, nonce.begin());

    std::vector<unsigned char> ciphertext(encrypted_data_with_nonce_and_hmac.begin() + NONCE_SIZE, encrypted_data_with_nonce_and_hmac.end() - HMAC_TAG_SIZE);

    std::vector<unsigned char> received_hmac_tag(HMAC_TAG_SIZE);
    std::copy(encrypted_data_with_nonce_and_hmac.end() - HMAC_TAG_SIZE, encrypted_data_with_nonce_and_hmac.end(), received_hmac_tag.begin());

    // Verify HMAC
    if (crypto_auth_hmacsha256_verify(received_hmac_tag.data(), ciphertext.data(), ciphertext.size(), hmac_key.data()) != 0) {
        std::cerr << "Error: HMAC verification failed. Data may be tampered or corrupted." << std::endl;
        return {};
    }

    // If HMAC is valid, proceed with decryption
    std::vector<unsigned char> decrypted_data(ciphertext.size() - crypto_secretbox_MACBYTES);
    if (crypto_secretbox_open_easy(decrypted_data.data(), ciphertext.data(), ciphertext.size(), nonce.data(), key.data()) != 0) {
        std::cerr << "Error: libsodium decryption failed (bad key or corrupted data)." << std::endl;
        return {};
    }

    return decrypted_data;
}

std::vector<unsigned char> EncryptionManager::deriveKey(const std::string& passphrase, const std::string& salt, uint32_t derived_key_len) {
    std::vector<unsigned char> derived_key(derived_key_len);
    if (crypto_pwhash(derived_key.data(), derived_key.size(),
                     passphrase.c_str(), passphrase.length(),
                     (const unsigned char*)salt.c_str(),
                     crypto_pwhash_OPSLIMIT_INTERACTIVE,
                     crypto_pwhash_MEMLIMIT_INTERACTIVE,
                     crypto_pwhash_ALG_DEFAULT) != 0) {
        std::cerr << "Error deriving key with Argon2." << std::endl;
        return {};
    }
    return derived_key;
}
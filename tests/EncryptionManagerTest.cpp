#include <gtest/gtest.h>
#include "EncryptionManager.h"
#include <vector>
#include <string>

// Test fixture for EncryptionManager
class EncryptionManagerTest : public ::testing::Test {
protected:
    EncryptionManager em;
};

TEST_F(EncryptionManagerTest, GenerateKeyReturnsCorrectSize) {
    std::vector<unsigned char> key = em.generateKey();
    ASSERT_EQ(key.size(), EncryptionManager::crypto_secretbox_KEYBYTES);
}

TEST_F(EncryptionManagerTest, GenerateHmacKeyReturnsCorrectSize) {
    std::vector<unsigned char> hmac_key = em.generateHmacKey();
    ASSERT_EQ(hmac_key.size(), EncryptionManager::HMAC_KEY_SIZE);
}

TEST_F(EncryptionManagerTest, DeriveKeyProducesConsistentOutput) {
    std::string passphrase = "mysecretpassword";
    std::string salt = "somesalt";
    uint32_t derived_key_len = 32;

    std::vector<unsigned char> key1 = EncryptionManager::deriveKey(passphrase, salt, derived_key_len);
    std::vector<unsigned char> key2 = EncryptionManager::deriveKey(passphrase, salt, derived_key_len);

    ASSERT_EQ(key1.size(), derived_key_len);
    ASSERT_EQ(key1, key2); // Derived key should be consistent for same input
}

TEST_F(EncryptionManagerTest, EncryptDecryptRoundTrip) {
    std::vector<unsigned char> original_data = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
    std::vector<unsigned char> key = em.generateKey();
    std::vector<unsigned char> hmac_key = em.generateHmacKey();

    std::vector<unsigned char> encrypted_data = em.encrypt(original_data, key, hmac_key);
    ASSERT_FALSE(encrypted_data.empty());

    std::vector<unsigned char> decrypted_data = em.decrypt(encrypted_data, key, hmac_key);
    ASSERT_FALSE(decrypted_data.empty());
    ASSERT_EQ(original_data, decrypted_data);
}

TEST_F(EncryptionManagerTest, DecryptFailsWithWrongKey) {
    std::vector<unsigned char> original_data = {'T', 'e', 's', 't', ' ', 'D', 'a', 't', 'a'};
    std::vector<unsigned char> key = em.generateKey();
    std::vector<unsigned char> hmac_key = em.generateHmacKey();

    std::vector<unsigned char> encrypted_data = em.encrypt(original_data, key, hmac_key);
    ASSERT_FALSE(encrypted_data.empty());

    std::vector<unsigned char> wrong_key = em.generateKey(); // Different key
    std::vector<unsigned char> decrypted_data = em.decrypt(encrypted_data, wrong_key, hmac_key);
    ASSERT_TRUE(decrypted_data.empty()); // Decryption should fail
}

TEST_F(EncryptionManagerTest, DecryptFailsWithWrongHmacKey) {
    std::vector<unsigned char> original_data = {'S', 'e', 'c', 'u', 'r', 'e', ' ', 'M', 'e', 's', 's', 'a', 'g', 'e'};
    std::vector<unsigned char> key = em.generateKey();
    std::vector<unsigned char> hmac_key = em.generateHmacKey();

    std::vector<unsigned char> encrypted_data = em.encrypt(original_data, key, hmac_key);
    ASSERT_FALSE(encrypted_data.empty());

    std::vector<unsigned char> wrong_hmac_key = em.generateHmacKey(); // Different HMAC key
    std::vector<unsigned char> decrypted_data = em.decrypt(encrypted_data, key, wrong_hmac_key);
    ASSERT_TRUE(decrypted_data.empty()); // Decryption should fail due to HMAC mismatch
}

TEST_F(EncryptionManagerTest, DecryptFailsWithTamperedData) {
    std::vector<unsigned char> original_data = {'I', 'n', 't', 'e', 'g', 'r', 'i', 't', 'y', ' ', 'T', 'e', 's', 't'};
    std::vector<unsigned char> key = em.generateKey();
    std::vector<unsigned char> hmac_key = em.generateHmacKey();

    std::vector<unsigned char> encrypted_data = em.encrypt(original_data, key, hmac_key);
    ASSERT_FALSE(encrypted_data.empty());

    // Tamper with the encrypted data (e.g., change a byte in the ciphertext)
    if (encrypted_data.size() > EncryptionManager::NONCE_SIZE) {
        encrypted_data[EncryptionManager::NONCE_SIZE + 5] ^= 0x01; // Flip a bit in ciphertext
    }

    std::vector<unsigned char> decrypted_data = em.decrypt(encrypted_data, key, hmac_key);
    ASSERT_TRUE(decrypted_data.empty()); // Decryption should fail due to HMAC mismatch
}

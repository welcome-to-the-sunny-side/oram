#pragma once

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

#include <string>
#include <vector>
#include <stdexcept>


namespace oram_lib
{
    class encryptor
    {
    private:
        static const int KEY_SIZE = 32;        // 256-bit key
        static const int IV_SIZE = 16;         // 128-bit IV for AES
        static std::vector<unsigned char> key; // Shared key for all blocks

    public:
        static void initialize()
        {
            // Generate a random key on startup
            key.resize(KEY_SIZE);
            RAND_bytes(key.data(), KEY_SIZE);
        }

        static std::string encrypt(const std::string &plaintext)
        {
            std::vector<unsigned char> iv(IV_SIZE);
            RAND_bytes(iv.data(), IV_SIZE);

            EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
            EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data());

            std::vector<unsigned char> ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH);
            int len1, len2;
            EVP_EncryptUpdate(ctx, ciphertext.data(), &len1,
                            (unsigned char *)plaintext.data(), plaintext.size());
            EVP_EncryptFinal_ex(ctx, ciphertext.data() + len1, &len2);
            EVP_CIPHER_CTX_free(ctx);

            // Prepend IV to ciphertext
            std::string result;
            result.reserve(IV_SIZE + len1 + len2);
            result.append((char *)iv.data(), IV_SIZE);
            result.append((char *)ciphertext.data(), len1 + len2);
            return result;
        }

        static std::string decrypt(const std::string &ciphertext)
        {
            if (ciphertext.size() < IV_SIZE)
            {
                throw std::runtime_error("Invalid ciphertext");
            }

            // Extract IV from the beginning of ciphertext
            std::vector<unsigned char> iv(ciphertext.begin(), ciphertext.begin() + IV_SIZE);

            EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
            EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data());

            std::vector<unsigned char> plaintext(ciphertext.size() - IV_SIZE);
            int len1, len2;
            EVP_DecryptUpdate(ctx, plaintext.data(), &len1,
                            (unsigned char *)ciphertext.data() + IV_SIZE, ciphertext.size() - IV_SIZE);
            EVP_DecryptFinal_ex(ctx, plaintext.data() + len1, &len2);
            EVP_CIPHER_CTX_free(ctx);

            return std::string((char *)plaintext.data(), len1 + len2);
        }
    };
}
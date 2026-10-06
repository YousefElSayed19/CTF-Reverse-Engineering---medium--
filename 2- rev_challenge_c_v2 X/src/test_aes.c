#include <stdio.h>
#include <string.h>
#include "aes.h"

static void print_hex(const uint8_t* buf, size_t len) {
    for (size_t i = 0; i < len; i++) printf("%02x", buf[i]);
    printf("\n");
}

int main() {
    // NIST AES-128 CBC test vector (FIPS-197 / SP800-38A)
    uint8_t key[16] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,
                        0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
    uint8_t iv[16]  = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
                        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    uint8_t plaintext[16] = {0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,
                              0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a};
    // Expected ciphertext from NIST SP800-38A CBC-AES128.Encrypt, block 1
    uint8_t expected[16] = {0x76,0x49,0xab,0xac,0x81,0x19,0xb2,0x46,
                             0xce,0xe9,0x8e,0x9b,0x12,0xe9,0x19,0x7d};

    uint8_t buf[16];
    memcpy(buf, plaintext, 16);

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_encrypt_buffer(&ctx, buf, 16);

    printf("Computed ciphertext: "); print_hex(buf, 16);
    printf("Expected ciphertext: "); print_hex(expected, 16);

    if (memcmp(buf, expected, 16) == 0) {
        printf("[PASS] Encryption matches NIST test vector\n");
    } else {
        printf("[FAIL] Encryption mismatch!\n");
        return 1;
    }

    // Now test decrypt roundtrip
    AES_ctx_set_iv(&ctx, iv);
    AES_CBC_decrypt_buffer(&ctx, buf, 16);
    printf("Decrypted:            "); print_hex(buf, 16);

    if (memcmp(buf, plaintext, 16) == 0) {
        printf("[PASS] Decryption roundtrip matches original plaintext\n");
    } else {
        printf("[FAIL] Decryption mismatch!\n");
        return 1;
    }

    return 0;
}

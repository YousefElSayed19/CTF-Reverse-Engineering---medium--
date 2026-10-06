#include <stdio.h>
#include <string.h>
#include "aes.h"
#include "base64.h"

int main() {
    // ---- PASSWORD_1: base64(xor(password, 0x4B)) ----
    const char *password1 = "r3v3rs3_m3_pl2";
    unsigned char buf1[64];
    int len1 = (int)strlen(password1);
    memcpy(buf1, password1, len1);
    for (int i = 0; i < len1; i++) buf1[i] ^= 0x4B;

    char b64out[128];
    b64_encode(buf1, len1, b64out);
    printf("ENCODED_PASSWORD_1 = \"%s\"\n", b64out);

    // ---- PASSWORD_2: AES-128-CBC(key, iv, pkcs7_pad(password2)) ----
    const unsigned char key[16] = {
        0x4f,0x75,0x5f,0x4b,0x33,0x79,0x5f,0x46,
        0x30,0x72,0x5f,0x47,0x32,0x5f,0x21,0x21
    };
    const unsigned char iv[16] = {
        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
        0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10
    };
    const char *password2 = "ch3ck3r_unl0ck_key"; // 19 chars
    int p2len = (int)strlen(password2);
    int padded_len = ((p2len / 16) + 1) * 16; // always pad at least 1 byte
    unsigned char plain[64];
    memcpy(plain, password2, p2len);
    int pad_val = padded_len - p2len;
    for (int i = p2len; i < padded_len; i++) plain[i] = (unsigned char)pad_val;

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_encrypt_buffer(&ctx, plain, padded_len);

    printf("P2_CIPHERTEXT_LEN = %d\n", padded_len);
    printf("static unsigned char P2_CIPHERTEXT[%d] = {\n    ", padded_len);
    for (int i = 0; i < padded_len; i++) {
        printf("0x%02x", plain[i]);
        if (i != padded_len - 1) printf(",");
        if ((i+1) % 8 == 0) printf("\n    ");
    }
    printf("\n};\n");

    return 0;
}

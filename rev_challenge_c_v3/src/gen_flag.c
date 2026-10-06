#include <stdio.h>
#include <string.h>
#include "aes.h"

static void derive_key(const char *password, unsigned char *key_out) {
    memset(key_out, 0, 16);
    int len = (int)strlen(password);
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        int idx = i % 16;
        key_out[idx] ^= c;
        key_out[idx] = (unsigned char)((key_out[idx] << 1) | (key_out[idx] >> 7));
    }
    key_out[len % 16] ^= (unsigned char)(len & 0xFF);
}

int main() {
    const char *password2 = "ch3ck3r_unl0ck_key";
    const char *flag = "duck{4yBlGzSTp+5sP4Q3!eG#fE$gHGYjkpQQ}";

    unsigned char key[16];
    derive_key(password2, key);

    const unsigned char iv[16] = {
        0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
        0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20
    };

    int flen = (int)strlen(flag);
    int padded_len = ((flen / 16) + 1) * 16;
    unsigned char plain[64];
    memcpy(plain, flag, flen);
    int pad_val = padded_len - flen;
    for (int i = flen; i < padded_len; i++) plain[i] = (unsigned char)pad_val;

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_encrypt_buffer(&ctx, plain, padded_len);

    printf("FLAG_CIPHERTEXT_LEN = %d\n", padded_len);
    printf("static unsigned char FLAG_CIPHERTEXT[%d] = {\n    ", padded_len);
    for (int i = 0; i < padded_len; i++) {
        printf("0x%02x", plain[i]);
        if (i != padded_len - 1) printf(",");
        if ((i+1) % 8 == 0) printf("\n    ");
    }
    printf("\n};\n");

    return 0;
}

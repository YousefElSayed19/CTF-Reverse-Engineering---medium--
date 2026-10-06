#include <stdio.h>
#include <string.h>
#include "base64.h"

static const unsigned char XOR_KEY[] = { 0x4B, 0x33, 0x79, 0x21 };
#define XOR_KEY_LEN 4
#define MASK_BYTE 0x01

static void xor_repeating(unsigned char *data, int len, const unsigned char *key, int key_len) {
    for (int i = 0; i < len; i++) data[i] ^= key[i % key_len];
}

int main() {
    const char *password1 = "r3v3rs3_m3_pl2";
    int len1 = (int)strlen(password1);

    unsigned char buf1[64];
    memcpy(buf1, password1, len1);
    xor_repeating(buf1, len1, XOR_KEY, XOR_KEY_LEN);

    char b64out[128];
    b64_encode(buf1, len1, b64out);
    int b64len = (int)strlen(b64out);

    printf("base64 (before masking) = \"%s\" (len=%d)\n", b64out, b64len);

    unsigned char masked[128];
    for (int i = 0; i < b64len; i++) {
        masked[i] = (unsigned char)b64out[i] ^ MASK_BYTE;
    }

    printf("\nstatic const unsigned char MASKED_B64[] = {\n    ");
    for (int i = 0; i < b64len; i++) {
        printf("0x%02x", masked[i]);
        if (i != b64len - 1) printf(",");
        if ((i+1) % 14 == 0) printf("\n    ");
    }
    printf("\n};\n#define MASKED_B64_LEN %d\n", b64len);

    return 0;
}

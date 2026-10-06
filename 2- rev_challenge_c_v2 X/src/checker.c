// checker.c
// =========================================================================
// Compile with (MinGW on Windows):
//   gcc -o checker.exe checker.c aes.c -Wall
//
// Flow:
//   The real flag is stored ONLY as an AES-128-CBC ciphertext embedded in
//   this binary. The AES key is derived directly from the password the
//   player received from game.exe (PASSWORD_2), padded/truncated to 16
//   bytes. Without the correct password, decryption produces garbage
//   bytes -- there is no valid flag to recover through static analysis
//   of this binary alone.
// =========================================================================

#include <stdio.h>
#include <string.h>
#include "aes.h"

// Fixed IV embedded in the binary -- IVs are not secret in AES-CBC, only
// the key matters for confidentiality here.
static const unsigned char FLAG_IV[16] = {
    0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
    0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20
};

// AES-CBC ciphertext of the real flag, PKCS7-padded to a 16-byte multiple.
static unsigned char FLAG_CIPHERTEXT[48] = {
    0xc2,0xfb,0x59,0x7c,0x94,0x74,0xeb,0x47,
    0x6a,0xf9,0x75,0xb1,0x12,0x10,0xa3,0x5a,
    0xd0,0x6b,0x19,0x7e,0x66,0x26,0xa9,0xee,
    0xc4,0xe4,0x84,0xef,0x8d,0xf9,0xd4,0x5d,
    0xd9,0x08,0x1c,0x20,0x30,0x26,0x14,0x40,
    0xb2,0xbc,0x67,0x7d,0xa3,0xd0,0xb0,0x64
};
static int FLAG_CIPHERTEXT_LEN = 48;

// Build a 16-byte AES key from the password string. Folds EVERY character
// of the password into the key (not just a 16-byte prefix), so passwords
// that only share a prefix or add extra trailing characters produce a
// completely different key instead of accidentally matching.
static void derive_key(const char *password, unsigned char *key_out) {
    memset(key_out, 0, 16);
    int len = (int)strlen(password);
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        int idx = i % 16;
        key_out[idx] ^= c;
        key_out[idx] = (unsigned char)((key_out[idx] << 1) | (key_out[idx] >> 7)); // rotate left 1
    }
    // Mix in the password length so pure-rotation collisions are harder too.
    key_out[len % 16] ^= (unsigned char)(len & 0xFF);
}

// Very small sanity check: does the decrypted buffer look like a
// plausible flag (starts with "duck{" and ends with "}")? This lets us
// tell the player "wrong password" without ever comparing against the
// real flag in plaintext anywhere in this file.
static int looks_like_flag(const unsigned char *buf, int len) {
    if (len < 6) return 0;
    if (memcmp(buf, "duck{", 5) != 0) return 0;
    if (buf[len - 1] != '}') return 0;
    for (int i = 0; i < len; i++) {
        if (buf[i] < 0x20 || buf[i] > 0x7e) return 0; // must be printable
    }
    return 1;
}

int main(void) {
    char password[128];
    unsigned char key[16];
    unsigned char buf[64];

    printf("============================================================\n");
    printf(" checker.exe - Flag Verification Tool\n");
    printf("============================================================\n");
    printf("Enter unlock password: ");
    if (!fgets(password, sizeof(password), stdin)) return 1;
    password[strcspn(password, "\n")] = 0;

    derive_key(password, key);

    memcpy(buf, FLAG_CIPHERTEXT, FLAG_CIPHERTEXT_LEN);

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, FLAG_IV);
    AES_CBC_decrypt_buffer(&ctx, buf, FLAG_CIPHERTEXT_LEN);

    // Remove PKCS7 padding (only meaningful if password was correct)
    int pad = buf[FLAG_CIPHERTEXT_LEN - 1];
    int plain_len = FLAG_CIPHERTEXT_LEN - pad;
    if (pad < 1 || pad > 16 || plain_len < 0 || plain_len > FLAG_CIPHERTEXT_LEN) {
        printf("\n[-] Invalid password. Cannot unlock verification logic.\n");
        return 1;
    }

    if (!looks_like_flag(buf, plain_len)) {
        printf("\n[-] Invalid password. Cannot unlock verification logic.\n");
        return 1;
    }

    buf[plain_len] = '\0';
    printf("\n[+] Correct! You have successfully solved the Reverse Engineering challenge.\n");
    printf("[+] FLAG: %s\n", buf);

    return 0;
}

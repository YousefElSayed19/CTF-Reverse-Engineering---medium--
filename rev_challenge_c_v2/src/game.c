// game.c
// =========================================================================
// Compile with (MinGW on Windows):
//   gcc -o game.exe game.c aes.c base64.c -Wall
//
// Flow:
//   1. Player reverse-engineers this binary to recover PASSWORD_1
//      (stored as base64(xor(real_password, KEY))).
//   2. After the correct password, a short knowledge quiz checks the
//      player actually traced the RE steps.
//   3. A tiny number-guessing mini-game follows.
//   4. Winning it reveals PASSWORD_2 (itself AES-128-CBC encrypted,
//      decrypted here with a fixed key/iv embedded in this binary just
//      for obfuscation of the string in a static disassembly view) -
//      PASSWORD_2 is the key needed for checker.exe.
// =========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aes.h"
#include "base64.h"

// -------------------------------------------------------------------------
// LAYER 1 + LAYER 2: XOR_KEY used to obfuscate PASSWORD_1 before base64.
// -------------------------------------------------------------------------
#define XOR_KEY 0x4B

// base64( xor("r3v3rs3_m3_pl2", 0x4B) )  -- precomputed, embedded as data
static const char *ENCODED_PASSWORD_1 = "OXg9eDk4eBQmeBQ7J3k=";

static void xor_buf(unsigned char *data, int len, unsigned char key) {
    for (int i = 0; i < len; i++) data[i] ^= key;
}

// Intentionally unrelated function name to avoid an obvious search target.
int verify_sys_integrity(const char *user_input) {
    unsigned char decoded[64];
    int decoded_len = b64_decode(ENCODED_PASSWORD_1, decoded, sizeof(decoded));
    if (decoded_len <= 0) return 0;
    xor_buf(decoded, decoded_len, XOR_KEY);
    decoded[decoded_len] = '\0';
    return strcmp((char*)decoded, user_input) == 0;
}

// -------------------------------------------------------------------------
// Quiz: proves the player traced the RE steps. Answers compared directly
// (case-sensitive) -- kept simple and readable for a Medium-difficulty
// challenge rather than hashed, since the binary itself is now the
// harder barrier (static analysis of compiled C, not readable Python).
// -------------------------------------------------------------------------
int run_quiz(void) {
    char answer[128];

    printf("\n--- Verification Quiz ---\n");
    printf("Answer based on what you found during your analysis.\n\n");

    printf("What is the XOR key used in the first decoding layer? (hex, e.g. 4B)\n> ");
    if (!fgets(answer, sizeof(answer), stdin)) return 0;
    answer[strcspn(answer, "\n")] = 0;
    if (strcmp(answer, "4B") != 0 && strcmp(answer, "4b") != 0) {
        printf("\n[-] Incorrect. Access denied.\n");
        return 0;
    }

    printf("Which was applied FIRST when encoding: XOR or Base64?\n> ");
    if (!fgets(answer, sizeof(answer), stdin)) return 0;
    answer[strcspn(answer, "\n")] = 0;
    if (strcmp(answer, "XOR") != 0 && strcmp(answer, "xor") != 0) {
        printf("\n[-] Incorrect. Access denied.\n");
        return 0;
    }

    printf("What is the name of the function that verifies the password?\n> ");
    if (!fgets(answer, sizeof(answer), stdin)) return 0;
    answer[strcspn(answer, "\n")] = 0;
    if (strcmp(answer, "verify_sys_integrity") != 0) {
        printf("\n[-] Incorrect. Access denied.\n");
        return 0;
    }

    return 1;
}

// -------------------------------------------------------------------------
// Mini-game: simple number guessing game, gatekeeper to PASSWORD_2.
// -------------------------------------------------------------------------
int run_mini_game(void) {
    char answer[16];
    int secret_number = 7;

    printf("\n--- Final Step: Mini Game ---\n");
    printf("Guess the secret number (1-10). You have 3 attempts.\n");

    for (int attempt = 1; attempt <= 3; attempt++) {
        printf("Attempt %d/3 > ", attempt);
        if (!fgets(answer, sizeof(answer), stdin)) return 0;
        if (atoi(answer) == secret_number) {
            printf("[+] Correct guess!\n");
            return 1;
        }
        printf("Wrong, try again.\n");
    }
    return 0;
}

// -------------------------------------------------------------------------
// PASSWORD_2: AES-128-CBC encrypted blob embedded in this binary.
// Key and IV are embedded too (this is static obfuscation, not a secret
// from this binary alone -- the real protection comes later, in
// checker.exe, which needs PASSWORD_2 as a key it does NOT already have).
// -------------------------------------------------------------------------
static const unsigned char P2_KEY[16] = {
    0x4f,0x75,0x5f,0x4b,0x33,0x79,0x5f,0x46,
    0x30,0x72,0x5f,0x47,0x32,0x5f,0x21,0x21
};
static const unsigned char P2_IV[16] = {
    0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
    0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10
};
// AES-CBC ciphertext of "ch3ck3r_unl0ck_k" + "ey\x02\x02" (16-byte PKCS7-padded block,
// 2 blocks total for "ch3ck3r_unl0ck_key"+padding) -- precomputed below.
static unsigned char P2_CIPHERTEXT[32] = {
    0x2d,0x0f,0x67,0xdb,0x8d,0x8d,0x62,0x4f,
    0x59,0xc2,0xad,0xd3,0x9f,0xdc,0xd4,0x4e,
    0xa8,0x8d,0x47,0xac,0x6c,0xc2,0x6f,0xbe,
    0x42,0xca,0xaf,0x19,0x24,0x99,0x75,0xb6
};
static int P2_CIPHERTEXT_LEN = 32;

void reveal_password_2(char *out, int out_size) {
    struct AES_ctx ctx;
    unsigned char buf[32];
    memcpy(buf, P2_CIPHERTEXT, P2_CIPHERTEXT_LEN);

    AES_init_ctx_iv(&ctx, P2_KEY, P2_IV);
    AES_CBC_decrypt_buffer(&ctx, buf, P2_CIPHERTEXT_LEN);

    // Remove PKCS7 padding
    int pad = buf[P2_CIPHERTEXT_LEN - 1];
    int plain_len = P2_CIPHERTEXT_LEN - pad;
    if (plain_len < 0 || plain_len >= out_size) plain_len = 0;
    memcpy(out, buf, plain_len);
    out[plain_len] = '\0';
}

// -------------------------------------------------------------------------
// Main flow
// -------------------------------------------------------------------------
int main(void) {
    char input[128];
    char password2[64];

    printf("============================================================\n");
    printf(" Welcome. Enter the password to continue.\n");
    printf("============================================================\n");
    printf("Password: ");
    if (!fgets(input, sizeof(input), stdin)) return 1;
    input[strcspn(input, "\n")] = 0;

    if (!verify_sys_integrity(input)) {
        printf("[-] Incorrect password. Goodbye.\n");
        return 1;
    }
    printf("\n[+] Password accepted.\n");

    if (!run_quiz()) {
        printf("[-] Quiz failed. Goodbye.\n");
        return 1;
    }
    printf("\n[+] Quiz passed!\n");

    if (!run_mini_game()) {
        printf("[-] Mini-game failed. Goodbye.\n");
        return 1;
    }

    printf("\n[+] You won the mini-game, but this is not the end...\n");
    reveal_password_2(password2, sizeof(password2));
    printf("[i] Here is a password you will need for the NEXT file (checker.exe):\n\n");
    printf("    %s\n", password2);
    printf("\n[i] This password alone is NOT the flag. Use it to unlock checker.exe.\n");

    return 0;
}

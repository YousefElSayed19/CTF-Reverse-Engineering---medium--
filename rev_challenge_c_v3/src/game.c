// game.c (v2 - Medium difficulty)
// =========================================================================
// Compile with (MinGW on Windows):
//   gcc -o game.exe game.c aes.c base64.c -Wall
//
// Difficulty additions over v1:
//   1. Multi-byte repeating-key XOR (instead of a single fixed key byte)
//   2. The base64 string itself is stored additionally masked with a
//      constant byte, so it does NOT appear as readable text to a plain
//      `strings` scan -- it only becomes a valid base64 string after an
//      extra XOR step at runtime.
//   3. A basic anti-debugging check: if a debugger is attached while
//      verify_sys_integrity() runs, the password check always fails,
//      regardless of what is typed -- forcing pure static analysis
//      (or a debugger-bypass technique) instead of naive step-through.
// =========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aes.h"
#include "base64.h"

#ifdef _WIN32
  #include <windows.h>
#else
  #include <sys/ptrace.h>
  #include <unistd.h>
#endif

// -------------------------------------------------------------------------
// Anti-debugging check.
// -------------------------------------------------------------------------
static int is_debugger_present(void) {
#ifdef _WIN32
    return IsDebuggerPresent() ? 1 : 0;
#else
    // Linux equivalent for cross-platform testing: a process can only be
    // ptrace-attached by one tracer at a time. If we can't trace
    // ourselves, something (e.g. gdb/strace) already is.
    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
        return 1;
    }
    ptrace(PTRACE_DETACH, 0, NULL, NULL);
    return 0;
#endif
}

// -------------------------------------------------------------------------
// LAYER 1: multi-byte repeating-key XOR used to obfuscate PASSWORD_1
// before base64 encoding it.
// -------------------------------------------------------------------------
static const unsigned char XOR_KEY[] = { 0x4B, 0x33, 0x79, 0x21 }; // "K3y!"
#define XOR_KEY_LEN 4

static void xor_repeating(unsigned char *data, int len, const unsigned char *key, int key_len) {
    for (int i = 0; i < len; i++) data[i] ^= key[i % key_len];
}

// -------------------------------------------------------------------------
// LAYER 2: the base64 string is stored additionally masked with a
// constant byte (MASK_BYTE), so it is NOT a readable string in the
// compiled binary's data section -- a plain `strings` scan will not show
// valid base64 text here. It must first be unmasked with MASK_BYTE,
// THEN treated as base64, THEN XOR-decoded with the repeating key above.
// -------------------------------------------------------------------------
#define MASK_BYTE 0x01

// Masked bytes of: base64( xor_repeating("r3v3rs3_m3_pl2", XOR_KEY) )
// (precomputed; each byte here is the real base64 character XOR'd with
// MASK_BYTE, so this does NOT read as plain text in a hex/strings view)
static const unsigned char MASKED_B64[] = {
    0x4e,0x50,0x40,0x51,0x44,0x6b,0x6d,0x40,0x52,0x6f,0x35,0x6c,0x40,0x42,
    0x5b,0x53,0x4b,0x76,0x44,0x3c
};
#define MASKED_B64_LEN 20

// Intentionally unrelated function name, as before.
int verify_sys_integrity(const char *user_input) {
    // Anti-debug gate: if a debugger is attached, always reject -- no
    // matter what the user typed -- without any visible difference in
    // the printed error message.
    if (is_debugger_present()) {
        return 0;
    }

    unsigned char unmasked_b64[MASKED_B64_LEN + 1];
    for (int i = 0; i < MASKED_B64_LEN; i++) {
        unmasked_b64[i] = MASKED_B64[i] ^ MASK_BYTE;
    }
    unmasked_b64[MASKED_B64_LEN] = '\0';

    unsigned char decoded[64];
    int decoded_len = b64_decode((const char*)unmasked_b64, decoded, sizeof(decoded));
    if (decoded_len <= 0) return 0;

    xor_repeating(decoded, decoded_len, XOR_KEY, XOR_KEY_LEN);
    decoded[decoded_len] = '\0';

    return strcmp((char*)decoded, user_input) == 0;
}

// -------------------------------------------------------------------------
// Quiz: proves the player traced the RE steps. Updated to reflect the
// new multi-key XOR scheme.
// -------------------------------------------------------------------------
int run_quiz(void) {
    char answer[128];

    printf("\n--- Verification Quiz ---\n");
    printf("Answer based on what you found during your analysis.\n\n");

    printf("How many distinct XOR key bytes are used in the repeating key? (number)\n> ");
    if (!fgets(answer, sizeof(answer), stdin)) return 0;
    answer[strcspn(answer, "\n")] = 0;
    if (strcmp(answer, "4") != 0) {
        printf("\n[-] Incorrect. Access denied.\n");
        return 0;
    }

    printf("What is the first byte of the XOR key, in hex? (e.g. 4B)\n> ");
    if (!fgets(answer, sizeof(answer), stdin)) return 0;
    answer[strcspn(answer, "\n")] = 0;
    if (strcmp(answer, "4B") != 0 && strcmp(answer, "4b") != 0) {
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
// Mini-game: unchanged from v1.
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
// PASSWORD_2: unchanged from v1 (AES-128-CBC encrypted blob).
// -------------------------------------------------------------------------
static const unsigned char P2_KEY[16] = {
    0x4f,0x75,0x5f,0x4b,0x33,0x79,0x5f,0x46,
    0x30,0x72,0x5f,0x47,0x32,0x5f,0x21,0x21
};
static const unsigned char P2_IV[16] = {
    0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
    0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10
};
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

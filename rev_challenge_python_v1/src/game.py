# game.py
# =========================================================================
# Compile this file into game.exe with:
#   pyinstaller --onefile --console game.py
#
# Flow:
#   1. Player must reverse-engineer this binary to recover PASSWORD_1
#      (it is base64(xor(real_password, KEY)) - stored as DECOY string below,
#      never in plaintext).
#   2. After entering the correct password, a short knowledge quiz checks
#      the player actually understood the RE steps (not just copy-pasted
#      a password from someone else).
#   3. After the quiz, a tiny number-guessing mini-game unlocks.
#   4. Winning the mini-game reveals PASSWORD_2, which is the decryption
#      key needed to reverse-engineer checker.exe and get the real flag.
# =========================================================================

import base64
import hashlib
import random

# -------------------------------------------------------------------------
# LAYER 1 + LAYER 2: the real password is XOR'd with a key, then base64
# encoded. This is what a player will find when running `strings` on the
# compiled exe. Decoding requires: base64-decode -> XOR with KEY.
# -------------------------------------------------------------------------

# XOR key used to obfuscate the real password before base64 encoding it.
# Change this value before building if you want a different key.
XOR_KEY = 0x4B

REAL_PASSWORD_1 = "r3v3rs3_m3_pl2"  # <-- the actual password the player must recover

def _xor_bytes(data: bytes, key: int) -> bytes:
    return bytes([b ^ key for b in data])

# Pre-compute the obfuscated blob exactly the way the player must reverse:
#   real_password -> XOR(key) -> base64 encode -> stored string
_ENCODED_PASSWORD = base64.b64encode(
    _xor_bytes(REAL_PASSWORD_1.encode(), XOR_KEY)
).decode()

# This is the "verify" function the player must locate during static/dynamic
# analysis. Its name is intentionally unrelated to "password" to force the
# player to actually trace program logic rather than just searching strings.
def verify_sys_integrity(user_input: str) -> bool:
    decoded_bytes = base64.b64decode(_ENCODED_PASSWORD)
    recovered = _xor_bytes(decoded_bytes, XOR_KEY).decode()
    return user_input == recovered


# -------------------------------------------------------------------------
# Knowledge-check quiz: proves the player actually traced the RE steps.
# Correct answers are stored as hashes, never as plaintext, so dumping the
# exe doesn't hand over the answers directly.
# -------------------------------------------------------------------------

QUIZ = [
    {
        "q": "What is the XOR key used in the first decoding layer? (answer in hex, e.g. 4B)",
        "answer_hash": hashlib.sha256(b"4B").hexdigest(),
    },
    {
        "q": "Which transformation was applied FIRST when the password was encoded: XOR or Base64?",
        "answer_hash": hashlib.sha256(b"XOR").hexdigest(),
    },
    {
        "q": "What is the name of the function that verifies the password?",
        "answer_hash": hashlib.sha256(b"verify_sys_integrity").hexdigest(),
    },
]

def run_quiz() -> bool:
    print("\n--- Verification Quiz ---")
    print("Answer the following based on what you found during your analysis.\n")
    for item in QUIZ:
        answer = input(item["q"] + "\n> ").strip()
        if hashlib.sha256(answer.encode()).hexdigest() != item["answer_hash"]:
            print("\n[-] Incorrect. Access denied.")
            return False
    return True


# -------------------------------------------------------------------------
# Mini-game: simple number guessing game, gatekeeper to PASSWORD_2.
# -------------------------------------------------------------------------

def run_mini_game() -> bool:
    print("\n--- Final Step: Mini Game ---")
    secret_number = 7
    print("Guess the secret number (1-10). You have 3 attempts.")
    for attempt in range(1, 4):
        guess = input(f"Attempt {attempt}/3 > ").strip()
        if guess.isdigit() and int(guess) == secret_number:
            print("[+] Correct guess!")
            return True
        print("Wrong, try again.")
    return False


# -------------------------------------------------------------------------
# PASSWORD_2: the decryption key for checker.exe. Obfuscated the same way
# (XOR + base64) with a DIFFERENT key than password 1, so this one also
# needs its own short reverse step if someone just dumps strings.
# -------------------------------------------------------------------------

PASSWORD_2_XOR_KEY = 0x6F
REAL_PASSWORD_2 = "ch3ck3r_unl0ck_key"  # <-- this is what unlocks checker.exe's logic

_ENCODED_PASSWORD_2 = base64.b64encode(
    _xor_bytes(REAL_PASSWORD_2.encode(), PASSWORD_2_XOR_KEY)
).decode()

def reveal_password_2() -> str:
    decoded_bytes = base64.b64decode(_ENCODED_PASSWORD_2)
    return _xor_bytes(decoded_bytes, PASSWORD_2_XOR_KEY).decode()


# -------------------------------------------------------------------------
# Main flow
# -------------------------------------------------------------------------

def main():
    print("=" * 60)
    print(" Welcome. Enter the password to continue.")
    print("=" * 60)

    user_pw = input("Password: ").strip()

    if not verify_sys_integrity(user_pw):
        print("[-] Incorrect password. Goodbye.")
        return

    print("\n[+] Password accepted.")

    if not run_quiz():
        print("[-] Quiz failed. Goodbye.")
        return

    print("\n[+] Quiz passed!")

    if not run_mini_game():
        print("[-] Mini-game failed. Goodbye.")
        return

    print("\n[+] You won the mini-game, but this is not the end...")
    print("[i] Here is a password you will need for the NEXT file (checker.exe):\n")
    print("    " + reveal_password_2())
    print("\n[i] This password alone is NOT the flag. Use it to unlock checker.exe.")


if __name__ == "__main__":
    main()

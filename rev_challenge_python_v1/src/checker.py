# checker.py
# =========================================================================
# Compile this file into checker.exe with:
#   pyinstaller --onefile --console checker.py
#
# Before building: run build_encrypt.py and paste its output below,
# replacing the ENCRYPTED_LOGIC placeholder.
#
# Without the correct password (PASSWORD_2 from game.exe), decryption
# fails and the real flag-checking logic (logic.py) never gets loaded
# into memory — so static analysis of this binary alone reveals nothing
# useful about the flag.
# =========================================================================

import base64
import hashlib
from cryptography.fernet import Fernet
from cryptography.fernet import InvalidToken

# ---- PASTE THE OUTPUT OF build_encrypt.py HERE -------------------------
ENCRYPTED_LOGIC = b'gAAAAABqwTDVUsN2rlcT4imxFvUQ5rg32py_Y8hQjGeJ79L9k0F5l6Ahj0mQF4Toe6LeH8v9HHtN1EFCy22ETDKptrYNpjJ7TNcmI3Hh9UXPe-kfKouVILhf5SD3UYU5PXKl4q7OAgPLdQLKsko6aANHFtj1zYTrt-15x_hS5asSrh7tk7wgVU49IVbPZXeVcAWKe3kBoBYN_OAvGw5bVQH01boPrKfEFMdgZa1lPppqQZodE85qI8vmBab34Gv0HTifQ4BMuiqBQNR40gCehmvGpnw3c7o5PumHdbegmeAUwF63WRJ4F_v0OO-WCGTPsIYskqIsBRlCClv7t7VijgwDQjTsAUVkRAJ4TA6rdkRfS1CHaOtjiNhIVZrR0NuI7fAjm8SUXal_R4TjxH0UpaHZ0TifTzcpELNEzrqH7kzp-ByvGQBLx5kIRFLCA1ioQRu4VAZyWV8CixSXyHltqRen97FLGcuvf_fMpzy8BFI5O9O1euSSMNKjCO7CS_VGHLkakFOfSjH3qi7vUWz0ZwqLzN9YyxQeQdnQifPggz6v8BsFRe_w10LBTmEDUQTQIwh-6_MK9FVk1q2E8lzbiFxSh9DfrrD4iUWasDjUxv34O2TMCeEa5ghEL2C4Qvfrf1wxY_Y59FXd0Nk3H2b1dnMaMySjUWm09MpappIn9nM_T7XuuwnwBX1171uij_bE6nvwhg-bmrONImkV3kMJ3oMcFQZFbagRQQ7oAX9pUXRN3NuDLBuwdJ36p6u6MgpPeqRo0gPv6GJJti7zvyHEXIOKzd_TRh9LDg6PV17_Y-gL49TxaDaD8F4Nn0akMY7RBYV0WFJBEydjdxy0xZlifuh-p2IdeOYs7prXZu_2cIqcjmY4ajHNiSz8B8of529lslEb6SalyojfPZOcI9QM9qd31-suZzNkSax_7t_GTGMVY_JvwpLCMtTp7EBfWP_v-za97wNevuoDiN9zU0mROOU_1awrcW2qNeJhDxuV80OZIAwDqPNG_DFc4U4F4PBuIyqKTxXo130l7IVQOFlLkwxC57XdDu2WyPWMEcmSJC1sPwjsMBsV1FjraWZvGWEj9A18QErivtwLhJJ77-mkQZaeNNV0zbVSfH4I_Ltdmd4M9o-DDWt3esgcpafc7w4f7kCiqmgj8IyxJCskUxhWs9pnjV4uiK45qpAGNmBDHhNqM-9qAnIvZtCs2nPL4z4hUWqleL-4ACpFTIGPDHiVZtfCCqpI4DYu71KFCKMVOKnMTyOUs3LIVKIpOppNY7EmjaIfN6rO7E6O726_BcKiIzQAr212rIncxb1O69jSYD1zeITf0JbLxQzNN8N39iGTzGa1T9gLCrMYjHjT4g0DNKvSPF3c2MgyGOZC3oijwo-K4IOIdUXpksi9rFPhXeieeXL6GSn0ssGMtaM8yyYoIF6FKi2hYK_9iYDyUhtCXq3tKp_eKB7ZikEeT-3M_a4-vVoH8sXI5MAOrXZkAy7JvVurlun3Vt-iVEMk6SQG3kcXcRFFP8Vw4_W_1ktb8JhtdEXSISMVxIl_5w-viPbCmLvz74MZnKG5J5C8iAjdZb58UB5x0Z777xQ7jgnGFjumvCbWPVbSVXekxFxioiurAHQKSrsFdsJtLA=='
# --------------------------------------------------------------------------


def derive_key(password: str) -> bytes:
    return base64.urlsafe_b64encode(hashlib.sha256(password.encode()).digest())


def main():
    print("=" * 60)
    print(" checker.exe — Flag Verification Tool")
    print("=" * 60)

    password = input("Enter unlock password: ").strip()

    try:
        key = derive_key(password)
        f = Fernet(key)
        decrypted_code = f.decrypt(ENCRYPTED_LOGIC)
    except InvalidToken:
        print("[-] Invalid password. Cannot unlock verification logic.")
        return
    except Exception as e:
        print(f"[-] Unexpected error: {e}")
        return

    print("[+] Password accepted. Verification logic unlocked.\n")

    # Execute the decrypted logic.py code in an isolated namespace.
    namespace = {}
    exec(decrypted_code, namespace)
    check_flag = namespace["check_flag"]
    get_success_message = namespace["get_success_message"]

    user_flag = input("Enter the flag: ").strip()

    if check_flag(user_flag):
        print("\n" + get_success_message())
    else:
        print("\n[-] Incorrect flag. Try again.")


if __name__ == "__main__":
    main()

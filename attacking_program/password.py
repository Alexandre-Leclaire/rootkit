import os
import uuid
import hashlib
import getpass
import sys

PASSWD_FILE = os.path.expanduser("~/.rt_passwd")

def hash_password(password):
    return hashlib.sha256(password.encode()).hexdigest()

def save_hashed_password(hash_str):
    with open(PASSWD_FILE, "w") as f:
        f.write(hash_str)

def read_hashed_password():
    with open(PASSWD_FILE, "r") as f:
        return f.read().strip()

def create_password():
    new_uuid = str(uuid.uuid4())
    print(f"[!] Please find the password: {new_uuid}")
    print(f"[!] Save it, we won't ever print it again.")
    hashed = hash_password(new_uuid)
    save_hashed_password(hashed)

def verify_password():
    stored_hash = read_hashed_password()
    user_input = getpass.getpass("[?] Password : ")
    input_hash = hash_password(user_input)
    if input_hash == stored_hash:
        print("[*] Access Granted")
        sys.exit(0)
    else:
        print("[!] Incorrect Password")
        sys.exit(1)

def main():
    if not os.path.exists(PASSWD_FILE):
        print("[!] Generating password")
        create_password()

    verify_password()

if __name__ == "__main__":
    main()

import hashlib
import secrets

def same_passwords_without_salt():
    user_password_hashes = {
        'frank_A': hashlib.sha256('kernel04'.encode('ascii')).hexdigest(),
        'frank_B': hashlib.sha256('kernel04'.encode('ascii')).hexdigest(),
        'frank_C': hashlib.sha256('island85'.encode('ascii')).hexdigest()
    }

    print('Same passwords without salt:')
    for user, password_hash in user_password_hashes.items():
        print(f'User: {user} | Password hash: {password_hash}')

def same_passwords_with_salt():
    users = ['frank_A', 'frank_B', 'frank_C']
    passwords = ['kernel04', 'kernel04', 'island85']
    salts = []
    password_hashes = []

    for i in range(len(users)):
        salt = secrets.token_bytes(16).hex()
        salts.append(salt)

        salted_password = salt + passwords[i]
        
        hashed_password = hashlib.sha256(salted_password.encode('ascii')).hexdigest()
        password_hashes.append(hashed_password)

    print('Same passwords with salt:')
    for user, salt, password_hash in zip(users, salts, password_hashes):
        print(f"User: {user} | Salt: {salt} | Password hash: {password_hash}")

same_passwords_without_salt()

print()

same_passwords_with_salt()

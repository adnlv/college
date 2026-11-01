import hashlib
import hmac
from argon2 import PasswordHasher, Type, exceptions

# 1. Vulnerable scheme: SHA256(random_salt || password)
legacy_username = 'frank_A'
password = 'kernel04'
wrong_password = 'kernel04X'

legacy_salt = '20b329b6383a1adb737b8f4a22a0d6c7'
legacy_hash = hashlib.sha256(
    (legacy_salt + password).encode('utf-8')).hexdigest()

print(f'Vulnerable scheme: SHA256(random_salt || password)')
print(f'User: {legacy_username}')
print(f'Stored salt (hex): {legacy_salt}')
print(f'Stored hash: {legacy_hash}\n')

# 2. Secure scheme: Argon2id (OWASP recommended parameters)
hasher = PasswordHasher(
    memory_cost=19456,  # 19 MiB
    time_cost=2,        # 2 iterations
    parallelism=1,      # 1 thread
    hash_len=32,        # 32 bytes
    salt_len=16,        # 16 bytes
    type=Type.ID
)

# 3. Transparent password migration mechanism
database = {
    legacy_username: {
        'scheme': 'sha256_salted',
        'salt': legacy_salt,
        'hash': legacy_hash,
    }
}


def authenticate_and_upgrade(user: str, candidate_pwd: str) -> bool:
    record = database.get(user)
    if not record:
        return False

    scheme = record.get('scheme')

    if scheme == 'sha256_salted':
        # Verify legacy hash using constant-time comparison
        calc_hash = hashlib.sha256(
            (record['salt'] + candidate_pwd).encode('utf-8')).hexdigest()
        if not hmac.compare_digest(calc_hash, record['hash']):
            return False

        # Password is correct; upgrade immediately to Argon2id
        new_hash = hasher.hash(candidate_pwd)
        database[user] = {
            'scheme': 'argon2id',
            'hash': new_hash,
        }
        print(f'[Migration] User {user} successfully migrated to Argon2id!')
        print(f'[Migration] New stored hash: {new_hash}')
        return True

    elif scheme == 'argon2id':
        try:
            return hasher.verify(record['hash'], candidate_pwd)
        except exceptions.VerifyMismatchError:
            return False

    return False


print('Simulating login attempts and migration:')
print('1. Attempt with wrong password on legacy record:')
login_wrong = authenticate_and_upgrade(legacy_username, wrong_password)
print(f'Login status: {login_wrong}')
print(f'Current DB scheme: {database[legacy_username]["scheme"]}\n')

print('2. Attempt with correct password on legacy record (triggers upgrade):')
login_success = authenticate_and_upgrade(legacy_username, password)
print(f'Login status: {login_success}')
print(f'Current DB scheme: {database[legacy_username]["scheme"]}\n')

print('3. Subsequent login attempt on migrated record:')
login_migrated = authenticate_and_upgrade(legacy_username, password)
print(f'Login status: {login_migrated}')
print(
    f'Wrong password verification: {authenticate_and_upgrade(legacy_username, wrong_password)}')

import time
from argon2 import PasswordHasher, Type, exceptions

password = 'kernel04'
wrong_password = password + 'X'

print(f'Password: {password}')
print(f'Wrong password: {wrong_password}\n')

configs = {
    'A': {'memory_cost': 8192, 'time_cost': 2, 'parallelism': 1},
    'B': {'memory_cost': 19456, 'time_cost': 2, 'parallelism': 1},
    'C': {'memory_cost': 19456, 'time_cost': 3, 'parallelism': 1},
}

hasher_b = PasswordHasher(
    memory_cost=configs['B']['memory_cost'],
    time_cost=configs['B']['time_cost'],
    parallelism=configs['B']['parallelism'],
    type=Type.ID,
)
hash1 = hasher_b.hash(password)
hash2 = hasher_b.hash(password)

print(f'Argon2id hash 1 (config B): {hash1}')
print(f'Argon2id hash 2 (config B): {hash2}')
print(f'Hashes are equal: {hash1 == hash2}\n')


def verify_password(hasher: PasswordHasher, hash_str: str, pwd: str) -> bool:
    try:
        return hasher.verify(hash_str, pwd)
    except exceptions.VerifyMismatchError:
        return False


is_valid = verify_password(hasher_b, hash1, password)
is_invalid = verify_password(hasher_b, hash1, wrong_password)

print(f'Verify correct password: {is_valid}')
print(f'Verify wrong password: {is_invalid}\n')


def measure_argon2_configs(
    pwd: str, configs: dict[str, dict[str, int]], repeats: int = 2
):
    for i, (name, params) in enumerate(configs.items()):
        hasher = PasswordHasher(
            memory_cost=params['memory_cost'],
            time_cost=params['time_cost'],
            parallelism=params['parallelism'],
            type=Type.ID,
        )
        hash_val = hasher.hash(pwd)
        print(
            f'Config {name} (m={params["memory_cost"]}, t={params["time_cost"]}, p={params["parallelism"]}):'
        )
        print(f'Hash: {hash_val}')
        timings = []
        for run in range(1, repeats + 1):
            start_time = time.perf_counter()
            hasher.hash(pwd)
            elapsed_time = time.perf_counter() - start_time
            timings.append(elapsed_time)
            print(f'Run {run}: {elapsed_time:.6f} (sec)')
        avg_time = sum(timings) / len(timings)
        print(f'Average: {avg_time:.6f} (sec)')
        if i < len(configs) - 1:
            print()


measure_argon2_configs(password, configs)

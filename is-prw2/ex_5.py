import time
import bcrypt

password = 'kernel04'
wrong_password = password + 'X'

print(f'Password: {password}')
print(f'Wrong password: {wrong_password}\n')

salt1 = bcrypt.gensalt(rounds=10)
hash1 = bcrypt.hashpw(password.encode('ascii'), salt1).decode('ascii')

salt2 = bcrypt.gensalt(rounds=10)
hash2 = bcrypt.hashpw(password.encode('ascii'), salt2).decode('ascii')

print(f'Bcrypt hash 1 (cost=10): {hash1}')
print(f'Bcrypt hash 2 (cost=10): {hash2}')
print(f'Hashes are equal: {hash1 == hash2}\n')

is_valid = bcrypt.checkpw(password.encode('ascii'), hash1.encode('ascii'))
is_invalid = bcrypt.checkpw(
    wrong_password.encode('ascii'), hash1.encode('ascii'))

print(f'Check correct password: {is_valid}')
print(f'Check wrong password: {is_invalid}\n')


def measure_cost_timing(pwd: str, costs: list[int], repeats: int = 2):
    for i, cost in enumerate(costs):
        print(f'Cost {cost}:')
        for run in range(1, repeats + 1):
            start_time = time.perf_counter()
            bcrypt.hashpw(pwd.encode('ascii'), bcrypt.gensalt(rounds=cost))
            elapsed_time = time.perf_counter() - start_time
            print(f'Run {run}: {elapsed_time:.6f} (sec)')
        if i < len(costs) - 1:
            print()


measure_cost_timing(password, [10, 11, 12])

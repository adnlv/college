import hashlib
import time
import timeit


def find_password():
    target = 'e1f12989372b5c4d69744b8bcdcb398076dae98495fdb713a2c5829d7af3b47b'
    bases = [
        'alpha', 'amber', 'atlas', 'binary', 'birch', 'bridge', 'cobalt', 'comet', 'delta',
        'eagle', 'ember', 'falcon', 'forest', 'galaxy', 'harbor', 'hazel', 'island', 'kernel',
        'laser', 'maple', 'matrix', 'meteor', 'neon', 'ocean', 'orbit', 'panda', 'pixel',
        'quartz', 'river', 'rocket', 'silver', 'signal', 'solar', 'tiger', 'vector', 'violet'
    ]

    start_time = time.perf_counter()
    attempt = 0

    for base in bases:
        for i in range(100):
            attempt += 1
            password = f'{base}{i:02d}'
            password_hash = hashlib.sha256(
                password.encode('ascii')).hexdigest()
            if password_hash == target:
                elapsed_time = time.perf_counter() - start_time
                print(f'Matching password: {password}')
                print(f'Found on attempt: {attempt}')
                print(f'Elapsed time: {elapsed_time:.6f} (sec)')
                break


def calc_avg_time():
    setup_code = "import hashlib; data = 'Hello World!'.encode('ascii')"
    stmt_code = "hashlib.sha256(data)"

    runs = timeit.repeat(stmt=stmt_code, setup=setup_code,
                         number=100_000, repeat=3)

    for i, total_time_secs in enumerate(runs, 1):
        avg_time_secs = total_time_secs / 100_000
        print(f"Run {i}: {avg_time_secs:.9f} seconds")


find_password()

print()

calc_avg_time()

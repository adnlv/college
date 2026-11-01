import hashlib

initial_msg = 'user=frank&operation=pay&amount=340&ref=06'
changed_msg = 'user=frank&operation=pay&amount=840&ref=06'

print(f'Initial message: {initial_msg}')
print(f'Changed message: {changed_msg}\n')

initial_hash = hashlib.sha256(initial_msg.encode('ascii')).hexdigest()
changed_hash = hashlib.sha256(changed_msg.encode('ascii')).hexdigest()

print(f'Initial hash: {initial_hash}')
print(f'Changed hash: {changed_hash}\n')

initial_hash_dec = int(initial_hash, 16)
changed_hash_dec = int(changed_hash, 16)

print(f'Initial hash decimal: {initial_hash_dec}')
print(f'Changed hash decimal: {changed_hash_dec}\n')

xored = initial_hash_dec ^ changed_hash_dec

print(f'XOR: {xored}')
print(f'XOR bit count: {xored.bit_count()}')

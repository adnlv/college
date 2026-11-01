import hashlib
import hmac

key_hex = '0d5552ceec77874c4079b42ffb324b86'
initial_msg = 'user=frank&operation=pay&amount=340&ref=06'
changed_msg = 'user=frank&operation=pay&amount=840&ref=06'

print(f'HMAC key (hex): {key_hex}')
print(f'Initial message: {initial_msg}')
print(f'Changed message: {changed_msg}\n')

key = bytes.fromhex(key_hex)
initial_mac = hmac.new(key, initial_msg.encode(
    'ascii'), hashlib.sha256).hexdigest()

print(f'Initial HMAC-SHA256: {initial_mac}\n')


def verify_mac(key: bytes, msg: str, expected_mac: str) -> bool:
    actual_mac = hmac.new(key, msg.encode('ascii'), hashlib.sha256).hexdigest()
    return hmac.compare_digest(actual_mac, expected_mac)


is_initial_valid = verify_mac(key, initial_msg, initial_mac)
is_changed_valid = verify_mac(key, changed_msg, initial_mac)

print(f'Verify initial message: {is_initial_valid}')
print(f'Verify changed message with initial MAC: {is_changed_valid}\n')

changed_sha256 = hashlib.sha256(changed_msg.encode('ascii')).hexdigest()

print(f'Changed message SHA-256: {changed_sha256}')

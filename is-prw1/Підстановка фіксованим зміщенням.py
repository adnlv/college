def get_decryption_shift(cipher_char: str, plain_char: str) -> int:
    """
    Обчислює необхідний зсув для розшифрування шифру Цезаря 
    на основі припущення про відповідність символів.
    """
    # Знаходимо різницю між позиціями в алфавіті.
    # Додаємо 26 і беремо остачу від ділення, щоб результат завжди був додатним (0-25).
    shift = (ord(plain_char.upper()) - ord(cipher_char.upper())) % 26
    return shift

def decrypt_caesar(ciphertext: str, shift: int) -> str:
    """
    Розшифровує текст з використанням знайденого зсуву.
    """
    plaintext = ""
    for char in ciphertext:
        if char.isalpha():
            # Визначаємо базовий код ASCII (для великих або малих літер)
            base = ord('A') if char.isupper() else ord('a')
            
            # Застосовуємо зсув і повертаємось до меж алфавіту
            decrypted_char = chr((ord(char) - base + shift) % 26 + base)
            plaintext += decrypted_char
        else:
            # Пробіли та розділові знаки залишаємо без змін
            plaintext += char
            
    return plaintext

if __name__ == "__main__":
    # Вхідні дані з нашої гіпотези
    cipher_char = 'A'
    plain_char = 'E'
    ciphertext = "W oayqna ydwjjah lnarajpo iaoowca ejpanyalpekj"
    
    # 1. Обчислюємо зсув
    shift = get_decryption_shift(cipher_char, plain_char)
    print(f"Гіпотеза: шифросимвол '{cipher_char}' = відкритий символ '{plain_char}'")
    print(f"Обчислений зсув для розшифрування: +{shift} позиції вправо")
    
    # 2. Розшифровуємо текст
    plaintext = decrypt_caesar(ciphertext, shift)
    print(f"\nШифротекст: {ciphertext}")
    print(f"Відкритий текст: {plaintext}")
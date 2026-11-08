import string
from collections import Counter

# Теоретичні частоти літер англійського алфавіту (в частках одиниці)
ENGLISH_FREQ = {
    'A': 0.08167, 'B': 0.01492, 'C': 0.02782, 'D': 0.04253, 'E': 0.12702,
    'F': 0.02228, 'G': 0.02015, 'H': 0.06094, 'I': 0.06966, 'J': 0.00153,
    'K': 0.00772, 'L': 0.04025, 'M': 0.02406, 'N': 0.06749, 'O': 0.07507,
    'P': 0.01929, 'Q': 0.00095, 'R': 0.05987, 'S': 0.06327, 'T': 0.09056,
    'U': 0.02758, 'V': 0.00978, 'W': 0.02360, 'X': 0.00150, 'Y': 0.01974,
    'Z': 0.00074
}

def chi_squared_score(text_slice: str, shift: int) -> float:
    """
    Обчислює критерій хі-квадрат (chi-squared) для конкретного зсуву підпослідовності.
    Чим менше значення, тим ближчий розподіл до природної англійської мови.
    """
    n = len(text_slice)
    if n == 0:
        return float('inf')
    
    # Виконуємо зворотний зсув для поточної групи символів
    decrypted_chars = [chr((ord(c) - ord('A') - shift) % 26 + ord('A')) for c in text_slice]
    counts = Counter(decrypted_chars)
    
    chi_sq = 0.0
    for letter, expected_freq in ENGLISH_FREQ.items():
        expected_count = n * expected_freq
        observed_count = counts.get(letter, 0)
        chi_sq += ((observed_count - expected_count) ** 2) / expected_count
        
    return chi_sq

def derive_vigenere_key(ciphertext: str, key_length: int) -> str:
    """
    Знаходить найбільш імовірне ключове слово заданої довжини, 
    мінімізуючи розбіжність розподілів частот.
    """
    clean_cipher = ''.join(filter(str.isalpha, ciphertext)).upper()
    key_chars = []
    
    print(f"--- АНАЛІЗ {key_length} НЕЗАЛЕЖНИХ ПІДПОСЛІДОВНОСТЕЙ ---")
    for col in range(key_length):
        slice_chars = clean_cipher[col::key_length]
        
        best_shift = 0
        min_chi_sq = float('inf')
        
        # Перебираємо всі 26 можливих зсувів для поточної позиції
        for shift in range(26):
            score = chi_squared_score(slice_chars, shift)
            if score < min_chi_sq:
                min_chi_sq = score
                best_shift = shift
                
        key_letter = chr(best_shift + ord('A'))
        key_chars.append(key_letter)
        print(f"Позиція ключа {col + 1}: зсув = {best_shift:<2} -> літера '{key_letter}' (хі-квадрат: {min_chi_sq:.2f})")
        
    recovered_key = ''.join(key_chars)
    return recovered_key

def decrypt_vigenere(ciphertext: str, key: str) -> str:
    """
    Розшифровує текст шифром Віженера за знайденим ключем, зберігаючи регістр.
    """
    key = key.upper()
    key_len = len(key)
    plaintext = []
    key_idx = 0
    
    for char in ciphertext:
        if char.isalpha():
            base = ord('A') if char.isupper() else ord('a')
            shift = ord(key[key_idx % key_len]) - ord('A')
            decrypted_char = chr((ord(char) - base - shift) % 26 + base)
            plaintext.append(decrypted_char)
            key_idx += 1
        else:
            plaintext.append(char)
            
    return ''.join(plaintext)

if __name__ == "__main__":
    target_ciphertext = (
        "AfwjqvsfhbzkenousvdfovfvtucuotoynlqkibpvtkayguhvdcgvdceZgtpvrfobgkuaflfjtnpkhyecwydfsrqmhyevtdcikfjhfvufgmiciahvfdagkvbrnqczywoejlzgwuguokafmistozgzrzfskjict"
        "QwywegndbgppeqqsttrxlbrsuqyhtoaxlfjagkvbtaarysmeavjcefhupceaafdojtrfltwoevDvvnrxlfpoagmsvlfevawoevhpceqkzqlsfkuudifvhyvsgjluiohrjoesbncsgrbdssdsrcyzzeecurue"
        "igscgpecjhzcnnprvafvooknbupbxlrrlfjoayvicduccsuifevjvrrfhzfnr"
    )
    
    # 1. Знаходження ключа за довжиною m = 6
    found_key = derive_vigenere_key(target_ciphertext, key_length=6)
    print(f"\n*** ВІДНОВЛЕНИЙ КЛЮЧ: {found_key} ***\n")
    
    # 2. Розшифрування відкритого тексту
    decrypted_text = decrypt_vigenere(target_ciphertext, found_key)
    print("--- РОЗШИФРОВАНИЙ ВІДКРИТИЙ ТЕКСТ ---")
    print(decrypted_text)
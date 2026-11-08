import collections

def analyze_frequencies(ciphertext: str) -> None:
    """
    Функція для проведення частотного аналізу шифротексту.
    """
    # Очищення тексту від пробілів та спецсимволів, зведення до верхнього регістру
    cleaned_text = ''.join(char.upper() for char in ciphertext if char.isalpha())
    
    if not cleaned_text:
        print("Помилка: Шифротекст не містить літер.")
        return

    total_chars = len(cleaned_text)
    frequencies = collections.Counter(cleaned_text)
    
    print(f"{'Символ':<10} | {'Кількість':<10} | {'Частка (%)':<10}")
    print("-" * 35)
    
    # Виведення результатів, відсортованих за спаданням частоти
    for char, count in frequencies.most_common():
        percentage = (count / total_chars) * 100
        print(f"{char:<10} | {count:<10} | {percentage:.2f}%")

ciphertext_sample = "AfwjqvsfhbzkenousvdfovfvtucuotoynlqkibpvtkayguhvdcgvdceZgtpvrfobgkuaflfjtnpkhyecwydfsrqmhyevtdcikfjhfvufgmiciahvfdagkvbrnqczywoejlzgwuguokafmistozgzrzfskjictQwywegndbgppeqqsttrxlbrsuqyhtoaxlfjagkvbtaarysmeavjcefhupceaafdojtrfltwoevDvvnrxlfpoagmsvlfevawoevhpceqkzqlsfkuudifvhyvsgjluiohrjoesbncsgrbdssdsrcyzzeecurueigscgpecjhzcnnprvafvooknbupbxlrrlfjoayvicduccsuifevjvrrfhzfnr"
analyze_frequencies(ciphertext_sample)

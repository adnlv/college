import math
from collections import defaultdict, Counter

def clean_text(text: str) -> str:
    """
    Очищує текст від розділових знаків та переводить у верхній регістр.
    """
    return ''.join(filter(str.isalpha, text)).upper()

def find_repeating_sequences(text: str, min_length: int = 4) -> dict:
    """
    Знаходить усі повторювані фрагменти заданої мінімальної довжини.
    Повертає словник {фрагмент: [список індексів (позицій)]}.
    """
    sequences = defaultdict(list)
    
    # Проходимо по всьому тексту вікном заданої довжини
    for i in range(len(text) - min_length + 1):
        seq = text[i:i+min_length]
        sequences[seq].append(i)
        
    # Відфільтровуємо лише ті послідовності, що зустрічаються більше одного разу
    return {seq: positions for seq, positions in sequences.items() if len(positions) > 1}

def get_divisors(n: int) -> list:
    """
    Повертає список дільників числа (більших за 1).
    """
    divisors = []
    for i in range(2, int(n / 2) + 1):
        if n % i == 0:
            divisors.append(i)
    divisors.append(n)
    return divisors

def kasiski_examination(ciphertext: str) -> None:
    """
    Реалізує метод Касіскі для визначення ймовірної довжини ключа поліалфавітного шифру.
    """
    print("--- АНАЛІЗ ЗА МЕТОДОМ КАСІСКІ ---")
    cleaned_text = clean_text(ciphertext)
    
    # Використовуємо довжину фрагмента 4, щоб відсіяти випадкові збіги
    sequences = find_repeating_sequences(cleaned_text, 4)
    
    if not sequences:
        print("Повторюваних фрагментів не знайдено.")
        return

    distances = []
    all_divisors = []

    for seq, positions in sequences.items():
        print(f"\nФрагмент '{seq}' знайдено на позиціях: {positions}")
        for i in range(len(positions) - 1):
            dist = positions[i+1] - positions[i]
            distances.append(dist)
            print(f"Відстань між {positions[i]} та {positions[i+1]} = {dist}")
            
            all_divisors.extend(get_divisors(dist))

    if distances:
        overall_gcd = distances[0]
        for d in distances[1:]:
            overall_gcd = math.gcd(overall_gcd, d)
        print(f"\n*** Найбільший спільний дільник (НСД) усіх відстаней: {overall_gcd} ***")

    counter = Counter(all_divisors)
    print("\n--- ЙМОВІРНА ДОВЖИНА КЛЮЧА (ЗА ЧАСТОТОЮ ДІЛЬНИКІВ) ---")
    for divisor, count in counter.most_common(5):
        print(f"Довжина {divisor}: зустрічається {count} разів")

if __name__ == "__main__":
    # Шифротекст Частини 2 Варіанту 6
    target_ciphertext = "AfwjqvsfhbzkenousvdfovfvtucuotoynlqkibpvtkayguhvdcgvdceZgtpvrfobgkuaflfjtnpkhyecwydfsrqmhyevtdcikfjhfvufgmiciahvfdagkvbrnqczywoejlzgwuguokafmistozgzrzfskjictQwywegndbgppeqqsttrxlbrsuqyhtoaxlfjagkvbtaarysmeavjcefhupceaafdojtrfltwoevDvvnrxlfpoagmsvlfevawoevhpceqkzqlsfkuudifvhyvsgjluiohrjoesbncsgrbdssdsrcyzzeecurueigscgpecjhzcnnprvafvooknbupbxlrrlfjoayvicduccsuifevjvrrfhzfnr"
    kasiski_examination(target_ciphertext)
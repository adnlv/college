# Звіт з практичної роботи № 3

---

<div align="center">

**Практична робота № 3:** Робота з OpenSSL: симетричне шифрування, асиметричне шифрування, підпис, перевірка підпису, генерація ключів  

**Виконав:** Данилов Артем  
**Група:** 80  
**Варіант:** 6  

</div>

---

## 1. Тема, мета та перелік завдань

**Тема:** Робота з OpenSSL: симетричне шифрування, асиметричне шифрування, підпис, перевірка підпису, генерація ключів.

Метою цієї практичної роботи є опанування командного інтерфейсу OpenSSL для виконання базових криптографічних операцій, розуміння обмежень окремих примітивів та правильна перевірка отриманих результатів. У ході виконання завдань ми обчислюємо хеші та HMAC, шифруємо й розшифровуємо дані за допомогою AES-CBC і AES-CTR, генеруємо ключові пари RSA й EC, досліджуємо RSA-OAEP, RSA-PSS та ECDSA, а наприкінці будуємо просту гібридну схему шифрування з перевіркою цілісності.

---

## 2. Індивідуальне завдання

| Параметр | Значення |
| :--- | :--- |
| **Номер варіанта** | `6` |
| **`data.txt` (4 рядки)** | `VARIANT=06`<br>`ASSET=helios-web`<br>`EVENT=Web service restarted during planned change.`<br>`TOKEN=F2U7` |
| **`msg.txt` (1 рядок)** | `HELIOS\|restart\|F2U7` |
| **`password.txt` (1 рядок)** | `Lab-HELIOS!46` |
| **Розмір RSA-ключа** | `2048` біт |
| **EC-крива / хеш** | `secp384r1` / `SHA-384` |
| **Індивідуальне аналітичне запитання** | Чому CTR без MAC дає конфіденційність, але не цілісність? |

Щоб усі етапи можна було легко відтворити повторно, у корені проєкту підготовлено скрипти [run_lab.sh](file:///home/adnlv/Documents/college/is-prw3/run_lab.sh) і [flip_bit.py](file:///home/adnlv/Documents/college/is-prw3/flip_bit.py). Усі згенеровані файли, ключі та паролі зберігаються локально в підкаталозі `work/`, який виключено з індексації у [.gitignore](file:///home/adnlv/Documents/college/is-prw3/.gitignore).

---

## 3. Хід виконання завдань

### Завдання 1. Перевірка середовища й підготовка даних

#### Виконані команди

```bash
mkdir -p work && cd work
openssl version
python3 --version

printf 'VARIANT=06\nASSET=helios-web\nEVENT=Web service restarted during planned change.\nTOKEN=F2U7\n' > data.txt
printf 'HELIOS|restart|F2U7\n' > msg.txt
printf 'Lab-HELIOS!46\n' > password.txt
chmod 600 password.txt

wc -l -c data.txt msg.txt password.txt
ls -l data.txt msg.txt password.txt
```

#### Результати у терміналі

```text
OpenSSL 4.1.0 23 Oct 2026 (Library: OpenSSL 4.1.0 23 Oct 2026)
Python 3.14.10
  4  90 data.txt
  1  20 msg.txt
  1  14 password.txt
  6 124 total
-rw-rw-rw- 1 adnlv adnlv 90 Nov 22 18:56 data.txt
-rw-rw-rw- 1 adnlv adnlv 20 Nov 22 18:56 msg.txt
-rw------- 1 adnlv adnlv 14 Nov 22 18:56 password.txt
```

#### Опис результатів

У робочому середовищі встановлено **OpenSSL 4.1.0** та **Python 3.14.10**. Для 6-го варіанта в підкаталозі `work/` створено три вхідні файли без зайвих лапок: `data.txt` (рівно 4 рядки, 90 байтів), `msg.txt` (1 рядок, 20 байтів) та `password.txt` (1 рядок, 14 байтів). Командою `chmod 600 password.txt` права доступу до файла з паролем одразу обмежено лише власником (`-rw-------`), тож на старті в робочій директорії `work/` знаходяться саме ці три файли.

---

### Завдання 2. Хешування SHA-2 та avalanche effect

#### Виконані команди

```bash
openssl dgst -sha256 data.txt
openssl dgst -sha512 data.txt
cp data.txt data_changed.txt
printf '!' >> data_changed.txt
openssl dgst -sha256 data_changed.txt
wc -c data.txt data_changed.txt
cmp data.txt data_changed.txt
```

#### Результати у терміналі

```text
SHA2-256(data.txt)= 9f39edf89b7d01bf2af38507f205a32100101df11ca2d657a886a36db3ad58fb
SHA2-512(data.txt)= 6421415c1b4ba52ff8333186f36b69c004f1d5c245818c240a133601e6109b9143be4e00bbb631b1cebe34420efd8bf9966fa9d1660cd8f3a8a61c9561b56c2d
SHA2-256(data_changed.txt)= 98483b88cb478bf74f9ce9009070b680b8e3c11381164c3ef1ce02764301763e
 90 data.txt
 91 data_changed.txt
181 total
cmp: EOF on ‘data.txt’ after byte 90, line 4
```

#### Опис результатів

| Параметр | Значення |
| :--- | :--- |
| **SHA-256 (`data.txt`)** | `9f39edf89b7d01bf2af38507f205a32100101df11ca2d657a886a36db3ad58fb` (64 hex-символи = 256 бітів = 32 байти) |
| **SHA-512 (`data.txt`)** | `6421415c1b4ba52ff8333186f36b69c004f1d5c245818c240a133601e6109b9143be4e00bbb631b1cebe34420efd8bf9966fa9d1660cd8f3a8a61c9561b56c2d` (128 hex-символів = 512 бітів = 64 байти) |
| **SHA-256 (`data_changed.txt`)** | `98483b88cb478bf74f9ce9009070b680b8e3c11381164c3ef1ce02764301763e` |
| **Різниця розмірів файлів** | `91 - 90 = 1` байт (додано символ `'!'`) |
| **Хеммінгова відстань (XOR)** | `122` із `256` бітів (`47.66%`) |

Як видно з результатів, додавання всього одного символу `'!'` наприкінці `data_changed.txt` збільшило розмір файла на 1 байт (із 90 до 91), але змінило `122` із `256` бітів у дайджесті SHA-256 (`47.66%`). Така реакція наближається до теоретичних `50%` і наочно ілюструє лавинний ефект: через нелінійне перемішування бітів у раундах функції стиснення мінімальна правка на вході повністю змінює вихідний хеш, тож за самим дайджестом неможливо визначити, де саме і як змінилися дані.

---

### Завдання 3. Випадкові дані та HMAC-SHA-256

#### Виконані команди

```bash
openssl rand -hex 16 > rand16.hex
openssl rand -hex 32 > hmac.key
chmod 600 hmac.key
cat rand16.hex
tr -d '\n' < hmac.key | wc -c

HMAC_KEY="$(cat hmac.key)"
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" data.txt
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" data_changed.txt
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" -binary -out data.hmac data.txt
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" -binary -out data_changed.hmac data_changed.txt
wc -c data.hmac data_changed.hmac
cmp data.hmac data_changed.hmac
```

#### Результати у терміналі

```text
588bffc8849f60ce7f2114cf76e81192
64
HMAC-SHA2-256(data.txt)= 142818f33153ad23188c4081db6304e265248ec3a7ca8df128a655328ca01a4e
HMAC-SHA2-256(data_changed.txt)= 7b619e71b0fc01d37417deae70535efc0c89158d8c346ad1cd1397a202160401
32 data.hmac
32 data_changed.hmac
64 total
data.hmac data_changed.hmac differ: byte 1, line 1
```

#### Опис результатів

На цьому етапі згенеровано 16 випадкових байтів у `rand16.hex` (`588bffc8849f60ce7f2114cf76e81192`, 32 hex-символи) та 32-байтовий секретний ключ у `hmac.key` (64 hex-символи). Під час виклику OpenSSL ключ передавався через `-mac hmac -macopt hexkey:...`, щоб він зчитувався саме як 32 сирі байти, а не як текстовий рядок. Обидва збережені теги `data.hmac` і `data_changed.hmac` мають фіксований розмір 32 байти (256 бітів), а перевірка `cmp` зафіксувала розбіжність між ними вже з першого байта (`differ: byte 1, line 1`).

Головна відмінність між звичайним SHA-256 та HMAC-SHA-256 полягає в наявності секретного ключа. Звичайний хеш може перерахувати будь-хто: якщо зловмисник підмінить файл, він без проблем оновить і його SHA-256. Натомість обчислити коректний HMAC без знання `hmac.key` неможливо, тому підробку одразу буде виявлено. Саме з цієї причини ключ HMAC потрібно тримати в суворій таємниці — у разі його витоку зловмисник зможе самостійно формувати валідні теги для змінених повідомлень.

---

### Завдання 4. Симетричне шифрування AES-256-CBC

#### Виконані команди

```bash
openssl enc -aes-256-cbc -salt -pbkdf2 -iter 200000 -md sha256 \
    -in data.txt -out aes_cbc.bin -pass file:password.txt
openssl enc -d -aes-256-cbc -pbkdf2 -iter 200000 -md sha256 \
    -in aes_cbc.bin -out data_dec.txt -pass file:password.txt
cmp -s data.txt data_dec.txt && echo "cmp data.txt data_dec.txt: MATCH"

openssl enc -aes-256-cbc -salt -pbkdf2 -iter 200000 -md sha256 \
    -in data.txt -out aes_cbc2.bin -pass file:password.txt
cmp aes_cbc.bin aes_cbc2.bin
wc -c data.txt aes_cbc.bin aes_cbc2.bin data_dec.txt
```

#### Результати у терміналі

```text
cmp data.txt data_dec.txt: MATCH
aes_cbc.bin aes_cbc2.bin differ: byte 9, line 1
 90 data.txt
112 aes_cbc.bin
112 aes_cbc2.bin
 90 data_dec.txt
404 total
aes_cbc.bin header=b'Salted__', salt=8bb641049043d635
aes_cbc2.bin header=b'Salted__', salt=03bb2271265d8167
```

#### Опис результатів

Після розшифрування `aes_cbc.bin` перша перевірка `cmp` показала повний побайтовий збіг (`MATCH`) між вихідним `data.txt` та відновленим `data_dec.txt` розміром 90 байтів. Натомість повторне шифрування того самого файла з тим самим паролем дало інший результат: `aes_cbc.bin` та `aes_cbc2.bin` відрізняються вже з 9-го байта. Так відбувається тому, що перші 8 байтів у файлі займає службовий заголовок `Salted__`, а одразу за ним іде 8-байтова випадкова сіль (`8bb641049043d635` у першому шифротексті та `03bb2271265d8167` у другому). Решту 96 байтів займають зашифровані дані: 90 байтів відкритого тексту доповнено 6 байтами за схемою PKCS#7 до шести повних 16-байтових блоків, що разом дає 112 байтів.

Випадкова сіль потрібна для того, щоб навіть за однакових пароля та відкритого тексту щоразу генерувалися інші ключ і вектор ініціалізації (IV) — це захищає від атак за заздалегідь обчисленими словниками. Разом із функцією PBKDF2 на `200 000` ітерацій SHA-256 перебір паролів сповільнюється у сотні тисяч разів. Водночас покладатися на помилку padding під час розшифрування в режимі CBC як на перевірку цілісності не варто: пошкоджений шифротекст може випадково мати правильні байти доповнення, а сама реакція програми на некоректний padding створює загрозу `padding-oracle` атак.

---

### Завдання 5. Режим AES-CTR і модифікація шифротексту

#### Виконані команди

```bash
openssl enc -aes-256-ctr -salt -pbkdf2 -iter 200000 -md sha256 \
    -in data.txt -out aes_ctr.bin -pass file:password.txt
openssl enc -d -aes-256-ctr -pbkdf2 -iter 200000 -md sha256 \
    -in aes_ctr.bin -out data_ctr_dec.txt -pass file:password.txt
cmp -s data.txt data_ctr_dec.txt && echo "cmp data.txt data_ctr_dec.txt: MATCH"

python3 ../flip_bit.py aes_ctr.bin aes_ctr_tampered.bin
openssl enc -d -aes-256-ctr -pbkdf2 -iter 200000 -md sha256 \
    -in aes_ctr_tampered.bin -out data_ctr_tampered.txt -pass file:password.txt
echo "Tampered CTR decryption exit code: $?"
cmp data.txt data_ctr_tampered.txt
wc -c data.txt aes_ctr.bin aes_ctr_tampered.bin data_ctr_tampered.txt

openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" aes_ctr.bin
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" aes_ctr_tampered.bin
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" -binary -out aes_ctr.hmac aes_ctr.bin
openssl dgst -sha256 -mac hmac -macopt "hexkey:$HMAC_KEY" -binary -out aes_ctr_tampered.hmac aes_ctr_tampered.bin
cmp aes_ctr.hmac aes_ctr_tampered.hmac
```

#### Результати у терміналі

```text
cmp data.txt data_ctr_dec.txt: MATCH
Flipped byte[105] (of 106 bytes): 0x45 (01000101) -> 0x44 (01000100)
Tampered CTR decryption exit code: 0
data.txt data_ctr_tampered.txt differ: byte 90, line 4
 90 data.txt
106 aes_ctr.bin
106 aes_ctr_tampered.bin
 90 data_ctr_tampered.txt
392 total
HMAC-SHA2-256(aes_ctr.bin)= 98ddd50daa04a05117145183eb730f1b3efbf8ff06baa89fd73b45610373f63f
HMAC-SHA2-256(aes_ctr_tampered.bin)= 260f17c45f2f96536d6dc3a13c505cee51f99129756b43c99656fbbff99b87bc
aes_ctr.hmac aes_ctr_tampered.hmac differ: byte 1, line 1
```

#### Опис результатів

У режимі Counter (CTR) блоковий шифр AES фактично працює як потоковий: він шифрує послідовні значення лічильника, формуючи гаму, яка накладається на відкритий текст через операцію XOR. Якщо останній блок неповний, зайві байти гами просто відкидаються, тому вирівнювати дані до 16-байтових блоків за допомогою padding не потрібно. Через це розмір `aes_ctr.bin` становить `106` байтів — рівно 90 байтів зашифрованого тексту плюс 16 байтів на заголовок `Salted__` і сіль.

Тест із пошкодженням шифротексту добре демонструє, чому успішне розшифрування не гарантує автентичності. Після інверсії всього 1 біта в останньому байті `aes_ctr.bin` (`0x45 -> 0x44`) команда `openssl enc -d` завершилася штатно з кодом `0`, але в результаті останній байт відновленого `data_ctr_tampered.txt` виявився спотвореним (`0x0a -> 0x0b`). У режимі CTR зміна будь-якого біта шифротексту автоматично інвертує відповідний біт у відкритому тексті без жодних помилок формату. Натомість обчислення HMAC-SHA-256 із ключем `hmac.key` одразу показало зовсім інші теги (`98ddd50d...` проти `260f17c4...`, розбіжність із 1-го байта у `cmp`), що дає змогу відхилити підмінений файл ще до спроби його розшифрувати.

---

### Завдання 6. Генерація RSA-ключів

#### Виконані команди

```bash
RSA_BITS=2048
openssl genpkey -algorithm RSA -pkeyopt "rsa_keygen_bits:$RSA_BITS" -out rsa_priv.pem
chmod 600 rsa_priv.pem
openssl pkey -in rsa_priv.pem -pubout -out rsa_pub.pem
openssl pkey -in rsa_priv.pem -check -noout
openssl pkey -pubin -in rsa_pub.pem -text -noout
ls -l rsa_priv.pem rsa_pub.pem
```

#### Результати у терміналі

```text
Key is valid
Public-Key: (2048 bit)
Modulus:
    ad:70:cb:be:eb:9a:a5:6a:a0:05:06:fd:11:a7:51:c1:
    21:fb:3e:3f:aa:34:61:50:de:8e:23:3e:8b:d5:f6:e0:
    ad:6d:13:55:51:88:07:e0:3f:75:44:84:e5:5d:42:8e:
    58:02:f4:31:08:79:1d:59:d4:d4:71:52:a3:98:41:f3:
    a0:2a:0d:f1:92:b8:a5:55:70:86:a1:ac:41:91:6a:c1:
    d7:65:27:78:93:95:77:eb:4c:cb:cb:88:72:2a:12:47:
    8f:50:c8:fd:b3:74:c9:09:f5:8c:4e:8c:94:e8:63:cd:
    56:37:53:3a:3e:62:6f:ac:0f:50:6f:a6:2f:98:70:67:
    2f:73:64:58:45:18:8e:64:53:14:e8:6f:28:c2:01:60:
    ea:9e:64:5a:55:9a:aa:0c:4f:14:36:35:df:7f:ee:f2:
    27:91:4f:24:23:a3:f6:9a:65:57:95:89:29:75:ab:b2:
    16:24:cf:e2:a1:92:5a:1c:14:9e:a7:76:65:0b:9a:ea:
    69:03:96:de:8c:de:ff:94:8c:a5:c0:b1:62:e4:ab:e7:
    64:a9:6b:17:57:01:bf:a0:b5:81:ab:79:75:4e:2b:f6:
    22:7d:20:4a:54:10:ae:e7:8c:5e:64:f0:ce:4c:2d:e1:
    fa:61:d2:60:94:72:ec:5a:ff:33:c4:64:cc:2c:e7:e5
Exponent: 65537 (0x10001)
-rw------- 1 adnlv adnlv 1704 Nov 22 18:56 rsa_priv.pem
-rw-rw-rw- 1 adnlv adnlv  451 Nov 22 18:56 rsa_pub.pem
```

#### Опис результатів

Згенерована ключова пара RSA має розмір `2048` біт і стандартний відкритий показник `65537` (`0x10001`). Команда перевірки `-check` повернула `Key is valid`, що підтверджує математичну узгодженість модуля, простих множників $p, q$ та показників $e, d$.

Приватний і публічний ключі суттєво відрізняються за вмістом та правилами зберігання. Файл `rsa_priv.pem` (1704 байти, права доступу `600`) збережено у форматі PKCS#8 із заголовком `BEGIN PRIVATE KEY`: усередині містяться прості множники $p, q$, секретний показник $d$ та CRT-коефіцієнти, тому передавати чи публікувати цей файл заборонено. У файлі `rsa_pub.pem` (451 байт, формат SubjectPublicKeyInfo / X.509 із заголовком `BEGIN PUBLIC KEY`) зберігаються лише модуль $n$ і відкритий показник $e$. Саме поняття PEM означає текстову обгортку, у якій двійкова структура ASN.1/DER закодована у Base64 між рядками `-----BEGIN ...-----` та `-----END ...-----`.

---

### Завдання 7. Асиметричне шифрування RSA-OAEP

#### Виконані команди

```bash
openssl pkeyutl -encrypt -pubin -inkey rsa_pub.pem -in msg.txt -out msg.enc \
    -pkeyopt rsa_padding_mode:oaep -pkeyopt rsa_oaep_md:sha256 -pkeyopt rsa_mgf1_md:sha256
openssl pkeyutl -decrypt -inkey rsa_priv.pem -in msg.enc -out msg_dec.txt \
    -pkeyopt rsa_padding_mode:oaep -pkeyopt rsa_oaep_md:sha256 -pkeyopt rsa_mgf1_md:sha256
cmp -s msg.txt msg_dec.txt && echo "cmp msg.txt msg_dec.txt: MATCH"
wc -c msg.txt msg.enc msg_dec.txt
```

#### Результати у терміналі

```text
cmp msg.txt msg_dec.txt: MATCH
 20 msg.txt
256 msg.enc
 20 msg_dec.txt
296 total
```

#### Опис результатів

Файл `msg.txt` має довжину `20` байтів — це значно менше за граничний розмір для RSA-2048 зі схемою OAEP-SHA-256, який становить $256 - 2 \cdot 32 - 2 = 190$ байтів. Після шифрування отримано `msg.enc` розміром `256` байтів (2048 бітів / 8), а подальше розшифрування приватним ключем дало `msg_dec.txt`, який побайтово збігається з оригіналом (`MATCH`).

На практиці RSA не застосовують для шифрування великих файлів, обмежуючись короткими повідомленнями або сесійними ключами. Причина в тому, що піднесення великих чисел до степеня за модулем $n$ працює в тисячі разів повільніше за симетричний AES, вихідний шифротекст завжди має розмір кратного модулю RSA, а схема OAEP додатково резервує $2 \cdot hLen + 2$ байти під два хеші та службові поля. Через це в реальних системах самі файли шифрують через AES, а за допомогою RSA захищають лише передачу короткого симетричного ключа.

---

### Завдання 8. Цифровий підпис RSA-PSS

#### Виконані команди

```bash
openssl dgst -sha256 -sign rsa_priv.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out data.rsa.sig data.txt
openssl dgst -sha256 -verify rsa_pub.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -signature data.rsa.sig data.txt
openssl dgst -sha256 -verify rsa_pub.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -signature data.rsa.sig data_changed.txt
openssl dgst -sha256 -sign rsa_priv.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out data2.rsa.sig data.txt
cmp data.rsa.sig data2.rsa.sig
wc -c data.rsa.sig data2.rsa.sig
```

#### Результати у терміналі

```text
Verified OK
Verification failure
80304236677F0000:error:0308010C:digital envelope routines:inner_evp_generic_fetch:unsupported:crypto/evp/evp_fetch.c:476:Global default library context, Algorithm (dgst : 0), Properties (<null>)
80304236677F0000:error:02000068:rsa routines:ossl_rsa_verify_PKCS1_PSS_mgf1:bad signature:crypto/rsa/rsa_pss.c:144:
80304236677F0000:error:1C880004:Provider routines:rsa_verify_directly:RSA lib:providers/implementations/signature/rsa_sig.c:1115:
data.rsa.sig data2.rsa.sig differ: byte 1, line 1
256 data.rsa.sig
256 data2.rsa.sig
512 total
```

#### Опис результатів

Перевірка створеного підпису для незміненого `data.txt` завершилася успішно (`Verified OK`), тоді як для `data_changed.txt` OpenSSL повернув `Verification failure` (`bad signature`). Під час повторного підписування того самого `data.txt` ми отримали другий підпис `data2.rsa.sig` такої самої довжини — `256` байтів, однак `cmp` показала, що `data.rsa.sig` та `data2.rsa.sig` відрізняються вже з першого байта (`differ: byte 1, line 1`).

У цифровому підписі ролі ключів зворотні до шифрування: підпис створюється приватним ключем (щоб підписувати міг лише власник), а перевіряється публічним (щоб переконатися у справжності міг кожен). Оскільки хеш SHA-256 обчислюється безпосередньо над сирими байтами файла, зміна хоча б одного символу або переведення рядка повністю змінює дайджест і зриває верифікацію. Різниця між двома підписами одного й того самого документа пояснюється роботою схеми RSA-PSS: під час кожного підписування вона генерує нову випадкову сіль (параметр `saltlen:-1` задає її розмір рівним довжині хешу SHA-256, тобто 32 байти), завдяки чому повторні підписи не збігаються і дають вищу криптографічну стійкість, ніж детермінований PKCS#1 v1.5.

---

### Завдання 9. EC-ключі та підпис ECDSA

#### Виконані команди

```bash
EC_CURVE=secp384r1
EC_HASH=sha384
openssl genpkey -algorithm EC -pkeyopt "ec_paramgen_curve:$EC_CURVE" -out ec_priv.pem
chmod 600 ec_priv.pem
openssl pkey -in ec_priv.pem -pubout -out ec_pub.pem
openssl pkey -pubin -in ec_pub.pem -text -noout
openssl dgst "-$EC_HASH" -sign ec_priv.pem -out data.ec.sig data.txt
openssl dgst "-$EC_HASH" -verify ec_pub.pem -signature data.ec.sig data.txt
openssl dgst "-$EC_HASH" -verify ec_pub.pem -signature data.ec.sig data_changed.txt
wc -c data.rsa.sig data.ec.sig
```

#### Результати у терміналі

```text
Public-Key: (384 bit field, 192 bit security level)
pub:
    04:1d:d9:e8:e7:df:dd:c6:7f:54:e1:0f:a6:0a:3d:97:
    1a:d4:e0:3c:6c:ae:da:29:14:7c:fa:5b:69:46:9b:ef:
    86:fd:d5:f3:70:33:93:76:24:9b:a7:99:90:ae:17:b5:
    cb:04:b9:2b:7d:13:81:2f:75:62:a7:ba:24:bc:b8:48:
    b2:82:4e:b4:ba:0c:89:09:70:a4:05:4e:8a:35:82:0a:
    86:b8:02:fd:c8:13:cb:a3:02:f6:de:75:39:a0:cc:d0:
    e1
ASN1 OID: secp384r1
NIST CURVE: P-384
Verified OK
Verification failure
8080FD4E077F0000:error:0308010C:digital envelope routines:inner_evp_generic_fetch:unsupported:crypto/evp/evp_fetch.c:476:Global default library context, Algorithm (dgst : 0), Properties (<null>)
8080FD4E077F0000:error:030000EA:digital envelope routines:EVP_DigestVerifyFinal:provider signature failure:crypto/evp/m_sigver.c:533:ECDSA digest_verify_final:
256 data.rsa.sig
103 data.ec.sig
359 total
```

#### Опис результатів

Для 6-го варіанта використано еліптичну криву `secp384r1` (`NIST CURVE: P-384` із 384-бітним полем, що забезпечує 192-бітний рівень безпеки) у поєднанні з хешуванням `SHA-384`. Як і в попередньому завданні, перевірка підпису для оригінального `data.txt` пройшла успішно (`Verified OK`), а для зміненого `data_changed.txt` завершилася помилкою (`Verification failure`).

Найпомітніша практична різниця виявилася в розмірі згенерованих підписів: `data.rsa.sig` (RSA-2048) має фіксовані `256` байтів, тоді як `data.ec.sig` (ECDSA P-384) займає лише `103` байти. Це пояснюється структурою підпису ECDSA — він містить дві 48-байтові компоненти $(r, s)$ у DER-кодуванні ASN.1 `SEQUENCE` з кількома службовими байтами, тому його довжина зазвичай становить 102–104 байти залежно від знакових бітів. Водночас крива P-384 дає 192 біти криптографічної стійкості проти приблизно 112 бітів у RSA-2048, тобто еліптична криптографія забезпечує вищий рівень захисту за набагато коротших ключів і підписів.

---

### Завдання 10. Гібридна схема: AES + RSA-OAEP + HMAC

#### Логічна схема руху даних

```text
Відправник:
  [aes.key (32B), mac.key (32B)] -> bundle.txt (130B) --(RSA-OAEP, rsa_pub.pem)--> bundle.rsa (256B)
  data.txt (90B) --(AES-256-CTR, K=aes.key, IV=iv.hex)--> hybrid.enc (90B)
  iv.hex (33B) || hybrid.enc (90B) -> hybrid.authdata (123B) --(HMAC-SHA-256, mac.key)--> hybrid.hmac (32B)

Пакет для адресата: { bundle.rsa, iv.hex, hybrid.enc, hybrid.hmac }

Отримувач:
  bundle.rsa --(RSA-OAEP decrypt, rsa_priv.pem)--> bundle_dec.txt -> aes_rx.key, mac_rx.key
  iv.hex || hybrid.enc -> hybrid_rx.authdata --(HMAC-SHA-256, mac_rx.key)--> hybrid_rx.hmac
  cmp(hybrid.hmac, hybrid_rx.hmac):
     ├── збіг (HMAC_OK)  --> AES-256-CTR decrypt(hybrid.enc, aes_rx.key, iv.hex) -> hybrid_dec.txt
     └── розбіжність     --> HMAC_FAIL (розшифрування заборонено!)
```

#### Виконані команди

```bash
openssl rand -hex 32 > aes.key
openssl rand -hex 32 > mac.key
openssl rand -hex 16 > iv.hex
chmod 600 aes.key mac.key
cat aes.key mac.key > bundle.txt
chmod 600 bundle.txt
wc -l -c bundle.txt iv.hex

openssl pkeyutl -encrypt -pubin -inkey rsa_pub.pem -in bundle.txt -out bundle.rsa \
    -pkeyopt rsa_padding_mode:oaep -pkeyopt rsa_oaep_md:sha256 -pkeyopt rsa_mgf1_md:sha256
openssl enc -aes-256-ctr -K "$(cat aes.key)" -iv "$(cat iv.hex)" -in data.txt -out hybrid.enc
cat iv.hex hybrid.enc > hybrid.authdata
openssl dgst -sha256 -mac hmac -macopt "hexkey:$(cat mac.key)" hybrid.authdata
openssl dgst -sha256 -mac hmac -macopt "hexkey:$(cat mac.key)" -binary -out hybrid.hmac hybrid.authdata
wc -c bundle.rsa iv.hex hybrid.enc hybrid.authdata hybrid.hmac

# Імітація отримувача
openssl pkeyutl -decrypt -inkey rsa_priv.pem -in bundle.rsa -out bundle_dec.txt \
    -pkeyopt rsa_padding_mode:oaep -pkeyopt rsa_oaep_md:sha256 -pkeyopt rsa_mgf1_md:sha256
sed -n '1p' bundle_dec.txt > aes_rx.key
sed -n '2p' bundle_dec.txt > mac_rx.key
cat iv.hex hybrid.enc > hybrid_rx.authdata
openssl dgst -sha256 -mac hmac -macopt "hexkey:$(cat mac_rx.key)" -binary -out hybrid_rx.hmac hybrid_rx.authdata
cmp -s hybrid.hmac hybrid_rx.hmac && echo "HMAC_OK"
openssl enc -d -aes-256-ctr -K "$(cat aes_rx.key)" -iv "$(cat iv.hex)" -in hybrid.enc -out hybrid_dec.txt
cmp -s data.txt hybrid_dec.txt && echo "cmp data.txt hybrid_dec.txt: MATCH"

# Додаткова перевірка підміни
python3 ../flip_bit.py hybrid.enc hybrid_tampered.enc
cat iv.hex hybrid_tampered.enc > hybrid_tampered.authdata
openssl dgst -sha256 -mac hmac -macopt "hexkey:$(cat mac_rx.key)" hybrid_tampered.authdata
openssl dgst -sha256 -mac hmac -macopt "hexkey:$(cat mac_rx.key)" -binary -out hybrid_tampered.hmac hybrid_tampered.authdata
cmp -s hybrid.hmac hybrid_tampered.hmac || echo "HMAC_FAIL (tampering detected; decryption aborted)"
```

#### Результати у терміналі

```text
  2 130 bundle.txt
  1  33 iv.hex
  3 163 total
iv.hex: ab2af860e545ee377f17be60be87f7fe
HMAC-SHA2-256(hybrid.authdata)= 46a947638f26a96cd557ae4b3ff73197eb26fd4337c628630e663c64a3e2a245
256 bundle.rsa
 33 iv.hex
 90 hybrid.enc
123 hybrid.authdata
 32 hybrid.hmac
534 total
HMAC_OK
cmp data.txt hybrid_dec.txt: MATCH
Flipped byte[89] (of 90 bytes): 0xa4 (10100100) -> 0xa5 (10100101)
HMAC-SHA2-256(hybrid_tampered.authdata)= 229c990fb67aee0106479dbb22686d5d69eda82d19c27432bbb59c73f92647a3
HMAC_FAIL (tampering detected; decryption aborted)
```

#### Опис результатів

У побудованій гібридній схемі відправник формує пакет для адресата з чотирьох файлів: зашифрованої зв'язки ключів `bundle.rsa` (256 байтів), вектора ініціалізації `iv.hex` (33 байти, значення `ab2af860e545ee377f17be60be87f7fe`), шифротексту `hybrid.enc` (90 байтів) та автентифікаційного тегу `hybrid.hmac` (32 байти). Приватний ключ `rsa_priv.pem` каналом зв'язку не передається — отримувач користується власним приватним ключем, щоб витягти сесійні ключі.

Під час імітації дій отримувача система коректно спрацювала в обох сценаріях. Для неушкодженого пакета повторно обчислений HMAC-SHA-256 точно збігся з переданим (`HMAC_OK`, значення `46a947638f26a96cd557ae4b3ff73197eb26fd4337c628630e663c64a3e2a245`), після чого `hybrid.enc` успішно розшифрувався у `hybrid_dec.txt` із повним збігом (`cmp data.txt hybrid_dec.txt: MATCH`). Коли ж ми інвертували 1 біт в останньому байті `hybrid.enc` (`0xa4 -> 0xa5`), тег змінився на `229c990fb67aee0106479dbb22686d5d69eda82d19c27432bbb59c73f92647a3`, порівняння повернуло `HMAC_FAIL`, і розшифрування було зупинено ще до виклику AES.

Генерація двох незалежних симетричних ключів для шифрування (AES-CTR) та автентифікації (HMAC-SHA-256) реалізує принцип криптографічного розділення ключів (*key separation*). Компрометація або математичні особливості одного алгоритму не повинні послаблювати інший. Водночас включення `iv.hex` до складу `hybrid.authdata` додатково захищає вектор ініціалізації від непомітної підміни зловмисником.

---

## 4. Відповіді на контрольні запитання

### 1. Що саме гарантує криптографічна хеш-функція і чого вона не гарантує?

Криптографічна хеш-функція гарантує детермінованість — однакові вхідні дані завжди дають однаковий дайджест фіксованої довжини — а також виражений лавинний ефект, коли зміна навіть одного біта кардинально змінює результат. Крім того, вона забезпечує три базові властивості стійкості: односторонність (за відомим хешем неможливо відновити початкові дані), стійкість до другого прообразу (маючи повідомлення, практично неможливо підібрати інше з таким самим хешем) і стійкість до колізій (неможливо знайти два різні повідомлення з однаковим дайджестом).

Водночас сама собою хеш-функція не гарантує ані конфіденційності, ані автентичності відправника. Хеш не приховує зміст файлу, а оскільки алгоритм відкритий, будь-який зловмисник може підмінити дані й перерахувати для них новий хеш, якщо передача не захищена секретним ключем або довіреним каналом.

### 2. У чому різниця між хешем, HMAC і цифровим підписом?

| Властивість | Звичайний хеш (SHA-256) | HMAC (HMAC-SHA-256) | Цифровий підпис (RSA-PSS / ECDSA) |
| :--- | :--- | :--- | :--- |
| **Ключ** | Не використовується | Спільний секретний ключ | Асиметрична пара (приватний підписує, публічний перевіряє) |
| **Захист від підміни зловмисником** | Ні (можна перерахувати хеш) | Так (без ключа тег не підробити) | Так (без приватного ключа підпис не створити) |
| **Неспростовність авторства** | Ні | Ні (будь-яка сторона зі спільним ключем могла створити тег) | Так (створити підпис може лише власник приватного ключа) |

Коротко кажучи, звичайний хеш захищає лише від випадкових помилок при передаванні, HMAC підтверджує цілісність даних для сторін зі спільним секретом, а цифровий підпис дає змогу перевірити справжність будь-кому за публічним ключем і доводить авторство власника приватного ключа.

### 3. Що таке salt і PBKDF2? Чому не варто виводити ключ із пароля простим SHA-256?

Сіль (`salt`) — це випадкове несекретне значення, яке додається до пароля перед формуванням ключа, щоб однакові паролі давали зовсім різні ключі й унеможливлювали використання райдужних таблиць. Своєю чергою, PBKDF2 є стандартною функцією виведення ключа з пароля, яка багаторазово застосовує псевдовипадкову функцію (наприклад, HMAC-SHA-256) задану кількість разів — у нашому випадку це `200 000` ітерацій (`-iter 200000`).

Виводити ключ через звичайний SHA-256 не варто, оскільки сучасні відеокарти обчислюють мільярди викликів SHA-256 за секунду, а без солі один перебір одразу працює проти всіх користувачів. Натомість унікальна сіль та високий лічильник ітерацій у PBKDF2 роблять brute-force у сотні тисяч разів повільнішим і дорожчим.

### 4. Чим AES-CBC відрізняється від AES-CTR?

У режимі AES-CBC блоки шифруються послідовно: кожен блок відкритого тексту перед шифруванням складається за XOR із попереднім блоком шифротексту, а перший блок — із вектором ініціалізації (IV). Через таку ланцюгову залежність шифрування виконується лише послідовно (розшифрування можна розпаралелити), а сам текст обов'язково доповнюється через padding до кратності 16 байтам.

Режим AES-CTR, навпаки, перетворює блоковий шифр на потоковий. Він незалежно шифрує значення лічильника (`nonce + counter`) і накладає отриману гаму на відкритий текст через XOR. Завдяки цьому CTR не потребує padding, дозволяє розпаралелювати як шифрування, так і розшифрування, а також дає довільний доступ до окремих блоків.

### 5. Чому AES-CTR не потребує padding?

У режимі CTR функція блокового шифру AES застосовується не до відкритого тексту напряму, а до 16-байтових блоків лічильника для генерації гами. Відкритий текст побайтово складається за XOR із цією гамою, і якщо останній шматок повідомлення коротший за 16 байтів, зайві байти гами просто відкидаються без жодного вирівнювання.

### 6. Чому `openssl enc -aes-256-cbc` не гарантує цілісності файла?

Команда `openssl enc -aes-256-cbc` виконує лише шифрування і не додає до файлу жодного тегу автентифікації (MAC). Якщо зловмисник змінить біти всередині шифротексту — особливо не зачіпаючи останній блок із padding, або підібравши значення з валідним доповненням, — розшифрування завершиться без помилки, але поверне спотворений текст. Більше того, сама реакція на помилки padding створює ризик `padding-oracle` атак.

### 7. Які дані можна безпечно публікувати: публічний ключ, приватний ключ, IV, salt?

Безпечно публікувати або передавати відкритим каналом можна **публічний ключ**, **IV** (вектор ініціалізації чи nonce) та **salt**. Ці параметри не є секретними: IV і сіль потрібні отримувачу або KDF для коректного розшифрування, а їхня надійність спирається на унікальність, а не на таємність (зокрема, для IV у режимах CTR чи GCM критично не повторювати його з тим самим ключем).

Натомість у жодному разі не можна розкривати **приватні ключі** (`rsa_priv.pem`, `ec_priv.pem`), паролі та симетричні секретні ключі (`hmac.key`, `aes.key`, `mac.key`), оскільки на їхній конфіденційності тримається весь захист.

### 8. Чому RSA-OAEP має обмеження на розмір відкритого повідомлення?

Математична операція RSA працює за модулем $n$ фіксованого розміру $k$ байтів (для 2048-бітного ключа це `256` байтів), тому вхід схеми доповнення OAEP не може перевищувати $k$. Сама схема OAEP резервує частину цього простору під випадкове початкове значення (seed) та службові поля довжиною $2 \cdot hLen + 2$, де $hLen = 32$ байти для SHA-256. Відповідно, максимальна довжина повідомлення становить $256 - 64 - 2 = 190$ байтів для RSA-2048 і $384 - 64 - 2 = 318$ байтів для RSA-3072.

### 9. Чим RSA-OAEP відрізняється від RSA-PSS за призначенням?

Обидві схеми базуються на RSA, але виконують різні криптографічні завдання. **RSA-OAEP** призначений для асиметричного шифрування та забезпечення конфіденційності: відправник шифрує короткі дані або симетричний ключ **публічним** ключем адресата, а розшифровує лише адресат своїм **приватним** ключем. **RSA-PSS**, навпаки, слугує для цифрового підпису (цілісності й автентичності): автор підписує хеш повідомлення власним **приватним** ключем, а будь-хто може перевірити цей підпис за **публічним** ключем автора.

### 10. Як зміниться результат перевірки підпису після зміни одного байта повідомлення?

Перевірка підпису гарантовано завершиться помилкою (`Verification failure`). Через лавинний ефект зміна навіть одного байта повністю змінює криптографічний хеш файлу, тож під час верифікації в RSA-PSS або ECDSA обчислений хеш не зійдеться з підписаним.

### 11. Що таке ECDSA і в чому його практична відмінність від RSA-PSS?

**ECDSA** (Elliptic Curve Digital Signature Algorithm) — алгоритм цифрового підпису на основі еліптичних кривих. Головна практична відмінність від RSA-PSS полягає в тому, що за однакового або вищого рівня стійкості ECDSA використовує набагато менші ключі та підписи: наприклад, у цій роботі ECDSA на `secp384r1` при 192-бітному рівні безпеки дав підпис `103` байти проти `256` байтів у RSA-2048 при ~112-бітній безпеці. До того ж генерація ключів і саме підписування на еліптичних кривих відбуваються швидше.

### 12. Чому для гібридного шифрування використовують два типи алгоритмів?

Симетричний алгоритм на зразок AES працює дуже швидко й дає змогу шифрувати файли довільного розміру, проте вимагає безпечного передавання спільного секретного ключа. Асиметричний RSA-OAEP легко вирішує задачу обміну ключами без попереднього спільного секрету, оскільки шифрує сесійний ключ публічним ключем адресата, але є надто повільним для великих обсягів даних і жорстко обмежений за довжиною повідомлення. Поєднання обох підходів дає водночас зручний обмін ключами та високу швидкість шифрування.

### 13. Чому в завданні 10 використовують окремі AES- і HMAC-ключі?

Використання незалежно згенерованих `aes.key` та `mac.key` реалізує принцип криптографічного розділення ключів (*key separation*). Один і той самий ключ не варто застосовувати одночасно для шифрування (AES-CTR) і для обчислення коду автентифікації (HMAC-SHA-256), щоб алгебраїчні чи протокольні взаємодії між двома примітивами не послаблювали захист і щоб компрометація одного ключа не призводила до зламу іншого механізму.

### 14. Чи підтверджує гібридний пакет завдання 10 особу відправника? Обґрунтуйте.

**Ні, не підтверджує.** Зв'язка ключів `bundle.rsa` шифрувалася **публічним** ключем отримувача (`rsa_pub.pem`), який доступний будь-кому. Через це будь-який сторонній учасник може самостійно згенерувати власні `aes.key` та `mac.key`, зашифрувати файл, обчислити валідний `hybrid.hmac` і надіслати пакет від чужого імені. Перевірка HMAC тут доводить лише те, що `hybrid.enc` та `iv.hex` не змінювалися після створення `bundle.rsa`, але не ідентифікує автора. Для підтвердження особи відправника пакет потрібно додатково підписати його **приватним** ключем через RSA-PSS або ECDSA.

---

### Індивідуальне аналітичне запитання (Варіант 6)

**Запитання:** *Чому CTR без MAC дає конфіденційність, але не цілісність?*

**Відповідь:**  
У режимі AES-CTR шифротекст обчислюється як $C = P \oplus E_K(\text{Nonce} \parallel \text{Counter})$. Поки ключ $K$ невідомий, а значення $\text{Nonce} \parallel \text{Counter}$ ніколи не повторюється, послідовність блоків на виході $E_K$ не відрізняється для стороннього спостерігача від випадкового потоку. Перехопивши лише шифротекст $C$, зловмисник не може відновити відкритий текст $P$, що й забезпечує **конфіденційність**.

Проте сам режим CTR залишається лінійним (пластичним) відносно операції XOR і не має жодного криптографічного тегу перевірки. Якщо зловмисник змінить шифротекст на $C' = C \oplus \Delta$, то отримувач під час розшифрування обчислить $P' = C' \oplus E_K(\text{Nonce} \parallel \text{Counter}) = P \oplus \Delta$. Розшифрування при цьому завершиться без жодних помилок формату чи padding (`exit code 0`), тож атакуючий може прицільно інвертувати конкретні біти або змінювати відомі фрагменти повідомлення. Гарантувати **цілісність** здатне лише додавання тегу автентифікації (за схемою Encrypt-then-MAC, як у завданні 10) або використання автентифікованого режиму AEAD на кшталт AES-GCM чи ChaCha20-Poly1305.

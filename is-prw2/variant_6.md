# Варіант 6

<table>
  <tr>
    <td>Цільовий SHA-256</td>
    <td>e1f12989372b5c4d69744b8bcdcb398076dae98495fdb713a2c5829d7af3b47b</td>
  </tr>
  <tr>
    <td>Облікові записи A, B, C</td>
    <td>frank_A, frank_B, frank_C</td>
  </tr>
  <tr>
    <td>Додатковий пароль C</td>
    <td>island85</td>
  </tr>
  <tr>
    <td>HMAC-ключ (hex)</td>
    <td>0d5552ceec77874c4079b42ffb324b86</td>
  </tr>
  <tr>
    <td>Початкове повідомлення</td>
    <td>user=frank&operation=pay&amount=340&ref=06</td>
  </tr>
  <tr>
    <td>Змінене повідомлення</td>
    <td>user=frank&operation=pay&amount=840&ref=06</td>
  </tr>
  <tr>
    <td>Ситуація для аналізу</td>
    <td>Зберігають SHA256(random_salt || password), але не застосовують жодної повільної функції.</td>
  </tr>
</table>

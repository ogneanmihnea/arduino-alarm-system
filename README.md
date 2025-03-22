# 🔒 Sistem de Securitate cu Arduino și Interfață Web

Acest proiect integrează o placă **Arduino** cu mai mulți senzori (flacără, sunet, temperatură, etc.) și o **interfață web (Dashboard)** ce permite monitorizarea în timp real și alertarea utilizatorului în cazul unor evenimente critice (precum incendii, șocuri sau zgomot puternic).

## 🛠️ Tehnologii utilizate

- **Frontend**: HTML, CSS, JavaScript
- **Backend**: Node.js + Express
- **Comunicare**: SerialPort între Arduino și server
- **Senzori**:
  - IR Flame Sensor
  - Modul sunet
  - Senzor temperatură (analogic)
  - Shock / Tilt / Light sensors

## ⚙️ Cum funcționează

1. Arduino trimite datele senzorilor prin portul serial.
2. Serverul Node.js primește aceste date și le procesează.
3. Interfața `dashboard.html` se conectează la backend și actualizează live valorile senzorilor.
4. Dacă un senzor depășește un prag critic (ex: temperatură mare, sunet puternic), sistemul activează **o alarmă vizuală** și **un popup de alertă** în dashboard.

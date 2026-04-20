# Terminalowa Gra Strategiczna (Klient-Serwer w C)

Prosta gra strategiczna działająca w terminalu, oparta na architekturze klient-serwer. Projekt został zrealizowany w języku C z wykorzystaniem mechanizmów komunikacji międzyprocesowej (IPC) w standardzie System V (kolejki komunikatów, pamięć współdzielona, semafory).

## 📝 Opis Projektu

W grze bierze udział maksymalnie dwóch graczy. Celem gry jest zdobycie **5 Punktów Zwycięstwa** poprzez udane ataki na bazę przeciwnika. Gracze zarządzają surowcem (złotem), rozbudowują swoją armię oraz gospodarkę (rekrutując robotników), a następnie wysyłają jednostki do walki.

## 🚀 Funkcje
* **Architektura klient-serwer** obsługująca wielu klientów (zoptymalizowana dla 2 graczy).
* **Odświeżanie stanu w czasie rzeczywistym** po stronie gracza.
* **System ekonomii i walki** oparty na statystykach różnych typów jednostek.
* **Asynchroniczne przetwarzanie** - wydzielony proces serwera (fork) generuje w tle surowce i zarządza upływem czasu produkcji.
* **Zarządzanie zasobami systemowymi** - automatyczne sprzątanie kolejek i semaforów po zatrzymaniu serwera za pomocą `Ctrl+C`.

## 🛠 Wymagania
* System operacyjny oparty na jądrze Linux (ze względu na użycie IPC System V).
* Kompilator `gcc`.
* Narzędzie `make`.

## ⚙️ Kompilacja i Uruchamianie

1. **Skompiluj projekt** za pomocą dołączonego pliku `Makefile`. W głównym katalogu wpisz:
   ```bash
   make all
   ```
2. **Uruchom serwer** (wymagany przed podłączeniem klientów):
   ```bash
   ./server
   ```
3. **Uruchom klientów** (najlepiej w osobnych oknach terminala):
   ```bash
   ./client
   ```
4. Aby wyczyścić pliki wykonywalne po zakończeniu pracy:
   ```bash
   make clean
   ```

## 🎮 Jak grać?

Podczas gry w terminalu wyświetlane jest menu akcji oraz aktualny stan surowców i wojsk gracza. 

### 💰 Surowce (Złoto)
Złoto jest generowane automatycznie co sekundę przez serwer.
* Bazowy przyrost: **+50 sztuk złota / s**
* Premia od każdego zrekrutowanego robotnika: **+5 sztuk złota / s**

### ⚔️ Jednostki
Każda jednostka kosztuje określoną ilość złota i wymaga czasu na wyprodukowanie.

| Typ Jednostki | Koszt (Złoto) | Czas budowy | Siła Ataku | Siła Obrony |
| :--- | :---: | :---: | :---: | :---: |
| **1. Lekka Piechota** | 100 🪙 | 2 s | 1.0 | 1.2 |
| **2. Ciężka Piechota**| 250 🪙 | 3 s | 1.5 | 3.0 |
| **3. Jazda** | 500 🪙 | 5 s | 3.5 | 1.2 |
| **4. Robotnicy** | 150 🪙 | 2 s | 0.0 | 0.0 |

*Uwaga: W danym momencie w kolejce produkcji może znajdować się tylko jedna jednostka (w innym przypadku serwer zwróci komunikat "Kolejka zajęta!").*

### ⚙️ Akcje gracza
1. **Odśwież (Pobierz stan)** – Synchronizuje ekran i wyświetla najnowszy stan surowców oraz armii bez podejmowania dodatkowych akcji.
2. **Kup jednostkę** – Wyświetla menu pozwalające na rozpoczęcie rekrutacji jednostki wybranego typu.
3. **Atakuj** – Rozpoczyna bitwę. Wysyła całą dostępną armię do ataku na przeciwnika. 

### 🏆 Walka i Punkty Zwycięstwa
Bitwy obliczane są automatycznie na serwerze po zsumowaniu siły ataku agresora i siły obrony obrońcy.
Jeżeli atakujący wygra (Całkowita Siła Ataku > Całkowita Siła Obrony):
* Zdobywa **1 Punkt Zwycięstwa**.
* Zwycięża ten, kto pierwszy zgromadzi **5 Punktów Zwycięstwa**.
* Wojsko przegranego obrońcy zostaje zniszczone.

W przypadku przewagi obrońcy, napastnik ponosi proporcjonalne straty w swojej armii.

## 🏗 Szczegóły Techniczne (Dla programistów)
W projekcie zaimplementowano kluczowe mechanizmy współbieżności i IPC w C:
* **Kolejki Komunikatów (`msgget`, `msgsnd`, `msgrcv`)**: Używane do asynchronicznej wymiany żądań pomiędzy klientami a serwerem (np. logowanie `MSG_LOGIN`, prośby o odświeżenie danych `MSG_DATA`, rozpoczęcie budowy `MSG_TRAIN` oraz ataki `MSG_ATTACK`).
* **Pamięć Współdzielona (`shmget`, `shmat`)**: Serwer utrzymuje główną strukturę stanu gry (`GameState`) w pamięci dzielonej, by podproces zajmujący się pętlą gry miał bezproblemowy dostęp do danych operacyjnych.
* **Semafory (`semget`, `semop`)**: Zabezpieczają sekcje krytyczne pamięci współdzielonej (podczas logowania nowych graczy, dodawania surowców, uaktualniania ilości jednostek czy przeprowadzania bitew).
* **Obsługa Sygnałów (`signal`)**: Bezpieczne kończenie pracy serwera po przechwyceniu sygnału `SIGINT` (Ctrl+C), co skutkuje wyczyszczeniem struktur i usunięciem instancji IPC (`IPC_RMID`), zapobiegając wyciekom zasobów z systemu operacyjnego.

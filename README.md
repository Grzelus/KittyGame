# KittyGame

Prosta gra survivalowa 2D napisana w C++17 z użyciem SFML 2.6.1. Gracz steruje kotem, porusza się po mapie, strzela do przeciwników, zbiera doświadczenie i rozwija postać, żeby przetrwać jak najdłużej.

## O co chodzi w grze

Rozgrywka działa w pętli przetrwania:

1. Gracz porusza się po planszy i eliminuje kolejne fale wrogów.
2. Co pewien czas pojawiają się silniejsze fale przeciwników.
3. Za zabójstwa zdobywane jest doświadczenie i punkty.
4. Po zdobyciu wystarczającej liczby punktów doświadczenia pojawia się ekran ulepszenia.
5. Po 60 sekundach rozgrywki pojawia się boss.
6. Gra kończy się, gdy pasek życia gracza spadnie do zera.

## Sterowanie

- `W`, `A`, `S`, `D` - ruch postaci.
- Mysz - celowanie.
- Broń podstawowa i kolejne bronie są przełączane klawiszami `Z`, `X`, `C`, `V`.
- Gdy pojawi się ekran ulepszenia, wybór wykonuje się klawiszami `1`, `2`, `3`, `4`.
- `R` - restart po przegranej.
- `Q` - zamknięcie gry po przegranej.

## System broni i ulepszeń

Postać startuje z pistoletem. Kolejne bronie odblokowują się wraz z poziomami postaci:

- poziom 2 - odblokowanie drugiej broni,
- poziom 4 - odblokowanie trzeciej broni,
- poziom 6 - odblokowanie czwartej broni.

W grze dostępne są cztery style strzału:

- pistolet,
- SMG,
- shotgun,
- golden shotgun.

Po zdobyciu poziomu gra zatrzymuje normalną akcję i pokazuje ekran wyboru jednego z czterech ulepszeń:

- `1` - większa szybkość ruchu,
- `2` - większy atak,
- `3` - większe maksymalne HP i pełne leczenie,
- `4` - większa szybkość ataku.

## Przeciwnicy

Przeciwnicy pojawiają się falami co 10 sekund. Każda kolejna fala jest większa i trudniejsza, a wrogowie z czasem mają więcej życia oraz zadają większe obrażenia. Przeciwnicy biegną w stronę gracza i zadają obrażenia przy kontakcie.

Boss pojawia się po 60 sekundach, jeśli wcześniej nie został pokonany. Porusza się szybciej niż zwykli wrogowie, ma więcej HP i co 2 sekundy wystrzeliwuje pociski wybuchowe. Trafienie pociskiem bossa zadaje obrażenia graczowi.

## Punkty i progresja

- zwykły wróg daje 3 XP,
- boss daje 50 XP i 100 punktów do wyniku,
- zabicie wroga zwiększa licznik zabójstw,
- zdobywanie XP powoduje poziomowanie postaci i odblokowywanie kolejnych broni.

Na ekranie HUD wyświetlane są:

- aktualne HP,
- licznik zabójstw,
- aktualnie używana broń.

## Warunki przegranej i restart

Gra kończy się, gdy HP gracza spadnie do zera. Po przegranej można:

- nacisnąć `R`, aby zrestartować rozgrywkę,
- nacisnąć `Q`, aby wyjść z gry.

## Podział prac

### Antek (Antoni Małecki)

- podstawowa architektura klas i rozwój `main.cpp`,
- mechanika ruchu (`WSAD`), wybór broni i logika strzelania,
- ekran ulepszeń i rozwijanie systemu wyboru bonusów,
- ekran game over i elementy przebiegu rundy,
- licznik zabójstw, etykiety zdrowia i część HUD,
- logika informowania o odblokowaniu nowej broni,
- integracja ikon broni po stronie kodu.

### Kacper (Kacper Grzelak)

- setup projektu (m.in. konfiguracja CMake i pliki startowe),
- implementacja i poprawki logiki bossa,
- zachowania bossa oraz poprawki hitboxów bomb/pocisków bossa,
- system fal przeciwników i rozwój zachowań wrogów,
- przygotowanie i integracja assetów graficznych (tła, sprite'y, ikony, ekrany),
- rozwój warstwy wizualnej gry (HUD, ekran broni, game over, tło).

Uwaga: część prac była rozwijana równolegle w tym samym pliku i poprawiana przez obie osoby, 

## Uruchomienie

Projekt korzysta z CMake i pobiera SFML przez `FetchContent`.

Wymagania:

- kompilator obsługujący C++17,
- CMake 3.16 lub nowszy,
- dostęp do internetu przy pierwszym buildzie, żeby pobrać SFML,
- plik `arial.ttf` w katalogu gry, jeśli chcesz widzieć tekst w interfejsie.

Przykładowe uruchomienie z katalogu głównego repozytorium:

```powershell
cmake -S . -B build
cmake --build build
```

Po zbudowaniu uruchamiany jest plik wykonywalny `KittyGame`.

## Zasoby

Gra ładuje grafiki z katalogu `KittyGame/assets`. Jeśli któregoś pliku brakuje, kod tworzy prosty zamiennik awaryjny, żeby gra nadal mogła się uruchomić.

## Krótko

To klasyczna gra survivalowa z falami przeciwników, rozwojem postaci, kilkoma typami broni i bossem pojawiającym się po czasie. Celem jest przetrwać jak najdłużej i nabić jak najwyższy wynik.

# GRK Underwater World

Autorzy: Aleksandr Petkevich, Aliaksei Shymanski (grupa 14)

Interaktywna scena podwodna napisana w C++ z użyciem OpenGL i GLSL. Projekt przedstawia nocną głębinę oceaniczną z batyskafem, ruchomym reflektorem, falującą powierzchnią wody, dnem morskim, roślinnością, skałami, rybami poruszającymi się po krzywych oraz efektami wolumetrycznego światła księżyca i reflektora.

## Cel Projektu

Celem projektu jest przygotowanie interaktywnej aplikacji graficznej w C++ z użyciem OpenGL i GLSL. Motywem projektu jest spójna scena podwodna: nocna głębina oceaniczna z batyskafem, dnem morskim, roślinnością, rybami i efektami światła w wodzie.

Projekt pokazuje poprawną implementację wymaganych metod renderingu oraz integrację kilku technik w jedną działającą scenę czasu rzeczywistego. Scena zawiera więcej niż trzy znaczące interakcje poza samym ruchem kamery, m.in. sterowanie reflektorem, zbieranie próbek roślinności, zmianę parametrów środowiska, zmianę materiałów i interakcję z rybami przez tryb prezentacji ich ścieżek.

## Wymagania Techniczne

- C++20
- Win32 API
- OpenGL 3.3
- GLSL
- Visual Studio / MSBuild
- Własne moduły matematyczne: wektory, macierze, kwaterniony
- Modele OBJ w katalogu `assets/models`

Projekt nie korzysta z gotowego silnika renderującego ani silnika gry, takiego jak Unity, Unreal Engine, Godot albo Blender Game Engine. HUD jest własnym prostym interfejsem renderowanym w OpenGL i nie zastępuje implementacji wymaganych metod renderingu.

## Najważniejsze Elementy Sceny

- Batyskaf sterowany jak kamera pierwszoosobowa lub obserwowany z kamery trzecioosobowej.
- Nocna scena podwodna z cubemapą, gwiazdami i księżycem.
- Proceduralne dno morskie z materiałami piasku i skał.
- Falująca powierzchnia wody.
- Roślinność morska, skały i unoszące się cząsteczki w wodzie.
- Ryby poruszające się po zamkniętych krzywych Catmull-Rom.
- Reflektor batyskafu oświetlający dno i obiekty.
- Wolumetryczne smugi światła w wodzie.
- HUD renderowany bezpośrednio w OpenGL.

## Zgodność Z Regulaminem GRK

### Metody Obowiązkowe

- [x] Normal mapping  
  Widoczne użycie map normalnych na dwóch materiałach dna: piasek i skała. Wierzchołki terenu zawierają tangent i bitangent, a shader używa poprawnej przestrzeni stycznej TBN.

- [x] PBR lighting  
  Shader terenu wykorzystuje wariant metallic/roughness z GGX, Fresnel-Schlick i parametrami materiałów możliwymi do zademonstrowania klawiszami.

- [x] Quaternion camera control  
  Kamera jest sterowana kwaternionami, bez gimbal lock. Ruch i rotacja batyskafu są płynne.

- [x] Shadow mapping  
  Cień od kierunkowego światła księżyca jest liczony przez depth texture. Podstawowe artefakty są ograniczane przez bias i PCF.

- [x] Parallel Transport Frames  
  Ramki transportu równoległego są wykorzystane znacząco do ruchu ryb po krzywych Catmull-Rom. Ich orientacja jest stabilizowana przez Parallel Transport Frame / Bishop frame. Tryb debug pozwala pokazać ścieżki i ramki.

- [x] Underwater skybox / cubemap  
  Cubemapa środowiskowa tworzy spójną podwodną atmosferę. Jest renderowana jako tło sceny i działa niezależnie od przesunięcia kamery.

### Wybrane Metody Dodatkowe

- [x] A01 - Volumetric underwater light shafts / single scattering  
  Scena zawiera podwodne smugi światła księżyca i reflektora batyskafu. Efekt jest zależny od położenia światła, gęstości ośrodka i intensywności ustawianej przez użytkownika. Implementacja korzysta z depth-aware ray marchingu oraz kopiowanej tekstury głębi sceny.

- [x] B13 - Moving point lights / submarine headlights  
  Reflektor batyskafu jest ruchomym źródłem światła sterowanym razem z pozycją i kierunkiem batyskafu. Oświetla teren, roślinność, skały, wodę, ryby i model batyskafu.

Wybrane metody A01 i B13 są widoczne w finalnej aplikacji, opisane w README i przygotowane do pokazania podczas krótkiego demo.

## Interakcje Użytkownika

Projekt zawiera więcej niż trzy znaczące interakcje poza samym ruchem kamery:

- Przełączanie reflektora batyskafu.
- Zmiana widoku pierwszoosobowego i trzecioosobowego.
- Zbieranie próbek roślinności morskiej.
- Zmiana parametrów PBR materiałów dna.
- Włączanie i regulacja światła wolumetrycznego.
- Zmiana poziomu wody i gęstości mgły.
- Przełączanie trybu debug ryb: ruch, ścieżki, zatrzymanie z ramkami PTF.
- Włączanie i wyłączanie HUD.

## Sterowanie

- `W`, `A`, `S`, `D` - ruch batyskafu.
- `Strzałki` - obrót batyskafu.
- `Q`, `E` - ruch pionowy w dół i w górę.
- `Page Up`, `Page Down` - zmiana poziomu wody.
- `Z`, `X` - zmiana gęstości mgły.
- `T` - przełączenie widoku pierwszoosobowego i trzecioosobowego.
- `L` lub `F1` - włączenie lub wyłączenie reflektora.
- `O` - włączenie lub wyłączenie efektu volumetric light shafts.
- `Y`, `U` - zwiększenie lub zmniejszenie intensywności efektu wolumetrycznego.
- `G` - zebranie najbliższej próbki roślinności.
- `P` - przełączenie trybu ryb: normalny ruch, widoczne ścieżki, pauza ze ścieżkami i ramkami PTF.
- `F2` - włączenie lub wyłączenie HUD.
- `C` - reset pozycji kamery / batyskafu.
- `1`, `2` - wybór aktywnego materiału: piasek albo skała.
- `R`, `F` - zwiększenie lub zmniejszenie roughness aktywnego materiału.
- `M`, `N` - zwiększenie lub zmniejszenie metallic aktywnego materiału.
- `B`, `V` - zwiększenie lub zmniejszenie siły normal mappingu.
- `H` - tryb debug materiałów.
- `Esc` - zamknięcie programu.

## Struktura Projektu

- `main.cpp` - główna pętla aplikacji, inicjalizacja sceny, obsługa wejścia i kolejność renderowania.
- `camera.*` - kamera kwaternionowa i ruch batyskafu.
- `math.*` - wektory, macierze, kwaterniony i pomocnicze funkcje matematyczne.
- `gl_loader.*` - ładowanie funkcji OpenGL.
- `shader.*` - wczytywanie i kompilacja shaderów GLSL.
- `terrain.*` - proceduralne dno, tangent, bitangent i dane pod normal mapping.
- `water.*` - siatka powierzchni wody i fale w shaderze.
- `skybox.*` - cubemapa nocnej sceny podwodnej.
- `shadow.*` - shadow mapping.
- `props.*` - roślinność morska i skały.
- `submarine.*` - model batyskafu.
- `fish.*` - ryby, modele OBJ i rysowanie ryb.
- `spline.*` - krzywe Catmull-Rom i Parallel Transport Frames.
- `volumetric.*` - wolumetryczne światło i depth-aware ray marching.
- `particles.*` - cząsteczki unoszące się w wodzie.
- `hud.*` - HUD renderowany w OpenGL.
- `shaders/` - shadery GLSL.
- `assets/models/` - modele OBJ używane w scenie.

## Modele I Zasoby

Projekt oczekuje modeli w katalogu `assets/models`:

- `submarine.obj`
- `seaweed.obj`
- `clownfish.obj`
- `carp.obj`

Plik projektu Visual Studio kopiuje katalog `assets` do katalogu wyjściowego po zbudowaniu aplikacji.

## Budowanie I Uruchamianie

### Visual Studio

1. Otwórz `grk-underwater-world.vcxproj` albo rozwiązanie projektu, jeśli jest dostępne.
2. Wybierz konfigurację `Debug` lub `Release`.
3. Wybierz platformę `x64`.
4. Zbuduj projekt.
5. Uruchom aplikację z Visual Studio.

### MSBuild

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" "grk-underwater-world.vcxproj" /p:Configuration=Debug /p:Platform=x64 /m
```

Po poprawnej kompilacji plik wykonywalny znajduje się w:

```text
x64/Debug/grk-underwater-world.exe
```

## Scenariusz Prezentacji

1. Pokazać nocną cubemapę i powierzchnię wody.
2. Zaprezentować ruch batyskafu oraz przełączanie widoku `T`.
3. Włączyć i wyłączyć reflektor `L` / `F1`, pokazując B13.
4. Włączyć i regulować volumetric light shafts przez `O`, `Y`, `U`, pokazując A01.
5. Pokazać PBR i normal mapping przez zmianę materiału `1` / `2`, roughness `R` / `F`, metallic `M` / `N` i siłę normal map `B` / `V`.
6. Pokazać shadow mapping na dnie i obiektach w scenie.
7. Zebrać próbkę roślinności klawiszem `G`.
8. Przełączyć tryb ryb klawiszem `P`, aby pokazać ścieżki i ramki Parallel Transport Frames.
9. Włączyć lub wyłączyć HUD klawiszem `F2`.

## Uwagi Wydajnościowe

- HUD jest buforowany i aktualizuje geometrię tylko wtedy, gdy zmienia się jego treść lub rozmiar okna.
- Depth texture dla efektu wolumetrycznego jest alokowana tylko przy zmianie rozmiaru, a potem aktualizowana przez `glCopyTexSubImage2D`.
- Gdy efekt wolumetryczny jest wyłączony klawiszem `O`, kosztowny pass wolumetryczny nie jest wykonywany.
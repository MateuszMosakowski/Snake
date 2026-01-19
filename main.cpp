#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

using namespace std;

enum class Typ_pola {
  puste,
  sciana,
  waz,
  owoc,
};

enum class Ruchy { gora, dol, prawo, lewo, brak };

struct Punkt {
  int x, y;
  bool operator==(const Punkt &other) const {
    return x == other.x && y == other.y;
  }
};

class Waz;
class Owoc;

class Plansza {
  int szerokosc, wysokosc;
  vector<vector<Typ_pola>> pola;

public:
  Plansza(int szer, int wys) : szerokosc(szer), wysokosc(wys) {
    pola.resize(wysokosc, vector<Typ_pola>(szerokosc, Typ_pola::puste));
  }

  int getSzerokosc() const { return szerokosc; }
  int getWysokosc() const { return wysokosc; }

  void czysc() {
    for (int y = 0; y < wysokosc; y++) {
      fill(pola[y].begin(), pola[y].end(), Typ_pola::puste);
    }
    // Sciany
    for (int x = 0; x < szerokosc; x++) {
      pola[0][x] = Typ_pola::sciana;
      pola[wysokosc - 1][x] = Typ_pola::sciana;
    }
    for (int y = 0; y < wysokosc; y++) {
      pola[y][0] = Typ_pola::sciana;
      pola[y][szerokosc - 1] = Typ_pola::sciana;
    }
  }

  void ustaw_pole(int x, int y, Typ_pola typ) {
    if (x >= 0 && x < szerokosc && y >= 0 && y < wysokosc)
      pola[y][x] = typ;
  }

  void narysuj() {
    // Ustawienie kursora na pocz¹tku zamiast cls, ¿eby mniej miga³o
    COORD cursorPosition;
    cursorPosition.X = 0;
    cursorPosition.Y = 0;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);

    string buffer = "";
    for (int y = 0; y < wysokosc; y++) {
      for (int x = 0; x < szerokosc; x++) {
        switch (pola[y][x]) {
        case Typ_pola::puste:
          buffer += " ";
          break;
        case Typ_pola::sciana:
          buffer += "#";
          break;
        case Typ_pola::waz:
          buffer += "O";
          break;
        case Typ_pola::owoc:
          buffer += "&";
          break;
        }
      }
      buffer += "\n";
    }
    cout << buffer;
  }
};

class Waz {
  deque<Punkt> cialo;
  Ruchy kierunek;

public:
  Waz(int startX, int startY) {
    cialo.push_front({startX, startY});
    kierunek = Ruchy::brak;
  }

  void ustaw_kierunek(Ruchy k) {
    // Zapobieganie zawracaniu w miejscu
    if (k == Ruchy::gora && kierunek != Ruchy::dol)
      kierunek = k;
    else if (k == Ruchy::dol && kierunek != Ruchy::gora)
      kierunek = k;
    else if (k == Ruchy::lewo && kierunek != Ruchy::prawo)
      kierunek = k;
    else if (k == Ruchy::prawo && kierunek != Ruchy::lewo)
      kierunek = k;
    else if (kierunek == Ruchy::brak)
      kierunek = k;
  }

  Ruchy pobierz_kierunek() { return kierunek; }

  Punkt nastepna_pozycja() {
    Punkt glowa = cialo.front();
    switch (kierunek) {
    case Ruchy::gora:
      glowa.y--;
      break;
    case Ruchy::dol:
      glowa.y++;
      break;
    case Ruchy::lewo:
      glowa.x--;
      break;
    case Ruchy::prawo:
      glowa.x++;
      break;
    case Ruchy::brak:
      break;
    }
    return glowa;
  }

  void rusz(bool zjadl_owoc) {
    if (kierunek == Ruchy::brak)
      return;

    Punkt nowa_glowa = nastepna_pozycja();
    cialo.push_front(nowa_glowa);
    if (!zjadl_owoc) {
      cialo.pop_back();
    }
  }

  bool sprawdz_kolizje(int szer, int wys) {
    if (kierunek == Ruchy::brak)
      return false;

    Punkt glowa = cialo.front();
    // Kolizja ze œcianami
    if (glowa.x <= 0 || glowa.x >= szer - 1 || glowa.y <= 0 ||
        glowa.y >= wys - 1)
      return true;

    // Kolizja z samym sob¹
    for (size_t i = 1; i < cialo.size(); ++i) {
      if (glowa == cialo[i])
        return true;
    }
    return false;
  }

  const deque<Punkt> &pobierz_cialo() const { return cialo; }

  Punkt pobierz_glowe() const { return cialo.front(); }
};

class Owoc {
  Punkt pozycja;

public:
  void generuj(const Plansza &p, const Waz &w) {
    bool ok = false;
    while (!ok) {
      int x = rand() % (p.getSzerokosc() - 2) + 1;
      int y = rand() % (p.getWysokosc() - 2) + 1;
      ok = true;
      for (const auto &segment : w.pobierz_cialo()) {
        if (segment.x == x && segment.y == y) {
          ok = false;
          break;
        }
      }
      if (ok)
        pozycja = {x, y};
    }
  }

  Punkt pobierz_pozycje() const { return pozycja; }
};

class Gra {
  Plansza plansza;
  Waz waz;
  Owoc owoc;
  bool koniec;
  int wynik;

public:
  Gra() : plansza(20, 20), waz(10, 10), koniec(false), wynik(0) {
    srand(static_cast<unsigned int>(time(NULL)));
    owoc.generuj(plansza, waz);
  }

  void ukryj_kursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
  }

  void graj() {
    koniec = false;
    wynik = 0;
    waz = Waz(10, 10);
    owoc.generuj(plansza, waz);
    waz.ustaw_kierunek(Ruchy::prawo);

    system("cls");
    ukryj_kursor();

    // Rysowanie stanu poczatkowego
    plansza.czysc();
    for (const auto &seg : waz.pobierz_cialo()) {
      plansza.ustaw_pole(seg.x, seg.y, Typ_pola::waz);
    }
    plansza.ustaw_pole(owoc.pobierz_pozycje().x, owoc.pobierz_pozycje().y,
                       Typ_pola::owoc);
    plansza.narysuj();
    cout << "Wynik: " << wynik << " (Sterowanie: W, A, S, D, Q-wyjscie)"
         << endl;
    cout << "Nacisnij 'F', aby rozpoczac gre..." << endl;
    while (true) {
      char c = _getch();
      if (c == 'f' || c == 'F')
        break;
    }

    while (!koniec) {
      // Obs³uga klawiszy
      if (_kbhit()) {
        char znak = _getch();
        switch (znak) {
        case 'w':
          waz.ustaw_kierunek(Ruchy::gora);
          break;
        case 's':
          waz.ustaw_kierunek(Ruchy::dol);
          break;
        case 'a':
          waz.ustaw_kierunek(Ruchy::lewo);
          break;
        case 'd':
          waz.ustaw_kierunek(Ruchy::prawo);
          break;
        case 'q':
          koniec = true;
          break;
        }
      }

      // Logika gry
      if (waz.pobierz_kierunek() != Ruchy::brak) {
        Punkt next = waz.nastepna_pozycja();
        bool zjadl = (next.x == owoc.pobierz_pozycje().x &&
                      next.y == owoc.pobierz_pozycje().y);

        waz.rusz(zjadl);

        if (zjadl) {
          wynik += 10;
          owoc.generuj(plansza, waz);
        }

        if (waz.sprawdz_kolizje(plansza.getSzerokosc(),
                                plansza.getWysokosc())) {
          koniec = true;
        }
      }

      // Rysowanie
      plansza.czysc();
      for (const auto &seg : waz.pobierz_cialo()) {
        plansza.ustaw_pole(seg.x, seg.y, Typ_pola::waz);
      }
      plansza.ustaw_pole(owoc.pobierz_pozycje().x, owoc.pobierz_pozycje().y,
                         Typ_pola::owoc);

      plansza.narysuj();
      cout << "Wynik: " << wynik << " (Sterowanie: W, A, S, D, Q-wyjscie)"
           << endl;
      Sleep(100);
    }
    cout << "Koniec gry! Wynik: " << wynik << endl;
    cout << "Nacisnij dowolny klawisz..." << endl;
    _getch();
  }

  void menu() {
    while (true) {
      system("cls");
      cout << "=== SNAKE ===" << endl;
      cout << "1. Nowa gra" << endl;
      cout << "2. Wyjscie" << endl;
      cout << "Wybierz: ";

      char opcja = _getch();
      // _getch() jest lepsze niz cin bo nie trzeba enter, ale trzeba obsluzyc
      // znak

      if (opcja == '1') {
        graj();
      } else if (opcja == '2') {
        exit(0);
      }
    }
  }
};

int main() {
  Gra gra;
  gra.menu();
  return 0;
}

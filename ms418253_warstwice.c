#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// #define PRINT(S,X) printf("%s %f\n", S, X);

// W naszych programie na początku wprowadzamy wielomian dwóch zmiennych oraz przehowujemy go za pomocą listy
// następnie, po wprowadzeniu współrzędnych punktów danego prostokąta, w którym to mają się znajdować warstwice
// dzielimy prostokąt na na mniejsze prostokąty (w zależności od wartości stałej LICZBA_PODZIAŁU)
// w owych prostokątach liczymy wartości wielomianu w zależności od współrzędnych w tym prostokącie
// oraz m.in. za pomocą bnsearchu staramy się znaleźć jak najlepsze jego przybliżenie
// otrzymane dane przesyłamy do gnuplota, który drukuje nam wykres

const float DUZE_M =  1000.0; /* okresla z jaka dokladnoscia szukamy w binsearchu, np jesli chcemy znac wynik z dokladnoscia 0.01 * delta, to DUZE_M = 100 */
const float EPSILON = 0.01; // stala okreslajaca zadowalajace przyblizenie do warstwicy przy szukaniu przyblizenia wśród prawie samych mniejszych/większych wartosci
const int LICZBA_PODZIALU = 100; // na ile mniejszych prostokątów będziemy dzielili nasz przedział

typedef struct {
    float x;
    float y;
} punkt;

punkt dodawanie_punktow(punkt a, punkt b)
{
    a.x += b.x;
    a.y += b.y;

    return a;
}

punkt dzielenie_punktu(punkt a, float b)
{
    a.x /= b;
    a.y /= b;

    return a;
}

punkt proporcje(punkt a, punkt b, float f)
{
    punkt wyn;
    wyn.x = ( a.x * f ) + (b.x * (1 - f) );
    //a.x + (b.x - a.x) * f;
    wyn.y = ( a.y * f ) + (b.y * (1 - f) );
    //a.y + (b.y - a.y) * f;
    return wyn;
}

bool porownywanie_punktu(punkt a, punkt b)
{
    return (a.x == b.x && a.y == b.y);
} /* porownuje czy dwa punkty z plaszczyzny są tym samym punktem */

punkt odleg(punkt a, punkt b) {

    punkt wyn;
    wyn.x = fabs(a.x - b.x);
    wyn.y = fabs(a.y - b.y);
    return wyn;

} /* zwraca odleglosci miedzy rzutami danych punktow na poszczegolne osie */

void drukuj_punkt(punkt a) {
    //printf("%f | ", a.x);
    //printf("%f\n", a.y);
    //FILE* PLIK_DANE = popen("gnuplot -persistent", "w");
    FILE* data = fopen("dane.temp", "a");
    fprintf(data, "%f %f \n", a.x, a.y);
    fclose(data);

} /* wypisuje wspolrzedne punktu oddzielone pionową kreską */

float pot(float x, int potega)
{
    float wyn;
    if (potega == 0) {
        wyn = 1;
    } else {
        wyn = x * pot(x, potega - 1);
    }
    return wyn;
} /* podnosi liczbę x do potęgi potęga */


float wartosc_jednomianu(punkt a, float wspolczynnik, int potega_x, int potega_y)
{
    float wyn;
    if (wspolczynnik == 0)
        wyn = 0;
    else
        wyn = wspolczynnik * pot(a.x, potega_x) * pot(a.y, potega_y);
    return wyn;
} /* funkcja w wyniku zwraca wartość wartosc_jednomianu dwóch zmiennych w zależności od wspolczynników oraz poteg */

typedef struct {
    
    int x;
    int y;
    float wsp;

} jednomian;

typedef struct Element {

    jednomian teraz;
    struct Element* nast;

} Element;

float wielomian_po_liscie(Element* a, punkt zmienna){

    float wyn = 0;
    if (a == NULL)
        return wyn;

    do {
    	wyn += wartosc_jednomianu(zmienna, a->teraz.wsp, a->teraz.x, a->teraz.y );
    	a = a-> nast;
    }
    	while (a != NULL);


    return wyn;
} /* wylicza wartość wieomianu trzymanego na liście dla danej zmiennej */


punkt dodaj_do_x(punkt k, float l)
{
    k.x += l;
    return k;
} /* zwieksza pierwsza wspolrzedna punktu o l */

punkt dodaj_do_y(punkt k, float l)
{
    k.y += l;
    return k;
} /* zwieksza druga wspolrzedna punktu o l */



// dodajemy element p do listy a
Element* dodaj_do_listy(jednomian p, Element* a)
{
    Element* nowa = (Element*)malloc(sizeof(Element));
    nowa->teraz = p;
    nowa->nast = a;
    return nowa;
}

void zwolnij_liste(Element* a)
{
    if (a == NULL)
        return;
    zwolnij_liste(a->nast);
    free(a);
}


void wypisz_liste(Element* w)
{
    if (w == NULL)
        return;
    printf("wsp: %f x^%d y^%d\n", w->teraz.wsp, w->teraz.x, w->teraz.y);
    wypisz_liste(w->nast);
}

void zczytywanie_listy(Element** w) {

    float coeff;
    int x, y;

    while(scanf("%f %d %d", &coeff, &x, &y) != EOF)
    {
        jednomian j;
        j.x = x;
        j.y = y;
        j.wsp = coeff;
        *w = dodaj_do_listy(j, *w);
    }
}

punkt znajdowanie_min_max(Element *w, punkt lewy_dol, punkt prawy_gorny) {

    punkt min_max;
    punkt tmp; tmp.x = 0; tmp.y = 0;

    min_max.y = min_max.x = wielomian_po_liscie(w, lewy_dol);

    float dl_x = fabs(lewy_dol.x - prawy_gorny.x);
    float dl_y = fabs(lewy_dol.y - prawy_gorny.y);
    for (int i = 0; i <= LICZBA_PODZIALU; ++i)
    {
        for (int j = 0; j <= LICZBA_PODZIALU; ++j)
        {
            punkt zmienna;
            zmienna.x = lewy_dol.x + ((i * dl_x) / LICZBA_PODZIALU);
            zmienna.y = lewy_dol.y + ((j * dl_y) / LICZBA_PODZIALU);
            float wiel = wielomian_po_liscie(w, zmienna);
            if (wiel < min_max.x) {
                min_max.x = wiel;
            } else {
                if (wiel > min_max.y) {
                	min_max.y = wiel;
                }
            }
        }
    }

    return min_max;
} /* szukamy minimum oraz maximum wśród punktów, które następnie mogą się znaleźć na wkresie */

float min_warstwic(Element* w, punkt lewy_dol, punkt prawy_gorny, float warstwica, float roznica) {


	punkt ekstrema = znajdowanie_min_max(w, lewy_dol, prawy_gorny);
	float tmp = warstwica;
	if (ekstrema.x < tmp) {
		while (ekstrema.x + roznica <= tmp ) {
			tmp -= roznica;
		}
	} else {
		while (ekstrema.x - roznica >= tmp) {
			tmp += roznica;
		}
	}
	return tmp;

}

punkt binsearch_dwa_punkty (Element *w, punkt lewo, punkt prawo, float a, float roznica) {

    if ( wielomian_po_liscie(w, lewo) > wielomian_po_liscie(w, prawo) ) {
        punkt tmp;
        tmp = lewo;
        lewo = prawo;
        prawo = tmp;
    }

    punkt s = dzielenie_punktu( dodawanie_punktow(lewo, prawo), 2);
    int i = 1;

    float f = a - wielomian_po_liscie(w, s);
    while (fabs(a - wielomian_po_liscie(w, s)) > ( roznica / DUZE_M ) ) {
        //printf ("%i obrót pętli\n", i);
        i++;

        // printf("%f\n",fabs(a - wielomian_po_liscie(w, s)) );

        s = dzielenie_punktu( dodawanie_punktow(lewo, prawo), 2);

        // drukuj_punkt(s);
        // printf("warotoc wielomianu w punkcie s:  %f\n", wielomian_po_liscie(w, s) );

        if (wielomian_po_liscie(w, s) > a) {
            prawo = s;
        } else {
            lewo = s;
        }
        
        if (porownywanie_punktu(s, dzielenie_punktu( dodawanie_punktow(lewo,prawo), 2 ) ) ||
            ( fabs(lewo.x - prawo.x) < (roznica / DUZE_M ) && fabs(lewo.y - prawo.y) < (roznica / DUZE_M )) )
        {
            a = wielomian_po_liscie(w, dzielenie_punktu( dodawanie_punktow(lewo, prawo), 2));
        }
        
    }
    
    // printf("%f\n",fabs(a - wielomian_po_liscie(w, s)) );
    // s = dzielenie_punktu( dodawanie_punktow(lewo, prawo), 2);

    return s;

} /* znajduje jedno z rozwiązań równania wielomianu = a z dokładnością co do roznica / DUZE_M na przedziale [lewo, prawo] */

int bool_calkowity(bool a)
{
    return (a ? 1 : 0);

} /* zwraca 1 gdy a == true oraz 0 gdy a == false */

int po_ktorej_stronie(Element *w, punkt k, float a) {

    return ( bool_calkowity ((wielomian_po_liscie(w, k) >= a))  );

} /*jezeli wielomian w punkcie k przyjmuje wartosc większą lub równą od a, to funkcja zwraca 1, w przeciwnym przypadku 0 */

int porow(float a, float b, int ktora_strona)
{
    int wyn;
    if (ktora_strona == 0) { // 0 czyli oba wierzcholki sa ponizej warstwicy
        wyn = bool_calkowity(a < b);
    } else {
        wyn = bool_calkowity(a >= b);
    }
    return wyn;
} /* sprawdza czy liczba a jest w zadanej relacji z b
    gdzie:
    '<'  = 0
    '>=' = 1

     na przykład, jeżeli ktora_strona == 0
     to jeżeli (a < b), to wyn = 1, czyli a < b, czyli są w tej samej relacji co ktora_strona
     gdyby np. ktora_strona == 1 oraz (a < b), to wyn = 0 */ 

float epsilon_strony(int ktora_strona, float roznica)
{
    float wyn;
    if (ktora_strona == 0) {
        wyn = -EPSILON * roznica;
    } else {
        wyn = EPSILON * roznica;
    }
    return wyn;
} /* zwraca dodatnią/ ujemną wartość w zależności od relacji, ktora wskazuje "ktora_strona" */

bool czy_jest_szansa(Element *w, punkt k, punkt l, float a, int ktora_strona, punkt *wskaznik, float roznica)
{
    bool wyn = false;
    punkt tmp;

    for (int i = 1; i < 5; ++i)
    {
        tmp = proporcje(k, l, 0.2 * i);
        if (porow(wielomian_po_liscie(w, tmp), a + epsilon_strony(ktora_strona, roznica), ktora_strona ) == 0) {
            wyn = true;
            *wskaznik = tmp;
        }
    }
    return wyn;

} /*funkcja sprawdza czy mimo tego, że pomiedzy wartosciami na wierzchołkach nie ma szukanej warstwicy, to czy jednak
gdzieś ona może się znaleźć, oczywiscie w przybliżeniu
oraz przekazuje przez wskaznik wartosc jednego z takich punktow (jezeli istnieje)*/


bool czy_po_roznych_stronach(punkt k, punkt l, float a, Element *w) {

    bool wyn;

    if ( (wielomian_po_liscie(w, k) - a) * (wielomian_po_liscie(w, l) - a)  <= 0) {
        wyn = true;
    } else {
        wyn = false;
    }

    return wyn;
}


punkt zamiana(punkt a, punkt b, int ktory_bok) {
    
    punkt wyn;
    punkt odleglosc;
    odleglosc = odleg(a, b);

    switch(ktory_bok) {

        /* teraz zmieniamy położenie pierwszego punktu */
        case 1: /* dół */ case 2: /* lewo */
            wyn = a;
            break;
        case 3: /* góra */ case 4: /* prawo */
            wyn = b;
            break;

        /* teraz zmieniamy położenie drugiego punktu */
        case 5: /* dół */
            wyn = dodaj_do_x(a, odleg(a,b).x);
            //  czemu jak napiszę "odleg.x" to działa poprawnie\\
                a jak napiszę odleglosc.x to juz nie????
            break;
        case 6: /* lewo */ case 7: /* góra */ 
            wyn = dodaj_do_y(a, odleg(a,b).y);
            break;
        case 8: /* prawo */
            wyn = dodaj_do_x(a, odleg(a,b).x);
            break;
    }

    return wyn;
} /* funkcja pomocnicza w binsearchu, służy do zamieniania wspołrzędnych punktów w zależności od tego,
na którym boku chcemy zobaczyć czy jest warstwica */



bool czy_jest_liczba(Element *w, punkt k, punkt l, float a, float roznica) {

    bool wyn = false;

    if (czy_po_roznych_stronach(k, l, a, w) ) {

        drukuj_punkt(binsearch_dwa_punkty(w, k, l, a, roznica));
        wyn = true;

    } else {

        int ktora_strona = po_ktorej_stronie(w, k, a);
        punkt tmp;
        if (czy_jest_szansa(w, k, l, a, ktora_strona, &tmp, roznica) ) {

            drukuj_punkt( binsearch_dwa_punkty(w, k, tmp, a, roznica) );
            wyn = true;

        }
    }

    return wyn;
} /* jeśli na danym odcinku znajduje się punkt z warstwicy zostanie on wypisany oraz wartość funkcji to TRUE
    w przeciwnym przypadku nie zostanie zostany żaden punkt, a wartość funkcji wyniesie FALSE */

void punkty_na_bokach_konkretnej_warstwicy(Element *w, float warstwica, punkt lewy_dol, punkt prawy_gorny, float roznica) {

    for (int i = 0; i < LICZBA_PODZIALU; ++i)
    {
        for (int j = 0; j < LICZBA_PODZIALU; ++j)
        {

            punkt odleglosc;

            punkt length = dzielenie_punktu(odleg(lewy_dol, prawy_gorny), LICZBA_PODZIALU);

            odleglosc.x = i * length.x;
            odleglosc.y = j * length.y;

            punkt pierwszy_rog;
            pierwszy_rog.x = lewy_dol.x + odleglosc.x;
            pierwszy_rog.y = lewy_dol.y + odleglosc.y;

            punkt drugi_rog;
            drugi_rog.x = pierwszy_rog.x + length.x;
            drugi_rog.y = pierwszy_rog.y + length.y;   
            
            
            for (int k = 1; k <= 4; k++)
            {
                punkt tmp_1 = zamiana(pierwszy_rog, drugi_rog, k);
                punkt tmp_2 = zamiana(pierwszy_rog, drugi_rog, k + 4);
                czy_jest_liczba(w, tmp_1, tmp_2, warstwica, roznica);   
            }
        }
    }
} /* lewy_dol ma wspolrzedne x_min, y_min, zaś prawy_gorny: x_max, y_max */


void czysc_plik_z_danymi() {

    FILE* tmp = fopen("dane.temp", "w");
    fclose(tmp); //czyścimy plik z danymi z poprzednich danych
}

void liczenie_po_kolei_wszystkich_warstwic(Element *w, punkt lewy_dol, punkt prawy_gorny, float warstwica_pocz, float roznica) {
    
    czysc_plik_z_danymi();
    
    punkt ekstrema = znajdowanie_min_max(w, lewy_dol, prawy_gorny);

    float aktualna_warstwica = min_warstwic(w, lewy_dol, prawy_gorny, warstwica_pocz, roznica);

    while(aktualna_warstwica <= ekstrema.y) {
        punkty_na_bokach_konkretnej_warstwicy(w, aktualna_warstwica, lewy_dol, prawy_gorny, roznica);
        aktualna_warstwica += roznica;
    }
} /*wypisuje wszystkie punkty, które znajdują się na warstwicach w zależności od DELTY oraz warstwicy początkowej */


int main()
{

    punkt lewy_dol, prawy_gorny;
    float warstwica_pocz, roznica;
    
    Element* w = NULL;

    printf("Wprowadź jednomiany według wzoru: współczynnik potęga x potęga y, a gdy skończysz je wpisywać wciśnij CTRL+D\n");
    zczytywanie_listy(&w);

    printf("Wprowadź punkty (x_min, y_min) oraz (x_max, y_max)\n");
    scanf("%f%f%f%f", &lewy_dol.x, &lewy_dol.y, &prawy_gorny.x, &prawy_gorny.y);

    printf("Wprowadź interesującą Cię warstwice oraz różnicę między warstwicami (pozostałe warstwice różniące się o różnicę zostaną również narysowane)\n");
    scanf("%f%f", &warstwica_pocz, &roznica);
    printf("OK, teraz czekamy\n");

    liczenie_po_kolei_wszystkich_warstwic(w, lewy_dol, prawy_gorny, warstwica_pocz, roznica);

    char* komendy_gnuplot[3] = {"set title \"warstwice\"", "unset key", "plot 'dane.temp' with points"};
    FILE * gnuplot_wykres = popen ("gnuplot -persistent", "w");

    for (int i = 0; i < 3 ; ++i)
    {
        fprintf(gnuplot_wykres, "%s \n", komendy_gnuplot[i]);
    }

    fclose(gnuplot_wykres);

    zwolnij_liste(w);

    return 0;
}

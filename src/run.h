

char kontrola_vstavljen_LCD(void);
void potek_kontrole(void);

#define POZ_Y_OPR_OK_START 16

//osnnovna stalna stanja preverjanja naprave
//vsa ostala stanja preverjanaja spadajo pod OPERACIJE
typedef enum{
  ZACETEK, CAKA_START, CAKAJ_RESET, NAPAKA, OPERACIJE, PREVERJAJ, GET_TESTNI_PROGRAM
}e_KontrolaStanja;


//privatne funkcije
static void izvediOperacije();
static int preveriLedice();
static int preveriPine();
static int operacijaFlash();
static int operacijaFlashOff();
static int preveri_backlight();
static char kontrola_vstavljen_LCD(void);
static int prev_kable_2(void);
static int preveriTipke();
static void izpisiKonec();
static void drawLoadingBar();
static void drawVersion(int x, int y);
static void drawOperationOK();
static void initOperation();
static void izpisiSporocilo(const char *sporocilo, const char *podSporocilo, int yZamik);


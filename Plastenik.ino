#include <DHT.h>
#include <LiquidCrystal.h>

int senzorVlageZemlje = A5;
int senzorVlageZemljeVrijednost;  // Sto je vrijednost veca, vlaga je manja
int senzorKvalitetaVazduha = A4;
int senzorKvalitetaVazduhaVrijednost;
int senzorNivoaVode = A6;
float senzorNivoaVodeVrijednost;
int senzorNivoaVodePostotak;
float a;
int anamometar = A3;
int vrijednostNapona;
int fotoOtpornik = A7;
int fotoOtpornikVrijednost;

int zatvaranjeProzor = 37;
int otvaranjeProzor = 23;
int prekidacGore = 43;
int prekidacDole = 45;
bool otvorenProzor = false;

int prekidacVrata = 49;
int otvaranjeVrata = 25;
int zatvaranjeVrata = 27;
bool otvorenaVrata = false;

int pumpa = 35;

int temperatura;
int vlaga;
bool upozorenje = false;

unsigned long previousTime = millis();

bool dan = false;
bool noviDan = true;
unsigned long brojac = 1;

byte stepen[8]
{
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

byte kapljicaPuna[8]
{
  0b00100,
  0b00100,
  0b01110,
  0b01110,
  0b11111,
  0b11111,
  0b11111,
  0b01110
};

byte kapljicaPrazna[8]
{
  0b00100,
  0b00100,
  0b01010,
  0b01010,
  0b10001,
  0b10001,
  0b10001,
  0b01110
};

byte dobro[8]
{
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b10001,
  0b01110,
  0b00000,
  0b00000
};

byte lose[8]
{
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b01110,
  0b10001,
  0b00000,
  0b00000
};

#define DHTPIN 6
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(13, 12, 5, 4, 3, 2);

void Read();
void Logic();
void Print();

void setup() {
  lcd.begin(16, 2);
  lcd.clear();
  dht.begin();

  lcd.createChar(0, stepen);
  lcd.createChar(1, kapljicaPuna);
  lcd.createChar(2, kapljicaPrazna);
  lcd.createChar(3, dobro);
  lcd.createChar(4, lose);

  //Prozor
  pinMode(otvaranjeProzor, OUTPUT);
  pinMode(zatvaranjeProzor, OUTPUT);
  pinMode(prekidacGore, INPUT);
  pinMode(prekidacDole, INPUT);
  digitalWrite(otvaranjeProzor, HIGH);
  digitalWrite(zatvaranjeProzor, HIGH);
  //Vrata
  pinMode(prekidacVrata, INPUT);
  pinMode(otvaranjeVrata, OUTPUT);
  pinMode(zatvaranjeVrata, OUTPUT);
  digitalWrite(otvaranjeVrata, HIGH);
  digitalWrite(zatvaranjeVrata, HIGH);
  //Pumpa
  pinMode(pumpa, OUTPUT);
  digitalWrite(pumpa, HIGH);
}

void loop() {
  upozorenje = false;
  unsigned long currentTime = millis();
  Read();
  Logic();
  if (currentTime - previousTime > 120000)
  {
    Print();
    previousTime = currentTime;
  }
}

void Read()
{
  senzorNivoaVodeVrijednost = analogRead(senzorNivoaVode);
  senzorVlageZemljeVrijednost = analogRead(senzorVlageZemlje);
  senzorKvalitetaVazduhaVrijednost = analogRead(senzorKvalitetaVazduha);
  temperatura = dht.readTemperature();
  vlaga = dht.readHumidity();
  vrijednostNapona = analogRead(anamometar);
  fotoOtpornikVrijednost = analogRead(fotoOtpornik);
}

void Logic()
{
  if (fotoOtpornikVrijednost > 300)
  {
    dan = true; //onda je dan
    brojac++;
  }
  else
  {
    dan = false; //onda je noc
    noviDan = false;
    brojac = 1;
  }
  if (dan == true && noviDan == false && brojac == 2) // Prvi put neka bude dan
  {
    noviDan = true;
  }
  //Prozor
  if (vrijednostNapona < 20 && temperatura > 30 && dan == true)
  {
    while (digitalRead(prekidacGore) == HIGH && otvorenProzor == false)
    {
      digitalWrite(otvaranjeProzor, LOW); // RELEJI OBRNU LOGIKU
      digitalWrite(zatvaranjeProzor, HIGH);
    }
    otvorenProzor = true;
    digitalWrite(otvaranjeProzor, HIGH);
    digitalWrite(zatvaranjeProzor, HIGH);
  }
  else
  {
    while (digitalRead(prekidacDole) == HIGH && otvorenProzor == true)
    {
      digitalWrite(otvaranjeProzor, HIGH); // RELEJI OBRNU LOGIKU
      digitalWrite(zatvaranjeProzor, LOW);
    }
    otvorenProzor = false;
    digitalWrite(otvaranjeProzor, HIGH);
    digitalWrite(zatvaranjeProzor, HIGH);
    delay(5000);
  }

  //Vrata
  if (digitalRead(prekidacVrata) == HIGH && otvorenaVrata == false)
  {
    digitalWrite(otvaranjeVrata, LOW); // RELEJI OBRNU LOGIKU
    digitalWrite(zatvaranjeVrata, HIGH);
    delay(1000);
    otvorenaVrata = true;
  }
  else if (digitalRead(prekidacVrata) == HIGH && otvorenaVrata == true)
  {
    digitalWrite(otvaranjeVrata, HIGH); // RELEJI OBRNU LOGIKU
    digitalWrite(zatvaranjeVrata, LOW);
    delay(1000);
    otvorenaVrata = false;
  }
  digitalWrite(otvaranjeVrata, HIGH);
  digitalWrite(zatvaranjeVrata, HIGH);

  //Pumpa
  if (noviDan == true)
  {
    while (senzorVlageZemljeVrijednost > 300 && senzorNivoaVodeVrijednost > 150)
    {
      digitalWrite(pumpa, LOW);
      senzorVlageZemljeVrijednost = analogRead(senzorVlageZemlje);
      noviDan = false;
      delay(5000);
    }
  }
  digitalWrite(pumpa, HIGH);

  //SenzorNivoaVode upozorenje
  if (senzorNivoaVodeVrijednost < 300)
    upozorenje = true;
}
void Print()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  a = 1024 / senzorNivoaVodeVrijednost;
  senzorNivoaVodePostotak = round(100 / a);

  lcd.print("Vlaga tla: ");
  lcd.print(senzorVlageZemljeVrijednost);

  lcd.setCursor(0, 1);
  lcd.print("Nivo C02: ");
  lcd.print(senzorKvalitetaVazduhaVrijednost);
  delay(2500);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Temperatura:");
  lcd.print(temperatura);
  lcd.write(byte(0));
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Vlaga: ");
  lcd.print(vlaga);
  lcd.print("%");
  delay(2500);
  lcd.clear();

  lcd.setCursor(0, 0);
  if (senzorKvalitetaVazduhaVrijednost > 70)
  {
    lcd.print("Visok % CO2");
    lcd.setCursor(15, 1);
    lcd.write(byte(4));
  }
  else
  {
    lcd.print("Nizak % CO2");
    lcd.setCursor(15, 1);
    lcd.write(byte(3));
  }
  delay(2500);

  if (upozorenje == true)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dopunite tank za");
    lcd.setCursor(0, 1);
    lcd.print("vodu! ");
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nivo vode u");
    lcd.setCursor(0, 1);
    lcd.print("tanku: ");
  }
  lcd.print(senzorNivoaVodePostotak);
  lcd.print("%");
  lcd.setCursor(11, 1);
  if (senzorNivoaVodePostotak == 0)
  {
    lcd.write(byte(2));
    lcd.write(byte(2));
    lcd.write(byte(2));
    lcd.write(byte(2));
    lcd.write(byte(2));
  }
  if (senzorNivoaVodePostotak > 0 && senzorNivoaVodePostotak < 20)
  {
    lcd.write(byte(1));
    lcd.write(byte(2));
    lcd.write(byte(2));
    lcd.write(byte(2));
    lcd.write(byte(2));
  }
  if (senzorNivoaVodePostotak >= 20 && senzorNivoaVodePostotak < 40)
  {
    lcd.write(byte(1));
    lcd.write(byte(1));
    lcd.write(byte(2));
    lcd.write(byte(2));
    lcd.write(byte(2));
  }
  if (senzorNivoaVodePostotak >= 40 && senzorNivoaVodePostotak < 60)
  {
    lcd.write(byte(1));
    lcd.write(byte(1));
    lcd.write(byte(1));
    lcd.write(byte(2));
    lcd.write(byte(2));
  }
  if (senzorNivoaVodePostotak >= 60 && senzorNivoaVodePostotak < 80)
  {
    lcd.write(byte(1));
    lcd.write(byte(1));
    lcd.write(byte(1));
    lcd.write(byte(1));
    lcd.write(byte(2));
  }
  if (senzorNivoaVodePostotak >= 80 && senzorNivoaVodePostotak <= 100)
  {
    lcd.write(byte(1));
    lcd.write(byte(1));
    lcd.write(byte(1));
    lcd.write(byte(1));
    lcd.write(byte(1));
  }
  delay(2500);
}

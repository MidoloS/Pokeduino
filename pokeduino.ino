#include <Adafruit_LiquidCrystal.h>
#include <Keypad.h>

int pos = 0;
int pick = 1;
int key = 1;
bool isReady = false;
int attackType = 1;
int currentPokemonIndex = 0;
bool isYourTurn = false; // Esta variable tiene que ser seteada en verdadero que uno de los arduinos empiece primero

const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 

Keypad keypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

Adafruit_LiquidCrystal lcd(0);

enum Pokemon_Type
{
  GRASS,
  FIRE,
  WATER,
  STEEL,
  DRAGON,
  FAIRY,
  NORMAL
};

bool isWeakAgainst(int type, int otherType) {
  switch (type) {
    case GRASS:
      return otherType == FIRE;
    case FIRE:
      return otherType == WATER;
    case WATER:
      return otherType == GRASS;
    case DRAGON:
      return otherType == DRAGON || otherType == FAIRY;
    case FAIRY:
      return otherType == STEEL;
    case STEEL:
      return otherType == FIRE;
    default:
      return false;
  }
  return false;
}

bool isResistantTo(int type, int otherType) {
  switch (type) {
    case GRASS:
      return otherType == WATER;
    case FIRE:
      return otherType == STEEL || otherType == GRASS;
    case WATER:
      return otherType == STEEL || otherType == FIRE;
    case DRAGON:
      return otherType == GRASS || otherType == FIRE || otherType == WATER;
    case FAIRY:
      return otherType == DRAGON;
    case STEEL:
      return otherType == DRAGON || otherType == FAIRY || otherType == GRASS;
    default:
      return false;
  }
}

class Pokemon {
  private:
    int id;
    String name;
    float hp;
    int atk;
    int specialAtk;
    int type;
  	int isDefeated;

public:
  Pokemon(int id, String name, int type, float hp, int atk, int specialAtk) {
    this->id = id;
    this->name = name;
    this->type = type;
    this->hp = hp;
    this->atk = atk;
    this->specialAtk = specialAtk;
    this->isDefeated = false;
  }
  Pokemon() {}
  String getName() {
	return this->name;
  }
  void setHp(int hp) {
	this->hp = hp;
  }
  int getSpecialAtk() {
    return this->specialAtk;
  }
  int getAtk() {
   	return this->atk; 
  }
  float getHp() {
    return this->hp;
  }
  int getType() {
    return this->type;
  }
  void setIsDefeated(bool defeated) {
    this->isDefeated = defeated;
  }
  int getId() {
    return this->id;
  }
  void attack(int atkType) {
    isYourTurn = false;
	byte buf[] = { 'a', getId() + 48, atkType + 48, 0 };
    Serial.write(buf, 4);
  }
};

Pokemon myPokemons[3];




Pokemon ALL_POKEMONS[6] = {
    Pokemon(1, "SERPERIOR", GRASS, 115, 10, 24),
    Pokemon(2, "BLASTOISE", WATER, 105, 12, 24),
    Pokemon(3, "ARCANINE", FIRE, 80, 18, 30),
    Pokemon(4, "GOODRA", DRAGON, 95, 14, 26),
    Pokemon(5, "SYLVEON", FAIRY, 110, 12, 24),
    Pokemon(6, "MELMETAL", STEEL, 100, 10, 28)
};

float getDamage(int attackerId, int atkType) {
  String ATTACK_TYPES[6] = {
    "Planta",
    "Fuego",
    "Agua",
    "Acero",
    "Dragon",
    "Hada"
  };
  Pokemon target = myPokemons[currentPokemonIndex];
  Pokemon attacker = ALL_POKEMONS[attackerId];
  
  if (atkType == 2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(attacker.getName());
    lcd.print(" uso");
    lcd.setCursor(0, 1);
    lcd.print("Ataque ");
    lcd.print(ATTACK_TYPES[attacker.getType()]);
    delay(2000);
    if (isWeakAgainst(target.getType(), attacker.getType())) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("...Es muy");
      lcd.setCursor(0, 1);
      lcd.print("efectivo!!!");
      delay(2000);
      lcd.clear();
      return target.getHp() - attacker.getSpecialAtk() * 1.5 <= 0 ? 0 : target.getHp() - attacker.getSpecialAtk() * 1.5;
    }
    if (isResistantTo(target.getType(), attacker.getType())) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("...Es poco");
      lcd.setCursor(0, 1);
      lcd.print("efectivo.");
      delay(2000);
      lcd.clear();
      return target.getHp() - attacker.getSpecialAtk() * 0.5 <= 0 ? 0 : target.getHp() - attacker.getSpecialAtk() * 0.5;
    }
    return target.getHp() - attacker.getSpecialAtk();
  }
  if (atkType == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(attacker.getName());
    lcd.print(" uso");
    lcd.setCursor(0, 1);
    lcd.print("Ataque Neutro");
    delay(2000);
    lcd.clear();
    return target.getHp() - attacker.getAtk() <= 0 ? 0 : target.getHp() - attacker.getAtk();
  }
  return 0;
}

void battleMenu()
{
  int key = keypad.getKey() - 48;
  int number;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1.Atk 2.Cambiar");
  lcd.setCursor(0, 1);
  lcd.print("3.Ver Info");
  
  while (key != 1 && key != 2 && key != 3)
  {
    recibirAtaque();
    key = keypad.getKey() - 48;
    switch (key)
    {
    case 1:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Neutro (1)");
      lcd.setCursor(0, 1);
      lcd.print("Elemental (2)");
      while (key != -6 && key != -13)
      {
        key = keypad.getKey() - 48;
        if (key >= 1 && key <= 2)
        {
          attackType = key;
        }
        if (key == -13) {
          myPokemons[currentPokemonIndex].attack(attackType);
          break;
        }
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("1.Atk 2.Cambiar");
      lcd.setCursor(0, 1);
      lcd.print("3.Ver Info");
      break;
    case 2:
      cambiarPokemon();
      break;
    case 3:
      lcd.clear();
      key = keypad.getKey() - 48;
      while (key != -13)
      {
        key = keypad.getKey() - 48;
        lcd.setCursor(0, 0);
        lcd.print(myPokemons[currentPokemonIndex].getName());
        lcd.setCursor(0, 1);
        lcd.print(myPokemons[currentPokemonIndex].getHp());
        lcd.print(" #Salir");
      }    
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("1.Atk 2.Cambiar");
      lcd.setCursor(0, 1);
      lcd.print("3.Ver Info");
      break;
    }
  }
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  myPokemons[0] = ALL_POKEMONS[0];
}

void cambiarPokemon() {
  int key = keypad.getKey() - 48;
  int number;
  for (int i = 0; i < 3; i++) {
    if (myPokemons[i].getHp() > 0 && i != currentPokemonIndex) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(i);
      lcd.print(".");
      lcd.print(myPokemons[i].getName());
      lcd.print(": ");
      lcd.print(myPokemons[i].getHp());
    }
    lcd.setCursor(0, 1);
  }
  while (key != -6 && key != -13)
  {
    key = keypad.getKey() - 48;
    if (key != currentPokemonIndex && myPokemons[key - 1].getHp() > 0)
    {
      number = key;
    }
    if (key == -13) {
      currentPokemonIndex = number;
      break;
    }
  }
}


void recibirAtaque() {
  char mystr[4];
  if (Serial.available() >= 4) {
    Serial.readBytes(mystr,4);
    if(mystr[0] == 'a'){
    isYourTurn = true;
    int attackedId = mystr[1] - 48;
    int atkType = mystr[2] - 48;
    myPokemons[currentPokemonIndex].setHp(getDamage(attackedId, atkType));
    if (myPokemons[currentPokemonIndex].getHp() <= 0) {
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(myPokemons[0].getName());
      lcd.print(" fue");
      lcd.setCursor(0, 1);
      lcd.print("debilitado!!!");
      delay(2000);
    }
  }
  }
  
}

void loop() {
  while (pos <= 2) {
    key = keypad.getKey() - 48;
    lcd.setCursor(0, 0);
    lcd.print("Pokemon #");
    lcd.print(pos+1);
    if (key >= 1 && key <= 6) {
      pick = key - 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pokemon #");
      lcd.print(pos+1);
      lcd.setCursor(0, 1);
      lcd.print(ALL_POKEMONS[pick].getName());
    }
    if (key == -13) {
      myPokemons[pos] = ALL_POKEMONS[pick];
      pos++;
    }
  }
  lcd.clear();
  if (isYourTurn) {
    battleMenu();
  } else {
    recibirAtaque();
  }
}

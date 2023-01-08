//Адреса пинов снимающих сигнал с ферритовых колец
uint8_t in_pins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};

//Пины управляющие сдвиговым регистром на двух 74HC595
//Пин подключен к ST_CP входу 74HC595
int latchPin = 12;
//Пин подключен к SH_CP входу 74HC595
int clockPin = 13;
//Пин подключен к DS входу 74HC595
int dataPin = 11;

//Вывод на 16-битный сдвиговый регистр числа, кодирующего пины
void shift_out_value(unsigned int val) {
  digitalWrite(latchPin, LOW);
  //Выведем на сдвиговый регистр последний байт
  shiftOut(dataPin, clockPin, MSBFIRST, highByte(val));
  //Выведем на сдвиговый регистр первый байт
  shiftOut(dataPin, clockPin, MSBFIRST, lowByte(val));
  digitalWrite(latchPin, HIGH);
}

//Разрядка конденсаторов на датчиках колец
void discharge() {
  for (int i = 0; i < 8; i++) {
    //Переключить пин на выход
    pinMode(in_pins[i], OUTPUT);
    //занулить
    digitalWrite(in_pins[i], LOW);
    //небольшая задержка
    delay(3);
    //Переключить обратно на вход
    pinMode(in_pins[i], INPUT);
  }
}

//Считывание байта с колец
uint8_t read_ferrite_byte(unsigned int address) {
  uint8_t values[] = {0, 0, 0, 0, 0, 0, 0, 0};

  //Для точности можно увеличить количество проходов чтения, но, 
  //работает и так
  //for (uint8_t phase = 0; phase < 2; phase++)
  {
    //Разрядим
    discharge();
    //Подергаем нужным проводком по нужному адресу через сдвиговый 
    //регистр
    //Это нужно чтобы создать в ферритовых кольцах магнитный поток и 
    //зарядить конденсаторы
    //на датчиках тех колец, через которые проходит проводник с нужным 
    //адресом
    for (uint8_t j = 0; j < 8; j++) {
      shift_out_value(address);
      shift_out_value(0);
    }
    //Считаем показания с датчиков всех колец
    for (uint8_t ix = 0; ix < 8; ix++)
    {
      values[ix] += analogRead(in_pins[ix]);
    }
  }

  discharge();

  //Упакуем все считанные значения в один байт, где 0 - не считано 
  //ничего, 1 - считано что-то
  uint8_t result = 0;
  for (uint8_t ix = 0; ix < 8; ix++)
  {
    result = (result << 1) | ((values[ix] > 1) ? 1 : 0);
  }
  return result;
}

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  for (int i = 0; i < 8; i++) {
    pinMode(in_pins[i], INPUT_PULLUP);
    digitalWrite(in_pins[i], LOW);
  }

  Serial.begin(115200);
  
  //разрядим конденсаторы
  discharge();
  Serial.println("------------------------------");
  //Убедимся, что с ферритовых колец ничего не читается
  for (uint8_t ix = 0; ix < 8; ix++)
  {
    Serial.print(analogRead(in_pins[ix]), DEC);
    Serial.print(" ");
  }

  Serial.println();
  Serial.println();

  //Считаем и выведем значения по адресам 1..16
  for (int i = 0; i < 16; i++) {
    Serial.print("address[");
    if (i < 10) Serial.print("0");
    Serial.print( i, DEC);
    Serial.print("] = ");
    uint8_t nibbles = read_ferrite_byte((unsigned int)1 << i);
    Serial.print(nibbles, DEC);
    Serial.print(" - ");
    Serial.print(nibbles, BIN);
    if (nibbles > 32) {
      Serial.print(" - ");
      Serial.print((char)nibbles);
    }
    Serial.println();
  }
}

void loop() {
}

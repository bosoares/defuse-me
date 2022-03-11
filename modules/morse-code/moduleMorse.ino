

/*
 * millis variables
 * Use of millis instead of delay()
 */
 
// general time keeper
unsigned long msFromStart = 0;
// LED morse code
unsigned long msPreviousLedUpdate = 0;
unsigned long checkMorse = 700; //Morse code speed. Increase to have slower blinking.

// LED fail - red
unsigned long msPreviousLedRedUpdate = 0;
unsigned long checkRedLed = 300;

// 7 segments display
unsigned long msPreviousMoveLED = 0;
unsigned long checkLed = 1000;
// Potentiometer
unsigned long checkPotentiometer1 = 100;
unsigned long msPreviousReadPot1 = 0;
unsigned long checkPotentiometer2 = 100;
unsigned long msPreviousReadPot2 = 0;

// push button - avoid debounce
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 300;

/*
 * Morse code
 */
#define LED_MORSE 4
bool led_state = true;
int currentLetter = 0;
int currentWordLength = 0;
bool booleanMorseWord[70];

/*
 * LEDs
 */
#define LED_FAIL 5
#define LED_WIN 6
 
/*
 * Shift-register 74HC595N
 */

#define SH_CP_1 9   // Shif-register clock input
#define ST_CP_1 8   // Storage register clock input
#define DS_1    7   // Serial data input

/*
 * 7 Segments display
 */
// Array with the symbols displayed on the 7 segment display
int pattern[]{
  B11111101, //B
  B11111110, //A
  B11011111, //F
  B10111111, //G
  B11111011, //C
  B11110111, //D
  B11101111, //E
  B01111111, //DOT
};
int selectedSymbol1=0;
bool unlockedSymbol = true;
int selectedSymbol2=0;
bool unlockedSymbol2 = true;

void write7SegmentsDisplay(int symbol1, int symbol2);

/*
 * Potentiometer 10k
 */
#define POTENT_PIN_1 A0
#define POTENT_PIN_2 A1
int potValue1 = 0;
int potValue2 = 0;


/*
 * Buttons
 */
#define BUTTON_PIN_1    3   // Switch button
bool lastButtonState_1 = false;
bool readSwitch = false;
void checkSwitch();

/*
 * Support
 */

int choosenWord = 0;
// Creates the list of words in morse code
void createDictionary();
bool checkAnswer(int symbol1,int symbol2);
bool firstTry=false;
bool unlockSecondTry=false;
bool gameOn = true;
bool gameOver = false;



/* #######################################################################
 *                                 SETUP
 * #######################################################################
 */
void setup() {
  // Set serial communication
  Serial.begin(9600);
  
  // Shift-registers
  pinMode(SH_CP_1, OUTPUT);
  pinMode(ST_CP_1, OUTPUT);
  pinMode(DS_1, OUTPUT);
  
  // Buttons
  pinMode(BUTTON_PIN_1, INPUT_PULLUP); 
  
  // LED victory (green)
  pinMode(LED_WIN, OUTPUT);
  digitalWrite(LED_WIN, LOW);
  
  // LED fail (red)
  pinMode(LED_FAIL, OUTPUT);
  digitalWrite(LED_FAIL, LOW);
  
  // LED Morse code
  pinMode(LED_MORSE, OUTPUT);
  digitalWrite(LED_MORSE, HIGH);

  // Select word
  randomSeed(analogRead(7));
  choosenWord = random(0,14);
  Serial.print("Random number: ");
  Serial.println(choosenWord);
  delay(100);
  // create the set of words available in the game
  createDictionary();
}

/* #######################################################################
 *                                 LOOP
 * #######################################################################
 */
 
void loop() {
  //Check time passed since the startup
  msFromStart = gameOver ? msFromStart : millis();

  
  // Switch
  checkSwitch();

  // Potentiometer
  if(msFromStart - msPreviousReadPot1 > checkPotentiometer1 && unlockedSymbol )
  {
    msPreviousReadPot1 = msFromStart;
    potValue1 = analogRead(POTENT_PIN_1);
    potValue2 = analogRead(POTENT_PIN_2);
    /** Analogue read of the potenciometer goes from 0 to 1024
     *  Display has 7 different positions (1024 / 7 ~150) 
     *  The dot in the left-bottom corner is used to indicate that
     *  the symbol is locked
     */
    
    selectedSymbol1=potValue1/150;
    selectedSymbol2=potValue2/150;
    write7SegmentsDisplay(pattern[selectedSymbol1],pattern[selectedSymbol2]);
  }

  // LED morse code
  if(msFromStart - msPreviousLedUpdate > checkMorse)
  {
    msPreviousLedUpdate = msFromStart;
    
    led_state = booleanMorseWord[currentLetter];
    digitalWrite(LED_MORSE, led_state);


    currentLetter++;
    if(currentLetter == currentWordLength){
      currentLetter = 0;
    }
  }

  // LED fail - red
  if(msFromStart - msPreviousLedRedUpdate > checkRedLed && firstTry)
  {
    msPreviousLedRedUpdate = msFromStart;
    digitalWrite(LED_FAIL, !digitalRead(LED_FAIL));
  }
 
}

/* #############################################################################
 *                                  FUNCTIONS
 * #############################################################################
 */
 
/*
 * @arg     symbol : int
 * @return  void
 * 
 * Description: Controls the shift-register
 * to put on the parallel output the bits 
 * of "symbol", to activate the 7 seg display.
 */
void write7SegmentsDisplay(int symbol1,int symbol2)
{
 
  // Start of data reception
  digitalWrite(ST_CP_1, LOW);
  // Data transfer to DS_1 pin
  //second 7 segments display - cascade shift register
  shiftOut(DS_1, SH_CP_1, MSBFIRST, symbol2);
  //first 7 segments display - primary shift register
  shiftOut(DS_1, SH_CP_1, MSBFIRST, symbol1);
  // End of the data transfer.
  digitalWrite(ST_CP_1, HIGH); 
}

/*
 * @arg     void
 * @return  sdfa : int
 */
void createDictionary()
{
  const char words_dictionary[16][40]
  {
    /*
     * Morse code table
     * s - short (dot)
     * l - long  (dash)
     * p - pause (time of 1 dot)
     * e - end of letter (time of 3 dots)
     */

    {   // COFRE - size:33 
        'l','p','s','p','l','p','s','e', // C - 8  
        'l','p','l','p','l',        'e', // O - 6 
        's','p','s','p','l','p','s','e', // F - 8 
        's','p','l','p','s',        'e', // R - 6 
        's',                        'e', // E - 2 
        'e','e','p'                      // END -3         
    },
    {   // LIVRO - size:33
        's','p','l','p','s','p','s','e', // L - 8  
        's','p','s',                'e', // I - 4 
        's','p','s','p','l','p','s','e', // F - 8 
        's','p','l','p','s',        'e', // R - 6 
        's',                        'e', // E - 2 
        'e','e','p'                      // END - 3         
    },
    {	// RIGOR - size:31
	    's','p','l','p','s','e', // R - 6 
	    's','p','s',        'e', // I - 4 
	    'l','p','l','p','s','e', // G - 6 
	    'l','p','l','p','l','e', // O - 6 
	    's','p','l','p','s','e', // R - 6 
        'e','e','p'              // END -3
    },
    {	// VULTO - size:33 
	    's','p','s','p','s','p','l','e', // V - 8 
	    's','p','s','p','l',        'e', // U - 6 
	    's','p','l','p','s','p','s','e', // L - 8  
	    'l',                        'e', // T - 2 
        'l','p','l','p','l',        'e', // O - 6 
        'e','e','p'                      // END -3
    },
    {   // BANAL - size:31
        'l','p','s','p','s','p','s','e', // B - 8 
        's','p','l',                'e', // A - 4 
        'l','p','s',                'e', // N - 4 
        's','p','l',                'e', // A - 4 
        's','p','l','p','s','p','s','e', // L - 8  
        'e','e','p'                      // END -3
    },
    {   // DOCIL - size:35
        'l','p','s','p','s',        'e', // D - 6 
        'l','p','l','p','l',        'e', // O - 6 
        'l','p','s','p','l','p','s','e', // C - 8  
        's','p','s',                'e', // I - 4 
        's','p','l','p','s','p','s','e', // L - 8  
        'e','e','p'                      // END -3
    },
    {   // ARIDO - size:29
        's','p','l',        'e', // A - 4 
        's','p','l','p','s','e', // R - 6 
        's','p','s',        'e', // I - 4 
        'l','p','s','p','s','e', // D - 6 
        'l','p','l','p','l','e', // O - 6 
        'e','e','p'              // END -3
    },
    {   // HORDA - size:33
        's','p','s','p','s','p','s','e', // H - 8 
        'l','p','l','p','l',        'e', // O - 6 
        's','p','l','p','s',        'e', // R - 6 
        'l','p','s','p','s',        'e', // D - 6 
        's','p','l',                'e', // A - 4
        'e','e','p'                      // END -3
    },
    {   // TORPE - size:27
        'l',                        'e', // T - 2 
        'l','p','l','p','l',        'e', // O - 6 
        's','p','l','p','s',        'e', // R - 6 
        's','p','l','p','l','p','s','e', // P - 8
        's',                        'e', // E - 2 
        'e','e','p'                      // END -3
    },
    {   // ANCIA - size:27
        's','p','l',                'e', // A - 4 
        'l','p','s',                'e', // N - 4 
        'l','p','s','p','l','p','s','e', // C - 8  
        's','p','s',                'e', // I - 4 
        's','p','l',                'e', // A - 4 
        'e','e','p'                      // END -3
    },
    {   // CISAO - size:31
        'l','p','s','p','l','p','s','e', // C - 8  
        's','p','s',                'e', // I - 4 
        's','p','s','p','s',        'e', // S - 6 
        's','p','l',                'e', // A - 4 
        'l','p','l','p','l',        'e', // O - 6
        'e','e','p'                      // END -3 
    },
    {   // OBITO - size:29
        'l','p','l','p','l',        'e', // O - 6 
        'l','p','s','p','s','p','s','e', // B - 8 
        's','p','s',                'e', // I - 4 
        'l',                        'e', // T - 2 
        'l','p','l','p','l',        'e', // O - 6
        'e','e','p'                      // END -3 
    },
    {   // AMAGO - size:27
        's','p','l',        'e', // A - 4 
        'l','p','l',        'e', // M - 4 
        's','p','l',        'e', // A - 4 
        'l','p','l','p','s','e', // G - 6  
        'l','p','l','p','l','e', // O - 6 
        'e','e','p'              // END -3
    },
    {   // FUTIL - size:31
        's','p','s','p','l','p','s','e', // F - 8 
        's','p','s','p','l',        'e', // U - 6 
        'l',                        'e', // T - 2 
        's','p','s',                'e', // I - 4 
        's','p','l','p','s','p','s','e', // L - 8
        'e','e','p'                      // END -3  
    },  
    {   // ALGOZ - size:35
        's','p','l',                'e', // A - 4 
        's','p','l','p','s','p','s','e', // L - 8  
        'l','p','l','p','s',        'e', // G - 6  
        'l','p','l','p','l',        'e', // O - 6 
        'l','p','l','p','s','p','s','e', // Z - 8
        'e','e','p'                      // END -3 
    },
  };

  // Stores the length of each word
  int a_wordLength[]
  {
    33, // COFRE - size:33
    33, // LIVRO - size:33
    31, // RIGOR - size:31
    33, // VULTO - size:33
    31, // BANAL - size:31
    35, // DOCIL - size:35
    29, // ARIDO - size:29
    33, // HORDA - size:33
    27, // TORPE - size:27
    27, // ANCIA - size:27
    31, // CISAO - size:31
    29, // OBITO - size:29
    27, // AMAGO - size:27
    31, // FUTIL - size:31
    35, // ALGOZ - size:35
  };
  delay(100);
  Serial.println("Calling convertMorseToBoolean");
  convertMorseToBoolean(a_wordLength[choosenWord], words_dictionary[choosenWord]);
  
}

void convertMorseToBoolean(int wordLength, const char* wordToConvert)
{
  Serial.println("Starting conversion");
  int counterLetterBoolean = 0;
  int counterLetterMorse = 0;
  
  
  while(counterLetterMorse < wordLength)
  {
    Serial.print(wordToConvert[counterLetterMorse]);
    Serial.print(": ");
    
    if(wordToConvert[counterLetterMorse] == 's' || wordToConvert[counterLetterMorse] == 'p')
    {
      //short (dot) - 1 interval true
      booleanMorseWord[counterLetterBoolean]= wordToConvert[counterLetterMorse] == 's' ? true : false;
      Serial.print(booleanMorseWord[counterLetterBoolean]);
      counterLetterBoolean++;
    }
    
    if(wordToConvert[counterLetterMorse] == 'l' || wordToConvert[counterLetterMorse] == 'e')
    {
      for(int i=0; i<3;i++)
      {
        booleanMorseWord[counterLetterBoolean]= wordToConvert[counterLetterMorse] == 'l' ? true : false;
        Serial.print(booleanMorseWord[counterLetterBoolean]);
        counterLetterBoolean++;
      }
    }
    
    counterLetterMorse++;
    Serial.println();
  }
  currentWordLength = counterLetterBoolean-1;
  for(int k; k<counterLetterBoolean;k++)
  {
    Serial.println(booleanMorseWord[k]);
    delay(100);
  }
}

bool checkAnswer(int symbol1,int symbol2)
{
  int answerTable[] =  {
    26, // COFRE
    15, // LIVRO
    53, // RIGOR
    43, // VULTO
    00, // BANAL
    60, // DOCIL
    56, // ARIDO
    01, // HORDA
    35, // TORPE
    42, // ANCIA
    31, // CISAO
    16, // OBITO
    13, // AMAGO
    24, // FUTIL
    56, // ALGOZ
  };

 
  return (answerTable[choosenWord] == (symbol1*10 + symbol2));
}

void checkSwitch()
{
  // Switch
  if(msFromStart - lastDebounceTime > debounceDelay)
  {
    lastDebounceTime = msFromStart;
    readSwitch = digitalRead(BUTTON_PIN_1);
  }
    
  if(readSwitch /*&& unlockedSymbol*/)
  {
    // Add the DOT on the left-bottom of the display on the displays
    write7SegmentsDisplay(pattern[7] & pattern[selectedSymbol1], pattern[7] & pattern[selectedSymbol2]);
    // After button 1 is pressed, it is not possible to change the symbol
    // Debounce ignored, as the button can be pressed only once.
    unlockedSymbol = false;
    // check if the answer is correct
    if(firstTry == false)
    {
      if(checkAnswer(selectedSymbol1,selectedSymbol2))
      {
          digitalWrite(LED_WIN, HIGH);
          digitalWrite(LED_FAIL, LOW);
          digitalWrite(LED_MORSE, LOW);
          gameOn = false;
          gameOver = true;
          Serial.println("Right answer");
      }
      else
      {
        // Fail
        //unlockedSymbol = true;
        firstTry = true;
        Serial.println("Wrong answer 1");  
      }
    }
    else
    {
      if(unlockSecondTry)
      {
        if(checkAnswer(selectedSymbol1,selectedSymbol2))
        {
          //WIN
          digitalWrite(LED_WIN, HIGH);
          digitalWrite(LED_FAIL, LOW);
          digitalWrite(LED_MORSE, LOW);
          gameOn = false;
          gameOver = true;
          Serial.println("Right answer");
        }
        else
        {
          // Fail
          digitalWrite(LED_WIN, LOW);
          digitalWrite(LED_FAIL, HIGH);
          digitalWrite(LED_MORSE, LOW);
          gameOn = false;
          gameOver = true;
          Serial.println("Wrong answer 2");
        }
      }
    }
  }
  else
  {
    unlockSecondTry = firstTry ? true : false;
    unlockedSymbol = true;
  }
}

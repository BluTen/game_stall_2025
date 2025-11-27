#define S1_BUTTON_PIN 5
#define S2_BUTTON_PIN 6
#define S3_BUTTON_PIN 7

#define MIN_DUR  150  // The fastest the game is allowed to start
#define INIT_DUR 850  // Duration added to minimum at the start of the game
#define DUR_DCAY .97  // Rate of decay of duration

void setup() {
    Serial.begin(9600);
    pinMode(S1_BUTTON_PIN, INPUT_PULLUP);
    pinMode(S2_BUTTON_PIN, INPUT_PULLUP);
    pinMode(S3_BUTTON_PIN, INPUT_PULLUP);

    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(13, OUTPUT);
    PORTD = PORTD | 0b11100000;
    PORTB = PORTB | 0b00111111;

    digitalWrite(8, HIGH);
    
    Serial.println("#Finished setup");
}

void loop() {
    // Busy wait until we recive start command from serial
    byte _i = 0;
    while (!Serial.available()) {
        unsigned long last_millis = millis();
        unsigned int bits;
        bits = (1 << 8 - (_i % 3)) | (1 << 5 - ((_i+1) % 3)) | (1 << 2 - ((_i+2) % 3));
        setScreen(bits);
        while (millis() - last_millis < 1000) {
            if (Serial.available()) goto game_start;
        }
        _i += 1;
    }
    
game_start:
    // If the data from serial data is anything but the letter 'S' restart
    if (!(Serial.available() == 1 && Serial.read() == 'S')) goto end;
    randomSeed(analogRead(0));
    Serial.println("#Game started");

    // Init game state
    byte score = 0;
    float duration = MIN_DUR + INIT_DUR;
    byte screen = 0;
    byte shape = 0;
    long start_time = 0;
    bool timeout = true;
    // Some fun patterns before game starts
    setScreen(0b100100100);
    delay(1000);
    setScreen(0b100100100 >> 1);
    delay(1000);
    setScreen(0b100100100 >> 2);
    delay(1000);
    setScreen(0);
    delay(1000);

    // Gameplay loop
    while (true) {
        duration = MIN_DUR + INIT_DUR * pow(DUR_DCAY, score);
        Serial.write('D');
        Serial.println(duration);
        screen = random(0,3);
        shape = random(0,3);
        timeout = true;

        showShape(screen, shape);
        start_time = millis();
        while (millis() - start_time < duration) {
            if (PIND & B00011100) {
                if (PIND & (B00000100 << shape)) {
                    Serial.write('H');
                    score += 1;
                    setScreen(0);
                    timeout = false;
                    delay(750);                 
                }
                break;
            }
        }
        if (timeout) break;
    }
    
    Serial.write('X');
    Serial.println(score);
    
    
end:
    Serial.println("#Ended, Restarting");
}

void showShape(byte screen, byte shape) {
    Serial.write('#');
    Serial.print(screen);
    Serial.print(".");
    Serial.println(shape);
    if (screen == 0) {
        PORTD = (PORTD & 0b00011111) | (0b00100000 << shape);
        PORTB = PORTB & 0b11000000;
    } else {
        PORTB = (PORTB & 0b11000000) | (0b00000001 << (3*(screen-1) + shape));
        PORTD = PORTD & 0b00011111;
    }
}

void setScreen(unsigned int bits) {
    PORTB = (PORTB & 0b11000000) | (bits &  0b00111111);
    PORTD = (PORTD & 0b00011111) | (bits & 0b111000000)>>1;
}

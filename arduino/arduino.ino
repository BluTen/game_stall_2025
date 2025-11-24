#define S1_BUTTON_PIN 5
#define S2_BUTTON_PIN 6
#define S3_BUTTON_PIN 7
#define SC0_SH0 

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
    PORTD = PORTD | B11100000;
    PORTB = PORTB | B00111111;

    digitalWrite(8, HIGH);
    
    Serial.println("#Finished setup");
}

void loop() {
    // Busy wait until we recive start command from serial
    while (!Serial.available());
    // If the data from serial data is anything but the letter 'S' restart
    if (!(Serial.available() == 1 && Serial.read() == 'S')) goto end;

    Serial.write('.');
    randomSeed(analogRead(0));

    // Init game state
    byte score = 0;
    int duration = 3000;
    byte screen = 0;
    byte shape = 0;
    long start_time = 0;
    bool timeout = true;
    // Some fun patterns before game starts

    // Gameplay loop
    while (true) {
        screen = random(0,3);
        shape = random(0,3);
        timeout = true;

        showShape(screen, shape);
        start_time = millis();
        while (millis() - start_time < duration) {
            if (PIND & B00011100) {
                if (PIND & (B00000100 << shape)) {
                    score += 1;
                    timeout = false;
                    showShape(8,8);
                    delay(500);
                    break;
                }
                break;
            }
        }
        if (timeout) break;
    }
    

end:
    Serial.println("#Ended, Restarting");
}

void showShape(byte screen, byte shape) {
    Serial.print("#");
    Serial.print(screen);
    Serial.print(".");
    Serial.println(shape);
    if (screen == 0) {
        PORTD = (PORTD & B00011111) | (B00100000 << shape);
        PORTB = PORTB & B11000000;
    } else {
        PORTB = (PORTB & B11000000) | (B00000001 << (3*(screen-1) + shape));
        PORTD = PORTD & B00011111;
    }
}

#include <ezButton.h>
#include <LiquidCrystal.h>
#define SW_PIN 8
#define VRX_PIN 0
#define VRY_PIN 1
#define DEADZONE 384
ezButton button(SW_PIN);
byte customChar[] = {
    B00000,
    B00000,
    B11111,
    B11111,
    B11111,
    B11111,
    B00000,
    B00000
};
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int xValue = 0;
int yValue = 0;
int prevXValue = 0;
int prevYValue = 0;

#define NUM_DICE_SIZES 9
const int DICE_SIZES[] = {2,3,4,6,8,10,12,20,100};
int dice_count = 1;
int dice_type = 7; // 1d20
int modifier = 0;
long last_refresh = 0;

enum MODIFY_MODE {
    DICE_COUNT,
    DICE_TYPE,
    MODIFIER
};
enum MODIFY_MODE currently_modifying = DICE_COUNT;

void setup() {
    Serial.begin(9600);
    button.setDebounceTime(50);
    //setup number of columns, rows
    lcd.begin(16, 2);
    // lcd.backlight();
    // lcd.print("hello, world!");
    // put your setup code here, to run once:
    randomSeed(analogRead(0));
    //pinMode(ledPin, OUTPUT);
    Serial.println("started!");
    clearDieRoll();
}

void loop() {
    // put your main code here, to run repeatedly:
    button.loop();
    //figure out where joystick is pointing
    xValue = analogRead(VRX_PIN);
    yValue = analogRead(VRY_PIN);
    if(xValue > 512 + DEADZONE && prevXValue <= 512 + DEADZONE) {
        // we're now pointing right
        clearDieRoll();
        // lcd.setCursor(0, 1);
        // lcd.print("right");
        switch(currently_modifying) {
            case DICE_COUNT:
                currently_modifying = DICE_TYPE;
                break;
            case DICE_TYPE:
                currently_modifying = MODIFIER;
                break;
            default:
                currently_modifying = DICE_COUNT;
        }
    }
    else if(xValue < 512 - DEADZONE && prevXValue >= 512 - DEADZONE) {
        // we're now pointing left
        clearDieRoll();
        // lcd.setCursor(0, 1);
        // lcd.print("left ");
        switch(currently_modifying) {
            case DICE_COUNT:
                currently_modifying = MODIFIER;
                break;
            case DICE_TYPE:
                currently_modifying = DICE_COUNT;
                break;
            default:
                currently_modifying = DICE_TYPE;
        }
    }

    if(yValue > 512 + DEADZONE && prevYValue <= 512 + DEADZONE) {
        // we're now pointing down
        // lcd.setCursor(0, 1);
        // lcd.print("down ");
        modifyCurrentSelection(-1);
    }
    else if(yValue < 512 - DEADZONE && prevYValue >= 512 - DEADZONE) {
        // we're now pointing up
        // lcd.setCursor(0, 1);
        // lcd.print("up   ");
        modifyCurrentSelection(+1);
    }

    prevXValue = xValue;
    prevYValue = yValue;

    if(button.isPressed()) {
        lcd.setCursor(0, 1);
        lcd.print("You rolled:     ");
        int accumulator = modifier;
        for(int i = 0; i < dice_count; i++)
            accumulator += 1 + random(DICE_SIZES[dice_type]);
        lcd.setCursor(12, 1);
        lcd.print(accumulator);
    }

    if(millis() >= last_refresh + 500) {
        last_refresh = millis();
        printInterface();
    }
}

const char txt[16];

void clearDieRoll() {
    lcd.setCursor(0, 1);
    lcd.print("Click joystick! ");
}

bool blink_on = true;
void printInterface() {
    if(currently_modifying == DICE_COUNT) {
        if(blink_on)
            printDiceCount();
        else
            clearDiceCount();
        printDiceType();
        printModifier();
    }
    if(currently_modifying == DICE_TYPE) {
        if(blink_on)
            printDiceType();
        else
            clearDiceType();
        printDiceCount();
        printModifier();
    }
    if(currently_modifying == MODIFIER) {
        if(blink_on)
            printModifier();
        else
            clearModifier();
        printDiceCount();
        printDiceType();
    }
    blink_on = !blink_on;
}

void modifyCurrentSelection(int change) {
    clearDieRoll();
    switch(currently_modifying) {
        case DICE_COUNT:
            dice_count += change;
            if(dice_count < 1)
                dice_count = 1;
            printDiceCount();
            break;
        case DICE_TYPE:
            dice_type += change;
            dice_type = dice_type % NUM_DICE_SIZES;
            if(dice_type < 0)
                dice_type = NUM_DICE_SIZES - 1;
            clearDiceType();
            printDiceType();
            break;
        default:
            modifier += change;
            printModifier();
    }
}

void clearDiceCount() {
    lcd.setCursor(0, 0);
    lcd.print("  ");
}

void printDiceCount() {
    lcd.setCursor(0, 0);
    sprintf(txt, "%2d", dice_count); // left padded to 2 chars wide
    lcd.print(txt);
}

void clearDiceType() {
    lcd.setCursor(2, 0);
    lcd.print("d   ");
}

void printDiceType() {
    lcd.setCursor(2, 0);
    sprintf(txt, "d%d", DICE_SIZES[dice_type]);
    lcd.print(txt);
}

void clearModifier() {
    lcd.setCursor(6, 0);
    lcd.print("    ");
}

void printModifier() {
    lcd.setCursor(6, 0);
    sprintf(txt, "%+d", modifier);
    lcd.print(txt);
}

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET
);

//Rotary encoder
#define ENCODER_CLK    32
#define ENCODER_DT     33
#define ENCODER_SW     27

//back 
#define BACK_BUTTON    14

enum Screen
{
    MAIN_MENU,
    MUSIC_BROWSER,
    SETTINGS
};

Screen currentScreen = MAIN_MENU;

//Button variables

int backButtonState;
int selectButtonState;

int lastBackButtonState = HIGH;
int lastSelectButtonState = HIGH;

//encoder variables

volatile int counter = 0;
volatile int lastCLKstate;

//Brightness
int brightness = 255;

//MENU

int selection = 0;
int topTrack = 0;

//Songs
const char *trackList[] =
{
    "Everybody wants to rule the world",
    "Gimme Gimme Gimme",
    "The Night Begins To Shine",
    "Lay All Your Love On Me",
    "Cheri Cheri Lady",
    "Happy Nation"
};

const int totalTracks =
    sizeof(trackList) / sizeof(trackList[0]);


void readEncoderISR();

void setup()
{
    Serial.begin(115200);

    Wire.begin();

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("SSD1306 allocation failed");

        while (true);
    }

    const char* splashText = "AudioDeck";
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    for (int i = 0; i <= strlen(splashText); i++)
    {
        display.clearDisplay();

        display.setCursor(10, 25);

        display.write((const uint8_t*)splashText, i);

        display.display();

        delay(160);
    }

    delay(1000);

    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);

    pinMode(BACK_BUTTON, INPUT_PULLUP);

    lastCLKstate = digitalRead(ENCODER_CLK);

    attachInterrupt(
        digitalPinToInterrupt(ENCODER_CLK),
        readEncoderISR,
        CHANGE
    );

    display.clearDisplay();
    display.display();
}

void loop()
{
    backButtonState = digitalRead(BACK_BUTTON);
    selectButtonState = digitalRead(ENCODER_SW);

    if (backButtonState == LOW &&
        lastBackButtonState == HIGH)
    {
        currentScreen = MAIN_MENU;
        counter = selection;
    }

    switch (currentScreen)
    {

    case MAIN_MENU:

        if (counter > 1)
            counter = 1;

        if (counter < 0)
            counter = 0;

        selection = counter;

        display.clearDisplay();

        if (selectButtonState == LOW &&
            lastSelectButtonState == HIGH)
        {
            if (selection == 0)
            {
                currentScreen = MUSIC_BROWSER;
                counter = 0;
                topTrack = 0;
            }

            if (selection == 1)
            {
                currentScreen = SETTINGS;
            }
        }

        display.setTextSize(1);

        if (selection == 0)
        {
            display.fillRoundRect(
                22,
                10,
                84,
                20,
                4,
                SSD1306_WHITE);

            display.setTextColor(SSD1306_BLACK);
        }
        else
        {
            display.drawRoundRect(
                22,
                10,
                84,
                20,
                4,
                SSD1306_WHITE);

            display.setTextColor(SSD1306_WHITE);
        }

        display.setCursor(48, 16);
        display.println("Music");


        if (selection == 1)
        {
            display.fillRoundRect(
                22,
                42,
                84,
                20,
                4,
                SSD1306_WHITE);

            display.setTextColor(SSD1306_BLACK);
        }
        else
        {
            display.drawRoundRect(
                22,
                42,
                84,
                20,
                4,
                SSD1306_WHITE);

            display.setTextColor(SSD1306_WHITE);
        }

        display.setCursor(42, 48);
        display.println("Settings");

        break;

    case MUSIC_BROWSER:

        if (counter > totalTracks - 1)
            counter = totalTracks - 1;

        if (counter < 0)
            counter = 0;

        display.clearDisplay();

        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setTextWrap(false);

        display.setCursor(0, 0);
        display.println("SELECT YOUR SONG:");


        if (counter >= topTrack + 4)
            topTrack = counter - 3;

        if (counter < topTrack)
            topTrack = counter;

        //Debugging :D

        if (selectButtonState == LOW &&
            lastSelectButtonState == HIGH)
        {
            Serial.print("Play Track: ");
            Serial.println(trackList[counter]);
 
        }

        for (int i = 0; i < 4; i++)
        {
            int currentItem = topTrack + i;

            if (currentItem >= totalTracks)
                break;

            int y = 16 + (i * 12);

            if (currentItem == counter)
            {
                display.fillTriangle(
                    0,
                    y,
                    0,
                    y + 6,
                    5,
                    y + 3,
                    SSD1306_WHITE);
            }

            display.setCursor(10, y);
            display.print(trackList[currentItem]);
        }

        break;

    case SETTINGS:

    //encoder limits
    counter = constrain(counter, 0, 255);
    brightness = counter;

    //contrast
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(brightness);

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0,0);
    display.println("Brightness");

    // % mapping
    int percent = map(brightness, 0, 255, 0, 100);

    display.setTextSize(2);
    display.setCursor(42,12);
    display.print(percent);
    display.print("%");

    //Bar outline
    display.drawRoundRect(10,42,108,14,4,SSD1306_WHITE);

    //Fill bar
    int barWidth = map(brightness,0,255,0,106);

    display.fillRoundRect(
        11,
        43,
        barWidth,
        12,
        3,
        SSD1306_WHITE
    );

    break;
    }

    display.display();
    //button stuff
    lastBackButtonState = backButtonState;
    lastSelectButtonState = selectButtonState;

    delay(20);
}


void readEncoderISR()
{
    int CLKstate = digitalRead(ENCODER_CLK);

    if (CLKstate != lastCLKstate)
    {
        if (digitalRead(ENCODER_DT) != CLKstate)
        {
            counter++;
        }
        else
        {
            counter--;
        }
    }

    lastCLKstate = CLKstate;
}       
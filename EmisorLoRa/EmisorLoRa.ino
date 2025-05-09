//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//define the pins used by the LoRa transceiver module
#define SCK     5
#define MISO   19
#define MOSI   27
#define SS     18
#define RST    23
#define DIO0   26

//LoRa band
#define BAND 915E6  // para Europa

//OLED pins (según imagen)
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST -1  // No conectado, se puede poner -1 si no se usa

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

int counter = 0;

// Inicializa la pantalla con RST deshabilitado (-1)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup() {
  Serial.begin(115200);

  // Inicializa la pantalla OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println(F("Fallo en inicialización de OLED"));
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("LORA SENDER");
  display.display();

  Serial.println("LoRa Sender Test");

  // Inicializa SPI para LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Fallo al iniciar LoRa");
    while (1);
  }

  Serial.println("LoRa Inicializado OK");
  display.setCursor(0, 10);
  display.print("LoRa Inicializado OK");
  display.display();
  delay(2000);
}

void loop() {
  Serial.print("Enviando paquete: ");
  Serial.println(counter);

  LoRa.beginPacket();
  LoRa.print("Paquete:  ");
  LoRa.print(counter);
  LoRa.endPacket();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("LORA SENDER");
  display.setCursor(0, 20);
  display.print("Paquete enviado.");
  display.setCursor(0, 30);
  display.print("Contador: ");
  display.print(counter);
  display.display();

  counter++;
  delay(10000);
}

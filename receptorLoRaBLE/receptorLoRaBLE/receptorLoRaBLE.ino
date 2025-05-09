// Librerías LoRa
#include <SPI.h>
#include <LoRa.h>

// Librerías BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Librerías OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pines LoRa
#define SCK     5
#define MISO   19
#define MOSI   27
#define SS     18
#define RST    23
#define DIO0   26

// Frecuencia LoRa
#define BAND 915E6

// Pines OLED
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST -1

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Se declara un servicio BLE
BLECharacteristic *mensajeCharacteristic;

#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-ab12-cd34-ef56-abcdef123456"


// Inicializa la pantalla OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup() {
  Serial.begin(115200);

  // Inicializa pantalla OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println(F("Fallo en inicialización OLED"));
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("LORA RECEPTOR");
  display.display();

  // Inicializa LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Fallo al iniciar LoRa");
    while (1);
  }

  Serial.println("LoRa Inicializado OK");
  display.setCursor(0, 10);
  display.println("LoRa listo");
  display.display();
  delay(2000);

  BLEDevice::init("LoRa32_Receptor");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  mensajeCharacteristic = pService->createCharacteristic(
                          CHARACTERISTIC_UUID,
                          BLECharacteristic::PROPERTY_READ |
                          BLECharacteristic::PROPERTY_NOTIFY
                        );

  mensajeCharacteristic->addDescriptor(new BLE2902());
  mensajeCharacteristic->setValue("Esperando datos...");
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();
  Serial.println("Servidor BLE iniciado");

}

void loop() {
  // Verifica si hay paquetes disponibles
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String mensaje = "";

    // Leer el paquete
    while (LoRa.available()) {
      mensaje += (char)LoRa.read();
    }

    // Obtener calidad de la señal
    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();

    // Imprimir en el monitor serial
    Serial.println("Mensaje recibido: " + mensaje);
    Serial.print("RSSI: ");
    Serial.print(rssi);
    Serial.print(" dBm, SNR: ");
    Serial.print(snr);
    Serial.println(" dB");

    // Mostrar en pantalla
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LORA RECEPTOR");
    display.setCursor(0, 15);
    display.print("Mensaje:");
    display.setCursor(0, 25);
    display.print(mensaje);
    display.setCursor(0, 40);
    display.print("RSSI: ");
    display.print(rssi);
    display.print(" dBm");
    display.setCursor(0, 50);
    display.print("SNR: ");
    display.print(snr);
    display.print(" dB");
    display.display();

    // Actualiza el valor BLE
    mensajeCharacteristic->setValue(mensaje.c_str());
    mensajeCharacteristic->notify();  // Notifica al cliente si está conectado

  }
}

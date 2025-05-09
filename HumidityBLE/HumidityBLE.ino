#include <DHT.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Definiciones para el sensor DHT11
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Definiciones para el sensor de distancia ultrasónico
const int trigPin = 5;
const int echoPin = 18;

// Variables para la duración del pulso y la distancia
long duration;
float distanceCm;

// Definiciones para BLE
#define SERVICE_UUID        "4fafc201-1d5b-459e-8f4f-929fbc0846f3" // UUID del servicio BLE
#define TEMPERATURE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8" // UUID de la característica de temperatura
#define HUMIDITY_CHARACTERISTIC_UUID    "beb5483f-36e1-4688-b7f5-ea07361b26a8"    // UUID de la característica de humedad
#define DISTANCE_CHARACTERISTIC_UUID    "beb54840-36e1-4688-b7f5-ea07361b26a8"  // UUID de la caracteristica de distancia

// Variables BLE
BLEServer *pServer = NULL;
BLECharacteristic *pTemperatureCharacteristic = NULL;
BLECharacteristic *pHumidityCharacteristic = NULL;
BLECharacteristic *pDistanceCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Función para enviar datos BLE
void sendDataBLE(float temperature, float humidity, float distance) {
  if (deviceConnected) {
    String tempData = String(temperature);
    String humidityData = String(humidity);
    String distanceData = String(distance);

    pTemperatureCharacteristic->setValue(tempData.c_str());
    pTemperatureCharacteristic->notify(); // Notificar al dispositivo conectado

    pHumidityCharacteristic->setValue(humidityData.c_str());
    pHumidityCharacteristic->notify();
    
    pDistanceCharacteristic->setValue(distanceData.c_str());
    pDistanceCharacteristic->notify();
  }
}

// Callback para manejar la conexión/desconexión de dispositivos BLE
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Dispositivo conectado");
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Dispositivo desconectado");
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando mediciones...");

  // Inicializar el sensor DHT
  dht.begin();

  // Configurar los pines del sensor ultrasónico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Inicializar BLE
  BLEDevice::init("ESP32_DHT_Ultrasonic"); // Nombre del dispositivo BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Crear el servicio BLE
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Crear las características BLE para temperatura, humedad y distancia
  pTemperatureCharacteristic = pService->createCharacteristic(
                               TEMPERATURE_CHARACTERISTIC_UUID,
                               BLECharacteristic::PROPERTY_READ |
                               BLECharacteristic::PROPERTY_NOTIFY
                             );
  pTemperatureCharacteristic->addDescriptor(new BLE2902());

  pHumidityCharacteristic = pService->createCharacteristic(
                              HUMIDITY_CHARACTERISTIC_UUID,
                              BLECharacteristic::PROPERTY_READ |
                              BLECharacteristic::PROPERTY_NOTIFY
                            );
  pHumidityCharacteristic->addDescriptor(new BLE2902());
  
  pDistanceCharacteristic = pService->createCharacteristic(
                              DISTANCE_CHARACTERISTIC_UUID,
                              BLECharacteristic::PROPERTY_READ |
                              BLECharacteristic::PROPERTY_NOTIFY
                            );
  pDistanceCharacteristic->addDescriptor(new BLE2902());

  // Iniciar el servicio BLE
  pService->start();

  // Iniciar la publicidad BLE
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setAppearance(0x00); //Establece la apariencia del dispositivo, 0x00 es desconocido
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();
  
  Serial.println("Esperando a que un dispositivo se conecte...");
}

void loop() {
  // Leer los valores del sensor DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Lectura en Celsius

  // Verificar si hubo algún error en la lectura del DHT
  if (isnan(h) || isnan(t)) {
    Serial.println("Error al leer el sensor DHT!");
  } else {
    Serial.print("Humedad: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.println(" *C");
  }

  // Medir la distancia con el sensor ultrasónico
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * 0.0343 / 2;

  Serial.print("Distancia: ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  // Enviar datos por BLE
  sendDataBLE(t, h, distanceCm);

  // Desconectar y reconectar
  if (!deviceConnected && oldDeviceConnected) {
      delay(500); // Esperar a que se complete la desconexión
      pServer->startAdvertising(); // Reiniciar la publicidad
      Serial.println("Iniciando publicidad");
  }
  oldDeviceConnected = deviceConnected;

  delay(1000);
}

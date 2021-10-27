/*
* This is a Bluetooth example using FreeRTOS on ESP32 dev board
*/

#include <Arduino.h>
#include "FreeRTOS.h"

#include <BluetoothSerial.h>
#include <Screens.h>


BluetoothSerial SerialBT;
static QueueHandle_t SerialQueue;
static QueueHandle_t OledQueue;


void BluetoothReceive(void * parameter)
{
  for(;;)
  {
    String BTReceived;
    if(SerialBT.available())
    {
      digitalWrite(BUILTIN_LED, HIGH);
      BTReceived = SerialBT.readString();
      if(xQueueSend(SerialQueue, (void *)&BTReceived, 10) != pdTRUE)
      {
        Serial.println("Serial Queue Full");
      }
      if(xQueueSend(OledQueue, (void *)&BTReceived, 10) != pdTRUE)
      {
        Serial.println("Oled Queue Full");
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void SerialWrite(void * parameter)
{
  /* Do your task thing here */
  String QReceive;
  for(;;)
  {
   if(xQueueReceive(SerialQueue, (void *)&QReceive, 0) == pdTRUE)
   {
      Serial.println(QReceive);
      digitalWrite(BUILTIN_LED, LOW);
   }
   vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void DisplayWrite(void * parameter)
{
  String DisplayString;
  display.setTextColor(WHITE);
  display.setTextSize(Font_Size);
  
  for(;;)
  {
    if((xQueueReceive(OledQueue, (void *)&DisplayString, 0) == pdTRUE))
    {
      if(DisplayString != "1")
      {
          display.clearDisplay();
          display.setCursor(0, 10);
          display.print(DisplayString);
          display.display();
      }
      else
      {
        testdrawbitmap();
      }
    }
   vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  SerialBT.begin("ESP32-FreeRTOS");
  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) // Address 0x3D for 128x64 
  { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // put your setup code here, to run once:
  xTaskCreate
  (
    BluetoothReceive,     /* Task Function                                    */
    "Bluetooth Receive",   /* Task Name                                        */
    2048,         /* Stack Size of Task                                */
    NULL,         /* Parameter of the task                             */
    1,            /* Task Priority (From 0 to (configMAX_PRIORITIES-1))*/
    NULL          /* Task Handle to Keep Track of Created Task         */
  );

  xTaskCreate
  (
    SerialWrite,   /* Task Function                                     */
    "Serial Write", /* Task Name                                         */
    2048,         /* Stack Size of Task                                */
    NULL,         /* Parameter of the task                             */
    0,            /* Task Priority (From 0 to (configMAX_PRIORITIES-1))*/
    NULL          /* Task Handle to Keep Track of Created Task         */
  );
  xTaskCreate
  (
    DisplayWrite,
    "Display Write",
    2048,
    NULL,
    0,
    NULL
  );

  SerialQueue = xQueueCreate(5, sizeof(String));
  OledQueue = xQueueCreate(5, sizeof(String));
  
  Serial.println("\nDevice Started!\n");
  //display.print("Device Started!");
  display.display();
}

void loop() {
  // put your main code here, to run repeatedly:
  //testdrawbitmap();
}


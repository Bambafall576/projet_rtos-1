#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>

typedef struct valeurCapteurs {
  int analogique;
  int numerique;
  unsigned long tempsEnMilliseconds;
} valeursCapteurs;

QueueHandle_t xQueueTask1;
QueueHandle_t xQueueTask2;
QueueHandle_t xQueueTask3;
QueueHandle_t xQueueTask4;

SemaphoreHandle_t  xMutexSerial = NULL;

valeursCapteurs vals;
int val_numerique;
int val_analogique;
int PIN_1 = 3;
int PIN_2 = 4;

void Task1(void*);
void Task2(void*);
void Task3(void*);
void Task4(void*);
void Task5(void*);

void printer(String);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  //Serial.println("debut");
  pinMode(PIN_1, INPUT);
  pinMode(PIN_2, INPUT);

  xMutexSerial = xSemaphoreCreateMutex();
  if (xMutexSerial != NULL) {
    Serial.println("Mutex 1 created");
  }
  xQueueTask1 = xQueueCreate(10, sizeof(int));
  xQueueTask2 = xQueueCreate(10, sizeof(int));
  xQueueTask3 = xQueueCreate(10, sizeof(valeurCapteurs));
  xQueueTask4 = xQueueCreate(10, sizeof(valeurCapteurs));

  if(xTaskCreate(Task1, "Task1", 100,  NULL,  3  ,  NULL ) != pdPASS) Serial.print("probleme creation tache");
  if(xTaskCreate(Task2, "Task2", 100,  NULL,  3  ,  NULL ) != pdPASS) Serial.print("probleme creation tache");
  if(xTaskCreate(Task3, "Task3", 100,  NULL,  3  ,  NULL ) != pdPASS) Serial.print("probleme creation tache");
  if(xTaskCreate(Task4, "Task4", 100,  NULL,  3  ,  NULL ) != pdPASS) Serial.print("probleme creation tache");
  if(xTaskCreate(Task5, "Task5", 200,  NULL,  3  ,  NULL ) != pdPASS) Serial.print("probleme creation tache");
}

void loop() {
  // put your main code here, to run repeatedly:
}




void Task1(void* p) {
  while (1) {
    val_analogique = analogRead(A0);
    xQueueSendToBack(xQueueTask1, &val_analogique, portMAX_DELAY );
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void Task2(void* p) {
  while (1) {
    val_numerique =  ((digitalRead(PIN_1)==HIGH)?1:0) + ((digitalRead(PIN_2)==HIGH)?1:0);
    xQueueSendToBack(xQueueTask2, &val_numerique, portMAX_DELAY );
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void Task3(void* p) {
  while (1) {
    if ((xQueueReceive( xQueueTask1, &vals.analogique, portMAX_DELAY ) == pdTRUE) && (xQueueReceive( xQueueTask2, &vals.numerique, portMAX_DELAY ) == pdTRUE)) {
      vals.tempsEnMilliseconds = millis();
      xQueueSendToBack(xQueueTask3, &vals, portMAX_DELAY );
    }
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void Task4(void* p) {
  valeursCapteurs val;
  while (1) {
    if (xQueueReceive( xQueueTask3, &val, portMAX_DELAY ) == pdTRUE) {
      xQueueSendToBack(xQueueTask4, &val, portMAX_DELAY );
      printer((String)("analogique : ") + val.analogique + (String)("\nnumerique : ") + val.numerique + (String)("\ntemps : ") + val.tempsEnMilliseconds + (String)("\n"));
    }
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void Task5(void* p) {
  while (1) {
    if ((xQueueReceive( xQueueTask4, &vals, portMAX_DELAY ) == pdTRUE)) {
      printer((String)("temps en minutes: ") + (vals.tempsEnMilliseconds/60000)+ (String)(" min\n"));
    }
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void printer(String s) {
  if( xSemaphoreTake(xMutexSerial, portMAX_DELAY) == pdTRUE){
    Serial.print(s);
    xSemaphoreGive(xMutexSerial); // release mutex
  }
 
}

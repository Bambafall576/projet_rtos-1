#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>

#define Potentio A0
#define Bouton1 3
#define Bouton2 4

//Declaration des taches
void Task1(void *pvParameters); 
void Task2(void *pvParameters); 
void Task3(void *pvParameters); 
void Task4(void *pvParameters); 
void Task5(void *pvParameters); 

QueueHandle_t arrayQueue;
TaskHandle_t TaskHandle_1;       
TaskHandle_t TaskHandle_2;
TaskHandle_t TaskHandle_3;       
TaskHandle_t TaskHandle_4;
TaskHandle_t TaskHandle_5;       
SemaphoreHandle_t myMutex = NULL;

//Creation d'une structure
  typedef struct  valeurCapteurs
 {  
  int analogique;
  int numerique;
  double tempsEnMillisecondes;
 }valeurCapteurs;

valeurCapteurs NewValeurCapteurs;

// Initialisation des variables à utiliser
int ValeurPotentio = 0;
int ValeurBouton1 = 0;
int ValeurBouton2 = 0;
int Numerique = 0;

void setup() {// la fonction void setup de l'arduino qui contient ici la creation des taches/Queue/mutex et la declaration du moniteur serie  
Serial.begin(9600);
arrayQueue=xQueueCreate(10,sizeof(int));
  myMutex = xSemaphoreCreateMutex();
  if(myMutex != NULL) Serial.println("Mutex created");
    if(arrayQueue!=NULL){
       xTaskCreate(Task1,"Task1",128,NULL,1,NULL);//xTaskCreate(Task1,"Task1",128,NULL,1,&TaskHandle_1);
       xTaskCreate(Task2,"Task2",128,NULL,1,NULL); 
       xTaskCreate(Task3,"Task3",128,NULL,1,NULL);
       xTaskCreate(Task4,"Task4",128,NULL,1,NULL); 
       xTaskCreate(Task5,"Task5",128,NULL,1,NULL);
       vTaskStartScheduler();
    }
}

void loop() {}
/******************************************************************************************************/
/**************************************** Les Taches *************************************************/
/****************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////Tache1
void Task1(void *pvParameters)
{
    (void) pvParameters;
  pinMode(Potentio, INPUT);
  //Serial.println("Task1 ...");// Tester si on est bien sur le Task1
  while(1)
  {
     ValeurPotentio = analogRead(Potentio);
     xQueueSend(arrayQueue,&ValeurPotentio,portMAX_DELAY);//envoi de la valeur lu sur A0 (valeur analogique)
     vTaskDelay(100);
  }

}
///////////////////////////////////////////////////////////////////////////////////Tache2
void Task2(void *pvParameters)
{
    (void) pvParameters;
    pinMode(Bouton1, INPUT);
    pinMode(Bouton2, INPUT);
    //Serial.println("Task2 ...");// Tester si on est bien sur le Task2
  while(1)
  {
     ValeurBouton1 = digitalRead(Bouton1);
     ValeurBouton2 = digitalRead(Bouton2);
     Numerique = ValeurBouton1 + ValeurBouton2;
     xQueueSend(arrayQueue,&Numerique,portMAX_DELAY);//envoi de la valeur numerique (somme de la valeur lu sur les pins 3 et 4)
     vTaskDelay(100);
  }


}
///////////////////////////////////////////////////////////////////////////////////Tache3
void Task3(void *pvParameters)
{
   (void) pvParameters;
  //Serial.println("Task3 ...");// Tester si on est bien sur le Task3
  while(1)
  {
    if(xQueueReceive(arrayQueue,&ValeurPotentio,portMAX_DELAY) == pdPASS )//Verifier la reception de la valeur envoyé depuis Task1
    {
     NewValeurCapteurs.analogique = ValeurPotentio;
    }
        if(xQueueReceive(arrayQueue,&Numerique,portMAX_DELAY) == pdPASS )//Verifier la reception de la valeur envoyé depuis Task2
    {
         NewValeurCapteurs.numerique = Numerique;
    }
    NewValeurCapteurs.tempsEnMillisecondes = 300000;
    xQueueSend(arrayQueue,&NewValeurCapteurs,portMAX_DELAY);//Envoie des nouvelles donnees de notre structure 
    vTaskDelay(1);
  }

}
///////////////////////////////////////////////////////////////////////////////////Tache4
void Task4(void *pvParameters)
{
    (void) pvParameters;
       // Serial.println("Task4 ...");// Tester si on est bien sur le Task4
      //xSemaphoreTake(myMutex,portMAX_DELAY);//while(myMutex == true); 
  while(1)
  {
     if(xQueueReceive(arrayQueue,&NewValeurCapteurs,portMAX_DELAY) == pdPASS )//Verification de la reception de NewValeurCapteurs
    {
      Serial.print("Analogique:");
      Serial.print(NewValeurCapteurs.analogique);
      Serial.print("\t\tNumerique:");
      Serial.print(NewValeurCapteurs.numerique);
      Serial.print("\t\tTempsEnMillisecondes:");
      Serial.println(NewValeurCapteurs.tempsEnMillisecondes);
    }

  }
     xQueueSend(arrayQueue,&NewValeurCapteurs,portMAX_DELAY);//Envoie des nouvelles donnees de notre structure
     vTaskDelay(1);
     //xSemaphoreGive(myMutex);//myMutex =true;
}
/////////////////////////////////////////////////////////////////////////////////////Tache5
void Task5(void *pvParameters)
{
    (void) pvParameters;
  //Serial.println("Task5 ...");// Tester si on est bien sur le Task5
        xSemaphoreTake(myMutex,portMAX_DELAY);//while(myMutex == true);//protection avec le semaphore mutex pour ne pas utiliser en meme tempps le moniteur serie entre le Task4 et Task5
  while(1)
  {
    if(xQueueReceive(arrayQueue,&NewValeurCapteurs,portMAX_DELAY) == pdPASS )//Verification de la reception de NewValeurCapteurs
    {

       Serial.print("Analogique:");
       Serial.print(NewValeurCapteurs.analogique);
       Serial.print("\t\tNumerique:");
       Serial.print(NewValeurCapteurs.numerique);
       Serial.print("\t\tTempsEnMinutes:");
       Serial.println(NewValeurCapteurs.tempsEnMillisecondes/60000);    
    }
  }
      xSemaphoreGive(myMutex);//Liberation du moniteur serie
}

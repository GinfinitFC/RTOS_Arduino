#include <Arduino_FreeRTOS.h>
#include <queue.h>

#define SWResume 0
#define button1 5
#define button2 6
#define button3 7
#define LED1 8
#define LED2 9
#define LED3 10

//Define tasks handler
TaskHandle_t resume_Handle;

TaskHandle_t but1_Handle;
TaskHandle_t but2_Handle;
TaskHandle_t but3_Handle;

TaskHandle_t led1_Handle;
TaskHandle_t led2_Handle;
TaskHandle_t led3_Handle;

//define queues handles variables
QueueHandle_t led1QueueHandle;
QueueHandle_t led2QueueHandle;
QueueHandle_t led3QueueHandle;

//define tasks functions
void resume_Task(void* param);

void Task_but1(void* param);
void Task_but2(void* param);
void Task_but3(void* param);

void Task_led1(void* param);
void Task_led2(void* param);
void Task_led3(void* param);

void setup() {
  // serial for debugging
  Serial.begin(9600);
  // pinmode assignation
  pinMode(SWResume, INPUT);
  
  pinMode(button1,INPUT);
  pinMode(button2,INPUT);
  pinMode(button3,INPUT);

  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);

  //tasks creations
  xTaskCreate(resume_Task,"resume",128,NULL,3,&resume_Handle);
  
  xTaskCreate(Task_but1,"button1",128,NULL,2,&but1_Handle);
  xTaskCreate(Task_but2,"button2",128,NULL,2,&but2_Handle);
  xTaskCreate(Task_but3,"button3",128,NULL,2,&but3_Handle);

  xTaskCreate(Task_led1,"led1",128,NULL,1,&led1_Handle);
  xTaskCreate(Task_led2,"led2",128,NULL,1,&led2_Handle);
  xTaskCreate(Task_led3,"led3",128,NULL,1,&led3_Handle);

  //Queues creations
  led1QueueHandle = xQueueCreate( 10, sizeof( uint8_t ) );
  led2QueueHandle = xQueueCreate( 10, sizeof( uint8_t ) );
  led3QueueHandle = xQueueCreate( 10, sizeof( uint8_t ) );
  
}

void loop() {/* Do not code in main loop */}

void resume_Task(void* param){
  (void) param;
  int sw = 0;

  for(;;){
    
    sw = digitalRead(SWResume);
    vTaskDelay(10/portTICK_PERIOD_MS);
    Serial.println(sw);
    
    if(sw == HIGH){
      Serial.println("Resuming tasks");
      vTaskResume(led1_Handle);
      vTaskDelay(400/portTICK_PERIOD_MS);
      vTaskResume(led2_Handle);
      vTaskDelay(1100/portTICK_PERIOD_MS);
      vTaskResume(led3_Handle);
    }
  }
    
}

void Task_but1(void* param){
  (void) param;
  uint8_t buttonval = 0;
  for(;;){
    buttonval = digitalRead(button1);
    //Serial.print("boton 1: ");
    Serial.println(buttonval);
    vTaskDelay(10/portTICK_PERIOD_MS);

    if(buttonval == HIGH){
      if(!xQueueSend(led3QueueHandle,&buttonval,100)){
      Serial.println("button 1 could not be sent");
      }
      vTaskDelay(10/portTICK_PERIOD_MS);
    }
  }
}

void Task_but2(void* param){
  (void) param;
  uint8_t buttonval2 = 0;
  for(;;){
    
    buttonval2 = digitalRead(button2);
    //Serial.print("boton 2: ");
    Serial.println(buttonval2);
    vTaskDelay(10/portTICK_PERIOD_MS);
    
    if(buttonval2 == HIGH){
      if(!xQueueSend(led1QueueHandle,&buttonval2,100)){
        Serial.println("button 2 could not be sent");
      }
      vTaskDelay(10/portTICK_PERIOD_MS);
    }
  }
}

void Task_but3(void* param){
  (void) param;
  uint8_t buttonval3 = 0;
  for(;;){
    buttonval3 = digitalRead(button3);
    //Serial.print("boton 3: ");
    Serial.println(buttonval3);
    vTaskDelay(10/portTICK_PERIOD_MS);

    if(buttonval3 == HIGH){
      if(!xQueueSend(led2QueueHandle,&buttonval3,100)){
        Serial.println("button 3 could not be sent");
      }
      vTaskDelay(10/portTICK_PERIOD_MS);
    }
  }
}

void Task_led1(void* param){
  (void) param;

  uint8_t flag = 0;
  for(;;){

    if(!xQueueReceive(led1QueueHandle,&flag,100)){
      Serial.println("Value not received");
    }
    if(flag == 1){//pause
      vTaskSuspend(led1_Handle);
    }else{
      digitalWrite(LED1,HIGH);
      vTaskDelay(500/portTICK_PERIOD_MS);
      digitalWrite(LED1,LOW);
      vTaskDelay(500/portTICK_PERIOD_MS); 
    }
    vTaskDelay(10/portTICK_PERIOD_MS);
  }
}

void Task_led2(void* param){
  (void) param;
  uint8_t flag = 0;
  
  for(;;){

    if(!xQueueReceive(led2QueueHandle,&flag,100)){
      Serial.println("Value not received");
    }
    if(flag == 1){//pause
      vTaskSuspend(led2_Handle);
    }else{
      digitalWrite(LED2,HIGH);
      vTaskDelay(500/portTICK_PERIOD_MS);
      digitalWrite(LED2,LOW);
      vTaskDelay(500/portTICK_PERIOD_MS); 
    }
    vTaskDelay(10/portTICK_PERIOD_MS);
  }
}

void Task_led3(void* param){
  (void) param;
  
 uint8_t flag = 0;
  for(;;){

   if(!xQueueReceive(led3QueueHandle,&flag,100)){
      Serial.println("Value not received");
    }
    if(flag == 1){//pause
      vTaskSuspend(led3_Handle);
    }else{
      digitalWrite(LED3,HIGH);
      vTaskDelay(500/portTICK_PERIOD_MS);
      digitalWrite(LED3,LOW);
      vTaskDelay(500/portTICK_PERIOD_MS); 
    }
    vTaskDelay(10/portTICK_PERIOD_MS);
  }
}

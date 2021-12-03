//RTOS Practice 1
//Read signal from a potentiometer and turn on diferent led colours to identify each task
//Giancarlo Franco Carrillo A01638108 IRS

#include <Arduino_FreeRTOS.h>
#include <queue.h>
// define tasks functions names
void ADC_ReadTask( void *pvParameters );
void LedTask( void *pvParameters );
void Set_LedTask( void *pvParameters );
//define queues handles variables
QueueHandle_t PotValue;
QueueHandle_t Flag;
//Define semaphores
// #include <semphr.h>
//SemaphoreHandle_t semaphore name

//for identifying leds
#define GreenLed 2
#define YellowLed 3
#define RedLed 4
//for masks
#define LedsOff 0
#define GreenLed_MaxValue 341
#define YellowLed_MaxValue 682
#define RedLed_MaxValue 1024

void setup() {
  //initializing Serial plot for debugging
  Serial.begin(9600);
  //initializing tasks: 
  xTaskCreate(
    ADC_ReadTask      // Function name
    ,  "DigitalRead"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL //Parameters for the task
    ,  2  // Priority
    ,  NULL ); //Task Handle

  xTaskCreate(
    LedTask         // Function name
    ,  "Leds" // A name just for humans
    ,  128  // Stack size
    ,  NULL //Parameters for the task
    ,  1  // Priority
    ,  NULL ); //Task Handle

      xTaskCreate(
    Set_LedTask       // Function name
    ,  "Set_Led"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL //Parameters for the task
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL ); //Task Handle
    //initializing Queues
    PotValue = xQueueCreate( 10, sizeof( uint16_t ) );
    Flag = xQueueCreate( 10, sizeof( uint16_t ) );

    //initializing semaphores
    // Semaphores are useful to stop a Task proceeding, where it should be paused to wait,
    // because it is sharing a resource, such as the Serial port.
    // Semaphores should only be used whilst the scheduler is running, but we can set it up here.
//    if ( xSerialSemaphore == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
//    {
//      xSerialSemaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore we will use to manage the Serial Port
//      if ( ( xSerialSemaphore ) != NULL )
//        xSemaphoreGive( ( xSerialSemaphore ) );  // Make the Serial Port available for use, by "Giving" the Semaphore.
//    }

}

void loop() {
  // Empty. Things are done in Tasks.
}

void ADC_ReadTask(void *pvParameters){

  //variables
  uint16_t analogValue = 0;//potentiometer attached to pin A0 from arduino uno
  pinMode(analogValue, INPUT);
  uint16_t raw_value = 0;
  for(;;){

    raw_value=analogRead(analogValue);
    Serial.print("Pot value: ");
    Serial.println(raw_value);
    if(!xQueueSend(PotValue,&raw_value,100)){
      Serial.println("Value could not be sent");
    }
    vTaskDelay(10/portTICK_PERIOD_MS);//debounce
  }
}

void LedTask(void *pvParameters){

  uint16_t ledFlags;
  pinMode(GreenLed, OUTPUT);
  pinMode(YellowLed, OUTPUT);
  pinMode(RedLed, OUTPUT);
  for(;;)
  {
    if(!xQueueReceive(Flag,&ledFlags,1000)){
      Serial.println("Value not received");
    }
    else{
      //GREEN LED sequence
      if(ledFlags==GreenLed_MaxValue){
        digitalWrite(GreenLed, HIGH);
      }
      else{
        digitalWrite(GreenLed, LOW);
      }
      //YELLOW LED sequence
      if(ledFlags==YellowLed_MaxValue){
        digitalWrite(YellowLed, HIGH);
      }
      else{
        digitalWrite(YellowLed, LOW);
      }
      //RED LED sequence
      if(ledFlags==RedLed_MaxValue){
        digitalWrite(RedLed, HIGH);
      }
      else{
        digitalWrite(RedLed, LOW);
      }
    }
    vTaskDelay(10/portTICK_PERIOD_MS);
  }
}

void Set_LedTask(void *pvParameters){

  uint16_t analogValue;
  uint16_t ledFlags;
  for(;;)
  {
    if(!xQueueReceive(PotValue,&analogValue,1000)){
      Serial.println("Value not received");
    }
    else{
      //set active led application
      if(analogValue<GreenLed_MaxValue && analogValue>0){
        ledFlags = GreenLed_MaxValue;
        Serial.print("Green: ");
        Serial.println(ledFlags);
      }
      else if(analogValue<YellowLed_MaxValue && analogValue>0){
        ledFlags = YellowLed_MaxValue;
        Serial.print("Yellow: ");
        Serial.println(ledFlags);
      }
      else if(analogValue<RedLed_MaxValue && analogValue>0){
        ledFlags = RedLed_MaxValue;
        Serial.print("Red: ");
        Serial.println(ledFlags);
      }
      else{
        ledFlags = LedsOff;
      }
      if(!xQueueSend(Flag,&ledFlags,1000)){
        Serial.println(ledFlags);
      }
    }
    vTaskDelay(10/portTICK_PERIOD_MS);
  }
}

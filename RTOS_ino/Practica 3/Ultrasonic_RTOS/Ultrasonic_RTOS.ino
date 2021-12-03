//Practice 3 Reading signal from ultrasonic sensor with RTOS
//Libraries
#include <Arduino_FreeRTOS.h>
#include <queue.h>

// define tasks functions names
void ReadDist_Task( void *pvParameters );
void LedTask( void *pvParameters );
void Set_LedTask( void *pvParameters );

//define queues handles variables
QueueHandle_t Ultrasonic;
QueueHandle_t Led;

//for identifying leds
#define GreenLed 2
#define YellowLed 3
#define RedLed 4
//for ultrasonic
#define Trigger 5 //Trigger from ultrasonic
#define Echo 6 //Echo from ultrasonic
//for masks
#define LedsOff 4
#define GreenLed_MaxValue 7
#define YellowLed_MaxValue 14
#define RedLed_MaxValue 22

void setup() {
  //initializing Serial plot for debugging
  Serial.begin(9600);
  //initializing tasks: 
  xTaskCreate(
    ReadDist_Task      // Function name
    ,  "Ultrasonic Read"  // A name just for humans
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
    Ultrasonic = xQueueCreate( 10, sizeof( uint16_t ) );
    Led = xQueueCreate( 10, sizeof( uint16_t ) );
    
}

void loop() { /* empty, things are done with tasks*/ }

void ReadDist_Task(void *pvParameters){//Reading distance sequence
  
  //local variables 
  uint16_t trigg = 0;
  uint16_t dist = 0;

   //ultrasonic
  pinMode(Trigger, OUTPUT); 
  digitalWrite(Trigger, LOW);//start trigger with 0
  pinMode(Echo, INPUT); 
  for(;;){
    digitalWrite(Trigger, HIGH);
    vTaskDelay(0.01/portTICK_PERIOD_MS); //pulse of 10us
    digitalWrite(Trigger, LOW);
    
    trigg = pulseIn(Echo, HIGH); //obtenemos el ancho del pulso
    dist = trigg/59;             //escalamos el tiempo a una distancia en cm
    Serial.print("Ultrasonic: ");
    Serial.println(dist);
    if(!xQueueSend(Ultrasonic,&dist,100)){
      Serial.println("Distance could not be sent");
    }
    vTaskDelay(20/portTICK_PERIOD_MS);//debounce
  }
}

void LedTask(void *pvParameters){

  uint16_t ledFlags;
  pinMode(GreenLed, OUTPUT);
  pinMode(YellowLed, OUTPUT);
  pinMode(RedLed, OUTPUT);
  for(;;)
  {
    if(!xQueueReceive(Led,&ledFlags,100)){
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
    if(!xQueueReceive(Ultrasonic,&analogValue,100)){
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
      if(!xQueueSend(Led,&ledFlags,100)){
        Serial.println(ledFlags);
      }
    }
    vTaskDelay(10/portTICK_PERIOD_MS);
  }
}

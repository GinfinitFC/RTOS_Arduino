//RTOS + PD Control 
//Linear Smart actuator for controlling the front chasis of a tractor
//José Miguel Figarola Prado A01632557
//Giancarlo Franco Carrillo A01638108
//Marco Alexis López Cruz A01638032


//Libraries
#include <Arduino_FreeRTOS.h>
#include <queue.h>

// define tasks functions names
void ReadPot_Task( void *pvParameters );
void ReadDist_Task( void *pvParameters );
void MoveMotor_Task( void *pvParameters );

//define queues handles variables
QueueHandle_t RefHandler;
QueueHandle_t DistHandler;

///////////////Definitions/////////////////////
             /////Digital/////
//Ultrasonic//
#define Trigger 5 //Trigger from ultrasonic
#define Echo 6 //Echo from ultrasonic
///MOTOR///
#define ina 9
#define inb 10
#define EN 11 //PWM h bridge
          /////Analog/////
///Reference///
#define Pot 0
//////////////////////////////////////////

void setup() {
 //initializing Serial plot for debugging
  Serial.begin(9600);
  /////PINOUT Declaration/////
  //motor
  pinMode(EN, OUTPUT);
  pinMode(ina, OUTPUT);
  pinMode(inb, OUTPUT);
  //Potentiometer
  pinMode(Pot, INPUT);
  //ultrasonic
  pinMode(Trigger, OUTPUT); 
  pinMode(Echo, INPUT); 

  //initializing tasks: 
  xTaskCreate(
    ReadPot_Task       // Function name
    ,  "Reference"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL //Parameters for the task
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL ); //Task Handler 
  xTaskCreate(ReadDist_Task, "Distance", 128, NULL, 2, NULL);  
  xTaskCreate(MoveMotor_Task, "Control", 128, NULL, 1, NULL); 
  
  //initializing Queues
  RefHandler = xQueueCreate( 10, sizeof( uint16_t ) );
  DistHandler = xQueueCreate( 10, sizeof( uint16_t ) );
  
}

void loop() {/* Empty. Things are done in Tasks. */}

void ReadPot_Task(void *pvParameters){ //Reading reference sequence
  
  //variable for reading value from potentiometer
  uint16_t raw_value = 0;
  uint16_t ref = 0;
  for(;;){

    raw_value=analogRead(Pot);
    ref=map(raw_value,0,1023,0,18);//this modifies the potentiometer value so that the position works now by the distance the cart moves
    if(!xQueueSend(RefHandler,&ref,100)){
      Serial.println("Reference could not be sent");
    }
    vTaskDelay(10/portTICK_PERIOD_MS);//debounce
  }
}

void ReadDist_Task(void *pvParameters){//Reading distance sequence

  //local variables 
  uint16_t trigg = 0;
  uint16_t dist = 0;
  digitalWrite(Trigger, LOW);//start trigger with 0
  for(;;){
    digitalWrite(Trigger, HIGH);
    vTaskDelay(0.01/portTICK_PERIOD_MS); //pulse of 10us
    digitalWrite(Trigger, LOW);
    
    trigg = pulseIn(Echo, HIGH); //obtenemos el ancho del pulso
    dist = trigg/59;             //escalamos el tiempo a una distancia en cm
    if(!xQueueSend(DistHandler,&dist,100)){
      Serial.println("Distance could not be sent");
    }
    vTaskDelay(20/portTICK_PERIOD_MS);//debounce
  }
}

void MoveMotor_Task(void *pvParameters){//Control PD sequence

  //external variables
  uint16_t reference = 0;
  uint16_t dist = 0;
  ////local variables////
  uint16_t PWM_ctrl = 0;
  uint16_t prev = 0; //previous distance
  const float n = 0.02;//step of time
  int error = 0;//error
  int eprev = 0;//previous error
  int kp = 30; //proportional control part
  float kd = 0.5; //derivative control
  float dedt = 0.0;//derivative error
  //flags for security
  bool Rdist = true;
  bool Rref = true;
  
  for(;;){
    //First we catch the messages from queues
    if(!xQueueReceive(RefHandler,&reference,100)){
      Serial.println("reference not received");
      Rref = false; //update flag
    }
    if(!xQueueReceive(DistHandler,&dist,100)){
      Serial.println("distance not received");
      Rdist = false;
    }
    if(Rref && Rdist){ //all values were read from queues it is safe tto continue with task
        error=dist-reference; //error now works under the distance read by ultrasonic
        dedt = (error-eprev)/n;//derivative error
        PWM_ctrl = kp*error + kd*dedt;
        if(PWM_ctrl>0){//clockwise routine
          if(PWM_ctrl>255){
            PWM_ctrl=255;
          }
          digitalWrite(EN, PWM_ctrl); 
          digitalWrite(ina, HIGH); 
          digitalWrite(inb, LOW);
        }else{//counterclockwise routine
          PWM_ctrl=PWM_ctrl*-1;//pwm cannot be negative
          if(PWM_ctrl>255){
            PWM_ctrl=255;
          }
          digitalWrite(EN, PWM_ctrl); //0-255
          digitalWrite(ina, LOW); 
          digitalWrite(inb, HIGH);
        }
        if(prev!=dist){
           //data print routine
          Serial.print(PWM_ctrl);
          Serial.print("  ");
          Serial.print(error);
          Serial.print("  ");
          Serial.print(reference);
          Serial.print("  ");
          Serial.println(dist);//invert read this is because of hardware design
          prev=dist;//save previous distance
          eprev=error;//save previous error
        }
        //update flags, lecture was fine
        Rdist = true;
        Rref = true;
    }else{//no values were read from tasks notify user
      Serial.println("!Warning theres a missing value to complete order");
      vTaskDelay(10000/portTICK_PERIOD_MS);
    }
    vTaskDelay(10/portTICK_PERIOD_MS);//wait order
  }
}

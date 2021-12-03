//Practica 2 Read encoder and move it acording to a reference
//Libraries
#include <Arduino_FreeRTOS.h>
#include <queue.h>


///////////////Definitions/////////////////////
             /////Digital/////
///button for changing spin of motor
#define spin 5
#define A 3 //Phase A from encoder isr
#define B 4 //Phase B from encoder
//Reference
#define Pot 0
///MOTOR///
#define ina 9
#define inb 10
#define EN 11 //PWM h bridge send by a potentiometer

// define tasks functions names
void ReadPot_Task( void *pvParameters );
void ChangeSpin_Task( void *pvParameters );
void ReadMotor_Task( void *pvParameters );
void MoveMotor_Task( void *pvParameters );

//define queues handles variables
QueueHandle_t RefHandler;
QueueHandle_t EncoderHandler;
QueueHandle_t ButtonHandler;

void setup() {
   //initializing Serial plot for debugging
  Serial.begin(9600);
  /////PINOUT Declaration/////
  //motor
  pinMode(EN, OUTPUT);
  pinMode(ina, OUTPUT);
  pinMode(inb, OUTPUT);
   //Encoder and Potentiometer
  pinMode(Pot, INPUT);
  pinMode(A, INPUT);
  pinMode(B, INPUT);
  //button
  pinMode(spin, INPUT);
  //initialize moto
  digitalWrite(EN, 0);
  digitalWrite(ina, LOW);
  digitalWrite(inb, LOW);

   //initializing tasks: 
  xTaskCreate(
    ReadPot_Task       // Function name
    ,  "Reference"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL //Parameters for the task
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL ); //Task Handler
  xTaskCreate(ChangeSpin_Task, "Spin", 128, NULL, 2, NULL);
  xTaskCreate(ReadMotor_Task, "Encoder", 128, NULL, 0, NULL);  
  xTaskCreate(MoveMotor_Task, "Control", 128, NULL, 1, NULL); 
  
  //initializing Queues
  RefHandler = xQueueCreate( 10, sizeof( uint16_t ) );
  EncoderHandler = xQueueCreate( 10, sizeof( uint16_t ) );
  ButtonHandler = xQueueCreate( 10, sizeof( uint16_t ) );
}

void loop() { /*NOTHING*/ }

void ReadPot_Task(void *pvParameters){ //Reading reference sequence
  
  //variable for reading value from potentiometer
  uint16_t raw_value = 0;
  uint16_t ref = 0;
  for(;;){
    raw_value=analogRead(Pot);
    ref=map(raw_value,0,1023,0,255);//this modifies the potentiometer value so that the position works now by the distance the cart moves
    if(!xQueueSend(RefHandler,&ref,100)){
      Serial.println("Reference could not be sent");
    }
    vTaskDelay(10/portTICK_PERIOD_MS);//debounce
  }
}

void ChangeSpin_Task(void *pvParameters){//changing motor direction
  uint16_t flag = 0;
  int button = 0;
  for(;;){
    button = digitalRead(spin);
    if(button == HIGH){
      flag = 1;
    }
    else{
      flag = 0;
    }
//    Serial.print(button);
//    Serial.print(",");
//    Serial.println(flag);
    if(!xQueueSend(ButtonHandler,&flag,100)){
      Serial.println("spin could not be sent");
    }
    vTaskDelay(20/portTICK_PERIOD_MS);//debounce
  }
}

void ReadMotor_Task(void *pvParameters){//Reading encoder sequence

  int Aprev = 0;
  int Anew = 0;
  //variable for reading position from encoder
  uint16_t pos = 0;
  
  for(;;){
//    Serial.print(digitalRead(A));
//    Serial.print(",");
//    Serial.println(digitalRead(B));
    Anew = digitalRead(A);
    if(Aprev != Anew){
      if (digitalRead(B) == HIGH){
        pos--;
      }
      else{
        pos++;
      }
      Aprev = Anew;
    }
    if(!xQueueSend(EncoderHandler,&pos,100)){
      Serial.println("Position could not be sent");
    }
    vTaskDelay(10/portTICK_PERIOD_MS);//debounce
  }
}

void MoveMotor_Task(void *pvParameters){//Control PD sequence

  //external variables
  uint16_t PWM = 0; // from pot
  uint16_t pos = 0; // from encoder
  uint16_t flag; // for spin
  uint16_t prev = 0; //previous position
  //flags for security
  bool Rspin = true;
  bool Rpos = true;
  bool Rref = true;
  
  for(;;){
    
    //First we catch the messages from queues
    if(!xQueueReceive(RefHandler,&PWM,100)){
      Serial.println("reference not received");
      Rref = false; //update flag
    }
    if(!xQueueReceive(EncoderHandler,&pos,100)){
      Serial.println("encoder position not received");
      Rpos = false;
    }
    if(!xQueueReceive(ButtonHandler,&flag,100)){
      Serial.println("spin not received");
      Rspin = false;
    }
    
    if(Rref && Rspin){ //all values were read from queues it is safe tto continue with task
        if(flag){//clockwise routine
          digitalWrite(EN, PWM); 
          digitalWrite(ina, HIGH); 
          digitalWrite(inb, LOW);
        }else{//counterclockwise routine
          digitalWrite(EN, PWM); //0-255
          digitalWrite(ina, LOW); 
          digitalWrite(inb, HIGH);
        }
        if(pos != prev){
          //data print routine
          Serial.print(flag);
          Serial.print("  ");
          Serial.print(PWM);
          Serial.print("  ");
          Serial.println(pos);
          prev=pos;//save previous position
        }
    }
    //update flags, lecture was fine
    Rref = true;
    Rspin = true;
    vTaskDelay(20/portTICK_PERIOD_MS);//wait order
  }
}

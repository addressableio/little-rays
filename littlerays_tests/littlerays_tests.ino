/* Inputs and Outputs*/
#define light 2 //the little rays output light
#define dark1 A2 //higher value corresponds to "darker" room
#define dark2 A3
#define power 0 //Power to the darkness circuits

/*Other Constants*/

/*Other variable setup*/
int threshold = 510;



void setup() {
  //Setting up
  pinMode(light, OUTPUT);
  pinMode(power, OUTPUT);
  digitalWrite(power, HIGH);
}

void loop() {
  
  if(analogRead(dark1) > threshold && analogRead(dark2) > threshold){
    digitalWrite(light, HIGH);
  }else{
    digitalWrite(light, LOW);
  }
}

#include <Servo.h>

boolean firstRun = true;

#define movAvgFillter_data 30

#define PIN_IR 3
#define PIN_LED 9
#define PIN_SERVO 10

#define _DIST_MIN 100.0
#define _DIST_MAX 250.0

#define _DUTY_MIN 553  // servo full clock-wise position (0 degree)
#define _DUTY_NEU 1476 // servo neutral position (90 degree)
#define _DUTY_MAX 2399 // servo full counter-clockwise position (180 degree)

//#define _EMA_ALPHA 0.3

#define LOOP_INTERVAL 30   // Loop Interval (unit: msec)

float  dist_ema, dist_prev = _DIST_MAX; // unit: mm
unsigned long last_loop_time;   // unit: msec

Servo myservo;

float movAvgFillter(float k) {  //이동평균필터
  float data_stack[movAvgFillter_data]; // 데이터스텍 10개  많으면 오차는 줄어드나 지연이 생김
  float preAvg;         // 이전에 계산산 평균값
  

  if(firstRun) {  // 처음 1회만 실행됨
    for(int f = 0; f <movAvgFillter_data ;f++) data_stack[f] = k;  // 초기화 첫 입력값으로 전부 입력, 평균이 입력값이 되게함
    
    preAvg = k;  
    //total = 0;
    firstRun = false;
  }  // 처음 1회만 실행됨

  for(int j = 0; j < movAvgFillter_data; j++) data_stack[j] = data_stack[j+1];  // 새로운 값을 박기 위해 한칸식 앞으로

  data_stack[movAvgFillter_data-1] = k; // 새로운값을 마지막에 넣음

  float total = 0;

  for(int f = 0; f <movAvgFillter_data ;f++) total = total + data_stack[f];  // 합계를 구함

  return (total/movAvgFillter_data);
  
}

void setup()
{
  pinMode(PIN_LED, OUTPUT);

  myservo.attach(PIN_SERVO); 
  myservo.writeMicroseconds(_DUTY_NEU);
  
  dist_prev = _DIST_MIN;

  Serial.begin(1000000);
}

void loop()
{
  unsigned long time_curr = millis();
  int a_value, duty;
  float dist;

  // wait until next event time
  if (time_curr < (last_loop_time + LOOP_INTERVAL))
    return;

  a_value = analogRead(PIN_IR);
  dist = (6762.0/(a_value-9)-4.0)*10.0 - 60.0;

  if (dist < _DIST_MIN) {
    dist = dist_prev;           // cut lower than minimum
    digitalWrite(PIN_LED, 1);       // LED OFF
  } else if (dist > _DIST_MAX) {
    dist = dist_prev;           // Cut higher than maximum
    digitalWrite(PIN_LED, 1);       // LED OFF
  } else {    // In desired Range
    digitalWrite(PIN_LED, 0);       // LED ON      
    dist_prev = dist;
  }

  //dist_ema = _EMA_ALPHA * dist + (1-_EMA_ALPHA) * dist_ema;



  dist_ema = movAvgFillter(dist);

  duty = dist_ema*12.3-1230;


  myservo.writeMicroseconds(duty);
  

  Serial.print("MIN:");  Serial.print(_DIST_MIN);
  Serial.print(",IR:");  Serial.print(a_value);
  Serial.print(",dist:");   Serial.print(dist);
  Serial.print(",ema:");   Serial.print(dist_ema);
  Serial.print(",servo:");   Serial.print(duty);
  Serial.print(",MAX:");   Serial.print(_DIST_MAX);
  Serial.println("");

  last_loop_time += LOOP_INTERVAL;
}

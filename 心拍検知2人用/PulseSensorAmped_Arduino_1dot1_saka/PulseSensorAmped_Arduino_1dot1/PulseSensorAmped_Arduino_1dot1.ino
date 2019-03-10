#include <FlexiTimer2.h>

/*
Pulse Sensor sample aquisition and processing happens in the background via Timer 2 interrupt. 2mS sample rate.
PWM on pins 3 and 11 will not work when using this code, because we are using Timer 2!
The following variables are automatically updated:
Signal: （int型）センサからのアナログデータ、2ミリ秒ごとに取得
IBI: （int型）心拍の時間間隔を保持、2ミリ秒解像度
BPM: （int型）心拍数、直近10のIBI値の平均
QS: （boolean型）心拍が見つかりBPMが更新されるときにtrue、ユーザがリセットする必要がある
Pulse: （boolean型）心拍検出時にtrue、pin13消灯時にfalse
*/


/******

実際手つなぎシューティングで使う心拍情報は、
1人目の心拍数 BPM1 と2人目の心拍数 BPM2
だけになると思います。
なので、LEDを点灯させる設定とか
変数IBIとかは省いても問題ないです。

******/

//てつなぎ判定とジョイスティック操作の変数///////////////////
const int AVERAGE = 100;
const int HAND = 5;
const int XIN = 3;
const int YIN = 4;
//const int BUTTON = 7;


//int button = 0;
long val[AVERAGE]={};
double valsum = 0;
long xinput = 0;
long yinput = 0;

double valave = 0;
int count = 0;
//int state = 0;
//int LEDstate = 0;


// ユーザ1の変数設定
int pulsePin1 = 1;                 // 心拍センサの紫ケーブルをA0に接続
//int blinkPin1 = 13;                // パルスごとに13ピンのLEDを点灯
//int fadePin1 = 5;                  // pin to do fancy classy fading blink at each beat
//int fadeRate1 = 0;                 // used to fade LED on with PWM on fadePin
// ユーザ2の変数設定
int pulsePin2 = 2;                 // 心拍センサの紫ケーブルをA1に接続
//int blinkPin2 = 12;                // パルスごとに13ピンのLEDを点灯
//int fadePin2 = 5;                  // pin to do fancy classy fading blink at each beat
//int fadeRate2 = 0;                 // used to fade LED on with PWM on fadePin

//心拍数の変数設定//////////////////////////////////////////////////////////////////////////////////////
// these variables are volatile because they are used during the interrupt service routine!
// ユーザ1の変数設定
volatile int BPM1;                   // 心拍数
volatile int Signal1;                // センサデータ
volatile int IBI1 = 600;             // 心拍の間隔
volatile boolean Pulse1 = false;     // パルス波が高いときtrue、低いときfalse
volatile boolean QS1 = false;        // 心拍検出時にtrue
// ユーザ2の変数設定
volatile int BPM2;                   // 心拍数
volatile int Signal2;                // センサデータ
volatile int IBI2 = 600;             // 心拍の間隔
volatile boolean Pulse2 = false;     // パルス波が高いときtrue、低いときfalse
volatile boolean QS2 = false;        // 心拍検出時にtrue
////////////////////////////////////////////////////////////////////////////////////////////


//以下旧interuptプログラムの変数////////////////////////////////////////////////////////////////////////
volatile int rate1[10];                    // used to hold last ten IBI values
volatile unsigned long sampleCounter1 = 0;          // used to determine pulse timing
volatile unsigned long lastBeatTime1 = 0;           // used to find the inter beat interval
volatile int P1 = 512;                      // used to find peak in pulse wave
volatile int T1 = 512;                     // used to find trough in pulse wave
volatile int thresh1 = 512;                // used to find instant moment of heart beat
volatile int amp1 = 100;                   // used to hold amplitude of pulse waveform
volatile boolean firstBeat1 = true;        // used to seed rate array so we startup with reasonable BPM
volatile boolean secondBeat1 = true;       // used to seed rate array so we startup with reasonable BPM

volatile int rate2[10];                    // used to hold last ten IBI values
volatile unsigned long sampleCounter2 = 0;          // used to determine pulse timing
volatile unsigned long lastBeatTime2 = 0;           // used to find the inter beat interval
volatile int P2 = 512;                      // used to find peak in pulse wave
volatile int T2 = 512;                     // used to find trough in pulse wave
volatile int thresh2 = 512;                // used to find instant moment of heart beat
volatile int amp2 = 100;                   // used to hold amplitude of pulse waveform
volatile boolean firstBeat2 = true;        // used to seed rate array so we startup with reasonable BPM
volatile boolean secondBeat2 = true;       // used to seed rate array so we startup with reasonable BPM
//ここまで//////////////////////////////////////////////////////////////////////////////////////////////


void setup(){
//  pinMode(A0, INPUT);
  Serial.begin(115200);             // シリアル通信開始
//  interruptSetup();                 // 2ミリ秒間隔でパルスを読み取るセットアップ
  FlexiTimer2::set(2,mS_interupt); // 500ms period   
  FlexiTimer2::start();
   // UN-COMMENT THE NEXT LINE IF YOU ARE POWERING The Pulse Sensor AT LOW VOLTAGE, 
   // AND APPLY THAT VOLTAGE TO THE A-REF PIN
   //analogReference(EXTERNAL);   
}

void loop(){
  // もし心拍を検出していれば
  if (QS1 == true){

//    Serial.print("一人目：");
//    Serial.println(BPM1);
    
    // QSをリセットしておく
    QS1 = false;
   }
   if (QS2 == true){
//    Serial.print("二人目：");
//    Serial.println(BPM2);
    
    // QSをリセットしておく
    QS2 = false;
   }
    Serial.print(valsum/4);
    Serial.print(",");
    Serial.print(xinput);
    Serial.print(",");
    Serial.print(yinput);
    Serial.print(",");
    Serial.print(BPM1);
    Serial.print(",");
    Serial.println(BPM2);
 
//  ledFadeToBeat();
  
  delay(20);
}

// フェードピンの設定

//Processingにシリアルデータを送る
void sendDataToProcessing(char symbol, int data ){
    Serial.print(symbol);                // symbol prefix tells Processing what type of data is coming
    Serial.println(data);                // the data to send culminating in a carriage return
}

void mS_interupt(){

    valsum = analogRead(0);
    xinput = analogRead(XIN);
    yinput = analogRead(YIN);
    cli();
    // disable interrupts while we do this
    // 心拍センサーの値を読み取る
    Signal1 = analogRead(pulsePin1);              // read the Pulse Sensor 
    Serial.println(analogRead(0));                // the data to send culminating in a carriage return
    sampleCounter1 += 2;     
  //  Serial.println(pulsePin1);// keep track of the time in mS with this variable
    int N1 = sampleCounter1 - lastBeatTime1;       // monitor the time since the last beat to avoid noise

    // 心拍センサーの値を読み取る
    Signal2 = analogRead(pulsePin2);              // read the Pulse Sensor 
    sampleCounter2 += 2;                         // keep track of the time in mS with this variable
    int N2 = sampleCounter2 - lastBeatTime2;       // monitor the time since the last beat to avoid noise

//  find the peak and trough of the pulse wave
// ピークを見つける
    if(Signal1 < thresh1 && N1 > (IBI1/5)*3){       // avoid dichrotic noise by waiting 3/5 of last IBI
        if (Signal1 < T1){                        // T is the trough
            T1 = Signal1;                         // keep track of lowest point in pulse wave 
         }
       }
      
    if(Signal1 > thresh1 && Signal1 > P1){          // thresh condition helps avoid noise
        P1 = Signal1;                             // P is the peak
       }                                        // keep track of highest point in pulse wave

    if(Signal2 < thresh2 && N2 > (IBI2/5)*3){       // avoid dichrotic noise by waiting 3/5 of last IBI
        if (Signal2 < T2){                        // T is the trough
            T2 = Signal2;                         // keep track of lowest point in pulse wave 
         }
       }
      
    if(Signal2 > thresh2 && Signal2 > P2){          // thresh condition helps avoid noise
        P2 = Signal2;                             // P is the peak
       }  

       
  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
if (N1 > 250){                                   // avoid high frequency noise
  if ( (Signal1 > thresh1) && (Pulse1 == false) && (N1 > (IBI1/5)*3) ){        
    Pulse1 = true;                               // set the Pulse flag when we think there is a pulse
//    digitalWrite(blinkPin1, HIGH);                // turn on pin 13 LED
    IBI1 = sampleCounter1 - lastBeatTime1;         // measure time between beats in mS
    lastBeatTime1 = sampleCounter1;               // keep track of time for next pulse
         
         if(firstBeat1){                         // if it's the first time we found a beat, if firstBeat == TRUE
             firstBeat1 = false;                 // clear firstBeat flag
             return;                            // IBI value is unreliable so discard it
            }   
         if(secondBeat1){                        // if this is the second beat, if secondBeat == TRUE
            secondBeat1 = false;                 // clear secondBeat flag
               for(int i=0; i<=9; i++){         // seed the running total to get a realisitic BPM at startup
                    rate1[i] = IBI1;                      
                    }
            }
          
    // keep a running total of the last 10 IBI values
    word runningTotal1 = 0;                   // clear the runningTotal variable    

    for(int i=0; i<=8; i++){                // shift data in the rate array
          rate1[i] = rate1[i+1];              // and drop the oldest IBI value 
          runningTotal1 += rate1[i];          // add up the 9 oldest IBI values
        }
        
    rate1[9] = IBI1;                          // add the latest IBI to the rate array
    runningTotal1 += rate1[9];                // add the latest IBI to runningTotal
    runningTotal1 /= 10;                     // average the last 10 IBI values 
    BPM1 = 60000/runningTotal1;               // how many beats can fit into a minute? that's BPM!
    QS1 = true;                              // set Quantified Self flag 
    // QS FLAG IS NOT CLEARED INSIDE THIS ISR
    }                       
}

  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
if (N2 > 250){                                   // avoid high frequency noise
  if ( (Signal2 > thresh2) && (Pulse2 == false) && (N2 > (IBI2/5)*3) ){        
    Pulse2 = true;                               // set the Pulse flag when we think there is a pulse
//    digitalWrite(blinkPin2, HIGH);                // turn on pin 13 LED
    IBI2 = sampleCounter2 - lastBeatTime2;         // measure time between beats in mS
    lastBeatTime2 = sampleCounter2;               // keep track of time for next pulse
         
         if(firstBeat2){                         // if it's the first time we found a beat, if firstBeat == TRUE
             firstBeat2 = false;                 // clear firstBeat flag
             return;                            // IBI value is unreliable so discard it
            }   
         if(secondBeat2){                        // if this is the second beat, if secondBeat == TRUE
            secondBeat2 = false;                 // clear secondBeat flag
               for(int i=0; i<=9; i++){         // seed the running total to get a realisitic BPM at startup
                    rate2[i] = IBI2;                      
                    }
            }
          
    // keep a running total of the last 10 IBI values
    word runningTotal2 = 0;                   // clear the runningTotal variable    

    for(int i=0; i<=8; i++){                // shift data in the rate array
          rate2[i] = rate2[i+1];              // and drop the oldest IBI value 
          runningTotal2 += rate2[i];          // add up the 9 oldest IBI values
        }
        
    rate2[9] = IBI2;                          // add the latest IBI to the rate array
    runningTotal2 += rate2[9];                // add the latest IBI to runningTotal
    runningTotal2 /= 10;                     // average the last 10 IBI values 
    BPM2 = 60000/runningTotal2;               // how many beats can fit into a minute? that's BPM!
    QS2 = true;                              // set Quantified Self flag 
    // QS FLAG IS NOT CLEARED INSIDE THIS ISR
    }                       
}



  if (Signal1 < thresh1 && Pulse1 == true){     // when the values are going down, the beat is over
//      digitalWrite(blinkPin1, LOW);            // turn off pin 13 LED
      Pulse1 = false;                         // reset the Pulse flag so we can do it again
      amp1 = P1 - T1;                           // get amplitude of the pulse wave
      thresh1 = amp1/2 + T1;                    // set thresh at 50% of the amplitude
      P1 = thresh1;                            // reset these for next time
      T1 = thresh1;
     }
  
  if (N1 > 2500){                             // if 2.5 seconds go by without a beat
      thresh1 = 512;                          // set thresh default
      P1 = 512;                               // set P default
      T1 = 512;                               // set T default
      lastBeatTime1 = sampleCounter1;          // bring the lastBeatTime up to date        
      firstBeat1 = true;                      // set these to avoid noise
      secondBeat1 = true;                     // when we get the heartbeat back
     }

       if (Signal2 < thresh2 && Pulse2 == true){     // when the values are going down, the beat is over
//      digitalWrite(blinkPin2, LOW);            // turn off pin 13 LED
      Pulse2 = false;                         // reset the Pulse flag so we can do it again
      amp2 = P2 - T2;                           // get amplitude of the pulse wave
      thresh2 = amp2/2 + T2;                    // set thresh at 50% of the amplitude
      P2 = thresh2;                            // reset these for next time
      T2 = thresh2;
     }
  
  if (N2 > 2500){                             // if 2.5 seconds go by without a beat
      thresh2 = 512;                          // set thresh default
      P2 = 512;                               // set P default
      T2 = 512;                               // set T default
      lastBeatTime2 = sampleCounter2;          // bring the lastBeatTime up to date        
      firstBeat2 = true;                      // set these to avoid noise
      secondBeat2 = true;                     // when we get the heartbeat back
     }
  
  sei();                                     // enable interrupts when youre done!
}// end isr

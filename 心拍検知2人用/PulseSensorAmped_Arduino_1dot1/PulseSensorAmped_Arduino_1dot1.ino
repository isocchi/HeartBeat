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

******/

// ユーザ1の変数設定
int pulsePin1 = 0;                 // 心拍センサの紫ケーブルをA0に接続
int blinkPin1 = 13;                // パルスごとに13ピンのLEDを点灯
int fadePin1 = 5;                  // pin to do fancy classy fading blink at each beat
int fadeRate1 = 0;                 // used to fade LED on with PWM on fadePin
// ユーザ2の変数設定
int pulsePin2 = 1;                 // 心拍センサの紫ケーブルをA1に接続
int blinkPin2 = 12;                // パルスごとに13ピンのLEDを点灯
int fadePin2 = 5;                  // pin to do fancy classy fading blink at each beat
int fadeRate2 = 0;                 // used to fade LED on with PWM on fadePin

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

void setup(){
  pinMode(blinkPin1, OUTPUT);        // LED点灯用13ピン
  pinMode(fadePin1, OUTPUT);          // pin that will fade to your heartbeat!
  pinMode(blinkPin2, OUTPUT);        // LED点灯用12ピン
  pinMode(fadePin2, OUTPUT);          // pin that will fade to your heartbeat!
  Serial.begin(115200);             // シリアル通信開始
  interruptSetup();                 // 2ミリ秒間隔でパルスを読み取るセットアップ
   // UN-COMMENT THE NEXT LINE IF YOU ARE POWERING The Pulse Sensor AT LOW VOLTAGE, 
   // AND APPLY THAT VOLTAGE TO THE A-REF PIN
   //analogReference(EXTERNAL);   
}

void loop(){
  // ProcessingにセンサデータSignalをラベルSで送る
  //sendDataToProcessing('S', Signal);
  // もし心拍を検出していれば
  if (QS1 == true){
    // Set 'fadeRate' Variable to 255 to fade LED with pulse
    fadeRate1 = 255;
    // Processingに心拍データBPMをラベルBで送る
    //sendDataToProcessing('B',BPM);

    // Processingに心拍間隔IBIをラベルQで送る
    //sendDataToProcessing('Q',IBI);

    Serial.print("一人目：");
    Serial.println(BPM1);
    
    // QSをリセットしておく
    QS1 = false;
 }
   if (QS2 == true){
    // Set 'fadeRate' Variable to 255 to fade LED with pulse
    fadeRate2 = 255;
    // Processingに心拍データBPMをラベルBで送る
    //sendDataToProcessing('B',BPM);

    // Processingに心拍間隔IBIをラベルQで送る
    //sendDataToProcessing('Q',IBI);

    Serial.print("二人目：");
    Serial.println(BPM2);
    
    // QSをリセットしておく
    QS2 = false;
 }
  
  ledFadeToBeat();
  
  delay(20);
}

// フェードピンの設定
void ledFadeToBeat(){
    fadeRate1 -= 15;                         //  set LED fade value
    fadeRate2 -= 15;                         //  set LED fade value
    fadeRate1 = constrain(fadeRate1,0,255);   //  keep LED fade value from going into negative numbers!
    fadeRate2 = constrain(fadeRate2,0,255);   //  keep LED fade value from going into negative numbers!
    analogWrite(fadePin1,fadeRate1);          //  fade LED
    analogWrite(fadePin2,fadeRate2);          //  fade LED
}

//Processingにシリアルデータを送る
void sendDataToProcessing(char symbol, int data ){
    Serial.print(symbol);                // symbol prefix tells Processing what type of data is coming
    Serial.println(data);                // the data to send culminating in a carriage return
}

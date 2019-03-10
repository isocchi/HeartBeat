


/*volatile int rate1[10];                    // used to hold last ten IBI values
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
*/

void interruptSetup(){     
  // Initializes Timer2 to throw an interrupt every 2mS.
  // よくわからん初期設定
//  TCCR2A = 0x02;     // DISABLE PWM ON DIGITAL PINS 3 AND 11, AND GO INTO CTC MODE
//  TCCR2B = 0x06;     // DON'T FORCE COMPARE, 256 PRESCALER 
//  OCR2A = 0X7C;      // SET THE TOP OF THE COUNT TO 124 FOR 500Hz SAMPLE RATE
//  TIMSK2 = 0x02;     // ENABLE INTERRUPT ON MATCH BETWEEN TIMER2 AND OCR2A
//  sei();             // MAKE SURE GLOBAL INTERRUPTS ARE ENABLED      
} 

/*
// THIS IS THE TIMER 2 INTERRUPT SERVICE ROUTINE. 
// Timer 2 makes sure that we take a reading every 2 miliseconds
ISR(TIMER2_COMPA_vect){                         // triggered when Timer2 counts to 124
    cli();
    // disable interrupts while we do this
    // 心拍センサーの値を読み取る
    Signal1 = analogRead(pulsePin1);              // read the Pulse Sensor 
    sampleCounter1 += 2;                         // keep track of the time in mS with this variable
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
    digitalWrite(blinkPin1, HIGH);                // turn on pin 13 LED
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
    digitalWrite(blinkPin2, HIGH);                // turn on pin 13 LED
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
      digitalWrite(blinkPin2, LOW);            // turn off pin 13 LED
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
*/

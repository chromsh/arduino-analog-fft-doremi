#include <TimerOne.h>

#define LIN_OUT 1 // wether using the linear output function or not
#define FFT_N 256  // number of samples

#define SAMPLE_MICRO_SEC 250

// led pins
#define PIN_DO 2
#define PIN_RE 3
#define PIN_MI 4
#define PIN_FA 5
#define PIN_SO 6
#define PIN_RA 7
#define PIN_SI 8
#define PIN_DO_HIGH 9
#define PIN_RE_HIGH 10
#define PIN_MI_HIGH 11
#define PIN_FA_HIGH 12
#define PIN_SO_HIGH 13
// Analog 4
#define PIN_RA_HIGH 18 
// Analog 5
#define PIN_SI_HIGH 19




#include <FFT.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Timer1.initialize(SAMPLE_MICRO_SEC); // microseconds
  Timer1.attachInterrupt(onTimer);
  Timer1.start();

  // LED Pins
  pinMode (PIN_DO, OUTPUT);
  pinMode (PIN_RE, OUTPUT);
  pinMode (PIN_MI, OUTPUT);
  pinMode (PIN_FA, OUTPUT);
  pinMode (PIN_SO, OUTPUT);
  pinMode (PIN_RA, OUTPUT);
  pinMode (PIN_SI, OUTPUT);
  pinMode (PIN_DO_HIGH, OUTPUT);
  pinMode (PIN_RE_HIGH, OUTPUT);
  pinMode (PIN_MI_HIGH, OUTPUT);
  pinMode (PIN_FA_HIGH, OUTPUT);
  pinMode (PIN_SO_HIGH, OUTPUT);
  pinMode (PIN_RA_HIGH, OUTPUT);
  pinMode (PIN_SI_HIGH, OUTPUT);
}

int analogPin = 0;
volatile int fft_idx = 0;

void onTimer() {
  int val = analogRead(analogPin);
  fft_input[fft_idx]      = val;
  fft_input[fft_idx + 1]  = 0;
  fft_idx += 2;
}

// ドレミで光らせる
void flush_led (uint16_t *fft) {
  // 大体、下記のような感じ
  // (FFT_N = 128, Timer = 500microsecでのFFTの場合)
/* ド：16, 17, 18
レ：18, 19, (20)
ミ：20, 21, 22
ファ：21, 22, 23
ソ：24, 25, 26
ラ：27, 28, 29
シ：31, 32, 33
ド：33, 34
*/
  // 各配列の全てがthresholdを超えていたらそれと判断する
  static const char onkai_freq[][3] = {
    {16, 17, -1}, // ド
    {18, 19, -1}, // レ
    {20, 21, 22}, // ミ
    {-1, 22, 23}, // ファ
    {-1, 25, 26}, // ソ
    {-1, 28, 29}, // ラ
    {31, 32, -1}, // シ
    {33, 34, -1}, // ド HIGH
    {37, 38, -1}, // レ HIGH
    {-1, 42, 43}, // ミ HIGH
    {44, 45, -1}, // ファ HIGH
    {-1, 50, 51}, // ソ HIGH
    {-1, 56, -1}, // ラ HIGH 
    {63, 64, -1}, // シ HIGH
  };
  static const char pins[] = {
    PIN_DO,       PIN_RE,       PIN_MI,       PIN_FA,       PIN_SO,       PIN_RA,       PIN_SI,
    PIN_DO_HIGH,  PIN_RE_HIGH,  PIN_MI_HIGH,  PIN_FA_HIGH,  PIN_SO_HIGH,  PIN_RA_HIGH,  PIN_SI_HIGH,
  };
  int threshold = 7;
  int i;
  for (i = 0 ; i < sizeof(onkai_freq) / sizeof(onkai_freq[0]) ; i++) {
    int j;
    int output  = HIGH;
    for (j = 0 ; j < sizeof(onkai_freq[i]) / sizeof(onkai_freq[i][0]) ; j++) {
      if (onkai_freq[i][j] == -1)  continue;
      if (fft[onkai_freq[i][j]] < threshold) {
        // 超えていない = NG
        output  = LOW;
        break;
      }
    }
    digitalWrite(pins[i], output);
  }
}

volatile int bunshu = 2;
void loop() {
  // put your main code here, to run repeatedly:
  
  if (fft_idx < FFT_N * 2)  return;
  if (bunshu > 0) {
    bunshu--;
    fft_idx = 0;
    return;
  } else {
    // 1秒間に表示する回数を抑える
    bunshu = (float)1000 * 1000 / (float)(SAMPLE_MICRO_SEC * FFT_N) / 5 - 1;
  }

  noInterrupts();

  // データが揃った場合
  fft_window();
  fft_reorder();
  fft_run();
  fft_mag_lin();

//*
  for (int i = 0 ; i < FFT_N / 2 ; i++) {    
    if (fft_lin_out[i] < 10)  continue;
    Serial.print("[idx: ");
    Serial.print(i);
    Serial.print(", val = ");
    Serial.print(fft_lin_out[i]);
    Serial.print("]  ");
  }
  Serial.println();
//*/
  flush_led(fft_lin_out);
  fft_idx = 0;
  interrupts();
}

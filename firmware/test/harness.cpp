// Arduino stubs so releaf_br01_v2.ino can be compiled and run on the desktop.
// Drives four scenarios and prints the lines the real board would emit.
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <string>

#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define FALLING 3
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19

static unsigned long g_ms = 0;
unsigned long millis() { return g_ms; }
void delay(unsigned long d) { g_ms += d; }

// analog channels the scenario sets
static int g_analog[24];
static int g_digital[24];
// Photometer model: detector sees ambient in the dark, and ambient + a beam
// attenuated by the culture when the source LED is on. Beer-Lambert with a
// blank of 900 counts, so OD 0.30 should come back as ~0.30.
static double g_trueOD = 0.30;
int analogRead(int pin) {
  if (pin == A0) {
    double ambient = 20.0;
    if (!g_digital[6]) return (int)ambient;               // LED off: dark read
    return (int)(ambient + 900.0 * pow(10.0, -g_trueOD));  // LED on
  }
  return g_analog[pin];
}
void digitalWrite(int pin, int v) { g_digital[pin] = v; }
void pinMode(int, int) {}
int digitalPinToInterrupt(int p) { return p; }
static void (*g_isr)() = nullptr;
void attachInterrupt(int, void (*f)(), int) { g_isr = f; }
void noInterrupts() {}
void interrupts() {}

struct SerialT {
  std::string line;
  void begin(long) {}
  int available() { return 0; }
  int read() { return -1; }
  void flushline() { printf("%s\n", line.c_str()); line.clear(); }
  void print(const char *s) { line += s; }
  void print(float v, int d) { char b[64]; snprintf(b, sizeof b, "%.*f", d, v); line += b; }
  void print(int v) { char b[32]; snprintf(b, sizeof b, "%d", v); line += b; }
  void print(long v) { char b[32]; snprintf(b, sizeof b, "%ld", v); line += b; }
  void print(unsigned long v) { char b[32]; snprintf(b, sizeof b, "%lu", v); line += b; }
  void println(const char *s) { line += s; flushline(); }
  void println(float v, int d) { print(v, d); flushline(); }
  void println(unsigned long v) { print(v); flushline(); }
  void println() { flushline(); }
} Serial;

template <class T> T constrain(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); }

#include "releaf_br01_v2.ino"

// feed a line to the sketch exactly as the serial reader would
void feed(const char *s) { char buf[128]; strncpy(buf, s, sizeof buf - 1); buf[sizeof buf - 1] = 0; handleLine(buf); }
void scan() { readSensors(); applyOutputs(); sendPV(); sendST(); }

// psi -> raw counts, inverse of PT_PSI_PER_COUNT
int psi(float v) { return (int)(v / (30.0f / 1023.0f)); }

int main() {
  setup();

  printf("# 1 normal running, valve open, light green (true OD 0.30)\n");
  g_analog[A5] = 350;
  g_analog[A1] = psi(3.1f); g_analog[A2] = psi(1.8f); g_analog[A3] = psi(0.35f); g_analog[A4] = psi(0.45f);
  g_ms += 1000; feed("SET XV01=OPEN LIGHT=GREEN");
  for (int i = 0; i < 12; i++) if (g_isr) g_isr();   // a few flow pulses
  g_ms += 1000; scan();

  printf("# 2 valve commanded shut, pressure climbs past the ceiling -> IL-1 opens it\n");
  g_ms += 1000; feed("SET XV01=CLOSED LIGHT=GREEN");
  g_ms += 1000; scan();
  g_analog[A1] = psi(9.0f); g_analog[A2] = psi(8.6f);   // TMP now ~0.57 bar
  g_ms += 1000; feed("SET XV01=CLOSED LIGHT=GREEN");
  g_ms += 1000; scan();
  printf("#   pressure falls back below the reset value\n");
  g_analog[A1] = psi(3.1f); g_analog[A2] = psi(1.8f);
  g_ms += 1000; feed("SET XV01=CLOSED LIGHT=GREEN");
  g_ms += 1000; scan();

  printf("# 3 page goes quiet for 6 s -> IL-2 safe state\n");
  g_ms += 6000; scan();

  printf("# 4 pH probe unplugged (input floats) -> channel omitted, not faked\n");
  g_ms += 1000; feed("SET XV01=OPEN LIGHT=OFF");
  g_analog[A5] = 2;
  g_ms += 1000; scan();

  printf("# 5 calibration pushed down from the page (blank 900 + 20 ambient)\n");
  g_ms += 1000; feed("CAL ODBLANK=900.0 PHSLOPE=-5.700 PHOFF=21.340");
  g_analog[A5] = 350;
  g_ms += 1000; feed("SET XV01=OPEN LIGHT=OFF");
  g_ms += 1000; scan();

  printf("# 5b culture grows to true OD 1.20\n");
  g_trueOD = 1.20;
  g_ms += 1000; feed("SET XV01=OPEN LIGHT=OFF");
  g_ms += 1000; scan();

  printf("# 6 an unknown key from a newer page must not break the line\n");
  g_ms += 1000; feed("SET XV01=OPEN LIGHT=GREEN PUMP01=16.0 FUTURE=xyz");
  g_ms += 1000; scan();
  return 0;
}

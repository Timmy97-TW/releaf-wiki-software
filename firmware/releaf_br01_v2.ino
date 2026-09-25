/*
  ReLeaf BR-01 — MCU firmware, protocol v2 (2026-09-24)
  Target: Arduino Uno Q, microcontroller side. The Linux side serves the
  operator page (Digital Twin/01_UI/0924UI.html) and talks to this sketch over
  USB serial.

  WHAT CHANGED FROM v1 (firmware/releaf_br01.ino, 2026-09-06)
    · The rig no longer has three lumen valves and three software-driven pumps.
      As of the 21 October scope there is ONE automated actuator pair:
        XV-01  lumen pinch valve   (OPEN / CLOSED)
        L-01   green light         (GREEN / OFF)
      The loop pump and the shell pump are set BY HAND at the pump head. This
      sketch does not drive them and must not pretend to.
    · Three channels became real measurements instead of model output:
        OD600 inline photometer, pH probe, FT-01 flow meter.
      The page therefore no longer computes flow from a pump curve.
    · Message format changed on both directions. See PROTOCOL below.

  PROTOCOL  115200 baud, newline terminated, ASCII, key=value pairs.

    IN   SET XV01=OPEN LIGHT=GREEN
         Sent by the page about every 2 s as a heartbeat, and immediately on
         any operator or twin action. Unknown keys are ignored, so the page can
         grow new fields without reflashing.

    IN   CAL ODBLANK=903.0 PHSLOPE=-5.700 PHOFF=21.340
         Pushed by the page on connect and after each calibration. Not stored
         in EEPROM on purpose: the page is the single source of truth and
         re-sends on every connect, so a reflashed board cannot run stale.

    OUT  PV OD=0.412 ODC=512 PH=7.02 FLOW=263 PT1=2.69 PT3=1.31 PT4=0.45 PT5=0.35
         One line per second. A channel whose sensor is absent or failing is
         omitted from the line rather than sent as a wrong number: the page
         treats a missing key as "no reading" and says so on screen.
         ODC is the dark-corrected detector count behind OD. The page captures
         it when the operator presses "Zero OD" and sends it back as the blank,
         so the conversion lives in one place (here) and the calibration lives
         in one place (the page's stored settings).

    OUT  ST XV01=OPEN LIGHT=GREEN LOCK=NONE UP=1234
         One line per second, after PV. Reports what the board is ACTUALLY
         doing, which is not always what was commanded: LOCK names the
         interlock in force (NONE / TMP / LINK). This is how the page can show
         an interlock without guessing.

    OUT  EV <text>
         A one-off event line (interlock fired, sensor lost, boot). Free text
         after the tag; the page logs it verbatim in the action log.

  SAFETY. The interlocks below run on this chip, every loop, and they override
  the commanded state. A browser crash, a closed laptop lid or a pulled USB
  cable cannot leave the rig in a dangerous state: after LINK_TIMEOUT_MS with
  no SET line, the board goes to its own safe state (valve OPEN, light OFF).
  Opening the pinch valve is the safe direction — a shut valve on a running
  pump is what damages the membrane.

  WIRING. Pin map matches Setup → I/O in the operator page. Edit both together.
  Sensor front-ends are isolated in the read*() functions at the bottom; if a
  probe changes, only its own function changes.
*/

// ---------------- pins ----------------
const uint8_t PIN_XV01   = 7;    // pinch valve relay/driver: HIGH = energised
const uint8_t PIN_GREEN  = 11;   // green LED ring around the reservoir (CcaS ON)
const uint8_t PIN_OD_LED = 6;    // photometer source LED
const uint8_t PIN_OD_DET = A0;   // photometer detector
const uint8_t PIN_PH     = A5;   // pH front-end analog out
const uint8_t PIN_FLOW   = 2;    // flow meter, interrupt-capable
const uint8_t PIN_PT1    = A1;   // lumen inlet
const uint8_t PIN_PT3    = A2;   // lumen outlet
const uint8_t PIN_PT4    = A3;   // shell outlet (top)
const uint8_t PIN_PT5    = A4;   // shell inlet (bottom, after the shell pump)

// XV-01 is normally-open: de-energised = OPEN = safe. If your valve is
// normally-closed, set this to false and re-test the power-loss case by hand.
const bool XV01_ENERGISE_TO_SHUT = true;

// ---------------- limits (keep in step with the page's safety table) --------
const float    TMP_CEILING_BAR  = 0.50f;   // IL-1: above this the valve opens
const float    TMP_RESET_BAR    = 0.42f;   // hysteresis, so it cannot chatter
const uint32_t LINK_TIMEOUT_MS  = 5000;    // IL-2: no SET for this long
const uint32_t SCAN_MS          = 1000;    // sensor scan and PV/ST period

// Sensor plausibility. A reading outside these is treated as a dead channel
// and omitted from PV, rather than published as a number someone might act on.
const float OD_MIN = -0.05f, OD_MAX = 4.0f;
const float PH_MIN =  2.0f,  PH_MAX = 12.0f;
const float PT_MIN = -1.0f,  PT_MAX = 29.0f;   // transducer span 0-30 psi
const float FLOW_MAX = 2000.0f;                // mL/min

// ---------------- calibration ----------------
// OD600 = OD_A * log10(I0 / I) + OD_B, I = detector counts, I0 = blank counts.
// I0 is captured by the operator with "Zero OD" on the page; it is stored
// there, not here, so a reflash cannot silently lose it. Until the page sends
// a blank the board reports raw absorbance against OD_I0_DEFAULT.
float OD_A = 1.00f, OD_B = 0.0f;
float odBlankCounts = 900.0f;          // OD_I0_DEFAULT: clean medium, set in lab

// pH = PH_SLOPE * volts + PH_OFFSET, from the 2-point buffer calibration.
float PH_SLOPE = -5.70f, PH_OFFSET = 21.34f;

// Flow meter: pulses per litre, from the datasheet, confirmed by bucket test.
const float FLOW_PULSES_PER_L = 4380.0f;

// Pressure transducers: ratiometric 0.5-4.5 V over 0-30 psi.
const float PT_PSI_PER_COUNT = 30.0f / 1023.0f;

// ---------------- commanded state ----------------
bool  cmdValveOpen = true;            // XV-01, safe default
bool  cmdLightGreen = false;          // L-01, safe default
uint32_t lastSetMs = 0;

// ---------------- measured state ----------------
// Each channel carries a validity flag. "I do not know" is a first-class
// value here: the page can only be honest if the board is.
float od = 0;    bool odOk = false;
float ph = 0;    bool phOk = false;
float flow = 0;  bool flowOk = false;
float pt1 = 0, pt3 = 0, pt4 = 0, pt5 = 0;
bool  pt1Ok = false, pt3Ok = false, pt4Ok = false, pt5Ok = false;

// ---------------- interlock state ----------------
enum Lock { LOCK_NONE, LOCK_TMP, LOCK_LINK };
Lock activeLock = LOCK_LINK;          // until the first SET arrives
bool tmpLatched = false;              // true while IL-1 holds the valve open

// Forward declarations. The Arduino IDE would generate these for us, but
// writing them out keeps the file compilable by a plain C++ compiler, which is
// what the desktop test harness uses.
void applyOutputs();
void readSensors();
void handleLine(char *line);
void sendPV();
void sendST();
bool interlockedValveOpen();
bool interlockedLightGreen();

volatile uint32_t flowPulses = 0;
uint32_t lastFlowMs = 0;

void onFlowPulse() { flowPulses++; }

void setup() {
  Serial.begin(115200);
  pinMode(PIN_XV01, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_OD_LED, OUTPUT);
  pinMode(PIN_FLOW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_FLOW), onFlowPulse, FALLING);
  applyOutputs();                     // safe state before anything else
  lastFlowMs = millis();
  Serial.println("EV boot BR-01 firmware v2 protocol=2");
}

// ---------------- trans-membrane pressure ----------------
// Mean lumen minus mean shell. Needs all four transducers: with any one dead
// the number would be wrong in an unknown direction, so IL-1 falls back to
// the safe side (treats it as over-pressure) rather than to a guess.
bool tmpKnown() { return pt1Ok && pt3Ok && pt4Ok && pt5Ok; }
float tmpBar() {
  const float PSI_TO_BAR = 0.0689476f;
  return (((pt1 + pt3) - (pt4 + pt5)) * 0.5f) * PSI_TO_BAR;
}

// ---------------- interlocks ----------------
// Returns the valve position the board will actually drive.
bool interlockedValveOpen() {
  if (millis() - lastSetMs > LINK_TIMEOUT_MS) {   // IL-2: page went quiet
    if (activeLock != LOCK_LINK) { activeLock = LOCK_LINK; Serial.println("EV IL-2 link lost, safe state"); }
    return true;
  }
  // IL-1: a shut valve dead-ends the lumen. If pressure across the membrane
  // climbs past the ceiling, open regardless of what was commanded, and stay
  // open until it falls back below the reset value.
  if (!cmdValveOpen) {
    bool over = !tmpKnown() || tmpBar() > TMP_CEILING_BAR;
    if (over && !tmpLatched) {
      tmpLatched = true;
      Serial.print("EV IL-1 TMP ");
      if (tmpKnown()) Serial.print(tmpBar(), 3); else Serial.print("unknown");
      Serial.println(" bar, valve forced open");
    }
    if (tmpLatched && tmpKnown() && tmpBar() < TMP_RESET_BAR) {
      tmpLatched = false;
      Serial.println("EV IL-1 cleared, valve returns to commanded");
    }
    if (tmpLatched) { activeLock = LOCK_TMP; return true; }
  } else {
    tmpLatched = false;
  }
  activeLock = LOCK_NONE;
  return cmdValveOpen;
}

// The light has no interlock of its own, but it follows the link: with the
// page gone, nothing should be driving production.
bool interlockedLightGreen() {
  if (millis() - lastSetMs > LINK_TIMEOUT_MS) return false;
  return cmdLightGreen;
}

void applyOutputs() {
  bool open = interlockedValveOpen();
  bool shut = !open;
  digitalWrite(PIN_XV01, XV01_ENERGISE_TO_SHUT ? shut : open);
  digitalWrite(PIN_GREEN, interlockedLightGreen());
}

// ---------------- sensor reads ----------------
// Each returns true if the reading is trustworthy. Keep the hardware detail
// inside these functions: everything above works in engineering units.

bool readPressure(uint8_t pin, float &out) {
  int raw = analogRead(pin);
  // A floating input sits near 0 or full scale; a wired transducer never does.
  if (raw < 5 || raw > 1018) return false;
  float psi = raw * PT_PSI_PER_COUNT;
  if (psi < PT_MIN || psi > PT_MAX) return false;
  out = psi;
  return true;
}

// Inline photometer. Dark reading first, then lit, so ambient light and the
// detector's own offset subtract out. Median of 5 rejects a bubble passing
// the window during the measurement; the page also takes a median over 6 min.
float lastODCounts = 0;      // dark-corrected detector counts behind `od`
bool odCountsOk = false;

bool readOD(float &out) {
  int lit[5];
  digitalWrite(PIN_OD_LED, LOW);
  delay(5);
  long dark = 0;
  for (int i = 0; i < 5; i++) { dark += analogRead(PIN_OD_DET); delay(2); }
  dark /= 5;
  digitalWrite(PIN_OD_LED, HIGH);
  delay(20);                                  // let the LED settle
  for (int i = 0; i < 5; i++) { lit[i] = analogRead(PIN_OD_DET); delay(2); }
  digitalWrite(PIN_OD_LED, LOW);
  for (int i = 1; i < 5; i++)                 // insertion sort, n = 5
    for (int j = i; j > 0 && lit[j - 1] > lit[j]; j--) { int t = lit[j]; lit[j] = lit[j - 1]; lit[j - 1] = t; }
  float I = lit[2] - dark;
  lastODCounts = I;
  odCountsOk = (I >= 2);
  if (I < 2) return false;                    // detector saturated or unlit
  float blank = odBlankCounts - dark;
  if (blank < 2) return false;
  float val = OD_A * log10(blank / I) + OD_B;
  if (val < OD_MIN || val > OD_MAX) return false;
  out = val;
  return true;
}

bool readPH(float &out) {
  int raw = analogRead(PIN_PH);
  if (raw < 5 || raw > 1018) return false;
  float volts = raw * (5.0f / 1023.0f);
  float val = PH_SLOPE * volts + PH_OFFSET;
  if (val < PH_MIN || val > PH_MAX) return false;
  out = val;
  return true;
}

// Flow meter: pulses counted by the interrupt, divided by the interval.
// Zero pulses is a legitimate zero (pump off), not a fault, so this stays
// valid at 0 — the page decides whether zero flow is a problem.
bool readFlow(float &out) {
  uint32_t now = millis();
  uint32_t dt = now - lastFlowMs;
  if (dt < 500) return flowOk;                // too short to be meaningful
  noInterrupts();
  uint32_t pulses = flowPulses;
  flowPulses = 0;
  interrupts();
  lastFlowMs = now;
  float mlPerMin = (pulses / FLOW_PULSES_PER_L) * 1000.0f * (60000.0f / dt);
  if (mlPerMin > FLOW_MAX) return false;
  out = mlPerMin;
  return true;
}

void readSensors() {
  odOk   = readOD(od);
  phOk   = readPH(ph);
  flowOk = readFlow(flow);
  pt1Ok  = readPressure(PIN_PT1, pt1);
  pt3Ok  = readPressure(PIN_PT3, pt3);
  pt4Ok  = readPressure(PIN_PT4, pt4);
  pt5Ok  = readPressure(PIN_PT5, pt5);
}

// ---------------- incoming lines ----------------
void handleLine(char *line) {
  if (!strncmp(line, "SET", 3)) {
    for (char *tok = strtok(line + 3, " "); tok; tok = strtok(NULL, " ")) {
      char *eq = strchr(tok, '='); if (!eq) continue;
      *eq = 0; char *key = tok, *val = eq + 1;
      if      (!strcmp(key, "XV01"))  cmdValveOpen  = !strcmp(val, "OPEN");
      else if (!strcmp(key, "LIGHT")) cmdLightGreen = !strcmp(val, "GREEN");
      // Unknown keys ignored on purpose: the page may send fields this
      // firmware predates. Never fail a whole line over one token.
    }
    if (activeLock == LOCK_LINK) Serial.println("EV link restored");
    lastSetMs = millis();
    applyOutputs();
    return;
  }
  // CAL lets the page push calibration down after the operator does it, so
  // the board's own numbers agree with the screen. Values are not persisted:
  // the page re-sends them on connect, which keeps one source of truth.
  if (!strncmp(line, "CAL", 3)) {
    for (char *tok = strtok(line + 3, " "); tok; tok = strtok(NULL, " ")) {
      char *eq = strchr(tok, '='); if (!eq) continue;
      *eq = 0; char *key = tok; float v = atof(eq + 1);
      if      (!strcmp(key, "ODBLANK")) odBlankCounts = v;
      else if (!strcmp(key, "ODA"))     OD_A = v;
      else if (!strcmp(key, "ODB"))     OD_B = v;
      else if (!strcmp(key, "PHSLOPE")) PH_SLOPE = v;
      else if (!strcmp(key, "PHOFF"))   PH_OFFSET = v;
    }
    Serial.println("EV calibration updated");
    return;
  }
}

// ---------------- outgoing lines ----------------
void sendPV() {
  Serial.print("PV");
  if (odOk)       { Serial.print(" OD=");  Serial.print(od, 4); }
  if (odCountsOk) { Serial.print(" ODC="); Serial.print(lastODCounts, 1); }
  if (phOk)   { Serial.print(" PH=");   Serial.print(ph, 2); }
  if (flowOk) { Serial.print(" FLOW="); Serial.print(flow, 1); }
  if (pt1Ok)  { Serial.print(" PT1=");  Serial.print(pt1, 2); }
  if (pt3Ok)  { Serial.print(" PT3=");  Serial.print(pt3, 2); }
  if (pt4Ok)  { Serial.print(" PT4=");  Serial.print(pt4, 2); }
  if (pt5Ok)  { Serial.print(" PT5=");  Serial.print(pt5, 2); }
  Serial.println();
}

void sendST() {
  Serial.print("ST XV01=");  Serial.print(interlockedValveOpen() ? "OPEN" : "CLOSED");
  Serial.print(" LIGHT=");   Serial.print(interlockedLightGreen() ? "GREEN" : "OFF");
  Serial.print(" LOCK=");
  Serial.print(activeLock == LOCK_TMP ? "TMP" : activeLock == LOCK_LINK ? "LINK" : "NONE");
  Serial.print(" UP=");      Serial.println(millis() / 1000);
}

void loop() {
  static char buf[128]; static uint8_t n = 0;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') { if (n) { buf[n] = 0; handleLine(buf); n = 0; } }
    else if (n < sizeof(buf) - 1) buf[n++] = c;
    else n = 0;                                 // overlong line: drop it
  }

  static uint32_t lastScan = 0;
  if (millis() - lastScan >= SCAN_MS) {
    lastScan = millis();
    readSensors();
    applyOutputs();          // re-assert every scan: interlocks may have changed
    sendPV();
    sendST();
  }
}

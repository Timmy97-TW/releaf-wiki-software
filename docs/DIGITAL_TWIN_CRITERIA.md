# When is this a digital twin?

**A ReLeaf audit against the biomanufacturing literature — 2026-09-06**

Written for someone new to biomanufacturing. It answers three questions in order:
what the field actually means by "digital twin", what bar ReLeaf has to clear to use
the phrase, and — line by line — where ReLeaf stands today.

Sources, all in `Adv Biochem Eng Biotechnol` vol. 176 (Springer, *Digital Twins for
Bioprocesses*), cited throughout by author and year:

| # | Chapter | Cited as |
|---|---|---|
| 01 | Gargalo, Caño de las Heras, Jones, Udugama, Mansouri, Krühne, Gernaey (2021), *Towards the Development of Digital Twins for the Bio-manufacturing Industry*, 176:1–34 | Gargalo 2021 |
| 02 | Portela, Varsakelis, Richelle, Giannelos, Pence, Dessoy, von Stosch (2021), *When Is an In Silico Representation a Digital Twin?*, 176:35–56 | Portela 2020 |
| 04 | Sinner, Daume, Herwig, Kager (2021), *Usage of Digital Twins Along a Typical Process Development Cycle*, 176:71–96 | Sinner 2021 |
| 06 | Moser, Appl, Brüning, Hass (2021), *Mechanistic Mathematical Models as a Basis for Digital Twins*, 176:133–180 | Moser 2021 |
| 07 | Zobel-Roos, Schmidt, Uhlenbrock, Ditz, Köster, Strube (2021), *Digital Twins in Biomanufacturing*, 176:181–262 | Zobel-Roos 2021 |

---

## 1. What the papers say a digital twin IS

### 1.1 The definition everyone else quotes

Portela 2020 §1 is the sentence to memorise:

> "A DT consists of the physical asset, the virtual (digital) asset, and the
> bidirectional connectivity between them that enables continuous communication,
> data/information exchange, and implementation of the DT-induced actions."

Three parts. Two of them are easy — you have a rig, and you have code. The third is
the one that fails audits.

### 1.2 The line that separates a twin from a shadow

Gargalo 2021 §1.1, following Fraunhofer Austria, draws the cleanest boundary in the
whole volume, and it is about **direction of dataflow**:

> "the dataflow of digital shadows is only one-way in contrast to DTs which have a
> two-way dataflow."

Expanded: in a digital shadow the flow from physical to digital is automated but the
flow from digital to physical is not. In a twin both are automated.

Sinner 2021 §1 makes the same cut with different words — what promotes a model to a
twin is "integration and synchronization with the real system", and a process model
with **manual** data flow is only a "digital object".

So: **a dashboard reading live sensors is a shadow. A remote control writing setpoints
with a simulation behind it is not a twin either. You need both directions.**

### 1.3 What the twin has to be able to DO

Zobel-Roos 2021 §1.1 quotes Altran's list of the four things that make a modern twin
different from a 1990s CAD model:

> - "The robustness of the models, with a focus on how they support specific business outcomes"
> - "The link to the real world, potentially in real time for monitoring and control"
> - "The application of advanced big data analytics and AI to drive new business opportunities"
> - **"The ability to interact with them and evaluate 'what if' scenarios"**

That last bullet is the one most student projects miss, and it is the cheapest of the
four to build once you already have a simulation engine.

Moser 2021 §6.6 gives the single sharpest operational test, and it is a test about the
*screen*:

> "Differently to a real plant, the Digital Twin is capable to show all modelled state
> variables to the user via the GUI. In real plants only the measured quantities can be
> shown."

Read that carefully, because it cuts both ways. A twin displays states no instrument
measures — that is the point of it. But it therefore has a duty the plant HMI does not
have: to say **which** of the numbers on the glass came from an instrument and which
came out of a model. A twin that hides that distinction is not being more capable than
a plant HMI, it is being less honest than one.

Moser 2021 §2.3 also warns where the ladder starts: an Operator Training Simulator is
"an early stage Digital Twin", and the difference is that OTS "are less frequently
updated and there is not necessarily a connection to the real system". A virtual
bioreactor "with no data connection to the physical bioreactor" is explicitly named as
early-stage. Before 2026-09-06 that description fitted ReLeaf exactly.

### 1.4 Lifecycle states

Zobel-Roos 2021 §1.1, again from Altran, gives three states a twin can be in at any time:

- **As designed** — "the intended structure and performance of the real world entity"
- **As built** — "the output of the manufacturing and assembly process"
- **As used and maintained** — "the status of the physical object in operation"

ReLeaf's P&ID mixes **as designed** (dashed orange: pH probe, thermometer, light panel,
stirrer, base loop) with **as built** (solid: module, two pumps, three valves, four
pressure transducers). It has no **as used and maintained** state at all — there is no
maintenance history, no run counter on the module, no record of how many hours the
fibres have seen. That is a real gap, and a cheap one to close.

### 1.5 The nine-step build list

Portela 2020 §2 is the checklist the interface now scores itself against:

1. Define the needs and functionalities through multi-disciplinary workshops
2. Develop PAT
3. Design, implement and validate the DT infrastructure
4. Develop, train and **validate** the mathematical models
5. Ensure real-time data processing and data integrity
6. Build the automation and model predictive control
7. Design the user interface
8. Document the efforts
9. Integrate the solution into a broader digital and data company strategy

And the sentence that tells you which step matters most:

> "it is the mathematical models that constitute the heart of the DT because its
> operational capacity is a strong function of the predictive capability of the
> employed models."

### 1.6 Model types, and which ones can predict

Zobel-Roos 2021 Table 1 ("Model types definition, required input and falsification from
process engineering point of view") sorts process models into eight classes and marks
them predictive or non-predictive. In the printed chapter that mark is a **colour**,
which is lost in text extraction; the surviving evidence is phrases inside the
"Required input" column. The rows, with those phrases verbatim:

| Class | Definition (quoted) | Predictive? |
|---|---|---|
| Rigorous | "Taking all relevant physical-chemical process effects of fluid dynamics, phase equilibrium and mass transfer into account by separating those effects from each other" | "Predictive in scale" |
| Short-cut | "Macroscopic equipment mass (and energy) balances overall parameter such as separation factor, capacity, etc." | "Non-predictive in scale and dimensions" |
| Cost modelling | "Use of short-cut models to add cost correlations" | "Non-predictive outside trainings range" |
| Hybrid | "Combining short-cut or rigorous models with statistical parts" | "Non-predictive due to statistical part" |
| Observer | "For process control statistically trained" | "Not fully predictive"; "Inline, at-line PAT needed" |
| Model-based process control | "Belong to advanced process control" | requires rigorous models |
| PCA / PLS regression | "Statistical model for analytical description" | falsification: "No model but analytics data regression equation" |
| Artificial intelligence | "Mostly neuronal networks" | **"Non-predictive outside trainings range"** |

Three consequences worth internalising:

- **A neural network is non-predictive outside its training range.** Not "less
  accurate" — non-predictive. It is in the table as a property of the class.
- **A hybrid model is only as predictive as its worst part.** Zobel-Roos 2021 §1.1: "The
  combination of physico-chemical models and integrated statistical parts reduces the
  predictability to the weakest part, i.e. in this case the statistics."
- **A PCA/PLS soft sensor is, in this chapter's terms, not a model at all** — its
  falsification cell reads "No model but analytics data regression equation."

Gargalo 2021 §3.2 says the same thing about surrogates in plainer language:
"extrapolating a surrogate model which has not yet been trained for the given input
range is risky and thus not recommended."

### 1.7 Validation is not optional

Zobel-Roos 2021 §1.1, citing Sargent's V&V framework:

> "any model derived does not have any benefit in industrial application if it is not
> validated by comprehensible quantitative criteria"

And on what validation actually requires (§4):

> "Model accuracy and prediction needs to be not better than reality but needs to be
> within such variance to be valid for appropriate predictions. Variance and probability
> has to be given for each number in order to file valid decisions."

Gargalo 2021 §3 adds the requirement in twin-specific form: "models used within a DT
must be validated. The generated results should be equivalent to the measured
properties."

Moser 2021 §3.1 gives the acceptance bar: "several different sets of experimental data
must be reproduced with a high qualitative and quantitative agreement" — note the
**plural**. Sinner 2021 §3 even names numbers they used: NRMSE below 15 % on target
states, parametric uncertainty below 40 %.

**Calibrating a model on a dataset is not validating it.** Validation means reproducing
data the model was not fitted to. By that standard ReLeaf has zero validated models,
and the interface now says so.

### 1.8 The QbD / PAT chain

Zobel-Roos 2021 §1.4 lays out the regulatory life cycle:

> - Risk assessment
> - Approval documentation based on Quality-by-Design approach (QbD)
> - Real time release testing (RTRT) via process analytical technology (PAT)
> - (advanced) process control

Working vocabulary, with the bits ReLeaf needs:

- **Risk assessment.** Ishikawa first ("quick and easy to understand, which makes it
  ideal for efficient risk assessment during early development phases"), then FMEA
  ("occurrence, impact and detectability"), then a risk priority number, RPN = impact ×
  occurrence × detectability, each 1–10, so 1–1000. Two warnings worth copying onto the
  wall: "risk assessment should not exclude risks based on presumptions", and
  quantitative ranking "may lead to underestimation of low scoring factors, as long as
  the risk factors are chosen presumptively."
- **CQA** — critical quality attribute. The property of the product that must be right.
- **CPP** — critical process parameter. A knob whose variation moves a CQA. Zobel-Roos 2021
  §4.1 gives the pattern with turbidity: "the turbidity as a CPP can directly be
  correlated to the bioburden of the product, which is a CQA and should therefore be
  part of the control strategy."
- **Tested space / design space / operation space.** Zobel-Roos 2021 Fig. 10's caption is
  "Relation of tested space, design space and operation space". The design space is
  "the resulting space, which is controlled by this strategy" after risk assessment and
  ranking. The tested space is where you have data. The operation space is where you
  actually run. You cannot draw a design space without a CQA measurement to draw it
  against.
- **PAT.** Defined in Zobel-Roos 2021 §4.1 as "a system for designing, analysing, and
  controlling manufacturing through timely measurements (i.e., during processing) of
  critical quality and performance attributes". The in-line / on-line / at-line /
  off-line class scheme is standard PAT/FDA vocabulary and is used in the title of that
  section, but note for honesty: **none of these five chapters defines the four terms.**
  Cite the FDA PAT guidance for the definitions, not this volume.
- **RTRT** and **APC** are named in the same list; again, neither is defined in the
  volume, so do not cite these chapters for those definitions.

One more line from Zobel-Roos 2021 Table 4 that matters for ReLeaf. Its scoring key marks
biomass, cell morphology and metabolite concentration as "+, not directly applicable
(in combination with process models, chemometrics, etc.)". In plain words: the
quantities you most want to control are the ones you can only reach through a model or
a soft sensor. That is not a ReLeaf weakness; it is the field's.

---

## 2. The bar ReLeaf must clear to say "digital twin"

Collapsing the above into five yes/no tests:

| # | Test | Source |
|---|---|---|
| T1 | A physical asset exists | Portela 2020 §1 |
| T2 | A virtual asset exists that carries a process model, not just a screen | Moser 2021 §2.2 |
| T3 | Data flows **automatically from the asset into the model** | Gargalo 2021 §1.1 |
| T4 | Actions flow **automatically from the model back to the asset** | Gargalo 2021 §1.1; Portela 2020 §1 |
| T5 | The models are validated against data they were not fitted to | Sargent, in Zobel-Roos 2021 §1.1 |

T1–T4 are about plumbing and can be answered with a cable. T5 is about experiments and
cannot be shortcut. A project that passes T1–T4 and fails T5 has built a twin-shaped
object whose predictions nobody should act on — which is exactly the situation Zobel-Roos
2021 §1.1 is warning about.

---

## 3. Point-by-point audit of ReLeaf, 2026-09-06

### 3.1 The five tests

| Test | ReLeaf | Evidence |
|---|---|---|
| T1 physical asset | **PASS** | BR-01: hollow-fibre module HFM-01 (11 fibres, 0.02 m²), P-01 loop pump, P-02 shell pump, V-01/02/04, PT-01/03/04/05, four sterile filters. `Setup → Equipment register`. |
| T2 virtual asset with a model | **PASS** | Eleven models, listed in `Twin → Model register`; a step loop that can be run forward faster than real time. |
| T3 data in, asset → model | **PASS, conditionally** | Implemented on 2026-09-06. The firmware has emitted `PV PH=… T=… PT1=… PT3=… PT4=… PT5=…` at 1 Hz since day one; the page never read it. `readLoop()` / `acceptPV()` now parse it and measured values overwrite the model in `step()`. **True only while a board is plugged in**, and the Twin view reports which. |
| T4 action out, model → asset | **PASS, conditionally** | `sendToBoard()` writes `SET P01=… P02=… P03=… STIR=… LIGHT=… V=…` on every state change. Same condition. |
| T5 validated models | **FAIL** | Two models are calibrated (pump curve, membrane ΔP). None is validated against held-out data. Seven of eleven are marked NOT VALIDATED on the face of the interface. |

**This was the single biggest finding of the audit.** Before this change the connection
was one-way — commands out, nothing back — which is the *inverse* of a digital shadow
and arguably worse: a shadow at least knows what the plant is doing. Every process
value on screen came from the in-page simulation whether or not a board was attached,
and nothing on screen said so. That is now fixed in two places: the code reads the PV
stream, and the Twin view prints when data last flowed in each direction, or that it
never did.

### 3.2 The nine steps, scored

Live in the interface at `Twin → Nine-step build checklist`.

| Step | Score | One line of evidence |
|---|---|---|
| 1 Needs and functionalities via workshops | partial | The need is stated and narrow — decide when to switch the light green. No workshop record, no user research with a grower, no functional spec. |
| 2 Develop PAT | partial | Four in-line pressure transducers built; pH and temperature planned; OD at-line only. **No analytic for the product at all** — no assay for the peptide, in-line or off-line. |
| 3 DT infrastructure | partial | Uno Q split is real and documented (Linux serves and logs; MCU owns IO and interlocks). No historian, no time sync, infrastructure never validated. |
| 4 Develop, train, **validate** models | **not done** | See §3.3. This is the step Portela 2020 calls the heart of the twin. |
| 5 Real-time data processing and integrity | partial | One row per 30 min of process time, units on every column, alarm log exported alongside. No audit trail, no operator identity, no tamper evidence — nothing that survives an ALCOA+ question. |
| 6 Automation and MPC | **not done** | No MPC, no APC. A deterministic rule exists; interlocks run on the MCU. Nothing optimises over a prediction horizon. |
| 7 User interface | **done** | ISA-101 high-performance HMI, four-level hierarchy, faceplates, plain-language layer, EN + 繁體中文, runs offline on the 7" panel. |
| 8 Document the efforts | partial | README, this file, the on-screen model register. No model qualification report, no validation protocol, no change control. |
| 9 Broader data strategy | **not done** | One HTML file and a folder of CSVs. Portela 2020 §2 warns specifically that leaving integration to the end is how components end up impossible to integrate. |

Score: 1 done, 5 partial, 3 not done.

### 3.3 The models, and what would falsify each

Full table lives in the interface (`Twin → Model register`) in both languages. Summary:

| Model | Zobel-Roos Table 1 class | Status |
|---|---|---|
| `pctToFlow()` pump curve | Short-cut, interpolated calibration | CALIBRATED ONLY — fitted on the 9 points it interpolates, no held-out set |
| `dpNet(Q)` membrane ΔP | Short-cut, empirical polynomial | CALIBRATED ONLY — clean module, 22 °C, one module |
| `S.foul` fouling factor | Short-cut, **assumed** rate law | NOT VALIDATED — no fouling data exists at all |
| `OD_SERIES` growth curve | Not a model: a data replay, n = 1 | NOT VALIDATED — replaying one batch is not prediction |
| `muRecent()` growth rate | Observer, two-point log slope | NOT VALIDATED |
| `yieldModel()` yield ceiling | Short-cut stoichiometric chain | NOT VALIDATED for BoPep4; the 103,006 MEFL input is validated for sfGFP in Castillo-Hair 2019 |
| `wxDerive()` ET₀ and VPD | Rigorous, published correlations | CALIBRATED ONLY — validated in the agronomic literature, not by us, not for this field; Ra assumed at 35 MJ/m²/d |
| `wxRisk()` crop stress index | Heuristic scoring | NOT VALIDATED — four thresholds are judgement, not fit |
| `trustScore()` | Heuristic | NOT VALIDATED — weights invented |
| pH / temperature dynamics in `step()` | Short-cut lumped ODE | NOT VALIDATED — neither probe is fitted, so there is nothing it could have been fitted to |
| `policy()` induction rule | Not a model — a deterministic rule | Auditable by reading four lines; what is unvalidated is its three inputs |

Two observations a judge will make faster than we would like:

1. **The growth curve is the weakest link and it looks like the strongest.** A real
   434-hour run is genuinely impressive data. Replaying it is not a model. Everything
   downstream of `OD` — readiness, time-to-ready, the yield estimate, the induction
   decision — inherits n = 1.
2. **The fouling rate is invented and it drives the only maintenance decision in the
   product.** `dR/dt ∝ (1+OD)` with a coefficient chosen so a demo reaches 2× at a
   plausible time. One perfusion run logging resistance against time fixes this, and it
   is the cheapest high-value experiment on the list.

### 3.4 Measured vs modelled — the provenance layer

Ten process channels. As of this build the interface classifies every one of them,
using the FDA PAT classes plus one more for values with no instrument at all:

| Class | Count | Channels |
|---|---|---|
| IN-LINE (measured, sensor in the stream) | 4 | PT-01, PT-03, PT-04, PT-05 |
| AT-LINE (measured on a sample beside the rig) | 1 | OD600 |
| SOFT (no instrument — a model produces the number) | 3 | loop flow, trans-membrane pressure, fouling factor |
| PLANNED (instrument not fitted) | 2 | pH AT-02, temperature TT-01 |

So **four of ten numbers on this screen are measurements, and three are model output
with no instrument behind them anywhere on the skid**. Before this build the interface
did not distinguish them, and the P&ID drew four pressure bubbles while leaving the
flow and the TMP off the diagram entirely — so an operator saw instruments and inferred
instruments. That was the quiet dishonesty in the old drawing.

What changed:

- Every readout cell on the Flow path carries its PAT class as a badge.
- The two soft sensors are now **on** the P&ID, drawn as the ISA-5.1 symbol for a
  function performed in software — a circle inscribed in a square. A different *shape*,
  not just a different colour, because dashed orange already means "planned" and the
  two ideas are unrelated.
- The at-line OD sample point is on the diagram too, with an AT-LINE label, so its
  staleness is visible where it is used.
- Every instrument faceplate now carries three rows: PAT class, physical source, and
  whether this session's numbers are live board data or demo data.
- The Overview carries a one-line provenance strip: 4 measured / 1 at-line / 3 modelled
  / 2 not fitted, plus live-or-demo and the sensor confidence figure. The DEMO DATA note
  used to live only in the nav rail, where an operator reads it once and then stops
  seeing it.

### 3.5 QbD state

- **CQA:** BoPep4 concentration in the harvested permeate, ≥ 100 nM at application
  (Wang 2022's effective dose against 200 mM NaCl). **Currently unmeasurable.** There is
  no assay for the peptide, in-line or off-line. This is the largest hole in the whole
  frame: no CQA measurement means no design space, and no design space means no control
  strategy in the sense Zobel-Roos 2021 §1.4 uses the phrase.
- **Secondary CQA:** permeate bioburden, controlled *by design* through the two 0.22 µm
  filters. A design control, not real-time release.
- **CPPs:** green-light irradiance and duration; cross-flow Q via P-01 (sets both shear
  and TMP); 37 °C; pH 7.0; OD at induction. Of the five, only Q has calibration data
  behind it. The other four are setpoints, not controlled variables, until their probes
  exist.
- **Spaces:** tested space on the P-01 axis is 0–24 % (the calibration range).
  Operation point is 16 % / 263 mL/min. Design space is **empty**, and the interface
  draws it as empty rather than pretending the tested space is a design space — which
  is the most common way student projects overclaim QbD.
- **Risk assessment:** none has been done in the Ishikawa/FMEA/RPN sense. The three
  interlocks are a control strategy arrived at by engineering judgement, not by risk
  ranking. Zobel-Roos 2021 §1.4.1's warning applies directly: "risk assessment should not
  exclude risks based on presumptions."

### 3.6 Autonomy and capability

The interface now separates two things that used to be one table:

**Capability ladder** — what the system can do at all:

| Rung | ReLeaf |
|---|---|
| Monitoring | have |
| Soft sensing | have |
| What-if / virtual experiment | have (added 2026-09-06) |
| Bidirectional connectivity | have, **only while a board is plugged in** |
| APC / MPC | not built — blocked on the fouling model and on the lead time L |
| Autonomous | not built, deliberately — the CQA is unmeasurable, so there is nothing to close the loop on |

**ReLeaf sits on rung three, and on rung four only while the cable is in.**

**Actuation authority** — what the machine is *allowed* to move — stays as the existing
tier table, gated by `trustScore()`. That is a separate question and now reads as one.

Caveat for honesty: none of the five chapters supplies a numbered autonomy ladder. The
rungs above are assembled from the volume's own vocabulary (monitoring, soft sensing,
what-if, bidirectional connectivity, APC/MPC, autonomous), not quoted from a table. Say
that if asked.

### 3.7 The what-if runner

Added 2026-09-06, at `Twin → What-if · virtual experiment`. It is the capability Altran
names last and most student twins lack.

Design decision worth defending out loud: **it runs the same engine, not a second one.**
`runScenario()` snapshots the live state, swaps in throwaway history and log containers,
applies the scenario's pump setting and light state, calls the real `step()` forward at
0.25 h resolution, records the trajectory, then restores every mutated field. Writing a
separate set of equations for the scenario would have been easier and would have drifted
away from the live model within a month. Because it is the same code, what the operator
sees predicted *is* the model that would run the plant.

It inherits every weakness of that model, and the card says so: with the fouling rate
assumed and the growth curve a replay of one batch, "time to clean" is the least
trustworthy number on the plot. It is for ranking two options against each other, not
for putting a date in a calendar.

It also never touches the plant. Recording is off during the run, no alarm is raised,
no SET frame goes out, and the batch log row count is unchanged before and after —
verified.

---

## 4. What a judge will ask, and the answer

**"Which of these numbers did you actually measure?"**
Four of ten. Point at the provenance strip on the Overview, then the PAT badges on the
Flow path, then the two square-in-circle soft-sensor symbols on the P&ID. This is the
question the whole provenance layer exists to answer in three seconds.

**"Is the connection two-way?"**
Yes, and here is the ledger — open the Twin view, first card. It shows when data last
came in and when an action last went out. With no board attached it says, in a yellow
banner, that this is not currently a digital twin and that every number on screen is
simulated. Do not connect a board for the demo and then let that banner stay yellow;
either connect it, or lead with the banner as a deliberate point about honesty.

**"Which of your models is validated?"**
None. Two are calibrated. Show the register — seven rows are marked NOT VALIDATED in
plain sight, with the specific experiment that would falsify each. Volunteering this is
worth more than hiding it: a judge who finds it themselves concludes you did not know.

**"What's your CQA and can you measure it?"**
BoPep4 ≥ 100 nM in the permeate, and no, we cannot measure it at all. That is the
honest and correct answer, and it is on the QbD card. Follow it with what you would
need: an assay. Everything else in the QbD frame is blocked behind it.

**"What is your design space?"**
We do not have one. We have a tested space of 0–24 % on P-01 and an operating point at
16 %. A design space requires a CQA to risk-assess against, and we do not have a CQA
measurement.

**"Is this a neural network deciding when to induce?"**
No. `policy()` is four lines of deterministic rule over three inputs and is auditable by
reading it. That is also the correct engineering choice here: Zobel-Roos 2021 Table 1 marks
AI models non-predictive outside their training range, and our training range is one
batch.

**"What's the one experiment that would most improve this?"**
Two, and say both. (a) Measure the lead time L — four of five cells in the lead-time
chain still read `verify`, and without L the interface is a dashboard rather than a
controller, because "when do we switch on" means "L before it is needed". (b) One
perfusion run logging membrane resistance against time, which turns the fouling model
from an assumption into a calibrated short-cut model and unblocks the only maintenance
decision in the product.

**"Do you have a second batch?"**
No. Expect this one; it is the fastest way to test whether a team understands the
difference between data and a model.

---

## 5. Verdict — how strongly ReLeaf can word the claim

**Do not write "we built a digital twin."** It is not false any more, but it is
unqualified, and unqualified is what gets taken apart.

**Wording that survives cross-examination:**

> ReLeaf BR-01 runs a **digital twin at the soft-sensing and virtual-experiment level**,
> with bidirectional connectivity to the physical skid (PV telemetry in at 1 Hz, SET
> frames out) implemented and demonstrable. Of its eleven models, two are calibrated
> against measured rig data and **none is yet validated against held-out data**; the
> interface marks every unvalidated model as unvalidated on its own face, together with
> the experiment that would falsify it. Against the nine-step build list of Portela et
> al. (2020) the project scores one step complete, five partial and three not started;
> the missing three are model validation, model predictive control, and integration into
> a wider data strategy.

**Or, in one sentence for a poster:**

> A digital twin of a perfusion bioreactor at the soft-sensing and what-if level —
> two-way link built, four of ten process values measured, three modelled, and every
> unvalidated model labelled as such on the screen.

**What would let ReLeaf drop the qualifiers:** validate two models against data they
were not fitted to (the fouling law and the growth model are the two that matter),
obtain any assay for the CQA, and measure L. Nothing else on the list is close in value.

**One thing to be proud of that the papers do not ask for.** Gargalo 2021 §5 notes
that "model-based decision-making that is not understandable to the personnel involved
in plant control is unlikely to be supported", and Sinner 2021 §6 asks for "human–
machine interfaces that make digital twin decisions explainable". The plain-language
layer, the bilingual system, and the provenance badges are exactly that, and step 7 —
"design the user interface" — is the one of the nine ReLeaf can claim outright.

---

## Appendix A — what changed in `0906UI.html` on 2026-09-06

| Area | Change |
|---|---|
| Connectivity | `readLoop()` and `acceptPV()` parse the firmware's 1 Hz `PV` stream; measured pH, temperature and the four pressures overwrite the model in `step()`; TMP becomes PT-01 − PT-03 when both are live. `sendToBoard()` now stamps `S.lastOut`. Disconnecting clears `LIVE` rather than freezing the last reading on screen. |
| Provenance | `PROV` register, `provCls()`, `patBadge()`, `patLong()`, `provCount()`. Badges on every Flow-path readout, on the membrane-health card, and in every instrument faceplate. |
| Flow path | Fits one screen at 1440×900 and 1280×800 without scrolling: the view is a flex column, the SVG sizes to the remaining box with `preserveAspectRatio="xMidYMid meet"`. viewBox 700 → 716, which un-clips the P-01 label. Readouts became an 11-across grid. Tap-to-faceplate unchanged. |
| P&ID | New `s.soft()` (ISA-5.1 software-function symbol) and `s.atline()`. Loop flow (`CALC`), trans-membrane pressure (`PDT-01`) and the OD sample point (`LAB`) are now drawn, with leaders to what they are derived from. Every pressure bubble carries an IN-LINE label. Three new faceplates. |
| Twin view | New cards: connectivity ledger with live verdict; what-if runner; model register; nine-step checklist; QbD frame with a tested-vs-design-space plot. Capability ladder added above the actuation-tier table. Existing cards (decision, lead-time chain, yield ceiling, layer map) kept. |
| Overview | Provenance strip under the action card. Stays farmer-first. |
| i18n | ~50 new DICT keys with real 繁體中文. Long prose inside the Twin and Environment renderers follows the file's existing bilingual-ternary convention rather than DICT, matching the code already there. |

Not touched: the physics step loop's equations, the alarm engine, the xlsx/csv export,
the hardware link protocol, the faceplate machinery, localStorage persistence.

## Appendix B — two things to check before the wiki goes up

1. **Organism — the interface is correct; the real issue is narrower than "one is wrong".**
   ReLeaf deliberately uses *two* organisms. Per `Miscellaneous/Strategic Review —
   2026 iGEM Plant Stress Project.md` §2.1, *B. subtilis* is the committed production
   chassis, and *E. coli* K-12 is the cloning host: "Build all constructs in *E. coli*
   first, then transfer to *B. subtilis* for functional testing." So *E. coli* turning
   up in project documents is intentional, not an error, and the interface is right to
   say *B. subtilis* — that is what runs in the reactor. Verified across
   `Bioreactor_UI/` (README, firmware `releaf_br01.ino`, `index.html`,
   `kiosk_7in.html`, `data/rig_constants.csv`) and `Math Model/`.

   The genuine problem is a **parameter-provenance** one, not a naming one: the
   Castillo-Hair 2019 Hill parameters (n = 1.88 ± 0.16, K½ = 4.66 ± 0.63
   µmol·m⁻²·s⁻¹) are recorded in the 2026-07-04 spec freeze as measured in
   ***E. coli*** at 526 nm, yet the model applies them to a *B. subtilis* culture.
   Borrowing a promoter dose-response across chassis is a real assumption and must be
   labelled as one wherever those numbers appear. This is exactly what experiment E1
   (own LPA dose-response fit in *B. subtilis*) would retire. See
   `WikiHomepage/ASSETS_NEEDED.md` lines 20-25, which already flags it.
2. **Fibre lumen radius.** `renderHealth()` carries a standing comment: `FIBER_R` is
   used as a radius of 0.5 mm, and if 0.5 mm is actually the bore *diameter* then wall
   shear (∝ 1/r³) is 8× low and Reynolds 2× low. Both numbers are on the Overview.
   Confirm against the physical module before either is quoted anywhere.

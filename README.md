# ReLeaf · iGEM 2026 software page, operator interface and firmware

**Team ReLeaf · GEMS Taiwan · iGEM 2026 · Biomanufacturing Village**

**Live page:** https://timmy97-tw.github.io/releaf-wiki-software/
**Live interface:** https://timmy97-tw.github.io/releaf-wiki-software/ui/0924UI.html

> **This is a draft, and it is on GitHub.** The 2026 iGEM Judge Handbook requires
> software competing for the Best Software award to be hosted on iGEM's GitLab, at
> `gitlab.igem.org/2026/software-tools/`. This repository is not that and does not
> substitute for it. **A mirror must be pushed before the wiki freeze on 21 October 2026.**

---

## What is here

| Path | What it is |
|---|---|
| `index.html` | The wiki `/software` page. One file, no build step, no CDN, no external font, no network call. English and 繁體中文 in the same file, toggled in the top bar. |
| `ui/0924UI.html` | **The operator interface**, the running file rather than a screenshot of one. |
| `firmware/releaf_br01_v2.ino` | MCU firmware for the serial protocol the interface speaks. |
| `firmware/test/` | Desktop test: compiles the firmware against a hardware stub, runs six scenarios, feeds its output through the interface's own parser. No board needed. |
| `assets/` | Page figures and photographs. The two SVGs are generated from measured data, not drawn by hand. |
| `docs/DIGITAL_TWIN_CRITERIA.md` | The written digital-twin audit the page's scorecard follows. |

## Which builds these are

| | Interface | Firmware |
|---|---|---|
| File | `ui/0924UI.html` | `firmware/releaf_br01_v2.ino` |
| Copied from | `Digital Twin/01_UI/0924UI.html` | `Digital Twin/06_Firmware/releaf_br01_v2.ino` |
| Copy taken | 2026-09-25 | 2026-09-25 |
| Lines | 1,522 | 365 |
| SHA-256 | `8e6f097901e985800df0d88f112e93e8290feaf24704f445663bcf809daf6d8c` | `27af5fedd35dc62b684e3d1ff65515b4ba7efac26f4d8531670ff7c7d4fbe822` |

Both are single files with zero dependencies. The previous interface,
`0906UI.html`, drove three pumps and three valves and predates the inline OD, pH
and flow sensors; **its serial protocol and this firmware do not overlap**, so do
not pair them.

## Looking at it

```bash
git clone https://github.com/Timmy97-TW/releaf-wiki-software
cd releaf-wiki-software
open index.html            # the page
open ui/0924UI.html        # the interface
```

With no board attached the interface runs a **simulated plant**, so the whole
thing can be driven without hardware. It says `demo data` in the top bar and on
every view for as long as it does.

**Connected to a board.** Web Serial does not work from `file://`, which is the
one thing that trips everybody up. Serve the folder over HTTP and use Chrome or
Edge:

```bash
python3 -m http.server 8080
# then open http://localhost:8080/ui/0924UI.html and press connect
```

**The firmware and protocol test**, which needs `g++` and `node` but no board:

```bash
sh firmware/test/run.sh
# ...
# PASS  24 firmware lines, 15 checks
```

## The serial protocol

```
board to page   PV OD=0.412 ODC=451 PH=7.02 FLOW=263 PT1=2.69 PT3=1.31 PT4=0.45 PT5=0.35
board to page   ST XV01=OPEN LIGHT=GREEN LOCK=NONE UP=1234
board to page   EV IL-1 TMP 0.579 bar, valve forced open
page to board   SET XV01=OPEN LIGHT=GREEN
page to board   CAL ODBLANK=903.0 PHSLOPE=-5.700 PHOFF=21.340
```

115200 baud, newline terminated. `PV` and `ST` go out once a second, `SET` every
two seconds as a heartbeat. Three decisions worth knowing:

- **A dead sensor is omitted, not faked.** A channel failing a plausibility check
  is left out of `PV` entirely, and the page then shows no reading rather than
  freezing the last good value where somebody would keep acting on it.
- **`ST` reports what the board is doing, not what it was told**, including the
  interlock in force, so the page never has to infer an override.
- **Calibration lives on the page** and is pushed down with `CAL` on connect and
  after every recalibration. A reflashed board cannot silently run on stale numbers.

Interlocks run on the microcontroller, not in the browser. **IL-1** opens the
pinch valve if trans-membrane pressure passes 0.50 bar while it is shut, with
hysteresis at 0.42 bar, and also if the four pressure sensors cannot agree a
value. **IL-2** opens the valve and turns the light off after 5 s with no `SET`.

> **Before flashing against a real rig**, check the pin map,
> `XV01_ENERGISE_TO_SHUT` (the valve's fail direction), `FLOW_PULSES_PER_L`, and
> the pH and OD calibration constants. All are placeholders until measured.

## Reproducing the figures

Both SVGs on the page are generated from the measured file, so the figure and the
data cannot drift apart:

```bash
python3 "Digital Twin/03_Scripts/fit_lpa_growth_20260919.py"   # xlsx -> fit
python3 "Digital Twin/03_Scripts/build_wiki_figures.py"        # fit -> assets/*.svg
python3 "Digital Twin/03_Scripts/embed_fit_into_ui.py"         # fit -> the interface's prior
```

| Result on the page | Comes from |
|---|---|
| r = 0.299 h⁻¹, K = 1.005, NRMSE 1.9 %, n = 56 | `Digital Twin/04_Results/lpa_growth_fit_20260919.json` |
| The light-arm comparison and the held-out test | the same file, fields `light_test` and `loo` |
| The run those resolve to | `Digital Twin/02_Data/20260919_OptoLPA_ACCD_Run1_ODresults.xlsx` |

## Keeping this repository in step

`ui/`, `firmware/` and the two generated figures are copies of files that live in
the team's working project:

```bash
cp "Digital Twin/01_UI/0924UI.html"              ui/0924UI.html
cp "Digital Twin/06_Firmware/releaf_br01_v2.ino" firmware/
cp -r "Digital Twin/06_Firmware/test"            firmware/
cp "Digital Twin/05_Wiki_Drafts/figures/"*.svg   assets/
# then update the SHA-256 table above
```

## How another team reuses this

The **shell** generalises: an offline, single-file, bilingual operator interface
with a live process model, a warning model, a recorder, a serial link and an
audit log. The **process model does not** generalise, it is ours, and the page
says so everywhere it appears.

| To change | Edit |
|---|---|
| Sensors, units, limits, provenance | the `CH` object near the top of the interface |
| The growth prior | re-run the fit on your own data, then `embed_fit_into_ui.py` |
| The decision rule | `decide()`, which is short on purpose so it can be read |
| Your second language | every string is `L("english", "中文")` |
| Pin map, interlock thresholds, sensor front-ends | the top of the firmware and its `read*()` functions |

**Hardware assumed:** a microcontroller speaking newline-delimited ASCII at 115200
baud; Chrome or Edge if you want to drive a board; anything that can serve a
directory over HTTP.

## What this is not

The page says this at length and it belongs here too.

- **Not a demonstrated two-way twin.** The link and its interlocks are written and
  tested against firmware output on a desktop, **but no run on the physical
  reactor has been performed with them**. Tests T3 and T4 on the page's scorecard
  read *software only* for that reason.
- **One model is validated, four are not.** The growth model survives a
  leave-one-arm-out test on our own light-plate run. Fouling, crop stress and
  lead time are unvalidated and say so on their own face in the interface, each
  with the experiment that would falsify it.
- **The held-out test is the right shape, not yet strong.** The six arms grew
  alike, so it measures reproducibility across wells of one plate on one day.
- **No assay for the secreted product exists.** No titre, no activity assay. With
  no measurable quality attribute there is no design space and no control strategy
  in the sense the literature means.
- **Temperature is not measured at all**, which is the most likely explanation for
  our light-plate run growing four times slower than an earlier 37 °C shake-flask fit.
- **No model predictive control**, because flow is set by hand. A deterministic
  rule, readable in four lines, decides the light.
- **No historian, no time synchronisation, no operator identity.** Records live in
  browser storage until downloaded.
- **The light layer is literature.** The CcaSR time constant is Castillo-Hair et
  al. (2019), measured in a different organism at a different wavelength.
- **No unit tests for the interface** beyond the protocol round-trip, no CI.
- **No SBOL, no SynBioHub**, and no user testing with a farmer yet.
- **No cost claim about the bioreactor.** No bill-of-materials total exists, so
  the project brief forbids *cheap*, *low-cost*, *affordable* and any per-unit
  price. This repository contains none.

## Rules this page is written under

From `WikiHomepage/PROJECT_BRIEF.md` in the team's project:

- Never invent a number, quote, date or result. Every figure resolves to a named
  file. A number borrowed from the literature says so in the same sentence, at the
  same type size.
- Do not infer from silence. A missing fact is written as missing.
- No cost claim about the bioreactor.
- Sentence-case headings. No em dashes.

## Repository layout

```
index.html                  the wiki /software page, single file, self-contained
assets/                     figures and photographs used on the page
ui/0924UI.html              the BR-01 operator interface, copy of 2026-09-25
firmware/
  releaf_br01_v2.ino        MCU firmware, protocol v2
  test/run.sh               desktop protocol round-trip test
docs/
  DIGITAL_TWIN_CRITERIA.md  the written digital-twin audit
README.md                   this file
LICENSE                     MIT
```

## Licence

**Code: MIT** (`LICENSE`), covering `index.html`, `ui/0924UI.html`, `firmware/`
and any snippet quoted from them. The iGEM Judge Handbook requires an
OSI-approved licence and MIT is one.

**Page text and the figures in `assets/`:** also offered under
[CC BY 4.0](https://creativecommons.org/licenses/by/4.0/), matching the licence on
the team's other public reports. Attribute to *Team ReLeaf, GEMS Taiwan, iGEM 2026*.

Photographs are the team's own.

## Citing the work this builds on

Full reference list, with what each source supplies, is section 7 of the page.

- Portela RMC, et al. (2021). When is an in silico representation a digital twin?
  *Adv Biochem Eng Biotechnol* 176:35–56.
  [10.1007/10_2020_138](https://doi.org/10.1007/10_2020_138)
- Gargalo CL, et al. (2021). Towards the development of digital twins for the
  bio-manufacturing industry. *Adv Biochem Eng Biotechnol* 176:1–34.
  [10.1007/10_2020_142](https://doi.org/10.1007/10_2020_142)
- Sinner P, Daume S, Herwig C, Kager J (2021). Usage of digital twins along a
  typical process development cycle. *Adv Biochem Eng Biotechnol* 176:71–96.
- Cabaneros Lopez P, et al. (2020). Towards a digital twin: a hybrid data-driven
  and mechanistic digital shadow to forecast the evolution of lignocellulosic
  fermentation. *Biofuels Bioprod Bioref*.
  [10.1002/bbb.2108](https://doi.org/10.1002/bbb.2108)
- Satwekar A, et al. (2026). Operationalizing digital twins in biomanufacturing
  through interoperable process analytical technology. *Bioprocess Biosyst Eng*
  49:2061–2083.
  [10.1007/s00449-026-03369-9](https://doi.org/10.1007/s00449-026-03369-9)
- Castillo-Hair SM, et al. (2019). Optogenetic control of *Bacillus subtilis* gene
  expression. *Nat Commun* 10:3099.
  [10.1038/s41467-019-10906-6](https://doi.org/10.1038/s41467-019-10906-6)

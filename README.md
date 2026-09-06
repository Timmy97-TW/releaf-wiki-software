# ReLeaf — iGEM 2026 software page and bioreactor operator interface

**Team ReLeaf · GEMS Taiwan · iGEM 2026 · Biomanufacturing Village**

This repository holds two things:

1. **`index.html`** — the draft of ReLeaf's iGEM wiki `/software` page. A single
   self-contained HTML file: no build step, no CDN, no external font, no network
   call. Bilingual English and 繁體中文 with a toggle.
2. **`ui/0906UI.html`** — a copy of the **live** BR-01 bioreactor operator
   interface that the page documents. Also a single self-contained file.

**Live page:** https://timmy97-tw.github.io/releaf-wiki-software/
**Live interface:** https://timmy97-tw.github.io/releaf-wiki-software/ui/0906UI.html

> **This is a draft, and it is on GitHub.** The 2026 iGEM Judge Handbook requires
> software competing for the Best Software award to be hosted on iGEM's GitLab, at
> `gitlab.igem.org/2026/software-tools/`. This repository is not that and does not
> substitute for it. A mirror must be pushed before the wiki freeze on
> **21 October 2026**.

---

## What the software is

ReLeaf BR-01 is a perfusion hollow-fibre bioreactor intended to run beside a crop
rather than in a facility. The operator interface is written for a grower who has
never used lab equipment, not for a trained bioprocess operator.

Eight views: Overview, Flow path (a real P&ID), Trends, Alarms, Data, Setup,
Environment, and Digital twin. Design basis is **ISA-101 High Performance HMI** —
grey canvas with colour reserved for abnormal conditions, a four-level display
hierarchy, analog context on every value, always-visible sparklines, one-touch
recipes, and an alarm model that will not fire on equipment the operator
deliberately stopped.

**The interlocks live on the microcontroller, not in the browser.** A browser
crash, a closed tab or a dropped link cannot move a pump or leave one running.
The page is a window onto the controller, not the controller.

**Green light switches the CcaS/CcaR system ON in *B. subtilis*. Red switches it
OFF.** This is the single easiest fact on the project to get backwards.

---

## Which version of the interface this is

| | |
|---|---|
| File | `ui/0906UI.html` |
| Copied from | `Bioreactor_UI/0906UI.html` in the team's working project |
| Copy taken | **2026-09-06** |
| Size | single file, vanilla JavaScript, zero dependencies |

The interface is a single file, so replacing it is a **one-file change**. Nothing
else in this repository depends on its internals. To drop in a newer build:

```bash
cp /path/to/Bioreactor_UI/0906UI.html ui/0906UI.html
# then update the "Copy taken" date in this table and in index.html
git commit -am "ui: refresh operator interface to <date> build"
```

---

## How to view it

**The wiki page.** Open `index.html` in any browser. Nothing else is required.

**The interface, to look at it.** Open `ui/0906UI.html` in Chrome or Edge.
Process values are simulated so the interface can be demonstrated with no rig
attached.

**The interface, connected to a board.** Web Serial does not work from `file://`,
which is the one thing that trips everybody up. Serve the folder over HTTP:

```bash
python3 -m http.server 8080
# then open http://localhost:8080/ui/0906UI.html and press "Connect board"
```

On the target hardware (Arduino Uno Q), the Linux side serves the folder and the
MCU side owns the pumps, the valves, the 1 Hz sensor scan and the interlocks.

---

## Reproducing the main results

Everything on the page resolves to a file in the team's working project. The three
datasets that carry the argument:

| Result | Data | Where the number comes from |
|---|---|---|
| 434-hour growth run, 221 binned points | `Bioreactor_UI/data/od600_growth_run_20260721.csv` | 2,671 valid rows of a 10-minute log, 2026-07-21 to 2026-08-08, 22 °C. Plotted directly in `index.html` from the same numbers, inline, unsmoothed. |
| Pump curve, model M1 | `Bioreactor_UI/data/hfm_pump_calibration.csv` | 9 settings, 0 to 24 %, measured at 22 °C with water |
| Membrane pressure drop, model M2 | `Bioreactor_UI/data/rig_constants.csv` | ΔP_net(Q) = 2.427e-4·Q + 4.604e-7·Q² bar, R² 0.9954 |
| Measurement trust layer, 26.7 h lead time | `Math Model/releaf-model/out/sentinel.json` | trained on t < 120 h only, alarm limit fixed before any result was inspected |
| Photometer calibration | `Math Model/releaf-model/out/sentinel.json` | 45 paired TiO₂ points, 2026-08-23 |

The growth curve in `index.html` is drawn in-page from the CSV values embedded
verbatim in the file's own script block, so the figure and the data cannot drift
apart.

**Tags.** The project's parameter file marks every value `fixed`, `lit`, `calc` or
`verify`. **Nothing tagged `verify` may be reported as a result**, on this page or
anywhere else.

---

## How another team reuses this

The **shell** generalises: an offline, single-file, bilingual, ISA-101 operator
interface with an alarm model, a batch recorder, a serial link and a device
register. The **process model does not** — it is ours, and the page says so
everywhere it appears.

| To change | Edit |
|---|---|
| Your sensors, units, alarm bands, plain-language lines | the `TAGS` object |
| Your pumps, valves, probes, and what is built vs planned | the `DEVICES` object |
| Your one-touch modes | the `RECIPES` array |
| Your second language | the `DICT` object — every string is `[english, 繁體中文]` |
| Your rig constants | `Bioreactor_UI/data/rig_constants.csv` |
| Your pump curve | `Bioreactor_UI/data/hfm_pump_calibration.csv` — measure your own |
| Your board and protocol | `Bioreactor_UI/firmware/releaf_br01.ino`, and the matching parser in the page |
| Your interlocks | `interlockedP01()`, `interlockedP02()` — **keep them on the MCU** |
| Your panel size | `Bioreactor_UI/build_kiosk.py` (stage is 1024×600, scales to fill) |

**Hardware assumed:** a microcontroller speaking newline-delimited ASCII at 115200
baud; a browser with Web Serial (Chrome or Edge) if you want to drive a board;
anything that can serve a directory over HTTP. Optionally a 7-inch panel.

Section 7 of `index.html` has the long version, including the conventions we would
actually recommend copying: tag every value the rig cannot measure so nobody
mistakes a calculation for a measurement, keep interlocks on the microcontroller,
alarm only on equipment that is supposed to be running, and write the
plain-language line for a reading before you write the code that displays it.

---

## What this is not

The page says this at length and it belongs here too:

- **It is not a digital twin.** Data flows automatically from the reactor to the
  software. It does not flow automatically back. By the definition in Portela et
  al. (2021) this is a digital shadow with a manual actuation path. Section 5 of
  the page scores it against the nine-step build list honestly: three met, four
  partial, two not met.
- **There is no closed loop** and no model predictive control. Deliberate — on our
  own 434-hour run, a threshold closed loop is not better than a fixed recipe.
- **Nine of eleven models are not validated** on our own data. Section 6 lists the
  sixteen wet-lab measurements that would close the gap, with the experiment for
  each.
- **The light layer is entirely literature.** The Hill parameters come from
  Castillo-Hair et al. (2019). No induction curve exists in our hands.
- **No unit tests, no CI**, and no lockfile for the Python model half.
- **No SBOL, no SynBioHub.** Aspect 1 of the software rubric, and we do not meet it.
- **No user testing with a farmer yet.** Every usability claim is a design
  intention until one has sat in front of it.
- **No cost claim about the bioreactor.** There is no bill-of-materials total, so
  the project brief forbids the words *cheap*, *low-cost* or *affordable* and any
  per-unit price. This repository contains none.

---

## Repository layout

```
index.html      the wiki /software page, single file, self-contained
assets/         photographs and figures used on the page, with honest captions
ui/
  0906UI.html   the live BR-01 operator interface, copy of 2026-09-06
README.md       this file
LICENSE         MIT
```

---

## Licence

**Code: MIT** (`LICENSE`). This covers `index.html`, `ui/0906UI.html` and any
snippet quoted from them. The iGEM Judge Handbook requires an OSI-approved licence
and MIT is one.

**Page text and the figures in `assets/`:** also offered under
[CC BY 4.0](https://creativecommons.org/licenses/by/4.0/), matching the licence on
the team's other public reports. Attribute to *Team ReLeaf, GEMS Taiwan, iGEM 2026*.

Photographs are the team's own. The LPA dose-response schematic was drawn in house
from original vector shapes for this wiki.

---

## Citing the work this builds on

- Castillo-Hair SM, Baerman EA, Fujita M, Igoshin OA, Tabor JJ (2019). Optogenetic
  control of *Bacillus subtilis* gene expression. *Nat Commun* 10:3099.
  [10.1038/s41467-019-10906-6](https://doi.org/10.1038/s41467-019-10906-6)
- Gerhardt KP, et al. (2016). An open-hardware platform for optogenetics and
  photobiology. *Sci Rep* 6:35363.
  [10.1038/srep35363](https://doi.org/10.1038/srep35363)
- Portela RMC, et al. (2021). When is an in silico representation a digital twin?
  *Adv Biochem Eng Biotechnol* 176:35–56.
  [10.1007/10_2020_138](https://doi.org/10.1007/10_2020_138)
- Zobel-Roos S, et al. (2021). Digital twins in biomanufacturing.
  *Adv Biochem Eng Biotechnol* 176:181–262.
  [10.1007/10_2020_146](https://doi.org/10.1007/10_2020_146)

Full reference list, with what each source supplies, is section 10 of the page.

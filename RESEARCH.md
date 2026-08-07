# Pebble Watchface Development — Initial Research

Sources:
- https://repebble.com/blog/spring-2026-pebble-app-contest
- https://developer.repebble.com/sdk/
- https://apps.repebble.com/timely_52978a53bbd0862701000002
- https://github.com/cynorg/PebbleTimely

---

## Overview

Pebble development is actively supported via [rePebble](https://repebble.com), which maintains
the SDK, app store, and community tooling for Pebble Time 2 and Round 2 hardware.

---

## Implementation Approach — Original Code, Not a Port

Timely (and the previously-considered ForecasWatch2) are **reference/inspiration only** — used to
understand what features and behaviors are possible with the SDK, not as a codebase to fork, copy,
or port. This watchface will be **written from scratch as original code**. When studying Timely's
source (or any other example app), treat it as documentation of an approach, not a starting point
to clone and modify.

---

## Languages

| Language | Framework | Notes |
|---|---|---|
| C | Native Pebble SDK | Full API support; more tutorials and reference apps available |
| JavaScript | Alloy (Moddable-based) | Recently out of developer preview; many C APIs not yet available |

Given the reference watchface (Timely) is written in mixed C/JS and the goal is a focused
time + calendar face, **C is the recommended starting point** for full API access and community resources.

---

## Development Environment Options

### Option A — Local SDK (recommended for full control)

- **SDK version:** 4.9.148 (latest)
- **Platforms:** macOS, Linux, Windows (via WSL only — no native Windows install)

**macOS dependency:**
```sh
brew install node
```

**Ubuntu dependency:**
```sh
sudo apt install nodejs npm libsdl2-2.0-0 libglib2.0-0 libpixman-1-0 zlib1g libsndio7.0
```

**Install `pebble-tool` (requires Python 3.10–3.13):**
```sh
uv tool install pebble-tool --python 3.13
```

**Install the SDK:**
```sh
pebble sdk install latest
```

**Authenticate:**
```sh
pebble login          # authenticates via GitHub
```

**Core workflow:**
```sh
pebble new-project my-watchface   # scaffold a new project
pebble build                      # compile
pebble install --emulator basalt  # run on Pebble Time emulator
pebble install --cloudpebble      # deploy via Pebble mobile app
pebble publish                    # publish to app store
```

**Emulator platform names:**

| Target | Platform flag |
|---|---|
| Pebble Time / Time 2 (rectangular) | `basalt` |
| Pebble Time Round / Round 2 | `chalk` |

### Option B — CloudPebble (browser-based, zero install)

- URL: https://cloudpebble.repebble.com/
- Full IDE, build, and publish flow in-browser
- Good for quick prototyping; less friction than local setup for first-time use

---

## Publishing to the App Store

1. Build (`pebble build`) or use CloudPebble
2. `pebble publish` or CloudPebble publish flow
3. System auto-generates screenshots and animated GIFs across all platforms
4. **Quick View support is recommended** for all watchfaces

App store: https://apps.repebble.com/

Developer dashboard: https://developer.repebble.com/dashboard

---

## Reference Watchface: Timely

**App store:** https://apps.repebble.com/timely_52978a53bbd0862701000002
**Source:** https://github.com/cynorg/PebbleTimely
**Author:** Martin Norland (@cynorg), with contributions from Ben Johnson; based on William Heaton's
PebbleCalendarWatch. Current release v2.6, last updated October 2015 — an older, more legacy
codebase than the previously-considered ForecasWatch2.
**License:** GPLv3
**Supported hardware:** OG Pebble, Steel, Time/Time Steel, Pebble 2, Pebble 2 Duo, Time 2 (with scaling)

### Features to keep as inspiration

- **Time + date display** — top half of the screen
- **3-week calendar** (configurable week count) — bottom half of the screen
- **Status bar** — connection (Bluetooth), charging, and battery indicators
- **Config UI via PebbleKit JS** (`configuration.html` + `pebble-js-app.js`)
- **Customizable vibration patterns** — hourly alert, connection status change

### Features to drop

- Weather integration (temperature, Climacons weather-icon font, weather fetching)
- Anything tied to weather-driven location services
- Name/version splash text (e.g. "Timely 2.6") shown when returning to the watchface — reads as an
  ad/branding banner and is distracting; the new face should just show the face, no splash

### Code structure (for reference)

```
src/
  Timely.c / Timely.h    # main watchface entry point + core logic (largely monolithic)
  effect_layer.c/h        # custom layer rendering with visual effects
  effects.c/h             # visual effect calculations
  blur.c                  # blur effect
  math.c/h                # math utilities
  js/
    pebble-js-app.js      # companion JS entry point (phone-side)
    configuration.html    # settings/config UI (plain HTML, not Clay-based)
```

Unlike ForecasWatch2's clean `layers/` vs `services/` split, Timely's C code is comparatively
monolithic — most logic lives in `Timely.c` rather than being broken into per-feature modules.
Plan to extract clear layer boundaries (time, calendar, status bar, steps) during implementation
rather than copying this structure directly.

---

## Design Specification — Status Bar & Info Layout

Current UI requirements for the new watchface (superseding the generic "Battery indicator" /
"Bluetooth connection indicator" bullets above). Captured 2026-08-07.

### Status bar (top row)

| Element | Old behavior (Timely-style) | New behavior |
|---|---|---|
| Bluetooth | Icon + "no link" text, always visible | Small icon only, no text label. Shown only when there is no phone connection; hidden while connected. |
| Battery | Larger icon with a percentage number | Small vertical battery icon, no percentage digits, moved to the left next to the Bluetooth icon |
| Steps | *(new)* | Fills the horizontal space vacated by the battery indicator's old position — see formatting rules below |
| UTC | Present | Removed entirely |

**Open question on the Bluetooth trigger:** the request says "when the watch is in airplane mode."
This is being read as "no phone connection" (the standard `bluetooth_connection_service` disconnected
callback), same condition Timely's "no link" text used. Flag for correction if a literal Airplane
Mode radio-off toggle is meant instead of a disconnected-but-radio-on state.

### Steps counter formatting

- Below 1,000 steps: show the plain integer — e.g. `25`, `821`
- At/above 1,000 steps: switch to `k` notation with one decimal place — e.g. `10.2k`, `2.5k`
- Data source: Pebble HealthService (`health_service_*` APIs). Confirm step data is actually
  available on the older hardware Timely supports (OG Pebble/Steel predate HealthService).

### Week indicator: week + quarter-end week

Format: `W<current ISO week>/Q<quarter-end week>`

- The year is split into four fixed 13-week quarters (13/26/39/52) — not tied to actual calendar
  quarter-end dates.
- `quarter_end_week = ceil(current_week / 13) * 13`
- Examples (verified against 2026-08-07 = ISO week 32, which falls in Q3 → ends week 39):
  - `W32/Q39`
  - `W2/Q13` (week 2 → Q1 ends week 13)
  - `W15/Q26` (week 15 → Q2 ends week 26)

### Time format option

- Add a settings toggle for **military time** — no colon, e.g. `1113`
- Keep the existing **24-hour** format as the other option — with colon, e.g. `11:13`
- Both are 24-hour clocks; the toggle only changes the separator

### App-return behavior

- No splash/banner text (e.g. Timely's "Timely 2.6" name+version overlay) when the watchface
  becomes visible again (app return, wrist raise, etc.) — it reads as an ad and should be removed

---

## Settings / Configuration UI (Phone App)

**Question raised:** is the "complicated web interface" style of Timely's settings page a requirement
of the Pebble framework, or something Timely chose to build that way?

**Answer: not a framework requirement.** Tapping a watchapp's settings in the Pebble mobile app fires
a `showConfiguration` event that opens a URL in an in-app webview — the page's content, layout, and
complexity are entirely up to the developer. Timely's config screen (hand-written `configuration.html`
+ `pebble-js-app.js`, manually wired to `showConfiguration`/`webviewclosed` and raw message keys) is
just one way to build it, not a constraint of the OS.

**Two implementation paths:**

| Approach | What it looks like |
|---|---|
| Raw HTML/JS (Timely's approach) | Hand-write the page (HTML/CSS/JS) and manually wire up `showConfiguration` / `webviewclosed` events plus message keys. Full control, more boilerplate, easy to end up looking dated/cluttered. |
| [Clay](https://github.com/pebble/clay) (recommended) | Declare settings in a `config.json` (toggles, sliders, color pickers, radio groups, headings, custom components) and let Clay generate the page + handle the messaging boilerplate. Much less code, cleaner default look, still supports fully custom components where needed. |

The original reference candidate, ForecasWatch2, already used Clay for its settings UI (see its
`pkjs/clay/` folder noted earlier) — worth carrying that choice forward here rather than replicating
Timely's raw HTML page. Settings for this watchface (status bar element toggles, week/quarter format,
24h vs. military time, etc.) map well onto Clay's standard component set.

---

## Tutorials & Learning Resources

| Resource | URL / Notes |
|---|---|
| SDK installation guide | https://developer.repebble.com/sdk/ |
| C watchface tutorial (Parts 1 & 5) | https://developer.repebble.com/tutorials/ |
| Alloy (JS) watchface tutorial (Parts 1 & 5) | https://developer.repebble.com/tutorials/ |
| Source-available apps | Filter on https://apps.repebble.com/ |
| PebbleOS Figma design elements | Template by Lavender (linked from contest page) |
| Claude/Codex watchface skill | https://github.com/coredevices/pebble-watchface-agent-skill |
| Hardware CAD / schematics | https://github.com/coredevices/hardware |
| Developer FAQ | https://developer.repebble.com/faqs/ |
| Developer forum | https://forum.repebble.com/c/developers-ask-questions-and-get-help/7 |
| Rebble Discord (#sdk-dev) | Via Rebble community invite |

---

## Alloy (JavaScript) Notes

- Built on the Moddable engine
- Verbose logging disabled by default; re-enable via `kModdableCreationFlagLogInstrumentation` in `mdbl.c`
- Run `pebble build` after upgrading the SDK to pick up Alloy changes
- Many C APIs are not yet available — check compatibility before choosing this path

---

## Key URLs at a Glance

| Purpose | URL |
|---|---|
| App store | https://apps.repebble.com/ |
| Developer dashboard | https://developer.repebble.com/dashboard |
| SDK setup guide | https://developer.repebble.com/sdk/ |
| Tutorials | https://developer.repebble.com/tutorials/ |
| CloudPebble IDE | https://cloudpebble.repebble.com/ |
| Watchface agent skill (AI) | https://github.com/coredevices/pebble-watchface-agent-skill |
| Hardware repo | https://github.com/coredevices/hardware |
| Reference watchface source | https://github.com/cynorg/PebbleTimely |
| Reference watchface app store page | https://apps.repebble.com/timely_52978a53bbd0862701000002 |

---

## Open Questions / Next Steps

- [x] Reference watchface chosen: **Timely** (PebbleTimely) — replaces the earlier ForecasWatch2 candidate, used as reference/inspiration only, not a base to fork
- [x] Settings UI approach: not framework-mandated — leaning **Clay** over Timely's hand-rolled HTML page
- [ ] Choose language: C (recommended — full API, more tutorials) vs JavaScript/Alloy
- [ ] Decide on local SDK vs CloudPebble for development
- [ ] Walk through C watchface tutorial Parts 1 & 5
- [ ] Study `Timely.c` / `js/` to understand time + calendar rendering — for reference only; the actual implementation will be original code, split into clear layers rather than mirroring Timely's monolithic structure
- [ ] Determine which Pebble hardware to target (Time 2 rectangular, Round 2, or both)
- [ ] Confirm the Bluetooth-icon trigger condition (disconnected vs. literal Airplane Mode) — see Design Specification section
- [ ] Confirm HealthService step-count availability across target hardware (older Pebbles may lack it)
- [ ] Assess Quick View support requirements for the planned design
- [ ] Check the Figma PebbleOS design elements template before starting UI work
- [ ] Decide on calendar scope: Timely defaults to 3 weeks (configurable) — consider if that's the right amount

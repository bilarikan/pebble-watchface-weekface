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

- **SDK version:** 4.17 (latest as of 2026-08-07; requires pebble-tool ≥ 5.0.38 — older 5.0.35 fails SDK install with "unmet requirements")
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

App store: https://apps.repebble.com/ · Developer dashboard: https://developer.repebble.com/dashboard

### Steps for this project (verified against pebble-tool 5.0.39, 2026-08-07)

1. **Authenticate** — `pebble login` (GitHub-backed). CI alternative: `PEBBLE_FIREBASE_ID_TOKEN`
   env var / `--firebase-id-token`.
2. **Finalize metadata** in `package.json` — `displayName` is **"Weekface" (final, confirmed
   2026-08-07)**, `version`, `author`. The `uuid` identifies the app across releases: keep it
   stable forever once published.
3. **Confirm target platforms** — currently `emery` only. Adding `gabbro` (Round 2) later
   means a round layout pass plus a republish.
4. ~~Choose a license~~ — **MIT, chosen 2026-08-07** (`LICENSE` in repo root).
5. ~~Prepare store icons~~ — done 2026-08-07: face capture with `12.4k` steps shown, padded
   square. `docs/store/icon-large-144.png` and `docs/store/icon-small-48.png` (regenerate
   from `docs/store/icon-square.png` if the store wants other sizes). Store screenshots in
   `docs/store/`: `emery_1_default.png`, `emery_2_military_abbrev.png`,
   `emery_3_minimal_12h.png`, `emery_4_translated.png` (Turkish) — named platform-first as
   `pebble publish --screenshots` requires.
6. **Build** — `pebble build` → `build/pebble-watchface-01.pbw`.
7. **Test on real hardware** before submitting — `pebble install --cloudpebble` with the
   phone app, exercise the settings page end-to-end (Clay → AppMessage → persistence).
8. **Publish** — `pebble publish` from the project directory. First run creates the store
   entry interactively (name, description, category = Faces, optional source URL, icons).
   Useful flags: `--release-notes "..."`; releases are created as **drafts by default** —
   pass `--is-published` to go live immediately. Rollover GIFs are auto-captured for all
   supported platforms (`--no-gif-all-platforms` to skip; `--all-platforms` adds static
   screenshots; manually supplied files must be named like `emery_screenshot.png`).
9. **Review the listing** in the developer dashboard and flip the draft release to published.
10. **Updates** — bump `version` in `package.json`, rebuild, `pebble publish --release-notes "..."`.

Quick View: **implemented 2026-08-07** via the unobstructed-area service — when the timeline
Quick View covers the bottom of the screen, the calendar hides entirely (rather than showing
clipped rows) and the status bar / date / time / indicator rows remain fully visible; the
calendar returns when the Quick View dismisses. Verified in the emulator with
`pebble emu-set-timeline-quick-view on|off`.

### App store description (draft)

**Short/tagline:** Time, a three-week calendar, and the numbers that structure your year.

**Full description:**

> Weekface is a clean, information-dense watchface built around how weeks structure a year.
>
> TOP TO BOTTOM
> • Status bar — battery, Bluetooth (shown while connected), and today's step count
> • Full date — Friday, August 07, 2026
> • Big, readable time
> • Week indicator — W32/Q39: the current ISO week and the closing week of its 13-week quarter
> • Year progress — D219/R146: day of the year and days remaining
> • Three-week calendar — last week, this week (today highlighted), next week. Monday-first,
>   always in agreement with the ISO week number.
>
> SETTINGS (from the Pebble app)
> • 12-hour, 24-hour, or military time (1305)
> • Full, abbreviated, or hidden weekday; full or abbreviated month
> • Show or hide the week and day-of-year indicators
> • Hourly and connect/disconnect vibrations — both respect Quiet Time
> • Full translations: rename every weekday, month, and indicator letter in your language;
>   the calendar header follows automatically
>
> No weather, no network needed, no branding splash. Just the face.
>
> Designed for Pebble Time 2.

Category: **Faces**.

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
| Bluetooth | Icon + "no link" text, always visible | Small icon only, no text label. Shown while the watch is connected to a phone/device via Bluetooth; hidden when disconnected or in airplane mode. *(Inverted from the earlier draft of this spec — confirmed 2026-08-07.)* |
| Battery | Larger icon with a percentage number | Small vertical battery icon, no percentage digits, moved to the left next to the Bluetooth icon |
| Steps | *(new)* | Fills the horizontal space vacated by the battery indicator's old position — see formatting rules below |
| UTC | Present | Removed entirely |

**Resolved (2026-08-07):** the Bluetooth icon indicates a *live connection* — shown when the watch
is connected to a phone/device via Bluetooth, hidden when disconnected or in airplane mode. This is
the inverse of Timely's "no link" indicator. Implemented with the standard connection service
callback (`connection_service` connected/disconnected events); no separate airplane-mode detection
exists in the SDK, and none is needed since airplane mode simply reads as disconnected.

### Steps counter formatting

- Below 1,000 steps: show the plain integer — e.g. `25`, `821`
- At/above 1,000 steps: switch to `k` notation with one decimal place — e.g. `10.2k`, `2.5k`
- Data source: Pebble HealthService (`health_service_*` APIs). Confirm step data is actually
  available on the older hardware Timely supports (OG Pebble/Steel predate HealthService).

### Vertical layout (updated 2026-08-07 after first build review)

Top to bottom:

1. **Status bar** — battery + Bluetooth icons (left), steps (right)
2. **Full date line** — `Friday, August 07, 2026` (weekday, month, zero-padded day, year)
3. **Time** — large digits
4. **Indicator row** — week/quarter-end (left) and day-of-year/days-remaining (right)
5. **3-week calendar**

### Day of year / days remaining indicator

Format: `D<day of year>/R<days remaining>` — e.g. `D219/R146` on 2026-08-07 (day 219, 146 left).
Carried over from Timely. Right-aligned on the indicator row, opposite the week indicator.

### Week indicator: week + quarter-end week

Position: left side of the indicator row between the time and the calendar (moved out of the
status bar, where it originally sat centered).

Format: `W<current ISO week>/Q<quarter-end week>`

- The year is split into four fixed 13-week quarters (13/26/39/52) — not tied to actual calendar
  quarter-end dates.
- `quarter_end_week = ceil(current_week / 13) * 13`
- Examples (verified against 2026-08-07 = ISO week 32, which falls in Q3 → ends week 39):
  - `W32/Q39`
  - `W2/Q13` (week 2 → Q1 ends week 13)
  - `W15/Q26` (week 15 → Q2 ends week 26)

### Time format option (updated 2026-08-07: now three-way)

Radio group in settings:

- **12-hour** — colon, leading zero stripped, no AM/PM marker: `1:05`
- **24-hour** (default) — colon: `13:05`
- **Military** — 24-hour, no colon: `1305`

### Display options (added 2026-08-07)

- **Weekday in date line** — Full (`Friday, …`) / Abbreviated (`Fri, …`) / Hidden
- **Month in date line** — Full (`August`) / Abbreviated (`Aug`)
- **Week / quarter-end indicator** — hide `W32/Q39` (left of indicator row)
- **Day of year indicator** — hide `D219/R146` (right of indicator row)

### Translations (added 2026-08-07, Timely-inspired)

Text fields in settings for all four name sets: 7 full weekdays, 7 abbreviated weekdays
(max 5 chars), 12 full months, 12 abbreviated months — plus the four indicator prefix
letters (W/Q/D/R, max 3 chars each, e.g. Turkish `H32/Ç39` and `G219/K146`).
Defaults are English. The abbreviated
weekday names also drive the calendar header (first two characters, UTF-8-aware). Stored on
the watch across four persist keys (each name group must stay under the 256-byte persist
entry limit). Empty fields keep their previous value.

Timely's full settings inventory (for future inspiration): dark/light theme; per-event vibration
*patterns* (none/1x/2x/3x/long/… for hourly, connect, disconnect); translation text fields for
day/month names and status labels; US vs. European date format; three configurable indicator
slots below the time (week / timezone / AM-PM / day); week numbering scheme (ISO 8601 vs.
Sun-first vs. Mon-first); calendar options — invert today, grid lines on/off, week start on any
day of the week.

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
- [x] Choose language: **C** (full API, more tutorials)
- [x] Decide on local SDK vs CloudPebble: **local SDK** — pebble-tool 5.0.39 + SDK 4.17 installed 2026-08-07
- [ ] Walk through C watchface tutorial Parts 1 & 5
- [ ] Study `Timely.c` / `js/` to understand time + calendar rendering — for reference only; the actual implementation will be original code, split into clear layers rather than mirroring Timely's monolithic structure
- [x] Determine which Pebble hardware to target: **Time 2 (rectangular) first**; Round 2 possibly later
- [x] Confirm the Bluetooth-icon trigger condition: **icon shown while connected**, hidden when disconnected or in airplane mode — see Design Specification section
- [x] Confirm HealthService step-count availability: not a concern — targeting Time 2 only, which has HealthService
- [x] Assess Quick View support requirements: **implemented** — calendar hides under Quick View, top rows stay visible
- [ ] Check the Figma PebbleOS design elements template before starting UI work
- [x] Decide on calendar scope: **3 weeks, fixed** (prev / current / next week)

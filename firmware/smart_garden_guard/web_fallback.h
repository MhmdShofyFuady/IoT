#ifndef WEB_FALLBACK_H
#define WEB_FALLBACK_H

#include <Arduino.h>

// Cadangan file web di memori Flash (PROGMEM) jika partisi LittleFS belum di-upload
const char FALLBACK_INDEX_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta name="description" content="Dashboard jaringan lokal untuk pemantauan sensor dan kendali penyiraman tanaman otomatis berbasis ESP32.">
  <title>Smart Garden Guard: Dashboard Penyiraman Otomatis ESP32</title>
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <header class="topbar">
    <div class="topbar-inner">
      <div class="brand-group">
        <h1 class="app-title">Smart Garden Guard</h1>
        <p class="app-subtitle">Sistem penyiraman tanaman otomatis berbasis ESP32</p>
      </div>
      <div class="meta-bar" aria-live="polite">
        <span id="connBadge" class="badge badge-online">Terhubung</span>
        <span id="sysStatusBadge" class="badge badge-status-normal">Status: Normal</span>
        <span class="meta-item">Terakhir update: <strong id="lastUpdateText">00:00:00</strong></span>
        <span class="meta-item">Uptime: <strong id="uptimeText">00:00:00</strong></span>
      </div>
    </div>
  </header>

  <main class="container">
    <div id="alarmBanner" class="alarm-banner" hidden role="alert">
      <strong>Peringatan: Air tandon habis.</strong> Pompa dikunci otomatis ke posisi mati dan alarm buzzer diaktifkan (Pin 13). Isi ulang tandon air untuk melanjutkan penyiraman.
    </div>

    <section class="metrics-grid" aria-label="Pembacaan sensor real-time">
      <article class="metric-panel">
        <div class="metric-head">
          <h2 class="metric-title">Kelembapan tanah</h2>
          <span class="pin-tag">Pin 34 (ADC1)</span>
        </div>
        <div class="metric-reading">
          <span id="soilPctVal" class="metric-number">55</span>
          <span class="metric-unit">%</span>
        </div>
        <div class="meter-track" aria-hidden="true">
          <div id="soilMeterBar" class="meter-fill meter-soil"></div>
        </div>
        <p class="metric-foot">ADC mentah: <span id="soilRawVal">1840</span> dari 4095 (kering &lt; <span id="lblDry">35</span>%, basah &ge; <span id="lblWet">65</span>%)</p>
      </article>

      <article class="metric-panel">
        <div class="metric-head">
          <h2 class="metric-title">Level air tandon</h2>
          <span class="pin-tag">Pin 5 &amp; 18</span>
        </div>
        <div class="metric-reading">
          <span id="waterPctVal" class="metric-number">72</span>
          <span class="metric-unit">%</span>
        </div>
        <div class="meter-track" aria-hidden="true">
          <div id="waterMeterBar" class="meter-fill meter-water"></div>
        </div>
        <p class="metric-foot">Jarak permukaan air: <span id="distanceCmVal">12.0</span> cm (batas minimum <span id="lblWaterMin">15</span>%)</p>
      </article>

      <article class="metric-panel">
        <div class="metric-head">
          <h2 class="metric-title">Suhu udara</h2>
          <span class="pin-tag">Pin 32 (DHT11)</span>
        </div>
        <div class="metric-reading">
          <span id="tempVal" class="metric-number">28.5</span>
          <span class="metric-unit">°C</span>
        </div>
        <div class="meter-track" aria-hidden="true">
          <div id="tempMeterBar" class="meter-fill meter-temp"></div>
        </div>
        <p class="metric-foot">Rentang pantau lingkungan kebun (0 sampai 50 °C)</p>
      </article>

      <article class="metric-panel">
        <div class="metric-head">
          <h2 class="metric-title">Kelembapan udara</h2>
          <span class="pin-tag">Pin 32 (DHT11)</span>
        </div>
        <div class="metric-reading">
          <span id="humVal" class="metric-number">68.0</span>
          <span class="metric-unit">%</span>
        </div>
        <div class="meter-track" aria-hidden="true">
          <div id="humMeterBar" class="meter-fill meter-hum"></div>
        </div>
        <p class="metric-foot">Kelembapan relatif udara sekitar tanaman</p>
      </article>
    </section>

    <section class="workspace-grid">
      <div class="panel chart-panel">
        <div class="panel-head">
          <div>
            <h2 class="panel-title">Grafik riwayat sensor (30 sampel terakhir)</h2>
            <p class="panel-desc">Pembaruan otomatis setiap 2 detik melalui jaringan lokal</p>
          </div>
          <div class="chart-legend">
            <span class="legend-key legend-soil">Tanah (%)</span>
            <span class="legend-key legend-water">Air tandon (%)</span>
            <span class="legend-key legend-temp">Suhu (°C)</span>
          </div>
        </div>
        <div class="canvas-wrap">
          <canvas id="historyChart" width="680" height="240" aria-label="Grafik riwayat kelembapan tanah, level air, dan suhu"></canvas>
        </div>
      </div>

      <div class="panel control-panel">
        <h2 class="panel-title">Kendali sistem &amp; aktuator</h2>
        <p class="panel-desc">Relay pompa (Pin 19 Active-High) &amp; Buzzer (Pin 13)</p>

        <div class="control-row">
          <div class="control-info">
            <span class="control-label">Mode kerja sistem</span>
            <span id="modeDescText" class="control-sub">Otomatis dengan hysteresis kelembapan tanah</span>
          </div>
          <div class="segmented-group" role="group" aria-label="Pilih mode kerja">
            <button type="button" id="btnModeAuto" class="btn-segment is-selected">Otomatis</button>
            <button type="button" id="btnModeManual" class="btn-segment">Manual</button>
          </div>
        </div>

        <div class="control-row">
          <div class="control-info">
            <span class="control-label">Status pompa air</span>
            <span id="pumpStateText" class="control-sub">Pompa mati (Relay nonaktif)</span>
          </div>
          <div class="action-group">
            <button type="button" id="btnPumpToggle" class="btn-primary" disabled>Nyalakan pompa</button>
          </div>
        </div>

        <form id="thresholdForm" class="threshold-form">
          <h3 class="form-title">Ambang batas otomatis (Preferences NVS)</h3>
          <div class="input-grid">
            <label class="field">
              <span class="field-label">Ambang kering (%)</span>
              <input type="number" id="inSoilDry" min="5" max="90" value="35" required>
            </label>
            <label class="field">
              <span class="field-label">Ambang basah (%)</span>
              <input type="number" id="inSoilWet" min="10" max="95" value="65" required>
            </label>
            <label class="field">
              <span class="field-label">Air minimum (%)</span>
              <input type="number" id="inWaterMin" min="5" max="80" value="15" required>
            </label>
          </div>
          <div class="form-actions">
            <button type="submit" id="btnSaveThresh" class="btn-secondary">Simpan ke memori</button>
            <span id="formFeedback" class="form-feedback" aria-live="polite"></span>
          </div>
        </form>
      </div>
    </section>

    <section class="panel sim-panel" aria-label="Panel simulasi pengujian">
      <div class="sim-header">
        <div>
          <h2 class="panel-title">Mode simulasi &amp; uji skenario cepat</h2>
          <p class="panel-desc">Gunakan saat pengujian dashboard tanpa sensor fisik atau untuk demonstrasi cepat tiap kondisi status</p>
        </div>
        <button type="button" id="btnSimToggle" class="btn-secondary">Mode sensor fisik</button>
      </div>
      <div class="sim-presets">
        <button type="button" class="btn-preset" data-soil="22" data-water="80">Simulasi tanah kering</button>
        <button type="button" class="btn-preset" data-soil="75" data-water="85">Simulasi normal</button>
        <button type="button" class="btn-preset" data-soil="30" data-water="8">Simulasi tandon habis</button>
      </div>
    </section>
  </main>

  <script src="/app.js"></script>
</body>
</html>
)rawliteral";

const char FALLBACK_STYLE_CSS[] PROGMEM = R"rawliteral(/* Hallmark · shape: app screen · genre: modern-minimal · theme: Field · contrast: pass · v2.0.0 */
/* critique: telemetry console for local ESP32 irrigation system; tabular figures and transform-based meters */

:root {
  --font-sans: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;

  --color-bg: #f4f6f2;
  --color-surface: #fbfcfa;
  --color-surface-subtle: #ecf0e8;
  --color-border: #d7dfd1;
  --color-border-strong: #b6c2ad;

  --color-ink: #161d15;
  --color-ink-2: #414d3f;
  --color-ink-3: #616e5e;

  --color-accent: #1d6b42;
  --color-accent-hover: #155232;
  --color-accent-soft: #e1f2e8;
  --color-accent-ink: #f7fbf8;

  --color-water: #18628c;
  --color-water-soft: #e0f0f8;

  --color-warn: #9c5708;
  --color-warn-soft: #fdf0dc;

  --color-danger: #a82020;
  --color-danger-soft: #fce6e6;

  --color-focus: #1d6b42;

  --radius-sm: 6px;
  --radius-md: 10px;
  --dur-micro: 120ms;
  --dur-short: 200ms;
}

*,
*::before,
*::after {
  box-sizing: border-box;
  margin: 0;
  padding: 0;
}

html,
body {
  overflow-x: clip;
  background-color: var(--color-bg);
  color: var(--color-ink);
  font-family: var(--font-sans);
  font-size: 16px;
  line-height: 1.5;
  caret-color: var(--color-accent);
  scrollbar-color: var(--color-border-strong) var(--color-bg);
  -webkit-font-smoothing: antialiased;
}

::selection {
  background-color: var(--color-accent-soft);
  color: var(--color-ink);
}

.topbar {
  background-color: var(--color-surface);
  border-bottom: 1px solid var(--color-border);
  padding: 1rem 1.25rem;
}

.topbar-inner {
  max-width: 1160px;
  margin: 0 auto;
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  justify-content: space-between;
  gap: 1rem;
}

.app-title {
  font-size: 1.25rem;
  font-weight: 650;
  color: var(--color-ink);
  overflow-wrap: break-word;
  min-width: 0;
}

.app-subtitle {
  font-size: 0.875rem;
  color: var(--color-ink-2);
}

.meta-bar {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 0.625rem;
  font-size: 0.875rem;
  color: var(--color-ink-2);
  font-variant-numeric: tabular-nums;
}

.meta-item {
  background-color: var(--color-surface-subtle);
  padding: 0.35rem 0.65rem;
  border-radius: var(--radius-sm);
  border: 1px solid var(--color-border);
}

.badge {
  display: inline-flex;
  align-items: center;
  padding: 0.35rem 0.75rem;
  border-radius: var(--radius-sm);
  font-size: 0.875rem;
  font-weight: 600;
  border: 1px solid transparent;
}

.badge-online {
  background-color: var(--color-accent-soft);
  color: var(--color-accent);
  border-color: var(--color-accent);
}

.badge-offline {
  background-color: var(--color-danger-soft);
  color: var(--color-danger);
  border-color: var(--color-danger);
}

.badge-status-normal {
  background-color: var(--color-accent-soft);
  color: var(--color-accent);
}

.badge-status-watering {
  background-color: var(--color-water-soft);
  color: var(--color-water);
}

.badge-status-dry {
  background-color: var(--color-warn-soft);
  color: var(--color-warn);
}

.badge-status-empty {
  background-color: var(--color-danger-soft);
  color: var(--color-danger);
}

.container {
  max-width: 1160px;
  margin: 0 auto;
  padding: 1.25rem;
  display: flex;
  flex-direction: column;
  gap: 1.25rem;
}

.alarm-banner {
  background-color: var(--color-danger-soft);
  color: var(--color-danger);
  border: 1px solid var(--color-danger);
  border-radius: var(--radius-md);
  padding: 0.875rem 1rem;
  font-size: 0.9375rem;
}

.metrics-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
  gap: 1rem;
}

.metric-panel,
.panel {
  background-color: var(--color-surface);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-md);
  padding: 1.125rem;
}

.metric-head {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 0.5rem;
  margin-bottom: 0.5rem;
}

.metric-title {
  font-size: 0.9375rem;
  font-weight: 600;
  color: var(--color-ink-2);
}

.pin-tag {
  font-size: 0.8125rem;
  color: var(--color-ink-3);
  background-color: var(--color-surface-subtle);
  padding: 0.15rem 0.5rem;
  border-radius: var(--radius-sm);
  font-variant-numeric: tabular-nums;
}

.metric-reading {
  display: flex;
  align-items: baseline;
  gap: 0.25rem;
  margin-bottom: 0.75rem;
  font-variant-numeric: tabular-nums;
}

.metric-number {
  font-size: 2.125rem;
  font-weight: 700;
  color: var(--color-ink);
  line-height: 1.1;
  font-variant-numeric: tabular-nums;
}

.metric-unit {
  font-size: 1rem;
  font-weight: 600;
  color: var(--color-ink-2);
}

.meter-track {
  height: 8px;
  background-color: var(--color-surface-subtle);
  border-radius: var(--radius-sm);
  overflow: clip;
  margin-bottom: 0.625rem;
}

.meter-fill {
  height: 100%;
  width: 100%;
  transform-origin: left center;
  transform: scaleX(0.5);
  transition: transform var(--dur-short) ease-out;
}

.meter-soil {
  background-color: var(--color-accent);
}

.meter-water {
  background-color: var(--color-water);
}

.meter-temp {
  background-color: var(--color-warn);
}

.meter-hum {
  background-color: var(--color-accent);
}

.metric-foot {
  font-size: 0.8125rem;
  color: var(--color-ink-3);
  font-variant-numeric: tabular-nums;
}

.workspace-grid {
  display: grid;
  grid-template-columns: minmax(0, 1.35fr) minmax(0, 1fr);
  gap: 1rem;
}

@media (max-width: 900px) {
  .workspace-grid {
    grid-template-columns: minmax(0, 1fr);
  }
}

.panel-head {
  display: flex;
  flex-wrap: wrap;
  align-items: flex-start;
  justify-content: space-between;
  gap: 0.75rem;
  margin-bottom: 1rem;
}

.panel-title {
  font-size: 1.0625rem;
  font-weight: 650;
  color: var(--color-ink);
}

.panel-desc {
  font-size: 0.875rem;
  color: var(--color-ink-2);
}

.chart-legend {
  display: flex;
  flex-wrap: wrap;
  gap: 0.5rem;
  font-size: 0.8125rem;
}

.legend-key {
  padding: 0.2rem 0.55rem;
  border-radius: var(--radius-sm);
  border: 1px solid var(--color-border);
}

.legend-soil {
  background-color: var(--color-accent-soft);
  color: var(--color-accent);
}

.legend-water {
  background-color: var(--color-water-soft);
  color: var(--color-water);
}

.legend-temp {
  background-color: var(--color-warn-soft);
  color: var(--color-warn);
}

.canvas-wrap {
  width: 100%;
  background-color: var(--color-surface-subtle);
  border: 1px solid var(--color-border);
  border-radius: var(--radius-sm);
  padding: 0.5rem;
}

#historyChart {
  display: block;
  width: 100%;
  height: 240px;
}

.control-row {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  justify-content: space-between;
  gap: 0.75rem;
  padding: 0.875rem 0;
  border-bottom: 1px solid var(--color-border);
}

.control-info {
  display: flex;
  flex-direction: column;
  gap: 0.15rem;
}

.control-label {
  font-size: 0.9375rem;
  font-weight: 600;
  color: var(--color-ink);
}

.control-sub {
  font-size: 0.8125rem;
  color: var(--color-ink-2);
}

.segmented-group {
  display: inline-flex;
  background-color: var(--color-surface-subtle);
  padding: 0.2rem;
  border-radius: var(--radius-sm);
  border: 1px solid var(--color-border);
}

button,
input {
  font-family: var(--font-sans);
  font-size: 0.875rem;
}

button {
  cursor: pointer;
  min-height: 38px;
  padding: 0.45rem 0.9rem;
  border-radius: var(--radius-sm);
  font-weight: 600;
  white-space: nowrap;
  border: 1px solid transparent;
  transition: background-color var(--dur-micro) ease, opacity var(--dur-micro) ease, transform var(--dur-micro) ease;
}

button:focus-visible,
input:focus-visible {
  outline: 2px solid var(--color-focus);
  outline-offset: 2px;
}

button:active:not(:disabled) {
  transform: translateY(1px);
}

button:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.btn-segment {
  background-color: transparent;
  color: var(--color-ink-2);
}

.btn-segment.is-selected {
  background-color: var(--color-surface);
  color: var(--color-ink);
  border-color: var(--color-border-strong);
}

.btn-primary {
  background-color: var(--color-accent);
  color: var(--color-accent-ink);
}

.btn-primary:hover:not(:disabled) {
  background-color: var(--color-accent-hover);
}

.btn-secondary,
.btn-preset {
  background-color: transparent;
  color: var(--color-ink);
  border-color: var(--color-border-strong);
}

.btn-secondary:hover:not(:disabled),
.btn-preset:hover:not(:disabled) {
  background-color: var(--color-surface-subtle);
}

.threshold-form {
  padding-top: 0.875rem;
  display: flex;
  flex-direction: column;
  gap: 0.75rem;
}

.form-title {
  font-size: 0.9375rem;
  font-weight: 600;
  color: var(--color-ink);
}

.input-grid {
  display: grid;
  grid-template-columns: repeat(3, minmax(0, 1fr));
  gap: 0.625rem;
}

.field {
  display: flex;
  flex-direction: column;
  gap: 0.25rem;
}

.field-label {
  font-size: 0.8125rem;
  color: var(--color-ink-2);
}

.field input {
  min-height: 38px;
  padding: 0.4rem 0.6rem;
  border: 1px solid var(--color-border-strong);
  border-radius: var(--radius-sm);
  background-color: var(--color-surface);
  color: var(--color-ink);
  font-variant-numeric: tabular-nums;
}

.form-actions {
  display: flex;
  align-items: center;
  gap: 0.75rem;
}

.form-feedback {
  font-size: 0.8125rem;
  color: var(--color-accent);
}

.sim-header {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  justify-content: space-between;
  gap: 0.75rem;
  margin-bottom: 0.875rem;
}

.sim-presets {
  display: flex;
  flex-wrap: wrap;
  gap: 0.625rem;
}

@media (prefers-reduced-motion: reduce) {
  *,
  *::before,
  *::after {
    transition-duration: 0.01ms !important;
    animation-duration: 0.01ms !important;
  }
}
)rawliteral";

const char FALLBACK_APP_JS[] PROGMEM = R"rawliteral(// Smart Garden Guard — Frontend Telemetry & Control (Polling 2s)
(function () {
  "use strict";

  const MAX_POINTS = 30;
  const history = {
    labels: ["Awal"],
    soil: [55],
    water: [72],
    temp: [28.5]
  };

  let currentState = {
    mode: "AUTO",
    pump: false,
    sim: false,
    thresholdsLoaded: false
  };

  // Referensi elemen DOM
  const el = {
    connBadge: document.getElementById("connBadge"),
    sysStatusBadge: document.getElementById("sysStatusBadge"),
    lastUpdateText: document.getElementById("lastUpdateText"),
    uptimeText: document.getElementById("uptimeText"),
    alarmBanner: document.getElementById("alarmBanner"),
    soilPctVal: document.getElementById("soilPctVal"),
    soilRawVal: document.getElementById("soilRawVal"),
    soilMeterBar: document.getElementById("soilMeterBar"),
    waterPctVal: document.getElementById("waterPctVal"),
    distanceCmVal: document.getElementById("distanceCmVal"),
    waterMeterBar: document.getElementById("waterMeterBar"),
    tempVal: document.getElementById("tempVal"),
    tempMeterBar: document.getElementById("tempMeterBar"),
    humVal: document.getElementById("humVal"),
    humMeterBar: document.getElementById("humMeterBar"),
    lblDry: document.getElementById("lblDry"),
    lblWet: document.getElementById("lblWet"),
    lblWaterMin: document.getElementById("lblWaterMin"),
    btnModeAuto: document.getElementById("btnModeAuto"),
    btnModeManual: document.getElementById("btnModeManual"),
    modeDescText: document.getElementById("modeDescText"),
    btnPumpToggle: document.getElementById("btnPumpToggle"),
    pumpStateText: document.getElementById("pumpStateText"),
    thresholdForm: document.getElementById("thresholdForm"),
    inSoilDry: document.getElementById("inSoilDry"),
    inSoilWet: document.getElementById("inSoilWet"),
    inWaterMin: document.getElementById("inWaterMin"),
    formFeedback: document.getElementById("formFeedback"),
    btnSimToggle: document.getElementById("btnSimToggle"),
    chartCanvas: document.getElementById("historyChart")
  };

  // Format detik uptime menjadi HH:MM:SS
  function formatDuration(totalSec) {
    const sec = Math.max(0, parseInt(totalSec, 10) || 0);
    const h = String(Math.floor(sec / 3600)).padStart(2, "0");
    const m = String(Math.floor((sec % 3600) / 60)).padStart(2, "0");
    const s = String(sec % 60).padStart(2, "0");
    return h + ":" + m + ":" + s;
  }

  // Ambil waktu lokal perangkat HH:MM:SS
  function getNowTimeStr() {
    const now = new Date();
    const h = String(now.getHours()).padStart(2, "0");
    const m = String(now.getMinutes()).padStart(2, "0");
    const s = String(now.getSeconds()).padStart(2, "0");
    return h + ":" + m + ":" + s;
  }

  // Atur panjang bar meter menggunakan transform scaleX (0.0 - 1.0)
  function setMeter(element, pct) {
    const clamped = Math.max(0, Math.min(100, Number(pct) || 0));
    element.style.transform = "scaleX(" + (clamped / 100).toFixed(3) + ")";
  }

  // Perbarui tampilan badge status kondisi sistem
  function updateStatusBadge(status) {
    el.sysStatusBadge.textContent = status;
    el.sysStatusBadge.className = "badge";
    if (status === "MENYIRAM") {
      el.sysStatusBadge.classList.add("badge-status-watering");
    } else if (status === "TANAH KERING") {
      el.sysStatusBadge.classList.add("badge-status-dry");
    } else if (status === "AIR TANDON HABIS") {
      el.sysStatusBadge.classList.add("badge-status-empty");
    } else {
      el.sysStatusBadge.classList.add("badge-status-normal");
    }
    el.alarmBanner.hidden = status !== "AIR TANDON HABIS";
  }

  // Gambar grafik riwayat menggunakan HTML5 Canvas native (100% offline)
  function drawChart() {
    const canvas = el.chartCanvas;
    if (!canvas || !canvas.getContext) return;
    const ctx = canvas.getContext("2d");
    const w = canvas.width;
    const h = canvas.height;
    const padL = 38;
    const padR = 16;
    const padT = 16;
    const padB = 28;
    const plotW = w - padL - padR;
    const plotH = h - padT - padB;

    ctx.clearRect(0, 0, w, h);

    // Garis bantu horizontal (0%, 25%, 50%, 75%, 100%)
    ctx.strokeStyle = "#d9dfd3";
    ctx.fillStyle = "#637060";
    ctx.font = "11px sans-serif";
    ctx.lineWidth = 1;

    const steps = [0, 25, 50, 75, 100];
    for (let i = 0; i < steps.length; i++) {
      const val = steps[i];
      const y = padT + plotH - (val / 100) * plotH;
      ctx.beginPath();
      ctx.moveTo(padL, y);
      ctx.lineTo(w - padR, y);
      ctx.stroke();
      ctx.fillText(String(val), 8, y + 4);
    }

    // Fungsi pembantu menggambar kurva garis
    function plotSeries(arr, strokeColor) {
      if (arr.length === 0) return;
      ctx.strokeStyle = strokeColor;
      ctx.lineWidth = 2.2;
      ctx.beginPath();
      for (let i = 0; i < arr.length; i++) {
        const x = arr.length === 1
          ? padL + plotW / 2
          : padL + (i / (MAX_POINTS - 1)) * plotW;
        const clamped = Math.max(0, Math.min(100, arr[i]));
        const y = padT + plotH - (clamped / 100) * plotH;
        if (i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      }
      ctx.stroke();
    }

    plotSeries(history.soil, "#1d6b42");
    plotSeries(history.water, "#18628c");
    plotSeries(history.temp, "#9c5708");

    // Label waktu di sumbu bawah
    if (history.labels.length > 0) {
      ctx.fillStyle = "#637060";
      ctx.fillText(history.labels[0], padL, h - 8);
      const lastLabel = history.labels[history.labels.length - 1];
      ctx.fillText(lastLabel, w - padR - 52, h - 8);
    }
  }

  // Terapkan data JSON dari ESP32 ke seluruh elemen UI
  function renderData(data) {
    el.connBadge.textContent = "Terhubung";
    el.connBadge.className = "badge badge-online";

    const nowStr = getNowTimeStr();
    el.lastUpdateText.textContent = nowStr;
    el.uptimeText.textContent = formatDuration(data.uptime);

    updateStatusBadge(data.status || "NORMAL");

    // Kartu sensor
    el.soilPctVal.textContent = data.soil_pct;
    el.soilRawVal.textContent = data.soil_raw !== undefined ? data.soil_raw : "-";
    setMeter(el.soilMeterBar, data.soil_pct);

    el.waterPctVal.textContent = data.water_level_pct;
    el.distanceCmVal.textContent = Number(data.distance_cm || 0).toFixed(1);
    setMeter(el.waterMeterBar, data.water_level_pct);

    el.tempVal.textContent = Number(data.temp_c || 0).toFixed(1);
    setMeter(el.tempMeterBar, (Number(data.temp_c || 0) / 50) * 100);

    el.humVal.textContent = Number(data.humidity_pct || 0).toFixed(1);
    setMeter(el.humMeterBar, data.humidity_pct);

    // Status mode & pompa
    currentState.mode = data.mode || "AUTO";
    currentState.pump = Boolean(data.pump);
    currentState.sim = Boolean(data.sim);

    const isAuto = currentState.mode === "AUTO";
    el.btnModeAuto.classList.toggle("is-selected", isAuto);
    el.btnModeManual.classList.toggle("is-selected", !isAuto);
    el.modeDescText.textContent = isAuto
      ? "Otomatis dengan hysteresis kelembapan tanah"
      : "Kendali manual langsung dari tombol dashboard";

    const tankEmpty = data.status === "AIR TANDON HABIS";
    el.btnPumpToggle.disabled = isAuto || tankEmpty;
    el.btnPumpToggle.textContent = currentState.pump ? "Matikan pompa" : "Nyalakan pompa";
    el.pumpStateText.textContent = currentState.pump
      ? "Pompa menyala (Relay GPIO19 ON)"
      : "Pompa mati (Relay GPIO19 OFF)";

    // Ambang batas
    if (data.thresholds) {
      el.lblDry.textContent = data.thresholds.soil_dry;
      el.lblWet.textContent = data.thresholds.soil_wet;
      el.lblWaterMin.textContent = data.thresholds.water_min;

      const activeTag = document.activeElement;
      if (
        !currentState.thresholdsLoaded &&
        activeTag !== el.inSoilDry &&
        activeTag !== el.inSoilWet &&
        activeTag !== el.inWaterMin
      ) {
        el.inSoilDry.value = data.thresholds.soil_dry;
        el.inSoilWet.value = data.thresholds.soil_wet;
        el.inWaterMin.value = data.thresholds.water_min;
        currentState.thresholdsLoaded = true;
      }
    }

    // Mode simulasi
    el.btnSimToggle.textContent = currentState.sim
      ? "Mode simulasi: Aktif (Klik untuk sensor fisik)"
      : "Mode sensor fisik (Klik untuk simulasi)";

    // Tambah ke riwayat grafik
    history.labels.push(nowStr);
    history.soil.push(Number(data.soil_pct) || 0);
    history.water.push(Number(data.water_level_pct) || 0);
    history.temp.push(Number(data.temp_c) || 0);

    if (history.labels.length > MAX_POINTS) {
      history.labels.shift();
      history.soil.shift();
      history.water.shift();
      history.temp.shift();
    }
    drawChart();
  }

  // Polling berkala GET /api/data setiap 2 detik
  async function fetchData() {
    try {
      const res = await fetch("/api/data", { cache: "no-store" });
      if (!res.ok) throw new Error("HTTP " + res.status);
      const data = await res.json();
      renderData(data);
    } catch (err) {
      el.connBadge.textContent = "Terputus (Offline)";
      el.connBadge.className = "badge badge-offline";
    }
  }

  // Kirim perintah POST JSON ke ESP32
  async function postJSON(url, payload) {
    const res = await fetch(url, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload)
    });
    if (!res.ok) throw new Error("Gagal mengirim perintah");
    await fetchData();
  }

  // Event Listeners
  el.btnModeAuto.addEventListener("click", function () {
    postJSON("/api/mode", { mode: "AUTO" });
  });

  el.btnModeManual.addEventListener("click", function () {
    postJSON("/api/mode", { mode: "MANUAL" });
  });

  el.btnPumpToggle.addEventListener("click", function () {
    postJSON("/api/pump", { state: !currentState.pump });
  });

  el.thresholdForm.addEventListener("submit", async function (e) {
    e.preventDefault();
    const dry = parseInt(el.inSoilDry.value, 10);
    const wet = parseInt(el.inSoilWet.value, 10);
    const waterMin = parseInt(el.inWaterMin.value, 10);

    if (dry >= wet) {
      el.formFeedback.textContent = "Ambang kering harus lebih kecil dari ambang basah.";
      return;
    }

    try {
      el.formFeedback.textContent = "Menyimpan...";
      await postJSON("/api/threshold", {
        soil_dry: dry,
        soil_wet: wet,
        water_min: waterMin
      });
      currentState.thresholdsLoaded = false;
      el.formFeedback.textContent = "Tersimpan di NVS.";
      setTimeout(function () {
        el.formFeedback.textContent = "";
      }, 3000);
    } catch (err) {
      el.formFeedback.textContent = "Gagal menyimpan.";
    }
  });

  el.btnSimToggle.addEventListener("click", function () {
    postJSON("/api/sim", { sim: !currentState.sim });
  });

  const presetBtns = document.querySelectorAll(".btn-preset");
  for (let i = 0; i < presetBtns.length; i++) {
    presetBtns[i].addEventListener("click", function () {
      const soil = parseInt(this.getAttribute("data-soil"), 10);
      const water = parseInt(this.getAttribute("data-water"), 10);
      postJSON("/api/sim", {
        sim: true,
        soil_pct: soil,
        water_level_pct: water
      });
    });
  }

  // Gambar grafik awal dan mulai polling tiap 2 detik
  drawChart();
  fetchData();
  setInterval(fetchData, 2000);
})();
)rawliteral";

#endif // WEB_FALLBACK_H

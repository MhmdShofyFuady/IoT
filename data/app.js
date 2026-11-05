// Smart Garden Guard — Frontend Telemetry & Control (Polling 2s)
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


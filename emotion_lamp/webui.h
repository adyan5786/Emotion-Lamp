#pragma once

/*
 * ============================================================
 *  webui.h — HTML/CSS/JS web interface (stored in flash)
 *
 *  This file contains the complete control page served at
 *  http://4.3.2.1 when connected to the Emotion-Lamp AP.
 *
 *  Edit the HTML/CSS here to customise the web UI.
 *  The PROGMEM keyword keeps this out of SRAM.
 * ============================================================
 */

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8"/>
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>Emotion Lamp</title>
  <style>
    /* ── Design tokens ── */
    :root {
      --bg:        #0d0d14;
      --surface:   #16162a;
      --border:    #2a2a48;
      --accent:    #7c6bff;
      --on-color:  #4fc3f7;
      --off-color: #ef5350;
      --text:      #e8e8f4;
      --sub:       #8888aa;
      --radius:    18px;
    }

    * { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      background: var(--bg);
      color: var(--text);
      font-family: 'Segoe UI', system-ui, sans-serif;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      padding: 24px;
    }

    /* ── Animated glow orb ── */
    .orb-wrap {
      position: relative;
      width: 130px;
      height: 130px;
      margin-bottom: 32px;
    }
    .orb {
      width: 130px;
      height: 130px;
      border-radius: 50%;
      background: radial-gradient(circle at 38% 38%,
        #a89cff 0%, #5c4fff 45%, #1a0070 100%);
      box-shadow: 0 0 60px 20px rgba(124,107,255,0.35);
      animation: pulse 3s ease-in-out infinite;
    }
    @keyframes pulse {
      0%,100% { box-shadow: 0 0 60px 20px rgba(124,107,255,0.35); }
      50%      { box-shadow: 0 0 90px 35px rgba(124,107,255,0.55); }
    }

    /* ── Card ── */
    .card {
      background: var(--surface);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      padding: 36px 28px;
      width: 100%;
      max-width: 380px;
      text-align: center;
      box-shadow: 0 8px 40px rgba(0,0,0,0.5);
    }

    h1 {
      font-size: 1.6rem;
      font-weight: 700;
      letter-spacing: 0.02em;
      margin-bottom: 6px;
    }
    h1 span { color: var(--accent); }

    .subtitle {
      font-size: 0.82rem;
      color: var(--sub);
      margin-bottom: 32px;
    }

    /* ── Status badge ── */
    .status-wrap {
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      margin-bottom: 32px;
    }
    .dot {
      width: 10px;
      height: 10px;
      border-radius: 50%;
      background: var(--on-color);
      animation: blink 1.4s ease-in-out infinite;
    }
    .dot.off { background: var(--off-color); animation: none; }
    @keyframes blink {
      0%,100% { opacity: 1; }
      50%      { opacity: 0.3; }
    }
    .status-text {
      font-size: 0.85rem;
      font-weight: 600;
      color: var(--sub);
      text-transform: uppercase;
      letter-spacing: 0.08em;
    }

    /* ── Buttons ── */
    .btn-group {
      display: flex;
      flex-direction: column;
      gap: 14px;
    }
    .btn {
      display: block;
      width: 100%;
      padding: 16px 24px;
      border: none;
      border-radius: 12px;
      font-size: 0.97rem;
      font-weight: 600;
      letter-spacing: 0.03em;
      cursor: pointer;
      transition: transform 0.12s ease, box-shadow 0.2s ease;
      text-decoration: none;
    }
    .btn:active { transform: scale(0.97); }

    .btn-on {
      background: linear-gradient(135deg, #4fc3f7 0%, #0288d1 100%);
      color: #fff;
      box-shadow: 0 4px 20px rgba(79,195,247,0.35);
    }
    .btn-on:hover { box-shadow: 0 6px 28px rgba(79,195,247,0.55); }

    .btn-off {
      background: linear-gradient(135deg, #ef5350 0%, #b71c1c 100%);
      color: #fff;
      box-shadow: 0 4px 20px rgba(239,83,80,0.30);
    }
    .btn-off:hover { box-shadow: 0 6px 28px rgba(239,83,80,0.50); }

    /* ── Footer ── */
    .footer {
      margin-top: 28px;
      font-size: 0.72rem;
      color: var(--sub);
      opacity: 0.6;
    }

    /* ── Toast notification ── */
    #toast {
      position: fixed;
      bottom: 28px;
      left: 50%;
      transform: translateX(-50%) translateY(80px);
      background: #23234a;
      border: 1px solid var(--border);
      color: var(--text);
      font-size: 0.82rem;
      padding: 10px 22px;
      border-radius: 30px;
      transition: transform 0.3s ease, opacity 0.3s ease;
      opacity: 0;
      pointer-events: none;
      white-space: nowrap;
    }
    #toast.show {
      transform: translateX(-50%) translateY(0);
      opacity: 1;
    }
  </style>
</head>
<body>

  <div class="orb-wrap">
    <div class="orb" id="orb"></div>
  </div>

  <div class="card">
    <h1>Emotion <span>Lamp</span></h1>
    <p class="subtitle">ESP32 &middot; INMP441 &middot; WS2812B &middot; 16 LEDs</p>

    <div class="status-wrap">
      <div class="dot" id="statusDot"></div>
      <span class="status-text" id="statusText">Audio Reactive</span>
    </div>

    <div class="btn-group">
      <button class="btn btn-on"  onclick="sendCmd('/on')">
        &#9654; &nbsp;Audio Reactive Mode (ON)
      </button>
      <button class="btn btn-off" onclick="sendCmd('/off')">
        &#9632; &nbsp;Turn OFF
      </button>
    </div>
  </div>

  <div class="footer">Connect via 4.3.2.1 &nbsp;&middot;&nbsp; Emotion Lamp v2.0</div>

  <div id="toast"></div>

  <script>
    function updateUI(on) {
      const dot = document.getElementById('statusDot');
      const txt = document.getElementById('statusText');
      const orb = document.getElementById('orb');
      if (on) {
        dot.classList.remove('off');
        txt.textContent = 'Audio Reactive';
        orb.style.animation   = '';
        orb.style.boxShadow   = '';
        orb.style.background  = '';
      } else {
        dot.classList.add('off');
        txt.textContent = 'Lamp OFF';
        orb.style.animation  = 'none';
        orb.style.boxShadow  = '0 0 20px 5px rgba(60,60,80,0.3)';
        orb.style.background = 'radial-gradient(circle at 38% 38%, #3a3a4a 0%, #1a1a2a 100%)';
      }
    }

    function showToast(msg) {
      const t = document.getElementById('toast');
      t.textContent = msg;
      t.classList.add('show');
      setTimeout(() => t.classList.remove('show'), 2200);
    }

    function sendCmd(path) {
      fetch(path)
        .then(r => r.text())
        .then(() => {
          const on = (path === '/on');
          updateUI(on);
          showToast(on ? '\u2736 Audio reactive mode enabled' : '\u2736 Lamp turned off');
        })
        .catch(() => showToast('\u26a0 Could not reach lamp'));
    }
  </script>
</body>
</html>
)rawliteral";

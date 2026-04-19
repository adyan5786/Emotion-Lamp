#pragma once

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1.0"/>
<title>Emotion Lamp</title>
<style>
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
:root{--bg:#111;--sf:#1a1a1a;--sf2:#222;--bd:#2c2c2c;--tx:#dedede;--dim:#555;--accent:#b8a0f0;--on:#7dff9a;--nav:64px;--side:200px}
html,body{height:100%;overflow:hidden}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:var(--bg);color:var(--tx);display:flex;height:100dvh}
.sidebar{display:none;width:var(--side);background:var(--sf);border-right:1px solid var(--bd);flex-direction:column;flex-shrink:0}
.sb-head{padding:22px 16px 18px;border-bottom:1px solid var(--bd)}
.sb-head h1{font-size:.88rem;font-weight:700;letter-spacing:.08em;text-transform:uppercase}
.sb-head p{font-size:.7rem;color:var(--dim);margin-top:3px}
.sb-nav{padding:8px 0;flex:1}
.ni{display:flex;align-items:center;gap:10px;padding:10px 16px;cursor:pointer;color:var(--dim);font-size:.84rem;font-weight:500;border-left:3px solid transparent;transition:all .15s;user-select:none}
.ni:hover{background:var(--sf2);color:var(--tx)}
.ni.active{color:var(--accent);border-left-color:var(--accent);background:rgba(184,160,240,.07)}
.content{flex:1;overflow-y:auto;display:flex;flex-direction:column;padding-bottom:var(--nav)}
.panel{display:none;flex-direction:column;flex:1}
.panel.active{display:flex}
.ph{display:flex;align-items:center;justify-content:space-between;padding:20px 20px 0}
.ph h2{font-size:1.15rem;font-weight:700}
.badge{font-size:.67rem;font-weight:700;letter-spacing:.06em;padding:4px 10px;border-radius:20px;border:1px solid var(--bd);color:var(--dim);background:rgba(255,255,255,.04);transition:all .3s;text-transform:uppercase}
.badge.on{color:var(--on);border-color:#2d6b3a;background:rgba(100,220,130,.08)}
.pw-wrap{display:flex;flex-direction:column;align-items:center;justify-content:center;gap:16px;padding:28px 20px 20px}
.pw-btn{width:120px;height:120px;border-radius:50%;border:2.5px solid #333;background:var(--sf);display:flex;align-items:center;justify-content:center;cursor:pointer;transition:border-color .3s,box-shadow .3s,transform .1s;-webkit-tap-highlight-color:transparent;user-select:none}
.pw-btn:active{transform:scale(.93)}
.pw-btn svg{width:48px;height:48px;stroke:#444;transition:stroke .3s}
.pw-btn.on{border-color:#ccc;box-shadow:0 0 0 5px rgba(255,255,255,.05),0 0 22px rgba(255,255,255,.09)}
.pw-btn.on svg{stroke:#e0e0e0}
.plbl{font-size:.73rem;font-weight:700;letter-spacing:.12em;color:var(--dim);text-transform:uppercase;transition:color .3s}
.plbl.on{color:var(--tx)}
.sec{padding:0 20px 20px}
.sec-title{font-size:.68rem;font-weight:700;letter-spacing:.1em;text-transform:uppercase;color:var(--dim);margin-bottom:10px;padding-bottom:8px;border-bottom:1px solid var(--bd)}
.zone-row{display:flex;align-items:center;gap:12px;padding:9px 0;border-bottom:1px solid rgba(255,255,255,.04)}
.zone-row:last-child{border-bottom:none}
.zswatch{width:20px;height:20px;border-radius:5px;flex-shrink:0;border:1px solid rgba(255,255,255,.1)}
.zinfo{flex:1}
.zname{font-size:.83rem;font-weight:600;color:var(--tx);display:block}
.zdesc{font-size:.68rem;color:var(--dim)}
.zlbl{font-size:.7rem;color:var(--dim);white-space:nowrap}
/* zone button (custom editor) */
.zone-btn{cursor:pointer;border-radius:8px;padding:9px 8px;transition:background .15s}
.zone-btn:hover{background:var(--sf2)}
/* mode selector */
.mode-row{display:flex;flex-direction:column;gap:8px;padding-top:12px}
.mdbtn{flex:1;display:flex;align-items:center;gap:10px;padding:15px 18px;border-radius:12px;border:2px solid var(--bd);background:var(--sf);cursor:pointer;transition:all .2s;user-select:none;-webkit-tap-highlight-color:transparent}
.mdbtn.active{border-color:var(--accent);background:rgba(184,160,240,.09)}
.mdindicator{width:13px;height:13px;border-radius:50%;border:2px solid var(--dim);flex-shrink:0;transition:all .2s}
.mdbtn.active .mdindicator{background:var(--accent);border-color:var(--accent)}
.mdlabel{flex:1;font-size:.84rem;font-weight:600;color:var(--dim);transition:color .2s}
.mdbtn.active .mdlabel{color:var(--tx)}
.pencil-btn{width:26px;height:26px;border-radius:50%;border:1px solid var(--bd);background:var(--sf2);display:flex;align-items:center;justify-content:center;cursor:pointer;transition:background .15s,border-color .15s;flex-shrink:0}
.pencil-btn:hover{background:var(--accent);border-color:var(--accent)}
.pencil-btn svg{stroke:var(--dim);transition:stroke .15s}
.pencil-btn:hover svg{stroke:#fff}
/* slider */
.slider-wrap{display:flex;align-items:center;gap:12px;margin-top:12px}
.slider{flex:1;-webkit-appearance:none;appearance:none;height:4px;border-radius:2px;background:var(--bd);outline:none;cursor:pointer}
.slider::-webkit-slider-thumb{-webkit-appearance:none;appearance:none;width:18px;height:18px;border-radius:50%;background:var(--tx);cursor:pointer}
.slider::-moz-range-thumb{width:18px;height:18px;border-radius:50%;background:var(--tx);cursor:pointer;border:none}
.bval{font-size:.88rem;font-weight:700;min-width:28px;text-align:right}
/* back/save button */
.back-btn{flex:1;padding:13px;border-radius:10px;border:1px solid var(--bd);background:var(--sf2);color:var(--tx);font-size:.88rem;font-weight:600;cursor:pointer;transition:background .15s,border-color .15s}
.back-btn:hover{background:var(--sf2);border-color:var(--dim)}
.save-btn{flex:1;padding:13px;border-radius:10px;border:1px solid var(--accent);background:rgba(184,160,240,.08);color:var(--accent);font-size:.88rem;font-weight:600;cursor:pointer;transition:all .2s}
.save-btn:hover{background:var(--accent);color:#111}
/* empty */
.empty{flex:1;display:flex;flex-direction:column;align-items:center;justify-content:center;gap:12px;padding:40px;color:var(--dim)}
.empty svg{opacity:.18}
.empty h3{font-size:.92rem;font-weight:600}
.empty p{font-size:.77rem;text-align:center;max-width:220px;line-height:1.5}
/* bottom nav */
.bnav{position:fixed;bottom:0;left:0;right:0;height:var(--nav);background:var(--sf);border-top:1px solid var(--bd);display:flex;z-index:50}
.bi{flex:1;display:flex;flex-direction:column;align-items:center;justify-content:center;gap:3px;cursor:pointer;color:var(--dim);font-size:.59rem;font-weight:600;letter-spacing:.04em;text-transform:uppercase;transition:color .15s;user-select:none;-webkit-tap-highlight-color:transparent}
.bi.active{color:var(--accent)}
.bi.active svg{stroke:var(--accent)!important}
@media(min-width:640px){.sidebar{display:flex}.bnav{display:none}.content{padding-bottom:0}}
/* toast notification */
.toast{position:fixed;top:-70px;left:50%;transform:translateX(-50%);z-index:400;background:var(--sf);border:1px solid #3a6b3a;border-radius:10px;padding:11px 22px;font-size:.84rem;font-weight:600;color:#7dff9a;transition:top .4s cubic-bezier(.22,1,.36,1);white-space:nowrap;box-shadow:0 6px 24px rgba(0,0,0,.5);pointer-events:none}
.toast.show{top:16px}
/* hue slider */
#hue-slider{-webkit-appearance:none;appearance:none;width:100%;height:14px;border-radius:7px;border:none;outline:none;cursor:pointer;background:linear-gradient(to right,hsl(0,100%,50%),hsl(60,100%,50%),hsl(120,100%,50%),hsl(180,100%,50%),hsl(240,100%,50%),hsl(300,100%,50%),hsl(360,100%,50%))}
#hue-slider::-webkit-slider-thumb{-webkit-appearance:none;width:18px;height:18px;border-radius:50%;background:#fff;border:2px solid rgba(0,0,0,.35);cursor:pointer;box-shadow:0 1px 4px rgba(0,0,0,.4)}
#hue-slider::-moz-range-thumb{width:18px;height:18px;border-radius:50%;background:#fff;border:2px solid rgba(0,0,0,.35);cursor:pointer;border:none}
/* rgb number inputs */
.inp-rgb{background:var(--sf2);border:1px solid var(--bd);border-radius:7px;color:var(--tx);font-size:.85rem;font-weight:600;padding:8px 4px;text-align:center;outline:none;width:100%;-moz-appearance:textfield;transition:border-color .15s}
.inp-rgb:focus{border-color:var(--accent)}
.inp-rgb::-webkit-outer-spin-button,.inp-rgb::-webkit-inner-spin-button{-webkit-appearance:none}
</style>
</head>
<body>

<nav class="sidebar">
  <div class="sb-head"><h1>Emotion Lamp</h1><p>ESP32 &middot; 4.3.2.1</p></div>
  <div class="sb-nav">
    <div class="ni active" onclick="go('home',this)">
      <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/><polyline points="9 22 9 12 15 12 15 22"/></svg>Home
    </div>
    <div class="ni" onclick="go('effects',this)">
      <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"/></svg>Effects
    </div>
    <div class="ni" onclick="go('settings',this)">
      <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>Settings
    </div>
  </div>
</nav>

<main class="content">

  <!-- HOME -->
  <div id="p-home" class="panel active">
    <div class="ph"><h2>Home</h2><span class="badge" id="badge">OFF</span></div>
    <div class="pw-wrap">
      <div class="pw-btn" id="pwrBtn" onclick="togglePower()">
        <svg viewBox="0 0 24 24" fill="none" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round">
          <path d="M18.36 6.64a9 9 0 1 1-12.73 0"/><line x1="12" y1="2" x2="12" y2="12"/>
        </svg>
      </div>
      <span class="plbl" id="plbl">STANDBY</span>
      <span id="profile-badge" style="font-size:.68rem;color:var(--dim);background:var(--sf2);padding:4px 14px;border-radius:20px;border:1px solid var(--bd);margin-top:-4px;transition:all .3s">Profile: Default</span>
    </div>
    <div class="sec">
      <div class="sec-title">Colour Zones</div>
      <div id="zones-list"><div class="zone-row" style="justify-content:center;padding:16px 0"><span style="color:var(--dim);font-size:.78rem">Loading...</span></div></div>
    </div>
  </div>

  <!-- EFFECTS -->
  <div id="p-effects" class="panel">
    <div class="ph"><h2>Effects</h2></div>

    <!-- Main effects view -->
    <div id="fx-main" style="display:flex;flex-direction:column;flex:1">

      <!-- Brightness (top) -->
      <div class="sec" style="padding-top:18px">
        <div class="sec-title">Brightness</div>
        <div class="slider-wrap">
          <span style="font-size:.9rem">&#9681;</span>
          <input type="range" id="brightSlider" class="slider" min="0" max="255" value="140" oninput="onBright(this.value)"/>
          <span style="font-size:.9rem">&#9728;</span>
          <span class="bval" id="bval">140</span>
        </div>
      </div>

      <!-- Mode selector -->
      <div class="sec">
        <div class="sec-title">Profile</div>
        <div class="mode-row" id="mode-row"><!-- populated by renderModeButtons() --></div>
      </div>
    </div>

    <!-- Custom colours editor view (hidden by default) -->
    <div id="fx-custom" style="display:none;flex-direction:column;flex:1;overflow-y:auto">
      <div class="ph" style="padding-bottom:12px;gap:8px">
        <input id="profile-name-input" type="text" maxlength="11" placeholder="Profile name"
          style="font-size:1rem;font-weight:700;background:transparent;border:none;border-bottom:2px solid var(--bd);color:var(--tx);outline:none;padding:2px 4px;flex:1;width:100%"
          oninput="onNameInput(this.value)" onblur="commitProfileName(this.value)">
      </div>
      <div class="sec" style="flex:1">
        <div class="sec-title">Tap a zone to edit its colour</div>
        <div id="custom-zones-list"></div>
      </div>
      <div style="display:flex;gap:10px;padding:12px 20px 24px">
        <button class="back-btn" onclick="backFromCustom()">&#8592; Back</button>
        <button class="save-btn" id="save-profile-btn" onclick="saveProfile()">Save Profile</button>
      </div>
    </div>
  </div>

  <!-- SETTINGS -->
  <div id="p-settings" class="panel">
    <div class="ph"><h2>Settings</h2></div>
    <div class="empty">
      <svg width="44" height="44" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>
      <h3>Settings</h3>
      <p>Device configuration options will be available here in a future update.</p>
    </div>
  </div>

</main>

<!-- Toast notification -->
<div id="toast" class="toast">&#10003; Profile Saved</div>

<!-- Unsaved changes warning modal -->
<div id="warn-modal" style="display:none;position:fixed;inset:0;z-index:300;background:rgba(0,0,0,.75);align-items:center;justify-content:center;padding:20px">
  <div style="background:var(--sf);border:1px solid var(--bd);border-radius:16px;width:min(300px,100%);overflow:hidden">
    <div style="padding:20px 20px 14px">
      <div style="display:flex;align-items:center;gap:10px;margin-bottom:10px">
        <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="#f0a050" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"/><line x1="12" y1="9" x2="12" y2="13"/><line x1="12" y1="17" x2="12.01" y2="17"/></svg>
        <span style="font-size:.96rem;font-weight:700">Unsaved Changes</span>
      </div>
      <p style="font-size:.8rem;color:var(--dim);line-height:1.55">You have unsaved colour changes. Exiting will discard them — the lamp's staged colours will reset on next boot.</p>
    </div>
    <div style="display:flex;gap:10px;padding:0 20px 20px">
      <button onclick="warnStay()" style="flex:1;padding:12px;border-radius:10px;border:1px solid var(--bd);background:var(--sf2);color:var(--tx);font-size:.85rem;font-weight:600;cursor:pointer">Stay</button>
      <button onclick="warnLeave()" style="flex:1;padding:12px;border-radius:10px;border:1px solid #7a3a3a;background:rgba(180,60,60,.12);color:#e07070;font-size:.85rem;font-weight:600;cursor:pointer">Exit Anyway</button>
    </div>
  </div>
</div>

<!-- Zone edit modal -->
<div id="zone-modal" style="display:none;position:fixed;inset:0;z-index:200;background:rgba(0,0,0,.72);align-items:center;justify-content:center;padding:20px">
  <div style="background:var(--sf);border:1px solid var(--bd);border-radius:16px;width:min(340px,100%);display:flex;flex-direction:column;overflow:hidden">
    <!-- Modal header -->
    <div style="display:flex;align-items:center;justify-content:space-between;padding:16px 16px 14px;border-bottom:1px solid var(--bd)">
      <span id="modal-title" style="font-size:.96rem;font-weight:700"></span>
      <button onclick="closeModal()" style="width:28px;height:28px;border-radius:50%;border:1px solid var(--bd);background:var(--sf2);color:var(--tx);font-size:1.1rem;line-height:1;cursor:pointer;display:flex;align-items:center;justify-content:center">&times;</button>
    </div>
    <!-- Modal body: Colour picker -->
    <div style="padding:16px">
      <!-- 2D saturation/value canvas -->
      <canvas id="picker-canvas" width="300" height="170"
        style="width:100%;border-radius:8px;cursor:crosshair;display:block;touch-action:none;
               border:1px solid var(--bd)">
      </canvas>
      <!-- Hue slider -->
      <div style="margin-top:12px">
        <input type="range" id="hue-slider" min="0" max="360" step="1" value="0"
               oninput="onPickerHue(this.value)">
      </div>
      <!-- Preview swatch + RGB inputs -->
      <div style="display:flex;align-items:center;gap:12px;margin-top:14px">
        <div id="picker-preview" style="width:38px;height:38px;border-radius:50%;border:2px solid var(--bd);flex-shrink:0"></div>
        <div style="display:flex;gap:8px;flex:1">
          <div style="display:flex;flex-direction:column;align-items:center;flex:1">
            <label style="font-size:.6rem;color:var(--dim);letter-spacing:.06em;margin-bottom:5px">R</label>
            <input type="number" id="inp-r" class="inp-rgb" min="0" max="255" value="0" oninput="onRgbInput()">
          </div>
          <div style="display:flex;flex-direction:column;align-items:center;flex:1">
            <label style="font-size:.6rem;color:var(--dim);letter-spacing:.06em;margin-bottom:5px">G</label>
            <input type="number" id="inp-g" class="inp-rgb" min="0" max="255" value="0" oninput="onRgbInput()">
          </div>
          <div style="display:flex;flex-direction:column;align-items:center;flex:1">
            <label style="font-size:.6rem;color:var(--dim);letter-spacing:.06em;margin-bottom:5px">B</label>
            <input type="number" id="inp-b" class="inp-rgb" min="0" max="255" value="0" oninput="onRgbInput()">
          </div>
        </div>
      </div>
    </div>
    <!-- Modal footer -->
    <div style="padding:12px 16px 16px;border-top:1px solid var(--bd)">
      <button onclick="saveZone()" style="width:100%;padding:12px;border-radius:10px;border:1px solid var(--accent);background:rgba(184,160,240,.1);color:var(--accent);font-size:.88rem;font-weight:700;cursor:pointer;transition:background .15s">Set</button>
    </div>
  </div>
</div>

<nav class="bnav">
  <div class="bi active" id="b-home" onclick="go('home',this,1)">
    <svg width="21" height="21" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/><polyline points="9 22 9 12 15 12 15 22"/></svg>Home
  </div>
  <div class="bi" id="b-effects" onclick="go('effects',this,1)">
    <svg width="21" height="21" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"/></svg>Effects
  </div>
  <div class="bi" id="b-settings" onclick="go('settings',this,1)">
    <svg width="21" height="21" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>Settings
  </div>
</nav>

<script>
var lampOn=false,tabs=['home','effects','settings'],btimer=null;
var sliderInteracted=false,lastZones=null;

// ── Tab navigation ────────────────────────────────────────────
var pendingNav=null; // stores blocked navigation when profileDirty
function go(name,el,mobile){
  // If the custom editor is open with unsaved changes, intercept the tab switch
  if(profileDirty&&document.getElementById('fx-custom').style.display!=='none'){
    pendingNav={name:name,el:el,mobile:mobile};
    showWarnModal();
    return;
  }
  tabs.forEach(function(t){
    document.getElementById('p-'+t).classList.remove('active');
    var b=document.getElementById('b-'+t);if(b)b.classList.remove('active');
  });
  document.querySelectorAll('.ni').forEach(function(n){n.classList.remove('active');});
  document.getElementById('p-'+name).classList.add('active');
  var b=document.getElementById('b-'+name);if(b)b.classList.add('active');
  if(mobile){
    var idx=tabs.indexOf(name);
    document.querySelectorAll('.ni')[idx].classList.add('active');
  }else{el.classList.add('active');}
}

// ── Power ─────────────────────────────────────────────────────
function setUI(on){
  lampOn=on;
  var btn=document.getElementById('pwrBtn');
  var lbl=document.getElementById('plbl');
  var bdg=document.getElementById('badge');
  if(on){btn.classList.add('on');lbl.classList.add('on');lbl.textContent='ACTIVE';bdg.textContent='ON';bdg.classList.add('on');}
  else{btn.classList.remove('on');lbl.classList.remove('on');lbl.textContent='STANDBY';bdg.textContent='OFF';bdg.classList.remove('on');}
}
function togglePower(){
  fetch(lampOn?'/off':'/on').then(function(){setUI(!lampOn);}).catch(function(){});
}

// ── Zones (home tab) ──────────────────────────────────────────
function renderZones(zones){
  lastZones=zones;
  var list=document.getElementById('zones-list');
  if(!list||!zones)return;
  list.innerHTML='';
  zones.forEach(function(z){
    var d=document.createElement('div');
    d.className='zone-row';
    d.innerHTML='<div class="zswatch" style="background:'+z.hex+'"></div>'
      +'<div class="zinfo"><span class="zname">'+z.name+'</span><span class="zdesc">'+z.desc+'</span></div>'
      +'<span class="zlbl">'+z.label+'</span>';
    list.appendChild(d);
  });
}

// ── Custom zones (effects editor) ─────────────────────────────
// Uses customZonesCache (from d.customColors in /status) so it always
// shows the true custom profile, not the active mode's colours.
function renderCustomZones(){
  var list=document.getElementById('custom-zones-list');
  if(!list)return;
  list.innerHTML='';
  var src=customZonesCache;
  if(!src&&lastZones){src=lastZones.slice(1);} // fallback
  if(!src)return;
  src.forEach(function(z,pi){
    var d=document.createElement('div');
    d.className='zone-row zone-btn';
    d.innerHTML='<div class="zswatch" id="czsw-'+pi+'" style="background:'+z.hex+'"></div>'
      +'<div class="zinfo"><span class="zname">'+z.name+'</span><span class="zdesc">'+z.desc+'</span></div>'
      +'<span class="zlbl" id="czlbl-'+pi+'">'+z.label+'</span>';
    d.onclick=(function(zone,profIdx){return function(){openZoneModal(zone,profIdx);};})(z,pi);
    list.appendChild(d);
  });
}

// ── Zone modal + Colour Picker ───────────────────────────────
var activeZone=null, activeZoneIdx=-1;
var profileDirty=false;
var customZonesCache=null;   // always holds true custom profile colours
var allProfiles=null;        // d.allProfiles from last /status response
var profileNames=null;       // d.profileNames from last /status response
var activeProfileIdx=0;      // which custom profile (0-4) is being edited
var originalProfileName='';  // name snapshot when editor opens, for discard
var pickerH=0,pickerS=1,pickerV=1,pickerDrag=false;

function h2(v){var s=Math.round(v).toString(16);return s.length<2?'0'+s:s;}

function hsvToRgb(h,s,v){
  var i=Math.floor(h/60)%6,f=h/60-Math.floor(h/60);
  var p=v*(1-s),q=v*(1-f*s),t=v*(1-(1-f)*s);
  var r,g,b;
  switch(i){
    case 0:r=v;g=t;b=p;break;case 1:r=q;g=v;b=p;break;
    case 2:r=p;g=v;b=t;break;case 3:r=p;g=q;b=v;break;
    case 4:r=t;g=p;b=v;break;default:r=v;g=p;b=q;
  }
  return[Math.round(r*255),Math.round(g*255),Math.round(b*255)];
}

function rgbToHsv(r,g,b){
  r/=255;g/=255;b/=255;
  var max=Math.max(r,g,b),min=Math.min(r,g,b),d=max-min,h,s=max===0?0:d/max,v=max;
  if(max===min){h=0;}else{
    switch(max){
      case r:h=(g-b)/d+(g<b?6:0);break;
      case g:h=(b-r)/d+2;break;
      default:h=(r-g)/d+4;
    }h*=60;
  }
  return[h,s,v];
}

function drawPicker(){
  var c=document.getElementById('picker-canvas');if(!c)return;
  var ctx=c.getContext('2d'),w=c.width,h=c.height;
  var hc='hsl('+pickerH+',100%,50%)';
  var gH=ctx.createLinearGradient(0,0,w,0);
  gH.addColorStop(0,'#fff');gH.addColorStop(1,hc);
  ctx.fillStyle=gH;ctx.fillRect(0,0,w,h);
  var gV=ctx.createLinearGradient(0,0,0,h);
  gV.addColorStop(0,'rgba(0,0,0,0)');gV.addColorStop(1,'#000');
  ctx.fillStyle=gV;ctx.fillRect(0,0,w,h);
  var cx=pickerS*w,cy=(1-pickerV)*h;
  ctx.beginPath();ctx.arc(cx,cy,7,0,Math.PI*2);
  ctx.strokeStyle='#fff';ctx.lineWidth=2;ctx.stroke();
  ctx.beginPath();ctx.arc(cx,cy,8.5,0,Math.PI*2);
  ctx.strokeStyle='rgba(0,0,0,.35)';ctx.lineWidth=1;ctx.stroke();
}

function updatePickerOutputs(){
  var rgb=hsvToRgb(pickerH,pickerS,pickerV);
  document.getElementById('inp-r').value=rgb[0];
  document.getElementById('inp-g').value=rgb[1];
  document.getElementById('inp-b').value=rgb[2];
  document.getElementById('picker-preview').style.background='#'+h2(rgb[0])+h2(rgb[1])+h2(rgb[2]);
  drawPicker();
}

function pickerMove(e){
  var c=document.getElementById('picker-canvas');
  var rect=c.getBoundingClientRect();
  var cx=e.touches?e.touches[0].clientX:e.clientX;
  var cy=e.touches?e.touches[0].clientY:e.clientY;
  pickerS=Math.max(0,Math.min(1,(cx-rect.left)/rect.width));
  pickerV=Math.max(0,Math.min(1,1-(cy-rect.top)/rect.height));
  updatePickerOutputs();
}

function onPickerHue(val){
  pickerH=parseFloat(val);
  updatePickerOutputs();
}

function onRgbInput(){
  var r=Math.max(0,Math.min(255,parseInt(document.getElementById('inp-r').value)||0));
  var g=Math.max(0,Math.min(255,parseInt(document.getElementById('inp-g').value)||0));
  var b=Math.max(0,Math.min(255,parseInt(document.getElementById('inp-b').value)||0));
  var hsv=rgbToHsv(r,g,b);
  pickerH=hsv[0];pickerS=hsv[1];pickerV=hsv[2];
  document.getElementById('hue-slider').value=pickerH;
  document.getElementById('picker-preview').style.background='#'+h2(r)+h2(g)+h2(b);
  drawPicker();
}

function openZoneModal(zone,profIdx){
  activeZoneIdx=(profIdx!==undefined)?profIdx:-1;
  // Always read fresh colour from cache to fix stale-closure bug
  var z=(customZonesCache&&customZonesCache[profIdx])?customZonesCache[profIdx]:zone;
  activeZone=z;
  document.getElementById('modal-title').textContent=z.name;
  var hx=z.hex,r=parseInt(hx.slice(1,3),16),g=parseInt(hx.slice(3,5),16),b=parseInt(hx.slice(5,7),16);
  var hsv=rgbToHsv(r,g,b);
  pickerH=hsv[0];pickerS=hsv[1];pickerV=hsv[2];
  document.getElementById('hue-slider').value=pickerH;
  document.getElementById('zone-modal').style.display='flex';
  setTimeout(function(){
    var c=document.getElementById('picker-canvas');
    c.width=c.offsetWidth;
    updatePickerOutputs();
  },10);
}
function closeModal(){
  document.getElementById('zone-modal').style.display='none';
  activeZone=null; activeZoneIdx=-1;
}
function saveZone(){
  var r=Math.max(0,Math.min(255,parseInt(document.getElementById('inp-r').value)||0));
  var g=Math.max(0,Math.min(255,parseInt(document.getElementById('inp-g').value)||0));
  var b=Math.max(0,Math.min(255,parseInt(document.getElementById('inp-b').value)||0));
  if(activeZoneIdx<0){closeModal();return;}
  fetch('/setzone?idx='+activeZoneIdx+'&r='+r+'&g='+g+'&b='+b).catch(function(){});
  var hex='#'+h2(r)+h2(g)+h2(b);
  var lbl='('+r+', '+g+', '+b+')';
  // Update the rendered list rows immediately
  var sw=document.getElementById('czsw-'+activeZoneIdx);
  var lb=document.getElementById('czlbl-'+activeZoneIdx);
  if(sw)sw.style.background=hex;
  if(lb)lb.textContent=lbl;
  // Update customZonesCache so re-renders stay correct
  if(customZonesCache&&customZonesCache[activeZoneIdx]){
    customZonesCache[activeZoneIdx].hex=hex;
    customZonesCache[activeZoneIdx].label=lbl;
  }
  profileDirty=true;
  closeModal();
}

// ── Mode selector (numeric: 0=default, 1-5=custom) ────────────────
// skipFetch=true when called from applyStatus (UI-only sync, no server call).
function selectMode(mode,skipFetch){
  for(var i=0;i<=5;i++){
    var b=document.getElementById('mode-'+i);
    if(b)b.classList.toggle('active',i===mode);
  }
  if(!skipFetch){
    fetch('/setmode?m='+mode)
      .then(function(){loadStatus();})
      .catch(function(){});
  }
}

// ── Mode buttons renderer ────────────────────────────────────────
var PENCIL_SVG='<svg width="11" height="11" viewBox="0 0 24 24" fill="none" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"><path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"/><path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"/></svg>';
function renderModeButtons(activeMode,names){
  var row=document.getElementById('mode-row');
  if(!row)return;
  row.innerHTML='';
  // Default
  var def=document.createElement('div');
  def.className='mdbtn'+(activeMode===0?' active':'');
  def.id='mode-0';
  def.onclick=function(){selectMode(0);};
  def.innerHTML='<div class="mdindicator"></div><span class="mdlabel">Default</span>';
  row.appendChild(def);
  // Custom 1-5
  for(var i=0;i<5;i++){
    var mn=i+1;
    var nm=(names&&names[i])?names[i]:('Custom '+mn);
    var btn=document.createElement('div');
    btn.className='mdbtn'+(activeMode===mn?' active':'');
    btn.id='mode-'+mn;
    (function(modeNum,pi){
      btn.onclick=function(){selectMode(modeNum);};
      var pb=document.createElement('div');
      pb.className='pencil-btn';
      pb.title='Edit '+nm;
      pb.innerHTML=PENCIL_SVG;
      pb.onclick=function(e){openCustomEditor(pi,e);};
      btn.innerHTML='<div class="mdindicator"></div><span class="mdlabel" id="mode-lbl-'+modeNum+'">'+nm+'</span>';
      btn.appendChild(pb);
    })(mn,i);
    row.appendChild(btn);
  }
}

// ── Custom editor open/close ──────────────────────────────────
function buildZonesFromRgb(rgbArr){
  if(!rgbArr||!lastZones)return null;
  return rgbArr.map(function(rgb,i){
    var z=lastZones[i+1]||{name:'Zone '+(i+1),desc:''};
    var r=rgb[0],g=rgb[1],b=rgb[2];
    var hex='#'+h2(r)+h2(g)+h2(b);
    return{name:z.name,desc:z.desc,hex:hex,label:'('+r+','+g+','+b+')'};
  });
}
function openCustomEditor(profileIdx,evt){
  if(evt){evt.stopPropagation();}
  activeProfileIdx=profileIdx;
  var modeNum=profileIdx+1;
  selectMode(modeNum);
  if(allProfiles&&allProfiles[profileIdx]){
    customZonesCache=buildZonesFromRgb(allProfiles[profileIdx]);
  }
  var inp=document.getElementById('profile-name-input');
  var nm=(profileNames&&profileNames[profileIdx])||('Custom '+modeNum);
  originalProfileName=nm; // snapshot for discard
  if(inp)inp.value=nm;
  document.getElementById('fx-main').style.display='none';
  document.getElementById('fx-custom').style.display='flex';
  profileDirty=false;
  renderCustomZones();
}
function onNameInput(v){
  var lbl=document.getElementById('mode-lbl-'+(activeProfileIdx+1));
  if(lbl)lbl.textContent=v||('Custom '+(activeProfileIdx+1));
  if(profileNames)profileNames[activeProfileIdx]=v;
  profileDirty=true; // name change counts as unsaved
}
function commitProfileName(v){
  v=(v||'').trim().substring(0,11);
  fetch('/setname?p='+activeProfileIdx+'&name='+encodeURIComponent(v)).catch(function(){});
}
function showWarnModal(){
  document.getElementById('warn-modal').style.display='flex';
}
function warnStay(){
  document.getElementById('warn-modal').style.display='none';
}
function warnLeave(){
  document.getElementById('warn-modal').style.display='none';
  profileDirty=false;
  // Tell ESP32 to reload this profile from NVS — truly reverts all staged changes
  fetch('/discardprofile?p='+activeProfileIdx)
    .then(function(){loadStatus();}) // refreshes UI with clean NVS data
    .catch(function(){});
  // Restore the name label immediately (don't wait for loadStatus)
  if(profileNames)profileNames[activeProfileIdx]=originalProfileName;
  var lbl=document.getElementById('mode-lbl-'+(activeProfileIdx+1));
  if(lbl)lbl.textContent=originalProfileName||('Custom '+(activeProfileIdx+1));
  var inp=document.getElementById('profile-name-input');
  if(inp)inp.value=originalProfileName;
  document.getElementById('fx-custom').style.display='none';
  document.getElementById('fx-main').style.display='flex';
  if(pendingNav){
    var nav=pendingNav; pendingNav=null;
    go(nav.name,nav.el,nav.mobile);
  }
}
function backFromCustom(){
  if(profileDirty){pendingNav=null;showWarnModal();return;}
  document.getElementById('fx-custom').style.display='none';
  document.getElementById('fx-main').style.display='flex';
}
function showToast(msg){
  var t=document.getElementById('toast');
  t.textContent=msg;
  t.classList.add('show');
  setTimeout(function(){t.classList.remove('show');},3000);
}
function saveProfile(){
  fetch('/saveprofile')
    .then(function(){
      profileDirty=false;
      // Return to home tab
      document.getElementById('fx-custom').style.display='none';
      document.getElementById('fx-main').style.display='flex';
      go('home',document.getElementById('b-home'),1);
      showToast('\u2713  Profile Saved');
      loadStatus(); // refresh home tab zone colours
    }).catch(function(){});
}

// ── Brightness slider ─────────────────────────────────────────
function onBright(val){
  sliderInteracted=true;
  document.getElementById('bval').textContent=val;
  clearTimeout(btimer);
  btimer=setTimeout(function(){fetch('/brightness?val='+val).catch(function(){});},300);
}

// ── Status fetch ──────────────────────────────────────────────
function applyStatus(d){
  setUI(d.on);
  if(!sliderInteracted&&d.brightness!==undefined){
    document.getElementById('brightSlider').value=d.brightness;
    document.getElementById('bval').textContent=d.brightness;
  }
  // Cache profile data for the custom editor
  if(d.allProfiles)allProfiles=d.allProfiles;
  if(d.profileNames)profileNames=d.profileNames;
  // Sync mode buttons and profile badge
  if(d.mode!==undefined){
    var isCustom=d.mode>0;
    // Only re-render buttons if names changed or first render
    renderModeButtons(d.mode,profileNames);
    var pb=document.getElementById('profile-badge');
    if(pb){
      var pLabel=isCustom?(profileNames&&profileNames[d.mode-1]?profileNames[d.mode-1]:('Custom '+d.mode)):'Default';
      pb.textContent='Profile: '+pLabel;
      pb.style.borderColor=isCustom?'rgba(184,160,240,.4)':'var(--bd)';
      pb.style.color=isCustom?'var(--accent)':'var(--dim)';
    }
  }
  renderZones(d.zones);
  // Rebuild customZonesCache for the active custom profile (if editor not open)
  if(d.allProfiles&&d.mode>0&&lastZones&&lastZones.length>1){
    customZonesCache=buildZonesFromRgb?buildZonesFromRgb(d.allProfiles[d.mode-1]):null;
  }
}
function loadStatus(){
  fetch('/status').then(function(r){return r.json();}).then(applyStatus).catch(function(){});
}
window.addEventListener('load',function(){
  loadStatus();
  setInterval(loadStatus,3000);
  // Wire canvas picker events
  var cv=document.getElementById('picker-canvas');
  if(cv){
    cv.addEventListener('mousedown',function(e){pickerDrag=true;pickerMove(e);e.preventDefault();});
    cv.addEventListener('mousemove',function(e){if(pickerDrag)pickerMove(e);});
    cv.addEventListener('mouseup',function(){pickerDrag=false;});
    cv.addEventListener('mouseleave',function(){pickerDrag=false;});
    cv.addEventListener('touchstart',function(e){pickerDrag=true;pickerMove(e);e.preventDefault();},{passive:false});
    cv.addEventListener('touchmove',function(e){if(pickerDrag){pickerMove(e);e.preventDefault();}},{passive:false});
    cv.addEventListener('touchend',function(){pickerDrag=false;});
  }
});
</script>
</body>
</html>
)rawliteral";

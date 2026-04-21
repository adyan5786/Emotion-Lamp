#pragma once

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1.0"/>
<title>Emotion Lamp</title>
<style>
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0;-webkit-tap-highlight-color:transparent}
:root{--bg:#111;--sf:#1a1a1a;--sf2:#222;--bd:#2c2c2c;--tx:#dedede;--dim:#555;--accent:#b8a0f0;--on:#7dff9a;--nav:64px;--side:200px}
/* Unified dark scrollbar theme for all modal scroll areas */
.modal-scroll{scrollbar-width:thin;scrollbar-color:#2b2b2b #090909}
.modal-scroll::-webkit-scrollbar{width:10px;height:10px}
.modal-scroll::-webkit-scrollbar-track{background:#090909;border-left:1px solid #141414}
.modal-scroll::-webkit-scrollbar-thumb{background:#2b2b2b;border-radius:999px;border:2px solid #090909}
.modal-scroll::-webkit-scrollbar-thumb:hover{background:#3a3a3a}
.modal-scroll::-webkit-scrollbar-corner{background:#090909}
html,body{height:100%;overflow:hidden}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:var(--bg);color:var(--tx);display:flex;height:100dvh}
button{-webkit-tap-highlight-color:transparent;touch-action:manipulation}
.sidebar{display:none;width:var(--side);background:var(--sf);border-right:1px solid var(--bd);flex-direction:column;flex-shrink:0}
.sb-head{padding:22px 16px 18px;border-bottom:1px solid var(--bd)}
.sb-head h1{font-size:.88rem;font-weight:700;letter-spacing:.08em;text-transform:uppercase}
.sb-head p{font-size:.7rem;color:var(--dim);margin-top:3px}
.sb-nav{padding:8px 0;flex:1}
.ni{display:flex;align-items:center;gap:10px;padding:10px 16px;cursor:pointer;color:var(--dim);font-size:.84rem;font-weight:500;border-left:3px solid transparent;transition:all .15s;user-select:none}
.ni:hover{background:var(--sf2);color:var(--tx)}
.ni.active{color:var(--accent);border-left-color:var(--accent);background:rgba(184,160,240,.07)}
.content{flex:1;display:flex;flex-direction:column;padding-bottom:var(--nav);overflow:hidden}
.panel{display:none;flex-direction:column;flex:1;overflow-y:auto}
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
.slider{flex:1;min-width:0;width:100%;display:block;-webkit-appearance:none;appearance:none;height:4px;border-radius:2px;background:var(--bd);outline:none;cursor:pointer}
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
/* settings */
.setting-item{display:flex;align-items:center;justify-content:space-between;padding:16px 20px;background:var(--sf2);border-radius:10px;margin-bottom:8px;cursor:pointer;transition:background .15s}
.setting-item:hover{background:var(--bd)}
.setting-item span{font-size:.9rem;font-weight:600}
.setting-item .chev{color:var(--dim);font-size:1.2rem}
.text-danger{color:#ff4a4a}
.about-content{display:flex;flex-direction:column;align-items:center;padding:24px 20px;text-align:center}
.about-content h2{margin:0 0 8px 0;font-size:1.4rem;color:var(--tx)}
.about-content p{margin:0 0 16px 0;font-size:.85rem;color:var(--dim);line-height:1.5}
.about-team{font-size:.8rem;color:var(--dim);margin-bottom:24px;line-height:1.6}
.about-team b{color:var(--tx)}
.gh-btn{display:flex;align-items:center;gap:8px;background:#24292e;color:#fff;padding:12px 20px;border-radius:8px;text-decoration:none;font-size:.9rem;font-weight:600;transition:background .15s}
.gh-btn:hover{background:#2f363d}
.wifi-modal-body{padding:16px;max-height:72vh;overflow-y:auto}
.wifi-modal-shell{display:flex;flex-direction:column;max-height:88vh}
.wifi-card{background:var(--sf2);border:1px solid var(--bd);border-radius:12px;padding:14px 12px;margin-bottom:12px}
.wifi-card h4{font-size:.8rem;letter-spacing:.08em;text-transform:uppercase;color:var(--dim);margin-bottom:10px}
.wifi-pill-row{display:flex;flex-wrap:wrap;gap:8px;margin-top:2px}
.wifi-grid{display:grid;grid-template-columns:1fr;gap:10px}
.wifi-label{font-size:.72rem;color:var(--dim);margin-bottom:4px;display:block}
.wifi-input{width:100%;padding:10px 11px;border-radius:9px;border:1px solid var(--bd);background:#181818;color:var(--tx);font-size:.84rem;outline:none;transition:border-color .15s}
.wifi-input:focus{border-color:var(--accent)}
.wifi-row{display:flex;gap:8px;align-items:center}
.wifi-pw-wrap{position:relative}
.wifi-pw-btn{position:absolute;right:6px;top:50%;transform:translateY(-50%);border:1px solid var(--bd);background:var(--sf2);color:var(--dim);font-size:.67rem;padding:5px 8px;border-radius:6px;cursor:pointer}
.wifi-meta{font-size:.8rem;color:var(--dim);line-height:1.65}
.wifi-pill{display:inline-block;padding:4px 9px;border-radius:999px;border:1px solid var(--bd);font-size:.67rem;letter-spacing:.05em;text-transform:uppercase;color:var(--dim);margin-right:0}
.wifi-pill.ok{color:var(--on);border-color:#2d6b3a;background:rgba(100,220,130,.08)}
.wifi-pill.warn{color:#f0b26a;border-color:#7d5a2e;background:rgba(180,120,50,.12)}
.wifi-btn{padding:11px 12px;border-radius:10px;border:1px solid var(--bd);background:var(--sf2);color:var(--tx);font-size:.82rem;font-weight:600;cursor:pointer}
.wifi-btn.primary{border-color:var(--accent);color:var(--accent);background:rgba(184,160,240,.08)}
.wifi-btn.danger{border-color:#7a3a3a;color:#e07070;background:rgba(180,60,60,.12)}
.wifi-note{font-size:.68rem;color:var(--dim);margin-top:6px;line-height:1.45}
.wifi-modal-footer{display:flex;gap:10px;padding:12px 16px 16px;border-top:1px solid var(--bd);background:var(--sf)}
.modal-close-btn{width:30px;height:30px;min-width:30px;min-height:30px;border-radius:50%;border:1px solid var(--bd);background:var(--sf2);color:var(--tx);font-size:1.1rem;line-height:1;cursor:pointer;display:flex;align-items:center;justify-content:center;flex:0 0 30px}
.spec-grid{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:8px}
.spec-item{background:#181818;border:1px solid var(--bd);border-radius:9px;padding:9px}
.spec-k{font-size:.62rem;color:var(--dim);text-transform:uppercase;letter-spacing:.06em}
.spec-v{font-size:.8rem;color:var(--tx);font-weight:600;margin-top:4px;word-break:break-word}
.meter{width:100%;height:10px;border-radius:999px;background:#0f0f0f;border:1px solid var(--bd);overflow:hidden;margin-top:8px}
.meter-fill{height:100%;width:0%;background:#b8a0f0;transition:width .35s ease}
.meter-meta{font-size:.7rem;color:var(--dim);margin-top:6px}
.audio-live-card{background:#181818;border:1px solid var(--bd);border-radius:12px;padding:12px}
.audio-live-row{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-bottom:10px}
.audio-live-k{font-size:.68rem;color:var(--dim);text-transform:uppercase;letter-spacing:.06em}
.audio-live-v{font-size:1.2rem;font-weight:700;color:var(--tx);margin-top:4px}
.audio-live-scroll{width:100%;overflow-x:auto;overflow-y:hidden;border-radius:10px;border:1px solid var(--bd);background:#101010}
.audio-live-canvas{height:180px;display:block}
@media(max-width:480px){.spec-grid{grid-template-columns:1fr}}
</style>
</head>
<body>

<nav class="sidebar">
  <div class="sb-head"><h1>Emotion Lamp</h1><p id="access-host">ESP32 &middot; 4.3.2.1</p></div>
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
    <div id="fx-main" style="display:flex;flex-direction:column;padding-bottom:20px">

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

      <!-- Animation Style -->
      <div class="sec">
        <div class="sec-title">Animation</div>
        <div class="mode-row">
          <div class="mdbtn" id="anim-0" onclick="setAnim(0)">
            <div class="mdindicator"></div>
            <span class="mdlabel">Solid</span>
          </div>
          <div class="mdbtn" id="anim-1" onclick="setAnim(1)">
            <div class="mdindicator"></div>
            <span class="mdlabel">Matrix</span>
          </div>
          <div class="mdbtn" id="anim-2" onclick="setAnim(2)">
            <div class="mdindicator"></div>
            <span class="mdlabel">Twinkle</span>
          </div>
          <div class="mdbtn" id="anim-3" onclick="setAnim(3)">
            <div class="mdindicator"></div>
            <span class="mdlabel">Ripple</span>
          </div>
          <div class="mdbtn" id="anim-4" onclick="setAnim(4)">
            <div class="mdindicator"></div>
            <span class="mdlabel">Wipe</span>
          </div>
        </div>
      </div>

      <!-- Mode selector -->
      <div class="sec">
        <div class="sec-title">Profile</div>
        <div class="mode-row" id="mode-row"><!-- populated by renderModeButtons() --></div>
      </div>
    </div>

    <!-- Custom colours editor view (hidden by default) -->
    <div id="fx-custom" style="display:none;flex-direction:column;min-height:100%">
      <div class="ph" style="padding-bottom:12px;gap:8px">
        <input id="profile-name-input" type="text" maxlength="11" placeholder="Profile name"
          style="font-size:1rem;font-weight:700;background:transparent;border:none;border-bottom:2px solid var(--bd);color:var(--tx);outline:none;padding:2px 4px;flex:1;width:100%"
          oninput="onNameInput(this.value)" onblur="commitProfileName(this.value)">
      </div>
      <div class="sec" style="flex:1">
        <div class="sec-title">Tap a zone to edit its colour</div>
        <div id="custom-zones-list"></div>
      </div>
      <div style="margin-top:auto;position:sticky;bottom:0;background:var(--bg);display:flex;gap:10px;padding:16px 20px 24px;border-top:1px solid rgba(255,255,255,.05);z-index:10">
        <button class="back-btn" onclick="backFromCustom()">&#8592; Back</button>
        <button class="wifi-btn primary" style="flex:1" id="save-profile-btn" onclick="saveProfile()">Save Profile</button>
      </div>
    </div>
  </div>

  <!-- SETTINGS -->
  <div id="p-settings" class="panel">
    <div class="ph"><h2>Settings</h2></div>
    <div style="padding-top:20px;padding-bottom:30px;">
      
      <div class="sec">
        <div class="sec-title">Device</div>
        <div class="setting-item" onclick="openAudioLiveModal()">
          <span>Live Audio Monitor</span>
          <span class="chev">&#8250;</span>
        </div>
        <div class="setting-item" onclick="openSensModal()">
          <span>Microphone Sensitivity</span>
          <span class="chev">&#8250;</span>
        </div>
        <div class="setting-item" onclick="openFreqModal()">
          <span>Advanced Frequency Tuning</span>
          <span class="chev">&#8250;</span>
        </div>
        <div class="setting-item" onclick="openWifiModal()">
          <span>Wi-Fi Configurator</span>
          <span class="chev">&#8250;</span>
        </div>
      </div>

      <div class="sec">
        <div class="sec-title">System</div>
        <div class="setting-item" onclick="openSysModal()">
          <span>System Specifications</span>
          <span class="chev">&#8250;</span>
        </div>
        <div class="setting-item" onclick="rebootDevice()">
          <span>Reboot Device</span>
          <span class="chev">&#8250;</span>
        </div>
        <div class="setting-item text-danger" onclick="factoryReset()">
          <span>Factory Reset</span>
          <span class="chev">&#8250;</span>
        </div>
      </div>

      <div class="sec">
        <div class="sec-title">Information</div>
        <div class="setting-item" onclick="openAbout()">
          <span>About</span>
          <span class="chev">&#8250;</span>
        </div>
      </div>

    </div>
  </div>

</main>

<!-- Toast notification -->
<div id="toast" class="toast">&#10003; Profile Saved</div>

<!-- About Modal -->
<div id="about-modal" style="display:none;position:fixed;inset:0;z-index:300;background:rgba(0,0,0,.75);align-items:center;justify-content:center;padding:20px">
  <div style="background:var(--sf);border:1px solid var(--bd);border-radius:16px;width:min(340px,100%);overflow:hidden">
    <div style="display:flex;align-items:center;justify-content:space-between;padding:16px 16px 14px;border-bottom:1px solid var(--bd)">
      <span style="font-size:.96rem;font-weight:700">About</span>
      <button onclick="closeAbout()" style="width:28px;height:28px;border-radius:50%;border:1px solid var(--bd);background:var(--sf2);color:var(--tx);font-size:1.1rem;line-height:1;cursor:pointer;display:flex;align-items:center;justify-content:center">&times;</button>
    </div>
    <div class="about-content">
      <h2>Emotion Lamp</h2>
      <p>An interactive, audio-reactive smart lamp built with an ESP32, INMP441 I2S microphone, and WS2812B LEDs. Designed for dynamic multi-profile mood lighting.</p>
      <div class="about-team">
        Created by:<br>
        <b>Adyan Shaikh, Abdul Rehman Choudhary, Mohammed Sadriwala, and Mugaira Pathan</b>
      </div>
      <a href="https://github.com/adyan5786/Emotion-Lamp" target="_blank" class="gh-btn">
        <svg width="20" height="20" viewBox="0 0 24 24" fill="currentColor"><path d="M12 2C6.477 2 2 6.484 2 12.017c0 4.425 2.865 8.18 6.839 9.504.5.092.682-.217.682-.483 0-.237-.008-.868-.013-1.703-2.782.605-3.369-1.343-3.369-1.343-.454-1.158-1.11-1.466-1.11-1.466-.908-.62.069-.608.069-.608 1.003.07 1.531 1.032 1.531 1.032.892 1.53 2.341 1.088 2.91.832.092-.647.35-1.088.636-1.338-2.22-.253-4.555-1.113-4.555-4.951 0-1.093.39-1.988 1.029-2.688-.103-.253-.446-1.272.098-2.65 0 0 .84-.27 2.75 1.026A9.564 9.564 0 0112 6.844c.85.004 1.705.115 2.504.337 1.909-1.296 2.747-1.027 2.747-1.027.546 1.379.202 2.398.1 2.651.64.7 1.028 1.595 1.028 2.688 0 3.848-2.339 4.695-4.566 4.943.359.309.678.92.678 1.855 0 1.338-.012 2.419-.012 2.747 0 .268.18.58.688.482A10.019 10.019 0 0022 12.017C22 6.484 17.522 2 12 2z"/></svg>
        View on GitHub
      </a>
    </div>
  </div>
</div>

<!-- Wi-Fi Configurator Modal -->
<div id="wifi-modal" style="display:none;position:fixed;inset:0;z-index:320;background:rgba(0,0,0,.78);align-items:center;justify-content:center;padding:20px">
  <div class="wifi-modal-shell" style="background:var(--sf);border:1px solid var(--bd);border-radius:16px;width:min(520px,100%);overflow:hidden;box-shadow:0 18px 60px rgba(0,0,0,.55)">
    <div style="display:flex;align-items:center;justify-content:space-between;padding:16px 16px 14px;border-bottom:1px solid var(--bd)">
      <div style="display:flex;flex-direction:column;gap:6px">
        <span style="font-size:.96rem;font-weight:700">Wi-Fi Configurator</span>
        <div class="wifi-pill-row">
          <span id="wifi-pill-ap" class="wifi-pill">AP</span>
          <span id="wifi-pill-sta" class="wifi-pill">STA</span>
          <span id="wifi-pill-pending" class="wifi-pill">No Pending Changes</span>
        </div>
      </div>
      <button onclick="closeWifiModal()" class="modal-close-btn">&times;</button>
    </div>

    <div class="wifi-modal-body modal-scroll">
      <div class="wifi-card">
        <h4>Current Configuration</h4>
        <div class="wifi-meta" id="wf-live">Loading saved configuration...</div>
      </div>

      <div class="wifi-card">
        <h4>Access Point</h4>
        <div class="wifi-grid">
          <div>
            <label class="wifi-label">AP Name (SSID)</label>
            <input id="wf-ap-ssid" class="wifi-input" maxlength="32" placeholder="Emotion-Lamp">
          </div>
          <div>
            <label class="wifi-label">AP Password</label>
            <div class="wifi-pw-wrap">
              <input id="wf-ap-pass" type="password" class="wifi-input" maxlength="63" placeholder="Enter new AP password">
              <button id="wf-ap-pass-toggle" type="button" class="wifi-pw-btn" onclick="togglePw('wf-ap-pass','wf-ap-pass-toggle')">Show</button>
            </div>
            <div id="wf-ap-pass-meta" class="wifi-note">Password is masked by firmware and never echoed back.</div>
          </div>
          <div>
            <label class="wifi-label">AP IP</label>
            <input id="wf-ap-ip" class="wifi-input" placeholder="4.3.2.1">
          </div>
          <div class="wifi-row">
            <button class="wifi-btn primary" onclick="saveApConfig()" style="flex:1">Save AP Settings</button>
          </div>
        </div>
      </div>

      <div class="wifi-card">
        <h4>Home Wi-Fi (STA)</h4>
        <div class="wifi-grid">
          <div>
            <label class="wifi-label">Router SSID</label>
            <input id="wf-sta-ssid" class="wifi-input" maxlength="32" placeholder="e.g. A">
          </div>
          <div>
            <label class="wifi-label">Router Password</label>
            <div class="wifi-pw-wrap">
              <input id="wf-sta-pass" type="password" class="wifi-input" maxlength="63" placeholder="Enter router password">
              <button id="wf-sta-pass-toggle" type="button" class="wifi-pw-btn" onclick="togglePw('wf-sta-pass','wf-sta-pass-toggle')">Show</button>
            </div>
            <div id="wf-sta-pass-meta" class="wifi-note">Leave blank only for open networks.</div>
          </div>
          <div class="wifi-row">
            <button class="wifi-btn primary" onclick="saveStaConfig()" style="flex:1">Save Home Wi-Fi</button>
            <button class="wifi-btn danger" onclick="forgetStaConfig()" style="flex:1">Forget Home Wi-Fi</button>
          </div>
        </div>
      </div>

      <div class="wifi-card">
        <h4>Hostname</h4>
        <div class="wifi-grid">
          <div>
            <label class="wifi-label">mDNS Hostname</label>
            <input id="wf-host" class="wifi-input" maxlength="32" placeholder="emotionlamp">
            <div class="wifi-note">Allowed: lowercase letters, numbers, hyphen.</div>
          </div>
          <button class="wifi-btn primary" onclick="saveHostConfig()">Save Hostname</button>
        </div>
      </div>
    </div>

    <div class="wifi-modal-footer">
      <button class="wifi-btn" style="flex:1" onclick="discardNetworkChanges()">Discard Staged Changes</button>
      <button class="wifi-btn primary" style="flex:1" onclick="applyNetworkChanges()">Apply & Reboot</button>
    </div>
  </div>
</div>

<!-- Live Audio Monitor Modal -->
<div id="audio-live-modal" style="display:none;position:fixed;inset:0;z-index:330;background:rgba(0,0,0,.78);align-items:center;justify-content:center;padding:20px">
  <div style="background:var(--sf);border:1px solid var(--bd);border-radius:16px;width:min(560px,100%);overflow:hidden;box-shadow:0 18px 60px rgba(0,0,0,.55)">
    <div style="display:flex;align-items:center;justify-content:space-between;padding:16px 16px 14px;border-bottom:1px solid var(--bd)">
      <span style="font-size:.96rem;font-weight:700">Live Audio Monitor</span>
      <button onclick="closeAudioLiveModal()" class="modal-close-btn">&times;</button>
    </div>
    <div class="modal-scroll" style="padding:16px;max-height:72vh;overflow-y:auto">
      <div class="audio-live-card">
        <div class="audio-live-row">
          <div>
            <div class="audio-live-k">Current RMS</div>
            <div class="audio-live-v" id="audio-live-rms">-</div>
          </div>
          <div>
            <div class="audio-live-k">Current Frequency</div>
            <div class="audio-live-v" id="audio-live-hz">- Hz</div>
          </div>
        </div>
        <div id="audio-live-scroll" class="audio-live-scroll">
          <canvas id="audio-live-canvas" class="audio-live-canvas" width="520" height="180"></canvas>
        </div>
        <div style="display:flex;justify-content:space-between;gap:10px;margin-top:8px;font-size:.7rem;color:var(--dim)">
          <span>Green: RMS (scaled)</span>
          <span>Blue: Frequency (scaled)</span>
        </div>
      </div>
      <div class="wifi-note" style="margin-top:10px">Scrollable timeline view. Updates every ~180 ms while this modal is open.</div>
    </div>
  </div>
</div>

<!-- System Specifications Modal -->
<div id="sys-modal" style="display:none;position:fixed;inset:0;z-index:310;background:rgba(0,0,0,.78);align-items:center;justify-content:center;padding:20px">
  <div style="background:var(--sf);border:1px solid var(--bd);border-radius:16px;width:min(560px,100%);overflow:hidden;box-shadow:0 18px 60px rgba(0,0,0,.55)">
    <div style="display:flex;align-items:center;justify-content:space-between;padding:16px 16px 14px;border-bottom:1px solid var(--bd)">
      <span style="font-size:.96rem;font-weight:700">System Specifications</span>
      <button onclick="closeSysModal()" style="width:28px;height:28px;border-radius:50%;border:1px solid var(--bd);background:var(--sf2);color:var(--tx);font-size:1.1rem;line-height:1;cursor:pointer;display:flex;align-items:center;justify-content:center">&times;</button>
    </div>
    <div class="modal-scroll" style="padding:16px;max-height:72vh;overflow-y:auto">
      <div class="sec-title" style="margin-bottom:8px">Firmware & Runtime</div>
      <div class="spec-grid">
        <div class="spec-item"><div class="spec-k">Firmware</div><div class="spec-v" id="spec-fw">-</div></div>
        <div class="spec-item"><div class="spec-k">Build</div><div class="spec-v" id="spec-build">-</div></div>
        <div class="spec-item"><div class="spec-k">ESP-IDF SDK</div><div class="spec-v" id="spec-sdk">-</div></div>
        <div class="spec-item"><div class="spec-k">Uptime</div><div class="spec-v" id="spec-up">-</div></div>
        <div class="spec-item"><div class="spec-k">CPU</div><div class="spec-v" id="spec-cpu">-</div></div>
      </div>

      <div class="sec-title" style="margin:14px 0 8px">Chip & Memory</div>
      <div class="spec-grid">
        <div class="spec-item"><div class="spec-k">Chip</div><div class="spec-v" id="spec-chip">-</div></div>
        <div class="spec-item"><div class="spec-k">Revision</div><div class="spec-v" id="spec-rev">-</div></div>
        <div class="spec-item"><div class="spec-k">Free Heap</div><div class="spec-v" id="spec-heap">-</div></div>
        <div class="spec-item"><div class="spec-k">Min Heap</div><div class="spec-v" id="spec-minheap">-</div></div>
        <div class="spec-item"><div class="spec-k">Sketch Size</div><div class="spec-v" id="spec-sketch">-</div></div>
        <div class="spec-item"><div class="spec-k">Flash</div><div class="spec-v" id="spec-flash">-</div></div>
      </div>

      <div class="sec-title" style="margin:14px 0 8px">Storage Usage</div>
      <div class="spec-item" style="padding:12px">
        <div class="spec-k">Firmware Partition Usage</div>
        <div class="meter"><div id="spec-sto-fill" class="meter-fill"></div></div>
        <div id="spec-sto-meta" class="meter-meta">-</div>
      </div>

      <div class="sec-title" style="margin:14px 0 8px">Network Identity</div>
      <div class="spec-grid">
        <div class="spec-item"><div class="spec-k">AP</div><div class="spec-v" id="spec-ap">-</div></div>
        <div class="spec-item"><div class="spec-k">STA</div><div class="spec-v" id="spec-sta">-</div></div>
        <div class="spec-item"><div class="spec-k">Hostname</div><div class="spec-v" id="spec-host">-</div></div>
        <div class="spec-item"><div class="spec-k">AP Clients</div><div class="spec-v" id="spec-clients">-</div></div>
      </div>
    </div>
  </div>
</div>

<!-- Feature Modal -->
<div id="feature-modal" style="display:none;position:fixed;inset:0;z-index:300;background:rgba(0,0,0,.75);align-items:center;justify-content:center;padding:20px">
  <div style="background:var(--sf);border:1px solid var(--bd);border-radius:16px;width:min(340px,100%);overflow:hidden">
    <div style="display:flex;align-items:center;justify-content:space-between;padding:16px 16px 14px;border-bottom:1px solid var(--bd)">
      <span id="feature-title" style="font-size:.96rem;font-weight:700">Feature</span>
      <button onclick="closeFeatureModal()" style="width:28px;height:28px;border-radius:50%;border:1px solid var(--bd);background:var(--sf2);color:var(--tx);font-size:1.1rem;line-height:1;cursor:pointer;display:flex;align-items:center;justify-content:center">&times;</button>
    </div>
    <div class="about-content">
      <svg width="40" height="40" viewBox="0 0 24 24" fill="none" stroke="var(--dim)" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round" style="margin-bottom:16px"><circle cx="12" cy="12" r="10"/><polyline points="12 8 12 12 14 14"/></svg>
      <h2>Coming Soon</h2>
      <p>The <b id="feature-name" style="color:var(--tx)"></b> functionality will be available in a future firmware update.</p>
      <button onclick="closeFeatureModal()" class="save-btn" style="width:100%;margin-top:8px">Got it</button>
    </div>
  </div>
</div>

<!-- Sensitivity Modal -->
<div id="sens-modal" style="display:none;position:fixed;inset:0;z-index:300;background:rgba(0,0,0,.75);align-items:center;justify-content:center;padding:20px">
  <div style="background:var(--sf);border:1px solid var(--bd);border-radius:16px;width:min(340px,100%);overflow:hidden">
    <div style="display:flex;align-items:center;justify-content:space-between;padding:16px 16px 14px;border-bottom:1px solid var(--bd)">
      <span style="font-size:.96rem;font-weight:700">Microphone Sensitivity</span>
      <button onclick="closeSensModal()" style="width:28px;height:28px;border-radius:50%;border:1px solid var(--bd);background:var(--sf2);color:var(--tx);font-size:1.1rem;line-height:1;cursor:pointer;display:flex;align-items:center;justify-content:center">&times;</button>
    </div>
    <div style="padding:20px 16px">
      <div style="margin-bottom:20px">
        <div style="display:flex;justify-content:space-between;margin-bottom:6px">
          <span style="font-size:.85rem;color:var(--dim)">Quiet Filter (RMS)</span>
          <span id="lbl-quiet" style="font-size:.85rem;font-weight:700;color:var(--tx)">150</span>
        </div>
        <input type="range" id="sl-quiet" class="slider" min="0" max="500" value="150" oninput="onSensInput()">
        <div style="font-size:.7rem;color:var(--dim);margin-top:6px;line-height:1.4">Ignore background noise below this volume.</div>
      </div>
      <div style="margin-bottom:20px">
        <div style="display:flex;justify-content:space-between;margin-bottom:6px">
          <span style="font-size:.85rem;color:var(--dim)">Loudness Cap (RMS)</span>
          <span id="lbl-noisy" style="font-size:.85rem;font-weight:700;color:var(--tx)">1800</span>
        </div>
        <input type="range" id="sl-noisy" class="slider" min="500" max="8000" step="50" value="1800" oninput="onSensInput()">
        <div style="font-size:.7rem;color:var(--dim);margin-top:6px;line-height:1.4">Hit max brightness at this volume.</div>
      </div>
      <button onclick="saveAudioTuning()" class="save-btn" style="width:100%">Save Sensitivity</button>
    </div>
  </div>
</div>

<!-- Frequency Tuning Modal -->
<div id="freq-modal" style="display:none;position:fixed;inset:0;z-index:300;background:rgba(0,0,0,.75);align-items:center;justify-content:center;padding:20px">
  <div style="background:var(--sf);border:1px solid var(--bd);border-radius:16px;width:min(340px,100%);overflow:hidden">
    <div style="display:flex;align-items:center;justify-content:space-between;padding:16px 16px 14px;border-bottom:1px solid var(--bd)">
      <span style="font-size:.96rem;font-weight:700">Frequency Tuning</span>
      <button onclick="closeFreqModal()" style="width:28px;height:28px;border-radius:50%;border:1px solid var(--bd);background:var(--sf2);color:var(--tx);font-size:1.1rem;line-height:1;cursor:pointer;display:flex;align-items:center;justify-content:center">&times;</button>
    </div>
    <div class="modal-scroll" style="padding:20px 16px;max-height:60vh;overflow-y:auto">
      <div style="margin-bottom:20px">
        <div style="display:flex;justify-content:space-between;margin-bottom:6px">
          <span style="font-size:.85rem;color:var(--dim)">Bass &#8594; Low Speech</span>
          <span id="lbl-fb0" style="font-size:.85rem;font-weight:700;color:var(--tx)">300 Hz</span>
        </div>
        <input type="range" id="sl-fb0" class="slider" min="0" max="300" step="10" value="300" oninput="onFreqInput(0)">
      </div>
      <div style="margin-bottom:20px">
        <div style="display:flex;justify-content:space-between;margin-bottom:6px">
          <span style="font-size:.85rem;color:var(--dim)">Low Speech &#8594; Mid</span>
          <span id="lbl-fb1" style="font-size:.85rem;font-weight:700;color:var(--tx)">600 Hz</span>
        </div>
        <input type="range" id="sl-fb1" class="slider" min="300" max="600" step="10" value="600" oninput="onFreqInput(1)">
      </div>
      <div style="margin-bottom:20px">
        <div style="display:flex;justify-content:space-between;margin-bottom:6px">
          <span style="font-size:.85rem;color:var(--dim)">Mid &#8594; High Speech</span>
          <span id="lbl-fb2" style="font-size:.85rem;font-weight:700;color:var(--tx)">900 Hz</span>
        </div>
        <input type="range" id="sl-fb2" class="slider" min="600" max="1200" step="10" value="900" oninput="onFreqInput(2)">
      </div>
      <div style="margin-bottom:20px">
        <div style="display:flex;justify-content:space-between;margin-bottom:6px">
          <span style="font-size:.85rem;color:var(--dim)">High Speech &#8594; Shrill</span>
          <span id="lbl-fb3" style="font-size:.85rem;font-weight:700;color:var(--tx)">1300 Hz</span>
        </div>
        <input type="range" id="sl-fb3" class="slider" min="1200" max="2500" step="10" value="1300" oninput="onFreqInput(3)">
      </div>
      <button onclick="saveAudioTuning()" class="save-btn" style="width:100%">Save Frequencies</button>
    </div>
  </div>
</div>

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
<!-- Action confirmation modal -->
<div id="action-modal" style="display:none;position:fixed;inset:0;z-index:360;background:rgba(0,0,0,.75);align-items:center;justify-content:center;padding:20px">
  <div style="background:var(--sf);border:1px solid var(--bd);border-radius:16px;width:min(320px,100%);overflow:hidden">
    <div style="padding:24px 20px 20px;text-align:center">
      <svg id="action-icon" width="36" height="36" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round" style="margin-bottom:12px;color:var(--dim)"><circle cx="12" cy="12" r="10"/><line x1="12" y1="8" x2="12" y2="12"/><line x1="12" y1="16" x2="12.01" y2="16"/></svg>
      <h3 id="action-title" style="margin:0 0 8px 0;font-size:1.1rem;color:var(--tx)">Action</h3>
      <p id="action-msg" style="margin:0;font-size:.85rem;color:var(--dim);line-height:1.5">Are you sure?</p>
    </div>
    <div style="display:flex;gap:10px;padding:0 20px 20px">
      <button onclick="closeActionModal()" style="flex:1;padding:12px;border-radius:10px;border:1px solid var(--bd);background:var(--sf2);color:var(--tx);font-size:.9rem;font-weight:600;cursor:pointer">Cancel</button>
      <button id="action-btn" onclick="confirmAction()" style="flex:1;padding:12px;border-radius:10px;font-size:.9rem;font-weight:600;cursor:pointer">Confirm</button>
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
var wifiModalOpen=false;
var wifiStatusCache=null;
var wifiCfgCache=null;
var sysModalOpen=false;
var audioLiveModalOpen=false;
var audioLiveTimer=null;
var audioHistoryRms=[];
var audioHistoryHz=[];
var lastLiveRms=0;
var lastLiveHz=0;
var statusTimer=null;
var audioGraphPxPerSample=1.8;
var audioGraphMaxPoints=1800;

function setAccessHostLabel(){
  var el=document.getElementById('access-host');
  if(!el)return;
  var host=window.location.hostname||'';
  var port=window.location.port?(':'+window.location.port):'';
  el.textContent='ESP32 \u00b7 '+(host?(host+port):'4.3.2.1');
}

function setLiveAudio(rms,hz){
  var r=(rms===undefined||rms===null)?0:Math.max(0,Number(rms)||0);
  var h=(hz===undefined||hz===null)?0:Math.max(0,Number(hz)||0);
  lastLiveRms=r;
  lastLiveHz=h;

  if(audioLiveModalOpen){
    var rmsEl=document.getElementById('audio-live-rms');
    var hzEl=document.getElementById('audio-live-hz');
    if(rmsEl) rmsEl.textContent=Math.round(r);
    if(hzEl) hzEl.textContent=Math.round(h)+' Hz';
  }
}

function startStatusPolling(){
  if(statusTimer) return;
  statusTimer=setInterval(loadStatus,3000);
}

function stopStatusPolling(){
  if(!statusTimer) return;
  clearInterval(statusTimer);
  statusTimer=null;
}

function pushAudioHistory(rms,hz){
  audioHistoryRms.push(rms);
  audioHistoryHz.push(hz);
  if(audioHistoryRms.length>audioGraphMaxPoints) audioHistoryRms.shift();
  if(audioHistoryHz.length>audioGraphMaxPoints) audioHistoryHz.shift();
}

function drawAudioGraph(){
  var scrollBox=document.getElementById('audio-live-scroll');
  var cv=document.getElementById('audio-live-canvas');
  if(!cv) return;
  var n=Math.max(audioHistoryRms.length,audioHistoryHz.length);
  var viewW=scrollBox?Math.max(300,scrollBox.clientWidth):520;
  var desiredW=Math.max(viewW,Math.floor((Math.max(1,n)-1)*audioGraphPxPerSample)+2);
  var desiredH=180;

  var followEnd=true;
  if(scrollBox){
    followEnd=(scrollBox.scrollLeft+scrollBox.clientWidth)>=((scrollBox.scrollWidth||0)-24);
  }

  if(cv.width!==desiredW||cv.height!==desiredH){
    cv.width=desiredW;
    cv.height=desiredH;
  }

  var ctx=cv.getContext('2d');
  var w=cv.width,h=cv.height;
  ctx.clearRect(0,0,w,h);

  ctx.strokeStyle='rgba(255,255,255,.08)';
  ctx.lineWidth=1;
  for(var gy=1;gy<=3;gy++){
    var y=(h/4)*gy;
    ctx.beginPath();ctx.moveTo(0,y);ctx.lineTo(w,y);ctx.stroke();
  }

  if(n<2) return;

  ctx.strokeStyle='rgba(255,255,255,.05)';
  for(var gx=120;gx<w;gx+=120){
    ctx.beginPath();ctx.moveTo(gx,0);ctx.lineTo(gx,h);ctx.stroke();
  }

  var rmsScale=Math.max(lastAudioSnapshot&&lastAudioSnapshot.n?Number(lastAudioSnapshot.n):1800,500);
  var hzScale=Math.max(lastAudioSnapshot&&lastAudioSnapshot.f?Number(lastAudioSnapshot.f[3]):1300,500);

  ctx.lineWidth=2;
  ctx.strokeStyle='rgba(100,220,130,.95)';
  ctx.beginPath();
  for(var i=0;i<audioHistoryRms.length;i++){
    var x=i*audioGraphPxPerSample;
    var yn=Math.max(0,Math.min(1,audioHistoryRms[i]/rmsScale));
    var y=h-(yn*h);
    if(i===0) ctx.moveTo(x,y); else ctx.lineTo(x,y);
  }
  ctx.stroke();

  ctx.strokeStyle='rgba(130,180,255,.95)';
  ctx.beginPath();
  for(var j=0;j<audioHistoryHz.length;j++){
    var x2=j*audioGraphPxPerSample;
    var fn=Math.max(0,Math.min(1,audioHistoryHz[j]/hzScale));
    var y2=h-(fn*h);
    if(j===0) ctx.moveTo(x2,y2); else ctx.lineTo(x2,y2);
  }
  ctx.stroke();

  if(scrollBox&&followEnd){
    scrollBox.scrollLeft=Math.max(0,scrollBox.scrollWidth-scrollBox.clientWidth);
  }
}

function loadLiveAudioSample(){
  fetch('/liveaudio').then(function(r){return r.json();}).then(function(d){
    if(d){
      setLiveAudio(d.rms,d.hz);
      pushAudioHistory(lastLiveRms,lastLiveHz);
      drawAudioGraph();
    }
  }).catch(function(){});
}

function openAudioLiveModal(){
  audioLiveModalOpen=true;
  stopStatusPolling();
  audioHistoryRms=[];
  audioHistoryHz=[];
  document.getElementById('audio-live-rms').textContent=Math.round(lastLiveRms||0);
  document.getElementById('audio-live-hz').textContent=Math.round(lastLiveHz||0)+' Hz';
  document.getElementById('audio-live-modal').style.display='flex';
  pushAudioHistory(lastLiveRms,lastLiveHz);
  drawAudioGraph();
  loadLiveAudioSample();
  if(audioLiveTimer) clearInterval(audioLiveTimer);
  audioLiveTimer=setInterval(loadLiveAudioSample,180);
}

function closeAudioLiveModal(){
  audioLiveModalOpen=false;
  document.getElementById('audio-live-modal').style.display='none';
  if(audioLiveTimer){
    clearInterval(audioLiveTimer);
    audioLiveTimer=null;
  }
  startStatusPolling();
}

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

// ── Animation selector ────────────────────────────────────────
function setAnim(anim){
  for(var i=0;i<=4;i++){
    var b=document.getElementById('anim-'+i);
    if(b) b.classList.toggle('active',anim===i);
  }
  fetch('/setanim?a='+anim).catch(function(){});
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
  document.getElementById('p-effects').scrollTop=0;
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
  document.getElementById('p-effects').scrollTop=0;
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
      document.getElementById('p-effects').scrollTop=0;
      go('home',document.getElementById('b-home'),1);
      showToast('\u2713  Profile Saved');
      loadStatus(); // refresh home tab zone colours
    }).catch(function(){});
}

// ── Settings ──────────────────────────────────────────────────
var audioModalOpen=false;
var lastAudioSnapshot=null;
function openAbout(){document.getElementById('about-modal').style.display='flex';}
function closeAbout(){document.getElementById('about-modal').style.display='none';}
function applyAudioSnapshotToControls(a){
  if(!a||!a.f||a.f.length<4)return;
  var sq=document.getElementById('sl-quiet'); if(sq) sq.value=a.q;
  var sn=document.getElementById('sl-noisy'); if(sn) sn.value=a.n;
  var sf0=document.getElementById('sl-fb0'); if(sf0) sf0.value=a.f[0];
  var sf1=document.getElementById('sl-fb1'); if(sf1) sf1.value=a.f[1];
  var sf2=document.getElementById('sl-fb2'); if(sf2) sf2.value=a.f[2];
  var sf3=document.getElementById('sl-fb3'); if(sf3) sf3.value=a.f[3];

  var lq=document.getElementById('lbl-quiet'); if(lq) lq.textContent=a.q;
  var ln=document.getElementById('lbl-noisy'); if(ln) ln.textContent=a.n;
  var lf0=document.getElementById('lbl-fb0'); if(lf0) lf0.textContent=a.f[0]+' Hz';
  var lf1=document.getElementById('lbl-fb1'); if(lf1) lf1.textContent=a.f[1]+' Hz';
  var lf2=document.getElementById('lbl-fb2'); if(lf2) lf2.textContent=a.f[2]+' Hz';
  var lf3=document.getElementById('lbl-fb3'); if(lf3) lf3.textContent=a.f[3]+' Hz';
}
function openSensModal(){applyAudioSnapshotToControls(lastAudioSnapshot);document.getElementById('sens-modal').style.display='flex'; audioModalOpen=true;}
function closeSensModal(){audioModalOpen=false;applyAudioSnapshotToControls(lastAudioSnapshot);document.getElementById('sens-modal').style.display='none';}
function openFreqModal(){applyAudioSnapshotToControls(lastAudioSnapshot);document.getElementById('freq-modal').style.display='flex'; audioModalOpen=true;}
function closeFreqModal(){audioModalOpen=false;applyAudioSnapshotToControls(lastAudioSnapshot);document.getElementById('freq-modal').style.display='none';}
function showFeatureModal(name){document.getElementById('feature-name').textContent=name;document.getElementById('feature-title').textContent=name;document.getElementById('feature-modal').style.display='flex';}
function closeFeatureModal(){document.getElementById('feature-modal').style.display='none';}

function fmtBytes(v){
  var n=parseInt(v||0,10),u=['B','KB','MB'];
  var i=0,x=n;
  while(x>=1024&&i<u.length-1){x/=1024;i++;}
  return (i===0?x.toString():x.toFixed(1))+' '+u[i];
}

function openSysModal(){
  sysModalOpen=true;
  document.getElementById('sys-modal').style.display='flex';
  loadSysInfo();
}
function closeSysModal(){
  sysModalOpen=false;
  document.getElementById('sys-modal').style.display='none';
}

function setSpec(id,val){
  var el=document.getElementById(id); if(el) el.textContent=(val===undefined||val===null||val==='')?'-':val;
}

function setMeter(percent,meta){
  var fill=document.getElementById('spec-sto-fill');
  var lbl=document.getElementById('spec-sto-meta');
  var p=Math.max(0,Math.min(100,Number(percent)||0));
  if(fill) fill.style.width=p.toFixed(1)+'%';
  if(lbl) lbl.textContent=meta||'-';
}

function applySysInfo(d){
  if(!d)return;
  var fw=d.firmware||{},chip=d.chip||{},mem=d.memory||{},sto=d.storage||{},up=d.uptime||{},net=d.network||{};
  setSpec('spec-fw',(fw.version||'-'));
  setSpec('spec-build',(fw.buildDate||'-')+' '+(fw.buildTime||''));
  setSpec('spec-sdk',fw.sdk||'-');
  setSpec('spec-up',up.pretty||'-');
  setSpec('spec-cpu',(chip.cpuMHz!==undefined?chip.cpuMHz:'-')+' MHz');
  setSpec('spec-chip',chip.model||'-');
  setSpec('spec-rev',chip.revision);
  setSpec('spec-heap',fmtBytes(mem.freeHeap));
  setSpec('spec-minheap',fmtBytes(mem.minFreeHeap));
  setSpec('spec-sketch',fmtBytes(mem.sketchSize));
  setSpec('spec-flash',fmtBytes(mem.flashSize)+(mem.flashSpeed?(' @ '+Math.round(mem.flashSpeed/1000000)+' MHz'):''));
  var fallbackApIp=net.fallbackApIp||net.apIp||'-';
  setSpec('spec-ap',(net.apSsid||'-')+' @ '+(net.apIp||'-')+' (fallback '+fallbackApIp+')');
  setSpec('spec-sta',net.staConnected?((net.staSsid||'-')+' @ '+(net.staIp||'-')):'Disconnected');
  var mdnsHost=(net.mdns&&net.mdns.host)?net.mdns.host:((net.hostname?net.hostname+'.local':'-'));
  setSpec('spec-host',mdnsHost);
  setSpec('spec-clients',net.apClients);
  var used=fmtBytes(sto.firmwareUsed), total=fmtBytes(sto.firmwareTotal), free=fmtBytes(sto.firmwareFree);
  var pct=(sto.firmwareUsedPct!==undefined)?Number(sto.firmwareUsedPct):0;
  setMeter(pct,used+' used of '+total+' ('+pct.toFixed(1)+'%), free '+free);
}

function loadSysInfo(){
  fetch('/sysinfo').then(function(r){return r.text();}).then(function(t){
    try{
      var d=JSON.parse(t);
      if(sysModalOpen) applySysInfo(d);
    }catch(_e){
      if(sysModalOpen) showToast('System info unavailable');
    }
  }).catch(function(){ if(sysModalOpen) showToast('System info unavailable'); });
}

function openWifiModal(){
  wifiModalOpen=true;
  document.getElementById('wifi-modal').style.display='flex';
  loadWiFiConfig();
  loadNetStatus();
}
function closeWifiModal(){
  wifiModalOpen=false;
  document.getElementById('wifi-modal').style.display='none';
}

function togglePw(inputId,btnId){
  var inp=document.getElementById(inputId),btn=document.getElementById(btnId);
  if(!inp||!btn)return;
  var show=(inp.type==='password');
  inp.type=show?'text':'password';
  btn.textContent=show?'Hide':'Show';
}

function maskMeta(labelId,len,label){
  var el=document.getElementById(labelId); if(!el)return;
  if(len>0) el.textContent=label+' is saved ('+len+' chars). Enter a new value only if you want to change it.';
  else el.textContent='No '+label.toLowerCase()+' saved yet.';
}

function updateWiFiPills(s){
  var ap=document.getElementById('wifi-pill-ap');
  var sta=document.getElementById('wifi-pill-sta');
  var pen=document.getElementById('wifi-pill-pending');
  if(!ap||!sta||!pen||!s)return;
  ap.className='wifi-pill'+(s.ap&&s.ap.enabled?' ok':'');
  ap.textContent=(s.ap&&s.ap.enabled)?'AP Online':'AP Offline';
  sta.className='wifi-pill'+(s.sta&&s.sta.connected?' ok':' warn');
  sta.textContent=(s.sta&&s.sta.connected)?'STA Connected':'STA Not Connected';
  var pending=!!s.pendingRestart;
  pen.className='wifi-pill'+(pending?' warn':' ok');
  pen.textContent=pending?'Pending Restart':'No Pending Changes';
}

function renderWiFiCurrentConfig(c){
  var live=document.getElementById('wf-live'); if(!live||!c)return;
  var ap=(c.ap||{}), sta=(c.sta||{});
  var apIp=ap.ip||'-';
  var fallbackApIp=apIp;
  var staSsid=sta.ssid||'-';
  var mdnsHost=(c.host&&c.host.length)?(c.host+'.local'):'-';
  live.innerHTML='AP IP: <b style="color:var(--tx)">'+apIp+'</b> &middot; Fallback AP IP: <b style="color:var(--tx)">'+fallbackApIp+'</b><br>'
    +'Home Wi-Fi SSID: <b style="color:var(--tx)">'+staSsid+'</b><br>'
    +'mDNS: <b style="color:var(--tx)">'+mdnsHost+'</b>';
}

function loadWiFiConfig(){
  fetch('/netcfg').then(function(r){return r.json();}).then(function(d){
    wifiCfgCache=d;
    var ap=d.ap||{},sta=d.sta||{};
    var apS=document.getElementById('wf-ap-ssid'); if(apS) apS.value=ap.ssid||'';
    var apI=document.getElementById('wf-ap-ip'); if(apI) apI.value=ap.ip||'4.3.2.1';
    var staS=document.getElementById('wf-sta-ssid'); if(staS) staS.value=sta.ssid||'';
    var host=document.getElementById('wf-host'); if(host) host.value=d.host||'';
    var apP=document.getElementById('wf-ap-pass'); if(apP) apP.value='';
    var staP=document.getElementById('wf-sta-pass'); if(staP) staP.value='';
    maskMeta('wf-ap-pass-meta',ap.passLen||0,'AP Password');
    maskMeta('wf-sta-pass-meta',sta.passLen||0,'Router Password');
    renderWiFiCurrentConfig(d);
    if(d.pendingRestart!==undefined) updateWiFiPills({ap:{enabled:true},sta:{connected:false},pendingRestart:d.pendingRestart,hostname:d.host});
  }).catch(function(){showToast('Wi-Fi config load failed');});
}

function loadNetStatus(){
  fetch('/netstatus').then(function(r){return r.json();}).then(function(d){
    wifiStatusCache=d;
    if(wifiModalOpen){
      updateWiFiPills(d);
    }
  }).catch(function(){});
}

function saveApConfig(){
  var ssid=(document.getElementById('wf-ap-ssid').value||'').trim();
  var pass=(document.getElementById('wf-ap-pass').value||'').trim();
  var ip=(document.getElementById('wf-ap-ip').value||'').trim();
  if(ssid.length<1||ssid.length>32){showToast('Invalid AP SSID');return;}
  if(pass.length>0&&(pass.length<8||pass.length>63)){showToast('AP pass must be 8-63 or empty');return;}
  if(pass.length===0&&wifiCfgCache&&wifiCfgCache.ap&&wifiCfgCache.ap.passLen>0){showToast('Enter AP password to keep/update security');return;}
  fetch('/setwifiap?ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent(pass)+'&ip='+encodeURIComponent(ip))
    .then(function(r){return r.text().then(function(t){return{ok:r.ok,text:t};});})
    .then(function(x){
      if(!x.ok){showToast(x.text||'AP save failed');return;}
      showToast('AP settings saved');
      loadWiFiConfig();
      loadNetStatus();
    }).catch(function(){showToast('AP save failed');});
}

function saveStaConfig(){
  var ssid=(document.getElementById('wf-sta-ssid').value||'').trim();
  var pass=(document.getElementById('wf-sta-pass').value||'').trim();
  if(ssid.length>32){showToast('Invalid router SSID');return;}
  if(ssid.length>0&&pass.length===0&&wifiCfgCache&&wifiCfgCache.sta&&wifiCfgCache.sta.passLen>0){showToast('Enter router password to keep/update STA');return;}
  if(pass.length>0&&(pass.length<8||pass.length>63)){showToast('Router pass must be 8-63 or empty');return;}
  fetch('/setwifista?ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent(pass))
    .then(function(r){return r.text().then(function(t){return{ok:r.ok,text:t};});})
    .then(function(x){
      if(!x.ok){showToast(x.text||'STA save failed');return;}
      showToast('Home Wi-Fi saved');
      loadWiFiConfig();
      loadNetStatus();
    }).catch(function(){showToast('STA save failed');});
}

function forgetStaConfig(){
  showActionModal('Forget Home Wi-Fi','This will clear saved router credentials. Continue?','Forget',true,function(){
    fetch('/forgetsta').then(function(r){return r.text().then(function(t){return{ok:r.ok,text:t};});})
      .then(function(x){
        if(!x.ok){showToast(x.text||'Forget failed');return;}
        showToast('Home Wi-Fi forgotten');
        loadWiFiConfig();
        loadNetStatus();
      }).catch(function(){showToast('Forget failed');});
  });
}

function saveHostConfig(){
  var host=(document.getElementById('wf-host').value||'').trim().toLowerCase();
  if(!/^[a-z0-9-]{1,32}$/.test(host)){showToast('Invalid hostname');return;}
  fetch('/sethost?name='+encodeURIComponent(host))
    .then(function(r){return r.text().then(function(t){return{ok:r.ok,text:t};});})
    .then(function(x){
      if(!x.ok){showToast(x.text||'Host save failed');return;}
      showToast('Hostname saved');
      loadWiFiConfig();
      loadNetStatus();
    }).catch(function(){showToast('Host save failed');});
}

function applyNetworkChanges(){
  showActionModal('Apply Network Changes','Apply pending network changes and reboot now?','Apply & Reboot',false,function(){
    fetch('/applynetwork').then(function(r){return r.text().then(function(t){return{ok:r.ok,text:t};});})
      .then(function(x){
        if(!x.ok){showToast(x.text||'Apply failed');return;}
        showToast('Applying network config...');
      }).catch(function(){showToast('Apply failed');});
  });
}

function discardNetworkChanges(){
  showActionModal('Discard Staged Changes','Discard all staged network changes and restore currently applied configuration?','Discard',true,function(){
    fetch('/discardnetwork').then(function(r){return r.text().then(function(t){return{ok:r.ok,text:t};});})
      .then(function(x){
        if(!x.ok){showToast(x.text||'Discard failed');return;}
        showToast('Staged changes discarded');
        loadWiFiConfig();
        loadNetStatus();
      }).catch(function(){showToast('Discard failed');});
  });
}

function onSensInput(){
  var sq=document.getElementById('sl-quiet'), sn=document.getElementById('sl-noisy');
  var vq=parseInt(sq.value), vn=parseInt(sn.value);
  if(vq>=vn) sq.value=vn-10; // Cap quiet to always be lower than noisy
  document.getElementById('lbl-quiet').textContent=sq.value;
  document.getElementById('lbl-noisy').textContent=sn.value;
}
function onFreqInput(idx){
  var s0=document.getElementById('sl-fb0'), s1=document.getElementById('sl-fb1'), s2=document.getElementById('sl-fb2'), s3=document.getElementById('sl-fb3');
  var v0=parseInt(s0.value), v1=parseInt(s1.value), v2=parseInt(s2.value), v3=parseInt(s3.value);
  if(idx===0){ if(v0>=v1) s0.value=v1-10; }
  else if(idx===1){ if(v1<=v0) s1.value=v0+10; if(v1>=v2) s1.value=v2-10; }
  else if(idx===2){ if(v2<=v1) s2.value=v1+10; if(v2>=v3) s2.value=v3-10; }
  else if(idx===3){ if(v3<=v2) s3.value=v2+10; }
  document.getElementById('lbl-fb0').textContent=s0.value+' Hz';
  document.getElementById('lbl-fb1').textContent=s1.value+' Hz';
  document.getElementById('lbl-fb2').textContent=s2.value+' Hz';
  document.getElementById('lbl-fb3').textContent=s3.value+' Hz';
}

function saveAudioTuning(){
  var q=document.getElementById('sl-quiet').value;
  var n=document.getElementById('sl-noisy').value;
  var f0=document.getElementById('sl-fb0').value;
  var f1=document.getElementById('sl-fb1').value;
  var f2=document.getElementById('sl-fb2').value;
  var f3=document.getElementById('sl-fb3').value;
  fetch('/setaudio?q='+q+'&n='+n+'&f0='+f0+'&f1='+f1+'&f2='+f2+'&f3='+f3)
    .then(function(){
      showToast('\u2713  Tuning Saved');
      closeSensModal(); closeFreqModal();
    }).catch(function(){});
}
var pendingAction=null;
function showActionModal(title,msg,btnText,danger,actionFn){
  document.getElementById('action-title').textContent=title;
  document.getElementById('action-msg').textContent=msg;
  var btn=document.getElementById('action-btn'), ico=document.getElementById('action-icon');
  btn.textContent=btnText;
  if(danger){
    btn.style.background='rgba(180,60,60,.12)'; btn.style.color='#e07070'; btn.style.border='1px solid #7a3a3a';
    ico.style.color='#e07070';
  } else {
    btn.style.background='var(--sf2)'; btn.style.color='var(--tx)'; btn.style.border='1px solid var(--bd)';
    ico.style.color='var(--dim)';
  }
  pendingAction=actionFn;
  document.getElementById('action-modal').style.display='flex';
}
function closeActionModal(){document.getElementById('action-modal').style.display='none'; pendingAction=null;}
function confirmAction(){if(pendingAction)pendingAction(); closeActionModal();}

function rebootDevice(){showActionModal('Reboot Device','Are you sure you want to reboot the lamp?','Reboot',false,function(){fetch('/reboot').then(function(){showToast('Rebooting...');}).catch(function(){});});}
function factoryReset(){showActionModal('Factory Reset','WARNING: This will delete all custom profiles and reset to factory defaults. Continue?','Reset',true,function(){fetch('/reset').then(function(){showToast('Resetting...');}).catch(function(){});});}

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
  if(d.live){
    setLiveAudio(d.live.rms,d.live.hz);
  }
  if(!sliderInteracted&&d.brightness!==undefined){
    document.getElementById('brightSlider').value=d.brightness;
    document.getElementById('bval').textContent=d.brightness;
  }
  // Sync animation buttons
  if(d.anim!==undefined){
    for(var i=0;i<=4;i++){
      var a=document.getElementById('anim-'+i);
      if(a) a.classList.toggle('active',d.anim===i);
    }
  }
  // Sync audio tunings
  if(d.audio){
    lastAudioSnapshot={q:d.audio.q,n:d.audio.n,f:[d.audio.f[0],d.audio.f[1],d.audio.f[2],d.audio.f[3]]};
    if(!audioModalOpen) applyAudioSnapshotToControls(lastAudioSnapshot);
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
      var animNames=['Solid','Matrix','Twinkle','Ripple','Wipe'];
      var aLabel=animNames[d.anim]||'Solid';
      pb.textContent='Profile: '+pLabel+' | Anim: '+aLabel;
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
  setAccessHostLabel();
  loadStatus();
  startStatusPolling();
  setInterval(loadNetStatus,3000);
  setInterval(function(){if(sysModalOpen)loadSysInfo();},3000);
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

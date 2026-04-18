#pragma once

/*
 * webui.h — Web Control Panel UI  (Phase 1)
 * Edit this file to change the look of the web interface.
 * Served at http://4.3.2.1 when connected to the Emotion-Lamp AP.
 */

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1.0"/>
<title>Emotion Lamp</title>
<style>
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
:root{
  --bg:#111;--surface:#1a1a1a;--surface2:#222;
  --border:#2c2c2c;--text:#dedede;--dim:#555;
  --accent:#b8a0f0;--on:#c8ffc8;--nav:64px;--sidebar:200px;
}
html,body{height:100%;overflow:hidden}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;
  background:var(--bg);color:var(--text);display:flex;height:100dvh}

/* ── Sidebar ── */
.sidebar{display:none;width:var(--sidebar);background:var(--surface);
  border-right:1px solid var(--border);flex-direction:column;flex-shrink:0}
.sb-head{padding:22px 16px 18px;border-bottom:1px solid var(--border)}
.sb-head h1{font-size:.9rem;font-weight:700;letter-spacing:.08em;
  text-transform:uppercase;color:var(--text)}
.sb-head p{font-size:.7rem;color:var(--dim);margin-top:3px}
.sb-nav{padding:8px 0;flex:1}
.nitem{display:flex;align-items:center;gap:10px;padding:10px 16px;
  cursor:pointer;color:var(--dim);font-size:.85rem;font-weight:500;
  border-left:3px solid transparent;transition:all .15s;user-select:none}
.nitem:hover{background:var(--surface2);color:var(--text)}
.nitem.active{color:var(--accent);border-left-color:var(--accent);
  background:rgba(184,160,240,.08)}

/* ── Content ── */
.content{flex:1;overflow-y:auto;display:flex;flex-direction:column;
  padding-bottom:var(--nav)}
.panel{display:none;flex-direction:column;flex:1;min-height:100%}
.panel.active{display:flex}

/* ── Page header ── */
.ph{display:flex;align-items:center;justify-content:space-between;
  padding:20px 20px 0}
.ph h2{font-size:1.2rem;font-weight:700}
.badge{font-size:.68rem;font-weight:700;letter-spacing:.06em;padding:4px 10px;
  border-radius:20px;border:1px solid var(--border);color:var(--dim);
  background:rgba(255,255,255,.04);transition:all .3s;text-transform:uppercase}
.badge.on{color:var(--on);border-color:#3a7a3a;background:rgba(100,200,100,.08)}

/* ── Home: power section ── */
.power-wrap{flex:1;display:flex;flex-direction:column;align-items:center;
  justify-content:center;gap:18px;padding:32px 20px}
.power-btn{width:130px;height:130px;border-radius:50%;border:2.5px solid #333;
  background:var(--surface);display:flex;align-items:center;justify-content:center;
  cursor:pointer;transition:border-color .3s,box-shadow .3s,transform .1s;
  -webkit-tap-highlight-color:transparent;user-select:none}
.power-btn:active{transform:scale(.93)}
.power-btn svg{width:50px;height:50px;stroke:#444;transition:stroke .3s}
.power-btn.on{border-color:#d0d0d0;
  box-shadow:0 0 0 5px rgba(255,255,255,.05),0 0 24px rgba(255,255,255,.1)}
.power-btn.on svg{stroke:#e8e8e8}
.plabel{font-size:.75rem;font-weight:700;letter-spacing:.12em;color:var(--dim);
  text-transform:uppercase;transition:color .3s}
.plabel.on{color:var(--text)}

/* ── Info chips ── */
.chips{display:flex;gap:10px;padding:0 20px 24px;flex-wrap:wrap}
.chip{background:var(--surface);border:1px solid var(--border);border-radius:8px;
  padding:9px 14px;font-size:.72rem;color:var(--dim);flex:1;min-width:90px;text-align:center}
.chip strong{display:block;color:var(--text);font-size:.82rem;margin-bottom:2px}

/* ── Empty tab placeholder ── */
.empty{flex:1;display:flex;flex-direction:column;align-items:center;
  justify-content:center;gap:12px;padding:40px;color:var(--dim)}
.empty svg{opacity:.2}
.empty h3{font-size:.95rem;font-weight:600}
.empty p{font-size:.78rem;text-align:center;max-width:240px;line-height:1.5}

/* ── Bottom nav (mobile) ── */
.bnav{position:fixed;bottom:0;left:0;right:0;height:var(--nav);
  background:var(--surface);border-top:1px solid var(--border);
  display:flex;z-index:50}
.bitem{flex:1;display:flex;flex-direction:column;align-items:center;
  justify-content:center;gap:4px;cursor:pointer;color:var(--dim);
  font-size:.6rem;font-weight:600;letter-spacing:.04em;text-transform:uppercase;
  transition:color .15s;user-select:none;-webkit-tap-highlight-color:transparent}
.bitem.active{color:var(--accent)}
.bitem.active svg{stroke:var(--accent)}
.bitem svg{stroke:var(--dim);transition:stroke .15s}

/* ── Desktop ── */
@media(min-width:640px){
  .sidebar{display:flex}
  .bnav{display:none}
  .content{padding-bottom:0}
}
</style>
</head>
<body>

<!-- Sidebar (desktop) -->
<nav class="sidebar">
  <div class="sb-head">
    <h1>Emotion Lamp</h1>
    <p>ESP32 &middot; 4.3.2.1</p>
  </div>
  <div class="sb-nav">
    <div class="nitem active" onclick="go('home',this)">
      <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/><polyline points="9 22 9 12 15 12 15 22"/></svg>
      Home
    </div>
    <div class="nitem" onclick="go('effects',this)">
      <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"/></svg>
      Effects
    </div>
    <div class="nitem" onclick="go('settings',this)">
      <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>
      Settings
    </div>
  </div>
</nav>

<!-- Main content -->
<main class="content">

  <!-- Home -->
  <div id="p-home" class="panel active">
    <div class="ph">
      <h2>Home</h2>
      <span class="badge" id="badge">OFF</span>
    </div>
    <div class="power-wrap">
      <div class="power-btn" id="pwrBtn" onclick="togglePower()">
        <svg viewBox="0 0 24 24" fill="none" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round">
          <path d="M18.36 6.64a9 9 0 1 1-12.73 0"/>
          <line x1="12" y1="2" x2="12" y2="12"/>
        </svg>
      </div>
      <span class="plabel" id="plabel">STANDBY</span>
    </div>
    <div class="chips">
      <div class="chip"><strong>ESP32</strong>WROOM-32E</div>
      <div class="chip"><strong>16</strong>LEDs</div>
      <div class="chip"><strong>4.3.2.1</strong>IP Address</div>
    </div>
  </div>

  <!-- Effects -->
  <div id="p-effects" class="panel">
    <div class="ph"><h2>Effects</h2></div>
    <div class="empty">
      <svg width="44" height="44" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"/></svg>
      <h3>Effects</h3>
      <p>Custom lighting effects will be available here in a future update.</p>
    </div>
  </div>

  <!-- Settings -->
  <div id="p-settings" class="panel">
    <div class="ph"><h2>Settings</h2></div>
    <div class="empty">
      <svg width="44" height="44" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>
      <h3>Settings</h3>
      <p>Device configuration options will be available here in a future update.</p>
    </div>
  </div>

</main>

<!-- Bottom nav (mobile) -->
<nav class="bnav">
  <div class="bitem active" id="b-home" onclick="go('home',this,1)">
    <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/><polyline points="9 22 9 12 15 12 15 22"/></svg>
    Home
  </div>
  <div class="bitem" id="b-effects" onclick="go('effects',this,1)">
    <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"/></svg>
    Effects
  </div>
  <div class="bitem" id="b-settings" onclick="go('settings',this,1)">
    <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>
    Settings
  </div>
</nav>

<script>
var lampOn=false;
var tabs=['home','effects','settings'];

function go(name,el,mobile){
  tabs.forEach(function(t){
    document.getElementById('p-'+t).classList.remove('active');
    var b=document.getElementById('b-'+t);if(b)b.classList.remove('active');
    document.querySelectorAll('.nitem').forEach(function(n,i){
      if(tabs[i]===t)n.classList.remove('active');
    });
  });
  document.getElementById('p-'+name).classList.add('active');
  var b=document.getElementById('b-'+name);if(b)b.classList.add('active');
  document.querySelectorAll('.nitem').forEach(function(n,i){
    if(tabs[i]===name)n.classList.add('active');
  });
}

function setUI(on){
  lampOn=on;
  var btn=document.getElementById('pwrBtn');
  var lbl=document.getElementById('plabel');
  var bdg=document.getElementById('badge');
  if(on){
    btn.classList.add('on');lbl.classList.add('on');
    lbl.textContent='ACTIVE';bdg.textContent='ON';bdg.classList.add('on');
  }else{
    btn.classList.remove('on');lbl.classList.remove('on');
    lbl.textContent='STANDBY';bdg.textContent='OFF';bdg.classList.remove('on');
  }
}

function togglePower(){
  fetch(lampOn?'/off':'/on').then(function(){setUI(!lampOn);}).catch(function(e){console.log(e);});
}
</script>
</body>
</html>
)rawliteral";

#pragma once

/*
 * webui.h — Web Control Panel UI  (Phase 2)
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
  --bg:#111;--sf:#1a1a1a;--sf2:#222;--bd:#2c2c2c;
  --tx:#dedede;--dim:#555;--accent:#b8a0f0;
  --on:#7dff9a;--nav:64px;--side:200px
}
html,body{height:100%;overflow:hidden}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;
  background:var(--bg);color:var(--tx);display:flex;height:100dvh}

/* sidebar */
.sidebar{display:none;width:var(--side);background:var(--sf);
  border-right:1px solid var(--bd);flex-direction:column;flex-shrink:0}
.sb-head{padding:22px 16px 18px;border-bottom:1px solid var(--bd)}
.sb-head h1{font-size:.88rem;font-weight:700;letter-spacing:.08em;text-transform:uppercase}
.sb-head p{font-size:.7rem;color:var(--dim);margin-top:3px}
.sb-nav{padding:8px 0;flex:1}
.ni{display:flex;align-items:center;gap:10px;padding:10px 16px;cursor:pointer;
  color:var(--dim);font-size:.84rem;font-weight:500;border-left:3px solid transparent;
  transition:all .15s;user-select:none}
.ni:hover{background:var(--sf2);color:var(--tx)}
.ni.active{color:var(--accent);border-left-color:var(--accent);background:rgba(184,160,240,.07)}

/* content */
.content{flex:1;overflow-y:auto;display:flex;flex-direction:column;padding-bottom:var(--nav)}
.panel{display:none;flex-direction:column;flex:1}
.panel.active{display:flex}

/* page header */
.ph{display:flex;align-items:center;justify-content:space-between;padding:20px 20px 0}
.ph h2{font-size:1.15rem;font-weight:700}
.badge{font-size:.67rem;font-weight:700;letter-spacing:.06em;padding:4px 10px;
  border-radius:20px;border:1px solid var(--bd);color:var(--dim);
  background:rgba(255,255,255,.04);transition:all .3s;text-transform:uppercase}
.badge.on{color:var(--on);border-color:#2d6b3a;background:rgba(100,220,130,.08)}

/* power */
.pw-wrap{display:flex;flex-direction:column;align-items:center;
  justify-content:center;gap:16px;padding:28px 20px 20px}
.pw-btn{width:120px;height:120px;border-radius:50%;border:2.5px solid #333;
  background:var(--sf);display:flex;align-items:center;justify-content:center;
  cursor:pointer;transition:border-color .3s,box-shadow .3s,transform .1s;
  -webkit-tap-highlight-color:transparent;user-select:none}
.pw-btn:active{transform:scale(.93)}
.pw-btn svg{width:48px;height:48px;stroke:#444;transition:stroke .3s}
.pw-btn.on{border-color:#ccc;box-shadow:0 0 0 5px rgba(255,255,255,.05),0 0 22px rgba(255,255,255,.09)}
.pw-btn.on svg{stroke:#e0e0e0}
.plbl{font-size:.73rem;font-weight:700;letter-spacing:.12em;color:var(--dim);
  text-transform:uppercase;transition:color .3s}
.plbl.on{color:var(--tx)}

/* live stats row */
.stats{display:flex;gap:8px;padding:0 20px 16px}
.stat{background:var(--sf);border:1px solid var(--bd);border-radius:8px;
  padding:8px 12px;flex:1;text-align:center;min-width:80px}
.stat strong{display:block;font-size:.95rem;font-weight:700;color:var(--tx)}
.stat span{font-size:.67rem;color:var(--dim);text-transform:uppercase;letter-spacing:.06em}

/* section */
.sec{padding:0 20px 20px}
.sec-title{font-size:.68rem;font-weight:700;letter-spacing:.1em;
  text-transform:uppercase;color:var(--dim);margin-bottom:10px;
  padding-bottom:8px;border-bottom:1px solid var(--bd)}

/* colour zones */
.zone-row{display:flex;align-items:center;gap:12px;padding:9px 0;
  border-bottom:1px solid rgba(255,255,255,.04)}
.zone-row:last-child{border-bottom:none}
.zswatch{width:20px;height:20px;border-radius:5px;flex-shrink:0;
  border:1px solid rgba(255,255,255,.1)}
.zinfo{flex:1}
.zname{font-size:.83rem;font-weight:600;color:var(--tx);display:block}
.zdesc{font-size:.68rem;color:var(--dim)}
.zlbl{font-size:.7rem;color:var(--dim);white-space:nowrap}

/* brightness slider */
.slider-wrap{display:flex;align-items:center;gap:12px;margin-top:12px}
.slider{flex:1;-webkit-appearance:none;appearance:none;height:4px;
  border-radius:2px;background:var(--bd);outline:none;cursor:pointer}
.slider::-webkit-slider-thumb{-webkit-appearance:none;appearance:none;
  width:18px;height:18px;border-radius:50%;background:var(--tx);cursor:pointer}
.slider::-moz-range-thumb{width:18px;height:18px;border-radius:50%;
  background:var(--tx);cursor:pointer;border:none}
.bval{font-size:.88rem;font-weight:700;min-width:28px;text-align:right}
.slider-icons{font-size:.9rem}

/* empty */
.empty{flex:1;display:flex;flex-direction:column;align-items:center;
  justify-content:center;gap:12px;padding:40px;color:var(--dim)}
.empty svg{opacity:.18}
.empty h3{font-size:.92rem;font-weight:600}
.empty p{font-size:.77rem;text-align:center;max-width:220px;line-height:1.5}

/* bottom nav */
.bnav{position:fixed;bottom:0;left:0;right:0;height:var(--nav);
  background:var(--sf);border-top:1px solid var(--bd);display:flex;z-index:50}
.bi{flex:1;display:flex;flex-direction:column;align-items:center;
  justify-content:center;gap:3px;cursor:pointer;color:var(--dim);
  font-size:.59rem;font-weight:600;letter-spacing:.04em;text-transform:uppercase;
  transition:color .15s;user-select:none;-webkit-tap-highlight-color:transparent}
.bi.active{color:var(--accent)}
.bi.active svg{stroke:var(--accent)!important}

/* desktop */
@media(min-width:640px){
  .sidebar{display:flex}
  .bnav{display:none}
  .content{padding-bottom:0}
}
</style>
</head>
<body>

<!-- Sidebar -->
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
    </div>

    <!-- Colour zones -->
    <div class="sec">
      <div class="sec-title">Colour Zones</div>
      <div id="zones-list">
        <!-- populated by JS from /status -->
        <div class="zone-row" style="justify-content:center;padding:16px 0">
          <span style="color:var(--dim);font-size:.78rem">Loading...</span>
        </div>
      </div>
    </div>
  </div>

  <!-- EFFECTS -->
  <div id="p-effects" class="panel">
    <div class="ph"><h2>Effects</h2></div>

    <!-- Brightness -->
    <div class="sec" style="padding-top:20px">
      <div class="sec-title">Brightness</div>
      <div class="slider-wrap">
        <span class="slider-icons">&#9681;</span>
        <input type="range" id="brightSlider" class="slider" min="0" max="255" value="140"
               oninput="onBright(this.value)"/>
        <span class="slider-icons">&#9728;</span>
        <span class="bval" id="bval">140</span>
      </div>
    </div>

    <!-- Effects placeholder -->
    <div class="empty">
      <svg width="44" height="44" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"/></svg>
      <h3>Effects</h3>
      <p>Custom lighting effects will be available here in a future update.</p>
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

<!-- Bottom nav -->
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
var sliderInteracted=false;  // once user touches slider, auto-refresh won't reset it

function go(name,el,mobile){
  tabs.forEach(function(t){
    document.getElementById('p-'+t).classList.remove('active');
    var b=document.getElementById('b-'+t);if(b)b.classList.remove('active');
  });
  document.querySelectorAll('.ni').forEach(function(n){n.classList.remove('active');});
  document.getElementById('p-'+name).classList.add('active');
  var b=document.getElementById('b-'+name);if(b)b.classList.add('active');
  if(mobile){
    var idx=tabs.indexOf(name);
    var nis=document.querySelectorAll('.ni');
    if(nis[idx])nis[idx].classList.add('active');
  } else { el.classList.add('active'); }
}

function setUI(on){
  lampOn=on;
  var btn=document.getElementById('pwrBtn');
  var lbl=document.getElementById('plbl');
  var bdg=document.getElementById('badge');
  if(on){
    btn.classList.add('on');lbl.classList.add('on');
    lbl.textContent='ACTIVE';bdg.textContent='ON';bdg.classList.add('on');
  }else{
    btn.classList.remove('on');lbl.classList.remove('on');
    lbl.textContent='STANDBY';bdg.textContent='OFF';bdg.classList.remove('on');
  }
}

function renderZones(zones){
  var list=document.getElementById('zones-list');
  if(!list||!zones)return;
  list.innerHTML='';
  zones.forEach(function(z){
    var d=document.createElement('div');
    d.className='zone-row';
    d.innerHTML='<div class="zswatch" style="background:'+z.hex+'"></div>'
      +'<div class="zinfo"><span class="zname">'+z.name+'</span>'
      +'<span class="zdesc">'+z.desc+'</span></div>'
      +'<span class="zlbl">'+z.label+'</span>';
    list.appendChild(d);
  });
}

function onBright(val){
  sliderInteracted=true;  // mark that user has manually set this
  document.getElementById('bval').textContent=val;
  clearTimeout(btimer);
  btimer=setTimeout(function(){
    fetch('/brightness?val='+val).catch(function(){});
  },300);  // 300ms debounce
}

function applyStatus(d){
  setUI(d.on);
  // Only update slider from server on first page load, never after user touches it
  if(!sliderInteracted && d.brightness!==undefined){
    document.getElementById('brightSlider').value=d.brightness;
    document.getElementById('bval').textContent=d.brightness;
  }
  renderZones(d.zones);
}

function togglePower(){
  fetch(lampOn?'/off':'/on').then(function(){setUI(!lampOn);}).catch(function(){});
}

// On-load sync + live refresh every 3s
function loadStatus(){
  fetch('/status').then(function(r){return r.json();}).then(applyStatus).catch(function(){});
}
window.addEventListener('load',function(){
  loadStatus();
  setInterval(loadStatus,3000);
});
</script>
</body>
</html>
)rawliteral";

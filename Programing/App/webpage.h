const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>BasketBot</title>
  <link href="https://fonts.googleapis.com/css2?family=Barlow+Condensed:wght@400;600;700;900&family=Rajdhani:wght@400;600;700&display=swap" rel="stylesheet">
  <style>
    :root {
      --orange:     #FF6B00;
      --orange-hot: #FF8C00;
      --blue-dark:  #0D1B2A;
      --blue-mid:   #1A3A5C;
      --blue-steel: #2E6DA4;
      --blue-light: #5BA3D9;
      --white:      #F0F4F8;
      --grey:       #8899AA;
      --danger:     #E63946;
    }

    * { box-sizing: border-box; margin: 0; padding: 0; -webkit-tap-highlight-color: transparent; }

    body {
      font-family: 'Rajdhani', sans-serif;
      background: var(--blue-dark);
      color: var(--white);
      min-height: 100vh;
      overflow-x: hidden;
      position: relative;
    }

    body::before {
      content: '';
      position: fixed;
      inset: 0;
      background:
        radial-gradient(ellipse 80% 50% at 50% 0%, rgba(255,107,0,0.08) 0%, transparent 70%),
        radial-gradient(ellipse 60% 40% at 50% 100%, rgba(46,109,164,0.15) 0%, transparent 70%),
        repeating-linear-gradient(90deg, transparent, transparent 59px, rgba(255,255,255,0.02) 59px, rgba(255,255,255,0.02) 60px);
      pointer-events: none;
      z-index: 0;
    }

    .app {
      position: relative;
      z-index: 1;
      max-width: 440px;
      margin: 0 auto;
      padding: 0 16px 32px;
    }

    /* HEADER */
    .header {
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px 0 12px;
    }

    .logo-svg {
      filter: drop-shadow(0 0 14px rgba(255,107,0,0.55));
    }

    /* CARDS */
    .card {
      background: linear-gradient(145deg, rgba(26,58,92,0.9), rgba(13,27,42,0.95));
      border: 1px solid rgba(255,107,0,0.15);
      border-radius: 16px;
      padding: 20px;
      margin-bottom: 14px;
      backdrop-filter: blur(10px);
      box-shadow: 0 4px 24px rgba(0,0,0,0.4), inset 0 1px 0 rgba(255,255,255,0.05);
      animation: slideUp 0.4s ease both;
    }
    .card:nth-child(2){ animation-delay:.05s }
    .card:nth-child(3){ animation-delay:.10s }
    .card:nth-child(4){ animation-delay:.15s }

    @keyframes slideUp { from{opacity:0;transform:translateY(20px)} to{opacity:1;transform:translateY(0)} }

    .card-title {
      font-family: 'Barlow Condensed', sans-serif;
      font-size: 1.1rem;
      font-weight: 700;
      letter-spacing: 3px;
      color: var(--orange);
      text-transform: uppercase;
      margin-bottom: 14px;
      display: flex;
      align-items: center;
      gap: 8px;
    }
    .card-title::before {
      content: '';
      display: block;
      width: 3px;
      height: 14px;
      background: var(--orange);
      border-radius: 2px;
    }

    /* MODE GRID */
    .mode-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 10px;
    }

    .mode-btn {
      position: relative;
      padding: 16px 8px;
      border: 2px solid rgba(255,107,0,0.2);
      border-radius: 12px;
      background: rgba(13,27,42,0.8);
      color: var(--grey);
      cursor: pointer;
      font-family: 'Barlow Condensed', sans-serif;
      font-weight: 700;
      text-align: center;
      transition: all 0.2s cubic-bezier(.25,.8,.25,1);
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 6px;
      overflow: hidden;
    }
    .mode-btn .icon    { font-size: 1.6rem; }
    .mode-btn .label   { font-size: 0.85rem; letter-spacing: 1px; }
    .mode-btn .sublabel{ font-size: 0.65rem; color: var(--grey); font-family:'Rajdhani',sans-serif; }
    .mode-btn:hover    { border-color:rgba(255,107,0,0.5); color:var(--white); transform:translateY(-1px); }
    .mode-btn.selected {
      border-color: var(--orange);
      background: linear-gradient(135deg, rgba(255,107,0,0.2), rgba(255,107,0,0.05));
      color: var(--white);
      box-shadow: 0 0 20px rgba(255,107,0,0.25), inset 0 1px 0 rgba(255,107,0,0.3);
    }

    /* PLAYER TOGGLE */
    .player-toggle {
      display: grid;
      grid-template-columns: 1fr 1fr 1fr 1fr;
      background: rgba(13,27,42,0.8);
      border: 1px solid rgba(255,107,0,0.15);
      border-radius: 10px;
      overflow: hidden;
    }
    .player-btn {
      padding: 13px;
      border: none;
      background: transparent;
      color: var(--grey);
      cursor: pointer;
      font-family: 'Barlow Condensed', sans-serif;
      font-weight: 700;
      font-size: 1rem;
      letter-spacing: 1px;
      transition: all 0.2s;
    }
    .player-btn + .player-btn { border-left: 1px solid rgba(255,107,0,0.1); }
    .player-btn.selected {
      background: linear-gradient(135deg, var(--orange), var(--orange-hot));
      color: white;
      box-shadow: inset 0 -2px 0 rgba(0,0,0,0.3);
    }

    /* DRUM PICKER */
    .drum-section { display: flex; flex-direction: column; gap: 16px; }
    .drum-row { display: flex; align-items: flex-start; justify-content: center; gap: 20px; }
    .drum-col { display: flex; flex-direction: column; align-items: center; gap: 8px; }
    .drum-col-label {
      font-family: 'Barlow Condensed', sans-serif;
      font-size: 1rem; font-weight: 700; letter-spacing: 2px;
      color: var(--white); text-transform: uppercase;
    }
    .drum {
      position: relative; width: 120px; height: 120px;
      overflow: hidden; border-radius: 14px;
      border: 2px solid rgba(255,107,0,0.35);
      background: rgba(13,27,42,0.95);
      cursor: grab; user-select: none; touch-action: none;
    }
    .drum:active { cursor: grabbing; }
    .drum::before, .drum::after {
      content: ''; position: absolute; left: 0; right: 0; z-index: 2; pointer-events: none;
    }
    .drum::before { top: 0;    height: 40px; background: linear-gradient(to bottom, rgba(13,27,42,1) 30%, transparent); }
    .drum::after  { bottom: 0; height: 40px; background: linear-gradient(to top,   rgba(13,27,42,1) 30%, transparent); }
    .drum-highlight {
      position: absolute; top: 50%; left: 0; right: 0; height: 40px;
      transform: translateY(-50%);
      border-top: 1.5px solid var(--orange);
      border-bottom: 1.5px solid var(--orange);
      background: rgba(255,107,0,0.12);
      pointer-events: none; z-index: 1;
    }
    .drum-list { position: absolute; left: 0; right: 0; top: 0; will-change: transform; }
    .drum-item {
      height: 40px; display: flex; align-items: center; justify-content: center;
      font-family: 'Barlow Condensed', sans-serif;
      font-size: 1.2rem; font-weight: 700; color: rgba(255,255,255,0.22);
      transition: color 0.12s, font-size 0.12s;
    }
    .drum-item.active {
      color: var(--orange);
      font-size: 1.5rem;
      text-shadow: 0 0 12px rgba(255,107,0,0.6);
    }
    .drum-arrows { display: flex; gap: 8px; }
    .drum-arrow {
      width: 52px; height: 34px; border-radius: 8px;
      border: 1px solid rgba(255,107,0,0.3);
      background: rgba(255,107,0,0.1); color: var(--orange);
      font-size: 1rem; display: flex; align-items: center; justify-content: center;
      cursor: pointer; transition: all 0.15s; font-weight: 700;
    }
    .drum-arrow:hover  { background: rgba(255,107,0,0.25); border-color: var(--orange); }
    .drum-arrow:active { transform: scale(0.91); background: rgba(255,107,0,0.35); }

    /* START BUTTON */
    .quick-cm-btn {
      font-family:'Barlow Condensed',sans-serif;font-size:0.85rem;font-weight:700;
      letter-spacing:1px;padding:7px 4px;border-radius:8px;cursor:pointer;
      border:1px solid rgba(255,255,255,0.12);background:rgba(255,255,255,0.05);
      color:var(--grey);transition:all 0.15s;
    }
    .quick-cm-btn:hover,.quick-cm-btn.active {
      background:rgba(255,107,0,0.18);border-color:rgba(255,107,0,0.5);
      color:var(--orange);
    }
    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance:none;width:22px;height:22px;border-radius:50%;
      background:var(--orange);cursor:pointer;
      box-shadow:0 0 8px rgba(255,107,0,0.5);border:2px solid var(--blue-dark);
    }
    input[type=range]::-moz-range-thumb {
      width:20px;height:20px;border-radius:50%;background:var(--orange);
      cursor:pointer;border:2px solid var(--blue-dark);
    }
    .btn-start {
      width: 100%;
      padding: 18px;
      border: none;
      border-radius: 14px;
      background: linear-gradient(135deg, var(--orange), #FF4500);
      color: white;
      font-family: 'Barlow Condensed', sans-serif;
      font-size: 1.3rem;
      font-weight: 900;
      letter-spacing: 3px;
      cursor: pointer;
      transition: all 0.2s;
      box-shadow: 0 4px 20px rgba(255,107,0,0.4), inset 0 1px 0 rgba(255,255,255,0.1);
      position: relative; overflow: hidden;
    }
    .btn-start::before {
      content:'';
      position:absolute; top:0; left:-100%;
      width:60%; height:100%;
      background:linear-gradient(90deg,transparent,rgba(255,255,255,0.15),transparent);
      transition: left 0.5s;
    }
    .btn-start:hover:not(:disabled)::before { left:150%; }
    .btn-start:hover:not(:disabled) { transform:translateY(-2px); box-shadow:0 8px 30px rgba(255,107,0,0.5); }
    .btn-start:disabled { background:rgba(255,255,255,0.05); color:var(--grey); box-shadow:none; cursor:not-allowed; }

    /* ── GAME PANEL ── */
    #game-panel { display: none; }

    .scoreboard-card {
      background: linear-gradient(145deg, rgba(26,58,92,0.95), rgba(13,27,42,0.98));
      border: 1px solid rgba(255,107,0,0.2);
      border-radius: 20px;
      overflow: hidden;
      margin-bottom: 14px;
      box-shadow: 0 8px 32px rgba(0,0,0,0.5);
    }

    .scoreboard-header {
      background: linear-gradient(90deg,rgba(255,107,0,0.15),rgba(46,109,164,0.15));
      padding: 12px 20px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      border-bottom: 1px solid rgba(255,255,255,0.05);
    }
    .mode-tag {
      font-family: 'Barlow Condensed',sans-serif;
      font-size: 0.7rem; font-weight: 700; letter-spacing: 3px;
      color: var(--orange);
    }
    .state-badge {
      display: flex; align-items: center; gap: 6px;
      font-family: 'Barlow Condensed',sans-serif;
      font-size: 0.75rem; font-weight: 700; letter-spacing: 2px;
      color: var(--grey);
    }
    .live-dot {
      width: 8px; height: 8px; border-radius: 50%;
      background: #00FF88; box-shadow: 0 0 8px #00FF88;
      animation: pulse-dot 1.5s ease-in-out infinite;
    }
    @keyframes pulse-dot { 0%,100%{opacity:1;transform:scale(1)} 50%{opacity:0.5;transform:scale(0.7)} }

    .scores-row {
      display: flex; align-items: stretch;
      padding: 24px 20px; gap: 0;
    }
    .player-col {
      flex: 1; text-align: center;
      display: flex; flex-direction: column; align-items: center; gap: 8px;
    }
    .player-label {
      font-family: 'Barlow Condensed',sans-serif;
      font-size: 0.7rem; font-weight: 700; letter-spacing: 3px;
      color: var(--grey); text-transform: uppercase;
    }
    .score-num {
      font-family: 'Barlow Condensed',sans-serif;
      font-size: 5rem; font-weight: 900; line-height: 1;
      transition: transform 0.15s, color 0.15s;
    }
    .score-num.p1 { color:var(--orange); text-shadow:0 0 30px rgba(255,107,0,0.4); }
    .score-num.p2 { color:var(--blue-light); text-shadow:0 0 30px rgba(91,163,217,0.4); }
    .score-num.bump { transform: scale(1.25); }

    .score-divider {
      display: flex; flex-direction: column;
      align-items: center; justify-content: center;
      padding: 0 16px; gap: 8px;
    }
    .divider-line { width:1px; flex:1; background:rgba(255,255,255,0.08); }
    .divider-vs {
      font-family: 'Barlow Condensed',sans-serif;
      font-size: 0.8rem; font-weight: 700;
      color: rgba(255,255,255,0.15); letter-spacing: 2px;
    }

    .timer-section {
      text-align: center;
      padding: 14px 20px 20px;
      border-top: 1px solid rgba(255,255,255,0.04);
    }
    .timer-num {
      font-family: 'Barlow Condensed',sans-serif;
      font-size: 3rem; font-weight: 900;
      color: var(--white); letter-spacing: 4px;
      text-shadow: 0 0 20px rgba(255,255,255,0.1);
    }
    .timer-num.warning { color:var(--danger); animation:blink 1s step-end infinite; }
    @keyframes blink { 0%,100%{opacity:1} 50%{opacity:0.3} }
    .timer-label { font-size:0.7rem; letter-spacing:3px; color:var(--grey); margin-top:2px; }

    /* MINI PLAYER CARD (juego 21) */
    .mini-player {
      display: flex; flex-direction: column; align-items: center; gap: 2px;
      padding: 8px 12px; border-radius: 10px;
      background: rgba(255,255,255,0.03);
      border: 1px solid rgba(255,255,255,0.06);
      min-width: 60px;
    }
    .mini-label {
      font-family: 'Barlow Condensed', sans-serif;
      font-size: 0.6rem; font-weight: 700; letter-spacing: 2px; color: var(--grey);
    }
    .mini-score {
      font-family: 'Barlow Condensed', sans-serif;
      font-size: 1.8rem; font-weight: 900; color: rgba(255,255,255,0.45); line-height: 1;
    }

    /* POINT BUTTONS */
    .point-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px; margin-bottom: 14px;
    }
    .point-btn {
      padding: 20px 10px;
      border-radius: 14px; border: none;
      cursor: pointer;
      font-family: 'Barlow Condensed',sans-serif;
      font-weight: 900; font-size: 1.1rem; letter-spacing: 1px;
      transition: all 0.15s;
      display: flex; flex-direction: column; align-items: center; gap: 4px;
    }
    .point-btn .btn-icon { font-size: 1.5rem; }
    .point-btn .btn-sub  { font-size: 0.65rem; letter-spacing: 2px; opacity: 0.7; }
    .btn-p1 {
      background: linear-gradient(135deg,rgba(255,107,0,0.25),rgba(255,107,0,0.1));
      color: var(--orange); border: 2px solid rgba(255,107,0,0.4);
    }
    .btn-p1:hover { background:linear-gradient(135deg,rgba(255,107,0,0.4),rgba(255,107,0,0.2)); box-shadow:0 4px 20px rgba(255,107,0,0.3); transform:translateY(-2px); }
    .btn-p1:active { transform:scale(0.96); }
    .btn-p2 {
      background: linear-gradient(135deg,rgba(91,163,217,0.25),rgba(91,163,217,0.1));
      color: var(--blue-light); border: 2px solid rgba(91,163,217,0.4);
    }
    .btn-p2:hover { background:linear-gradient(135deg,rgba(91,163,217,0.4),rgba(91,163,217,0.2)); box-shadow:0 4px 20px rgba(91,163,217,0.3); transform:translateY(-2px); }
    .btn-p2:active { transform:scale(0.96); }

    /* CONTROL BAR */
    .ctrl-bar {
      display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 10px;
    }
    .ctrl-btn {
      padding: 12px 8px;
      border-radius: 10px;
      border: 1px solid rgba(255,255,255,0.08);
      background: rgba(255,255,255,0.04);
      color: var(--grey); cursor: pointer;
      font-family: 'Barlow Condensed',sans-serif;
      font-weight: 700; font-size: 0.85rem; letter-spacing: 1px;
      transition: all 0.2s;
      display: flex; flex-direction: column; align-items: center; gap: 4px;
    }
    .ctrl-btn .btn-icon { font-size: 1.2rem; }
    .ctrl-btn:hover { background:rgba(255,255,255,0.08); color:var(--white); border-color:rgba(255,255,255,0.15); }
    #btn-pause.paused { color:#00FF88; border-color:rgba(0,255,136,0.3); background:rgba(0,255,136,0.05); }

    /* WIN OVERLAY */
    .win-overlay {
      display: none;
      position: fixed; inset: 0; z-index: 100;
      background: rgba(13,27,42,0.95);
      backdrop-filter: blur(10px);
      flex-direction: column; align-items: center; justify-content: center;
      text-align: center; padding: 32px;
    }
    .win-overlay.show { display: flex; }
    .win-trophy { font-size: 5rem; margin-bottom: 16px; animation: bounce 0.6s ease infinite alternate; }
    @keyframes bounce { from{transform:translateY(0)} to{transform:translateY(-12px)} }
    .win-title {
      font-family: 'Barlow Condensed',sans-serif;
      font-size: 3rem; font-weight: 900; color: var(--orange); line-height: 1; margin-bottom: 8px;
    }
    .win-sub { color:var(--grey); margin-bottom:32px; font-size:1rem; letter-spacing:2px; }
    .btn-win-reset {
      padding: 16px 40px;
      background: linear-gradient(135deg, var(--orange), #FF4500);
      border: none; border-radius: 12px; color: white;
      font-family: 'Barlow Condensed',sans-serif;
      font-size: 1.2rem; font-weight: 900; letter-spacing: 3px;
      cursor: pointer; box-shadow: 0 4px 20px rgba(255,107,0,0.4);
    }

    .status-msg {
      text-align: center; font-size: 0.8rem; letter-spacing: 2px;
      color: var(--grey); padding: 8px 0; min-height: 28px;
    }
    .status-msg.error { color: var(--danger); }
  </style>
</head>
<body>

<!-- WIN OVERLAY -->
<div class="win-overlay" id="win-overlay">
  <div class="win-trophy">🏆</div>
  <div class="win-title" id="win-title">¡GANADOR!</div>
  <div class="win-sub" id="win-sub">JUGADOR 1</div>
  <button class="btn-win-reset" onclick="resetGame()">NUEVA PARTIDA</button>
</div>

<div class="app">

  <!-- LOGO SVG -->
  <div class="header">
    <svg class="logo-svg" width="220" height="88" viewBox="0 0 360 144" xmlns="http://www.w3.org/2000/svg">
      <!-- Tablero -->
      <polygon points="30,10 250,10 290,35 290,100 250,100 30,100" fill="none" stroke="#FF6B00" stroke-width="8" stroke-linejoin="round"/>
      <!-- Aro -->
      <ellipse cx="160" cy="103" rx="38" ry="7" fill="none" stroke="#FF6B00" stroke-width="6"/>
      <!-- Red -->
      <line x1="128" y1="108" x2="136" y2="128" stroke="#FF6B00" stroke-width="3"/>
      <line x1="145" y1="110" x2="148" y2="130" stroke="#FF6B00" stroke-width="3"/>
      <line x1="162" y1="110" x2="162" y2="130" stroke="#FF6B00" stroke-width="3"/>
      <line x1="179" y1="110" x2="176" y2="130" stroke="#FF6B00" stroke-width="3"/>
      <line x1="194" y1="108" x2="188" y2="128" stroke="#FF6B00" stroke-width="3"/>
      <line x1="136" y1="128" x2="188" y2="128" stroke="#FF6B00" stroke-width="3"/>
      <!-- Estela -->
      <defs>
        <linearGradient id="tg" x1="0%" y1="0%" x2="100%" y2="0%">
          <stop offset="0%" stop-color="#FF6B00" stop-opacity="0"/>
          <stop offset="100%" stop-color="#FF6B00" stop-opacity="0.65"/>
        </linearGradient>
        <radialGradient id="bg" cx="35%" cy="30%">
          <stop offset="0%" stop-color="#FFB347"/>
          <stop offset="100%" stop-color="#CC4400"/>
        </radialGradient>
        <linearGradient id="txtg" x1="0%" y1="0%" x2="100%" y2="0%">
          <stop offset="0%" stop-color="#2E6DA4"/>
          <stop offset="100%" stop-color="#5BA3D9"/>
        </linearGradient>
      </defs>
      <path d="M100,90 Q130,70 210,30" fill="none" stroke="url(#tg)" stroke-width="14" stroke-linecap="round"/>
      <!-- Balón -->
      <circle cx="224" cy="26" r="28" fill="url(#bg)"/>
      <path d="M200,26 Q224,10 248,26" fill="none" stroke="#1a0a00" stroke-width="2.5"/>
      <path d="M200,26 Q224,42 248,26" fill="none" stroke="#1a0a00" stroke-width="2.5"/>
      <line x1="224" y1="0" x2="224" y2="54" stroke="#1a0a00" stroke-width="2.5"/>
      <!-- BASKET -->
      <text x="18" y="138" font-family="'Barlow Condensed',Arial Black,sans-serif" font-weight="900" font-size="52" fill="url(#txtg)" letter-spacing="2">BASKET</text>
      <!-- BOT -->
      <text x="200" y="138" font-family="'Barlow Condensed',Arial Black,sans-serif" font-weight="900" font-size="52" fill="#FF6B00" letter-spacing="2">BOT</text>
    </svg>
  </div>

  <!-- ═══ SETUP PANEL ═══ -->
  <div id="setup-panel">

    <div class="card">
      <div class="card-title">Modo de juego</div>
      <div class="mode-grid" style="grid-template-columns:1fr 1fr;gap:10px">
        <div class="mode-btn selected" onclick="selectMode('1vs1',this)">
          <span class="icon">🏀</span>
          <span class="label">1 VS 1</span>
          <span class="sublabel">Equipos · tiempo</span>
        </div>
        <div class="mode-btn" onclick="selectMode('21',this)">
          <span class="icon">🎯</span>
          <span class="label">JUEGO 21</span>
          <span class="sublabel">1-4 jugadores</span>
        </div>
        <div class="mode-btn" onclick="selectMode('shootout',this)">
          <span class="icon">⏱️</span>
          <span class="label">SHOOT OUT</span>
          <span class="sublabel">Turnos · más puntos</span>
        </div>
        <div class="mode-btn" onclick="selectMode('libre',this)">
          <span class="icon">🔓</span>
          <span class="label">LIBRE</span>
          <span class="sublabel">Un marcador</span>
        </div>
      </div>
    </div>

    <!-- ═══ TARJETA ALTURA CANASTA ═══ -->
    <div class="card" id="motor-card">
      <div class="card-title">Altura de canasta</div>

      <!-- Estado del motor -->
      <div id="motor-status-row" style="display:flex;align-items:center;gap:10px;
           margin-bottom:14px;padding:8px 12px;background:rgba(255,255,255,0.04);
           border-radius:10px;border:1px solid rgba(255,255,255,0.07);">
        <div id="motor-dot" style="width:8px;height:8px;border-radius:50%;
             background:#00FF88;box-shadow:0 0 6px #00FF88;flex-shrink:0;"></div>
        <div style="flex:1">
          <span id="motor-state-txt" style="font-family:'Barlow Condensed',sans-serif;
                font-size:0.75rem;font-weight:700;letter-spacing:2px;color:var(--grey);">
            LISTO
          </span>
          <span id="motor-pos-txt" style="font-family:'Barlow Condensed',sans-serif;
                font-size:0.75rem;letter-spacing:1px;color:var(--orange);margin-left:10px;">
            0 cm
          </span>
        </div>
        <button onclick="motorHoming()" style="font-family:'Barlow Condensed',sans-serif;
                font-size:0.7rem;font-weight:700;letter-spacing:2px;padding:5px 12px;
                border-radius:8px;border:1px solid rgba(255,255,255,0.15);
                background:rgba(255,255,255,0.06);color:var(--grey);cursor:pointer;">
          ⟳ CALIBRAR
        </button>
      </div>

      <!-- Slider medida 0-20 cm -->
      <div style="margin-bottom:6px;display:flex;justify-content:space-between;
                  align-items:baseline;">
        <div style="font-family:'Barlow Condensed',sans-serif;font-size:1rem;
             font-weight:700;letter-spacing:2px;color:var(--white);">
          MEDIDA
        </div>
        <div>
          <span id="motor-cm-val" style="font-family:'Barlow Condensed',sans-serif;
                font-size:2rem;font-weight:900;color:var(--orange);">0</span>
          <span style="font-family:'Barlow Condensed',sans-serif;font-size:0.9rem;
                color:var(--grey);letter-spacing:1px;margin-left:3px;">cm</span>
        </div>
      </div>

      <!-- Track slider personalizado -->
      <div style="position:relative;height:36px;display:flex;align-items:center;
                  margin-bottom:8px;">
        <input type="range" id="motor-slider" min="0" max="20" step="1" value="0"
               oninput="motorSliderInput(this.value)"
               onchange="motorSliderChange(this.value)"
               style="width:100%;-webkit-appearance:none;appearance:none;height:4px;
                      border-radius:2px;outline:none;cursor:pointer;
                      background:linear-gradient(to right,var(--orange) 0%,
                        rgba(255,255,255,0.15) 0%);">
      </div>

      <!-- Marcas 0 / 5 / 10 / 15 / 20 -->
      <div style="display:flex;justify-content:space-between;
                  font-family:'Barlow Condensed',sans-serif;font-size:0.6rem;
                  letter-spacing:1px;color:var(--grey);margin-bottom:4px;">
        <span>0</span><span>5</span><span>10</span><span>15</span><span>20 cm</span>
      </div>

      <!-- Botones rápidos -->
      <div style="display:grid;grid-template-columns:repeat(5,1fr);gap:6px;margin-top:10px;">
        <button class="quick-cm-btn" onclick="motorSetCm(0)">0</button>
        <button class="quick-cm-btn" onclick="motorSetCm(5)">5</button>
        <button class="quick-cm-btn" onclick="motorSetCm(10)">10</button>
        <button class="quick-cm-btn" onclick="motorSetCm(15)">15</button>
        <button class="quick-cm-btn" onclick="motorSetCm(20)">20</button>
      </div>
    </div>

    <div class="card">
      <div class="card-title">Configuración del partido</div>

      <!-- Selector jugadores (solo visible en modo 21) -->
      <div id="players-selector" style="display:none; margin-bottom:16px;">
        <div style="font-family:'Barlow Condensed',sans-serif;font-size:1rem;font-weight:700;letter-spacing:2px;color:var(--white);text-transform:uppercase;margin-bottom:10px;">Jugadores</div>
        <div class="player-toggle" id="player-toggle">
          <button class="player-btn" onclick="selectPlayers(1,this)">1</button>
          <button class="player-btn" onclick="selectPlayers(2,this)">2</button>
          <button class="player-btn" onclick="selectPlayers(3,this)">3</button>
          <button class="player-btn" onclick="selectPlayers(4,this)">4</button>
        </div>
      </div>

      <div class="drum-section">
        <div class="drum-row">
          <!-- Duracion: solo en modo 1vs1 -->
          <div class="drum-col" id="drum-col-time">
            <div class="drum-col-label">Duración</div>
            <div class="drum" id="drum-time">
              <div class="drum-highlight"></div>
              <div class="drum-list" id="drum-time-list"></div>
            </div>
            <div class="drum-arrows">
              <div class="drum-arrow" onclick="drumStep('time',-1)">▲</div>
              <div class="drum-arrow" onclick="drumStep('time',1)">▼</div>
            </div>
          </div>
          <!-- Tiempo por turno: solo shootout -->
          <div class="drum-col" id="drum-col-turno" style="display:none">
            <div class="drum-col-label">Tiempo/turno</div>
            <div class="drum" id="drum-turno">
              <div class="drum-highlight"></div>
              <div class="drum-list" id="drum-turno-list"></div>
            </div>
            <div class="drum-arrows">
              <div class="drum-arrow" onclick="drumStep('turno',-1)">▲</div>
              <div class="drum-arrow" onclick="drumStep('turno',1)">▼</div>
            </div>
          </div>
          <!-- Tiempo espera: modos 1vs1 y 21 -->
          <div class="drum-col" id="drum-col-ventana">
            <div class="drum-col-label">Tiempo espera</div>
            <div class="drum" id="drum-ventana">
              <div class="drum-highlight"></div>
              <div class="drum-list" id="drum-ventana-list"></div>
            </div>
            <div class="drum-arrows">
              <div class="drum-arrow" onclick="drumStep('ventana',-1)">▲</div>
              <div class="drum-arrow" onclick="drumStep('ventana',1)">▼</div>
            </div>
          </div>
        </div>
      </div>
    </div>

    <button class="btn-start" id="btn-start" onclick="startGame()" disabled>
      ▶ &nbsp; INICIAR PARTIDO
    </button>
    <div class="status-msg" id="setup-msg"></div>

  </div>

  <!-- ═══ GAME PANEL ═══ -->
  <div id="game-panel">

    <!-- === SCOREBOARD 1VS1 === -->
    <div id="sb-1vs1">
      <div class="scoreboard-card">
        <div class="scoreboard-header">
          <div class="mode-tag" id="sb-mode">1 VS 1</div>
          <div class="state-badge">
            <div class="live-dot" id="live-dot"></div>
            <span id="sb-state">EN JUEGO</span>
          </div>
        </div>
        <div class="scores-row">
          <div class="player-col">
            <div class="player-label">EQUIPO A</div>
            <div class="score-num p1" id="score1">0</div>
          </div>
          <div class="score-divider">
            <div class="divider-line"></div>
            <div class="divider-vs">VS</div>
            <div class="divider-line"></div>
          </div>
          <div class="player-col">
            <div class="player-label">EQUIPO B</div>
            <div class="score-num p2" id="score2">0</div>
          </div>
        </div>
        <div class="timer-section">
          <div class="timer-num" id="timer-display">00:00</div>
          <div class="timer-label">TIEMPO DE JUEGO</div>
        </div>
      </div>
      <div class="point-grid">
        <button class="point-btn btn-p1" onclick="addPoint(1)">
          <span class="btn-icon">🏀</span>
          <span>+1 EQUIPO A</span>
          <span class="btn-sub">PUNTO MANUAL</span>
        </button>
        <button class="point-btn btn-p2" onclick="addPoint(2)">
          <span class="btn-icon">🏀</span>
          <span>+1 EQUIPO B</span>
          <span class="btn-sub">PUNTO MANUAL</span>
        </button>
      </div>
    </div>

    <!-- === SCOREBOARD 21 === -->
    <div id="sb-21" style="display:none;">
      <div class="scoreboard-card">
        <div class="scoreboard-header">
          <div class="mode-tag">JUEGO 21</div>
          <div class="state-badge">
            <div class="live-dot" id="live-dot-21"></div>
            <span id="sb-state-21">EN JUEGO</span>
          </div>
        </div>
        <!-- Jugador activo grande -->
        <div id="active-player-section" style="padding:18px 20px 12px; text-align:center;">
          <div id="active-tiro-tag" style="font-family:'Barlow Condensed',sans-serif;font-size:0.75rem;letter-spacing:3px;color:var(--grey);margin-bottom:2px;">TIRO NORMAL</div>
          <div id="active-name" style="font-family:'Barlow Condensed',sans-serif;font-size:2rem;font-weight:900;letter-spacing:4px;color:var(--orange);margin-bottom:0;">JUGADOR 1</div>
          <div class="score-num p1" id="active-score" style="font-size:6.5rem;line-height:1;">0</div>
          <div id="ventana-bar-wrap" style="margin-top:8px;height:5px;background:rgba(255,255,255,0.08);border-radius:3px;overflow:hidden;">
            <div id="ventana-bar" style="height:100%;background:var(--orange);width:100%;border-radius:3px;transition:width 0.5s linear;"></div>
          </div>
          <div id="ventana-countdown" style="font-size:0.65rem;letter-spacing:2px;color:var(--grey);margin-top:4px;">—</div>
        </div>
        <!-- Todos los jugadores en fila, el activo resaltado -->
        <div id="all-players-row" style="display:flex;justify-content:space-around;align-items:flex-end;padding:10px 12px 16px;border-top:1px solid rgba(255,255,255,0.05);gap:8px;"></div>
      </div>
      <!-- Botones juego 21 -->
      <div class="point-grid" style="grid-template-columns:1fr 1fr;">
        <button class="point-btn btn-p1" onclick="g21Canasta()">
          <span class="btn-icon">🏀</span>
          <span id="g21-canasta-label">CANASTA</span>
          <span class="btn-sub">+2 / TIRO LIBRE +1</span>
        </button>
        <button class="point-btn btn-p2" onclick="g21Fallo()" style="color:#E63946;border-color:rgba(230,57,70,0.4);background:linear-gradient(135deg,rgba(230,57,70,0.15),rgba(230,57,70,0.05));">
          <span class="btn-icon">❌</span>
          <span>FALLO</span>
          <span class="btn-sub">CAMBIO DE TURNO</span>
        </button>
      </div>
    </div>

    <!-- === SCOREBOARD LIBRE === -->
    <div id="sb-libre" style="display:none;">
      <div class="scoreboard-card">
        <div class="scoreboard-header">
          <div class="mode-tag">MODO LIBRE</div>
          <div class="state-badge">
            <div class="live-dot" id="live-dot-libre"></div>
            <span id="sb-state-libre">EN JUEGO</span>
          </div>
        </div>
        <div style="padding:32px 20px; text-align:center;">
          <div style="font-family:'Barlow Condensed',sans-serif;font-size:0.7rem;font-weight:700;letter-spacing:3px;color:var(--grey);margin-bottom:4px;">CANASTAS TOTALES</div>
          <div class="score-num p1" id="libre-score" style="font-size:8rem;line-height:1;">0</div>
          <div style="font-size:0.7rem;letter-spacing:2px;color:var(--grey);margin-top:8px;">PUNTOS (×2 por canasta)</div>
        </div>
      </div>
      <div class="point-grid" style="grid-template-columns:1fr 1fr;">
        <button class="point-btn btn-p1" onclick="libreAdd()">
          <span class="btn-icon">🏀</span>
          <span>+1 CANASTA</span>
          <span class="btn-sub">+2 PUNTOS</span>
        </button>
        <button class="point-btn btn-p2" onclick="libreSub()" style="color:var(--grey);border-color:rgba(255,255,255,0.15);background:rgba(255,255,255,0.04);">
          <span class="btn-icon">↩</span>
          <span>DESHACER</span>
          <span class="btn-sub">-2 PUNTOS</span>
        </button>
      </div>
    </div>    </div>

    <!-- === SCOREBOARD SHOOT OUT === -->
    <div id="sb-shootout" style="display:none">

      <!-- ── Overlay cuenta atrás (superpuesto a toda la pantalla) ── -->
      <div id="so-overlay" style="display:none;position:fixed;inset:0;z-index:100;
           background:rgba(13,27,42,0.97);flex-direction:column;align-items:center;
           justify-content:center;text-align:center;">
        <div id="so-ov-jugador" style="font-family:'Barlow Condensed',sans-serif;
             font-size:1.1rem;font-weight:700;letter-spacing:4px;color:var(--grey);
             margin-bottom:6px;">JUGADOR 1</div>
        <div id="so-ov-num" style="font-family:'Barlow Condensed',sans-serif;
             font-size:13rem;font-weight:900;color:var(--orange);line-height:1;
             text-shadow:0 0 80px rgba(255,107,0,0.6);">3</div>
        <div style="font-family:'Barlow Condensed',sans-serif;font-size:0.9rem;
             letter-spacing:4px;color:var(--grey);margin-top:10px;">PREPARADO...</div>
      </div>

      <div class="scoreboard-card">
        <div class="scoreboard-header">
          <div class="mode-tag">SHOOT OUT</div>
          <div class="state-badge">
            <div class="live-dot" id="live-dot-so"></div>
            <span id="sb-state-so">EN JUEGO</span>
          </div>
        </div>

        <!-- Jugador activo -->
        <div style="padding:16px 20px 10px;text-align:center;
                    border-bottom:1px solid rgba(255,255,255,0.05)">
          <div id="so-ronda-tag" style="font-family:'Barlow Condensed',sans-serif;
               font-size:0.65rem;font-weight:700;letter-spacing:3px;
               color:var(--grey);margin-bottom:2px;">RONDA 1</div>
          <div id="so-nombre" style="font-family:'Barlow Condensed',sans-serif;
               font-size:2.2rem;font-weight:900;letter-spacing:4px;
               color:var(--orange);line-height:1;">JUGADOR 1</div>
          <!-- Timer -->
          <div id="so-timer" style="font-family:'Barlow Condensed',sans-serif;
               font-size:3.8rem;font-weight:900;letter-spacing:3px;
               color:var(--white);line-height:1;margin:4px 0 2px;">--</div>
          <!-- Score del turno -->
          <div class="score-num p1" id="so-score-activo"
               style="font-size:5rem;line-height:1;">0</div>
          <!-- Barra progreso tiempo -->
          <div style="margin-top:10px;height:5px;background:rgba(255,255,255,0.08);
                      border-radius:3px;overflow:hidden">
            <div id="so-barra" style="height:100%;width:100%;background:var(--orange);
                 border-radius:3px;transition:width 0.4s linear;"></div>
          </div>
        </div>

        <!-- Mini tarjetas todos los jugadores -->
        <div id="so-todos" style="display:flex;justify-content:space-around;
             padding:10px 12px 14px;gap:6px;"></div>
      </div>

      <!-- Botón LISTO (visible solo cuando state=Listo) -->
      <div id="so-listo-wrap" style="display:none;margin-bottom:12px">
        <button class="btn-start"
                style="background:linear-gradient(135deg,#00C853,#007A29);
                       box-shadow:0 4px 20px rgba(0,200,83,0.35);"
                onclick="soListo()">
          ✋ &nbsp; LISTO — SOY EL J<span id="so-listo-num">1</span>
        </button>
        <div style="text-align:center;font-size:0.7rem;letter-spacing:2px;
                    color:var(--grey);margin-top:8px;">
          Pulsa el botón físico o el de arriba
        </div>
      </div>

      <!-- Botón canasta manual (visible solo cuando state=Jugando) -->
      <div id="so-canasta-wrap" style="display:none;margin-bottom:12px">
        <button class="point-btn btn-p1" style="width:100%;padding:20px 0"
                onclick="soCanastaManual()">
          <span class="btn-icon">🏀</span>
          <span>CANASTA MANUAL</span>
          <span class="btn-sub">+1 PUNTO</span>
        </button>
      </div>

    </div>
    <div class="ctrl-bar">
      <button class="ctrl-btn" id="btn-pause" onclick="togglePause()">
        <span class="btn-icon">⏸</span><span>PAUSAR</span>
      </button>
      <button class="ctrl-btn" onclick="resetGame()">
        <span class="btn-icon">🔄</span><span>RESET</span>
      </button>
      <button class="ctrl-btn" onclick="goBack()">
        <span class="btn-icon">⚙️</span><span>CONFIG</span>
      </button>
    </div>

    <div class="status-msg" id="game-msg"></div>
  </div>

</div>

<script>
  // ── Estado global ─────────────────────────────────────────────────────────
  let selectedMode    = '1vs1';
  let selectedPlayers = 2;
  let pollingInterval = null;
  let paused          = false;
  let g21VentanaTotalMs = 3000;
  let soTurnoTotalSeg   = 30;

  // ── Drum Picker ───────────────────────────────────────────────────────────
  const TIEMPO_OPTS = [
    {label:'1 min',val:60},{label:'2 min',val:120},{label:'3 min',val:180},
    {label:'5 min',val:300},{label:'7 min',val:420},{label:'10 min',val:600},
    {label:'12 min',val:720},{label:'15 min',val:900},{label:'20 min',val:1200},
    {label:'25 min',val:1500},{label:'30 min',val:1800},{label:'40 min',val:2400},
    {label:'45 min',val:2700},{label:'60 min',val:3600}
  ];
  const VENTANA_OPTS = [
    {label:'1 s',val:1000},{label:'2 s',val:2000},{label:'3 s',val:3000},
    {label:'4 s',val:4000},{label:'5 s',val:5000},{label:'7 s',val:7000},
    {label:'10 s',val:10000},{label:'15 s',val:15000}
  ];
  const TURNO_OPTS = [
    {label:'10 s',val:10},{label:'15 s',val:15},{label:'20 s',val:20},
    {label:'30 s',val:30},{label:'45 s',val:45},{label:'1 min',val:60},
    {label:'90 s',val:90},{label:'2 min',val:120},{label:'3 min',val:180},
    {label:'5 min',val:300}
  ];

  const DRUM_H = 120, ITEM_H = 40, CENTER_Y = (DRUM_H - ITEM_H) / 2;
  const drums = {
    time:    {opts:TIEMPO_OPTS,  idx:5},
    ventana: {opts:VENTANA_OPTS, idx:2},
    turno:   {opts:TURNO_OPTS,   idx:3}   // default: 30 s
  };

  function buildDrum(key) {
    const d = drums[key];
    const listEl = document.getElementById('drum-'+key+'-list');
    const drumEl = document.getElementById('drum-'+key);
    if (!listEl || !drumEl) return;
    d.listEl = listEl; d.el = drumEl;
    listEl.innerHTML = d.opts.map((o,i)=>`<div class="drum-item" data-i="${i}">${o.label}</div>`).join('');
    drumRender(key, false);
    let dragging=false, startY=0, startIdx=0;
    const startDrag = y => { dragging=true; startY=y; startIdx=d.idx; listEl.style.transition='none'; };
    const moveDrag  = y => {
      if(!dragging) return;
      const raw = startIdx + (startY-y)/ITEM_H;
      listEl.style.transform=`translateY(${CENTER_Y-Math.max(0,Math.min(d.opts.length-1,raw))*ITEM_H}px)`;
    };
    const endDrag = y => {
      if(!dragging) return; dragging=false;
      d.idx=Math.max(0,Math.min(d.opts.length-1,Math.round(startIdx+(startY-y)/ITEM_H)));
      drumRender(key,true);
    };
    drumEl.addEventListener('mousedown', e=>{startDrag(e.clientY);e.preventDefault();});
    drumEl.addEventListener('mousemove', e=>moveDrag(e.clientY));
    drumEl.addEventListener('mouseup',   e=>endDrag(e.clientY));
    drumEl.addEventListener('mouseleave',e=>{if(dragging)endDrag(e.clientY);});
    drumEl.addEventListener('touchstart',e=>startDrag(e.touches[0].clientY),{passive:true});
    drumEl.addEventListener('touchmove', e=>moveDrag(e.touches[0].clientY),{passive:true});
    drumEl.addEventListener('touchend',  e=>endDrag(e.changedTouches[0].clientY));
  }
  function drumRender(key, animate) {
    const d=drums[key]; if(!d.listEl) return;
    const y=CENTER_Y-d.idx*ITEM_H;
    d.listEl.style.transition=animate?'transform 0.2s cubic-bezier(.25,.8,.25,1)':'none';
    d.listEl.style.transform=`translateY(${y}px)`;
    d.listEl.querySelectorAll('.drum-item').forEach((el,i)=>el.classList.toggle('active',i===d.idx));
  }
  function drumStep(key,dir){
    const d=drums[key];
    d.idx=Math.max(0,Math.min(d.opts.length-1,d.idx+dir));
    drumRender(key,true);
  }

  // ── Selección de modo ─────────────────────────────────────────────────────
  function selectMode(mode, el) {
    selectedMode = mode;
    document.querySelectorAll('.mode-btn').forEach(b=>b.classList.remove('selected'));
    el.classList.add('selected');

    const is1vs1     = mode==='1vs1';
    const is21       = mode==='21';
    const isShootout = mode==='shootout';
    const isLibre    = mode==='libre';

    // Selector de jugadores: 21 y shootout
    document.getElementById('players-selector').style.display = (is21||isShootout) ? 'block':'none';
    // Drum duración partido: solo 1vs1
    document.getElementById('drum-col-time').style.display    = is1vs1 ? 'flex':'none';
    // Drum tiempo/turno: solo shootout
    document.getElementById('drum-col-turno').style.display   = isShootout ? 'flex':'none';
    // Drum ventana espera: 1vs1 y 21
    document.getElementById('drum-col-ventana').style.display = (is1vs1||is21) ? 'flex':'none';

    // Reset selección de jugadores
    if (is21||isShootout) {
      document.querySelectorAll('#player-toggle .player-btn').forEach(b=>b.classList.remove('selected'));
      selectedPlayers = 2;
    }
    checkReady();
  }

  function selectPlayers(n, el) {
    selectedPlayers=n;
    document.querySelectorAll('#player-toggle .player-btn').forEach(b=>b.classList.remove('selected'));
    el.classList.add('selected');
    checkReady();
  }

  function checkReady() {
    const needPlayers = selectedMode==='21' || selectedMode==='shootout';
    const ok = !needPlayers || selectedPlayers>=1;
    document.getElementById('btn-start').disabled=!ok;
  }

  window.addEventListener('DOMContentLoaded', ()=>{
    buildDrum('time');
    buildDrum('ventana');
    buildDrum('turno');
    document.getElementById('btn-start').disabled=false;
  });

  // ── Inicio de partida ─────────────────────────────────────────────────────
  async function startGame() {
    setMsg('setup-msg','Conectando...','');
    let body='';
    if (selectedMode==='1vs1') {
      const v=drums.ventana.opts[drums.ventana.idx].val;
      g21VentanaTotalMs=v;
      body=`mode=1vs1&time=${drums.time.opts[drums.time.idx].val}&ventana=${v}`;
    } else if (selectedMode==='21') {
      const v=drums.ventana.opts[drums.ventana.idx].val;
      g21VentanaTotalMs=v;
      body=`mode=21&players=${selectedPlayers}&ventana=${v}`;
    } else if (selectedMode==='shootout') {
      const t=drums.turno.opts[drums.turno.idx].val;
      soTurnoTotalSeg=t;
      body=`mode=shootout&players=${selectedPlayers}&time=${t}`;
    } else if (selectedMode==='libre') {
      body='mode=libre';
    }
    try {
      const r=await fetch('/start',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});
      if(r.ok){setMsg('setup-msg','','');openGamePanel();}
      else setMsg('setup-msg','Error al iniciar','error');
    } catch(e){setMsg('setup-msg','Sin conexión','error');}
  }

  function openGamePanel() {
    document.getElementById('setup-panel').style.display='none';
    document.getElementById('game-panel').style.display='block';
    document.getElementById('sb-1vs1').style.display      = selectedMode==='1vs1'     ? 'block':'none';
    document.getElementById('sb-21').style.display        = selectedMode==='21'       ? 'block':'none';
    document.getElementById('sb-shootout').style.display  = selectedMode==='shootout' ? 'block':'none';
    document.getElementById('sb-libre').style.display     = selectedMode==='libre'    ? 'block':'none';
    paused=false;
    if(selectedMode==='21')       buildMiniScores('all-players-row', selectedPlayers, 'pcard', 'pcard-score');
    if(selectedMode==='shootout') buildMiniScores('so-todos',        selectedPlayers, 'so-pcard', 'so-pscore');
    startPolling();
  }

  // Construye fila de mini-tarjetas para todos los jugadores
  function buildMiniScores(containerId, n, cardPfx, scorePfx) {
    const row=document.getElementById(containerId); if(!row) return;
    row.innerHTML='';
    for(let i=1;i<=n;i++){
      const div=document.createElement('div');
      div.id=cardPfx+'-'+i;
      div.style.cssText='flex:1;padding:8px 4px;border-radius:10px;transition:all 0.25s;';
      div.innerHTML=`
        <div style="font-family:'Barlow Condensed',sans-serif;font-size:0.65rem;
             font-weight:700;letter-spacing:2px;color:var(--grey);text-align:center;">J${i}</div>
        <div id="${scorePfx}-${i}" style="font-family:'Barlow Condensed',sans-serif;
             font-size:2rem;font-weight:900;text-align:center;line-height:1;
             transition:all 0.2s;">0</div>`;
      row.appendChild(div);
    }
  }

  // Resalta el jugador activo en la fila de mini-tarjetas
  function highlightMini(cardPfx, scorePfx, n, turno) {
    for(let i=1;i<=n;i++){
      const card=document.getElementById(cardPfx+'-'+i);
      const sc=document.getElementById(scorePfx+'-'+i);
      if(!card||!sc) continue;
      const active=(i-1===turno);
      card.style.background  = active?'rgba(255,107,0,0.12)':'transparent';
      card.style.border      = active?'1.5px solid rgba(255,107,0,0.5)':'1px solid rgba(255,255,255,0.05)';
      sc.style.color         = active?'var(--orange)':'rgba(255,255,255,0.3)';
      sc.style.fontSize      = active?'2.4rem':'2rem';
      sc.style.textShadow    = active?'0 0 10px rgba(255,107,0,0.4)':'none';
    }
  }

  // ── Polling ───────────────────────────────────────────────────────────────
  function startPolling(){pollingInterval=setInterval(pollStatus,400);}
  function stopPolling() {clearInterval(pollingInterval);}

  async function pollStatus() {
    try {
      const d=await (await fetch('/status')).json();
      if     (selectedMode==='1vs1')     updateUI_1vs1(d);
      else if(selectedMode==='21')       updateUI_21(d);
      else if(selectedMode==='shootout') updateUI_shootout(d);
      else if(selectedMode==='libre')    updateUI_libre(d);
    } catch(e){}
  }

  // ── UI 1vs1 ───────────────────────────────────────────────────────────────
  function updateUI_1vs1(d) {
    updateScore('score1',d.score1); updateScore('score2',d.score2);
    const t=document.getElementById('timer-display');
    t.textContent=fmt(d.seconds);
    t.className='timer-num'+(d.seconds<=30?' warning':'');
    document.getElementById('sb-state').textContent=d.state.toUpperCase();
    setDot('live-dot',d.state);
    if(d.state==='Fin'){stopPolling();showWin_1vs1(d.score1,d.score2);}
  }

  // ── UI juego 21 ───────────────────────────────────────────────────────────
  function updateUI_21(d) {
    if(!d.puntos) return;
    const turno=d.turno;
    document.getElementById('active-name').textContent='JUGADOR '+(turno+1);
    document.getElementById('active-tiro-tag').textContent=d.tiroLibre?'🎯 TIRO LIBRE':'🏀 TIRO NORMAL';
    const as=document.getElementById('active-score');
    const oldV=parseInt(as.textContent)||0;
    as.textContent=d.puntos[turno];
    if(d.puntos[turno]!==oldV){as.classList.add('bump');setTimeout(()=>as.classList.remove('bump'),200);}
    const total=(d.ventanaTotalMs||g21VentanaTotalMs)/1000;
    const pct=Math.max(0,Math.min(100,(d.ventanaRestante/total)*100));
    const bar=document.getElementById('ventana-bar');
    bar.style.width=pct+'%';
    bar.style.background=pct>50?'var(--orange)':(pct>20?'#f4a261':'var(--danger)');
    document.getElementById('ventana-countdown').textContent=d.ventanaRestante>0?d.ventanaRestante+' s':'—';
    for(let i=0;i<d.numJugadores;i++){
      const sc=document.getElementById('pcard-score-'+(i+1));
      if(sc) sc.textContent=d.puntos[i];
    }
    highlightMini('pcard','pcard-score',d.numJugadores,turno);
    setDot('live-dot-21',d.state);
    document.getElementById('sb-state-21').textContent=d.state.toUpperCase();
    if(d.state==='Fin'){stopPolling();showWin_21(d.ganador);}
  }

  // ── UI Shoot Out ──────────────────────────────────────────────────────────
  function updateUI_shootout(d) {
    if(!d.puntos) return;
    const turno = d.turno;
    const n     = d.numJugadores;
    const state = d.state;

    // Ronda y jugador
    const esDesempate = d.ronda > 1;
    document.getElementById('so-ronda-tag').textContent =
      'RONDA ' + d.ronda + (esDesempate ? ' · DESEMPATE' : '');
    document.getElementById('so-nombre').textContent = 'JUGADOR ' + (turno+1);

    // Estado badge
    setDot('live-dot-so', state);
    document.getElementById('sb-state-so').textContent = state.toUpperCase();

    // Overlay cuenta atrás
    const overlay=document.getElementById('so-overlay');
    if(state==='Cuenta') {
      overlay.style.display='flex';
      document.getElementById('so-ov-jugador').textContent='JUGADOR '+(turno+1);
      document.getElementById('so-ov-num').textContent = d.cuentaAtras>0 ? d.cuentaAtras : '¡YA!';
    } else {
      overlay.style.display='none';
    }

    // Timer y barra de progreso
    const timerEl=document.getElementById('so-timer');
    const barEl  =document.getElementById('so-barra');
    if(state==='Jugando') {
      timerEl.textContent = d.segundos+'s';
      timerEl.style.color = d.segundos<=10?'var(--danger)':'var(--white)';
      const total = d.turnoTotalSeg || soTurnoTotalSeg;
      const pct   = Math.max(0,Math.min(100,(d.segundos/total)*100));
      barEl.style.width      = pct+'%';
      barEl.style.background = pct>40?'var(--orange)':(pct>15?'#f4a261':'var(--danger)');
    } else if(state==='Listo') {
      timerEl.textContent='--'; timerEl.style.color='var(--white)';
      barEl.style.width='100%'; barEl.style.background='var(--orange)';
    } else if(state==='Cuenta') {
      timerEl.textContent=''; barEl.style.width='100%';
    } else {
      timerEl.textContent='--'; timerEl.style.color='var(--white)';
    }

    // Score jugador activo
    const sa=document.getElementById('so-score-activo');
    const oldV=parseInt(sa.textContent)||0;
    sa.textContent=d.puntos[turno];
    if(d.puntos[turno]>oldV){sa.classList.add('bump');setTimeout(()=>sa.classList.remove('bump'),200);}

    // Mini scores todos los jugadores
    for(let i=0;i<n;i++){
      const sc=document.getElementById('so-pscore-'+(i+1));
      if(sc) sc.textContent=d.puntos[i];
    }
    highlightMini('so-pcard','so-pscore',n,turno);

    // Botones contextuales
    document.getElementById('so-listo-wrap').style.display   = state==='Listo'   ? 'block':'none';
    document.getElementById('so-canasta-wrap').style.display = state==='Jugando' ? 'block':'none';
    if(state==='Listo') document.getElementById('so-listo-num').textContent=(turno+1);

    if(state==='Fin'){stopPolling();showWin_shootout(d.ganador,d.puntos);}
  }

  // ── UI Libre ──────────────────────────────────────────────────────────────
  function updateUI_libre(d) {
    const el=document.getElementById('libre-score');
    const old=parseInt(el.textContent)||0;
    el.textContent=d.puntos;
    if(d.puntos!==old){el.classList.add('bump');setTimeout(()=>el.classList.remove('bump'),200);}
    setDot('live-dot-libre',d.state);
    document.getElementById('sb-state-libre').textContent=d.state.toUpperCase();
  }

  // ── Helpers UI ────────────────────────────────────────────────────────────
  function setDot(id,state) {
    const dot=document.getElementById(id); if(!dot) return;
    const c=state==='Fin'?'#aaa':(state==='Pausado'?'#f4a261':'#00FF88');
    dot.style.background=c;
    dot.style.boxShadow=state==='Fin'?'none':'0 0 8px '+c;
  }
  function updateScore(id,val){
    const el=document.getElementById(id); if(!el) return;
    const old=parseInt(el.textContent)||0; el.textContent=val;
    if(val>old){el.classList.add('bump');setTimeout(()=>el.classList.remove('bump'),200);}
  }
  function fmt(s){return String(Math.floor(s/60)).padStart(2,'0')+':'+String(s%60).padStart(2,'0');}

  // ── Acciones ──────────────────────────────────────────────────────────────
  async function addPoint(p){await fetch('/point?player='+p,{method:'POST'});}

  async function g21Canasta(){
    const d=await(await fetch('/status')).json();
    if(d.turno!==undefined) await fetch('/point?player='+(d.turno+1),{method:'POST'});
  }
  async function g21Fallo(){await fetch('/fallo',{method:'POST'});}

  async function libreAdd(){await fetch('/point?action=add',{method:'POST'});}
  async function libreSub(){await fetch('/point?action=sub',{method:'POST'});}

  async function soListo()        {await fetch('/listo',{method:'POST'});}
  async function soCanastaManual(){await fetch('/point',{method:'POST'});}

  async function togglePause(){
    paused=!paused;
    await fetch('/pause?state='+(paused?1:0),{method:'POST'});
    const btn=document.getElementById('btn-pause');
    if(paused){btn.innerHTML='<span class="btn-icon">▶</span><span>REANUDAR</span>';btn.classList.add('paused');}
    else      {btn.innerHTML='<span class="btn-icon">⏸</span><span>PAUSAR</span>';btn.classList.remove('paused');}
  }
  async function resetGame(){
    document.getElementById('win-overlay').classList.remove('show');
    await fetch('/reset',{method:'POST'});
    paused=false;
    document.getElementById('btn-pause').innerHTML='<span class="btn-icon">⏸</span><span>PAUSAR</span>';
    document.getElementById('btn-pause').classList.remove('paused');
    startPolling();
  }
  function goBack(){
    stopPolling();
    document.getElementById('game-panel').style.display='none';
    document.getElementById('setup-panel').style.display='block';
  }

  // ── Overlays de victoria ──────────────────────────────────────────────────
  function showWin_1vs1(s1,s2){
    const w=s1>s2?'A':(s2>s1?'B':null);
    document.getElementById('win-title').textContent=w?'¡GANADOR!':'¡EMPATE!';
    document.getElementById('win-sub').textContent=w?`EQUIPO ${w}  •  ${s1}–${s2}`:`${s1} — ${s2}`;
    document.getElementById('win-overlay').classList.add('show');
  }
  function showWin_21(idx){
    document.getElementById('win-title').textContent='¡GANADOR!';
    document.getElementById('win-sub').textContent='JUGADOR '+(idx+1)+' · 21 PUNTOS';
    document.getElementById('win-overlay').classList.add('show');
  }
  function showWin_shootout(idx, puntos){
    document.getElementById('win-title').textContent='¡GANADOR!';
    document.getElementById('win-sub').textContent='JUGADOR '+(idx+1)+' · '+puntos[idx]+' PTS';
    document.getElementById('win-overlay').classList.add('show');
  }

  function setMsg(id,text,cls){
    const el=document.getElementById(id); el.textContent=text;
    el.className='status-msg'+(cls?' '+cls:'');
  }

  // ── Motor canasta ─────────────────────────────────────────────────────────
  let _motorTarget = 0;
  let _motorPolling = null;

  function motorSliderInput(val) {
    // Actualiza display en tiempo real mientras se arrastra
    document.getElementById('motor-cm-val').textContent = val;
    _updateSliderTrack(val);
    _updateQuickBtns(parseInt(val));
  }

  function motorSliderChange(val) {
    // Envía al ESP solo al soltar
    motorSetCm(parseInt(val));
  }

  function motorSetCm(cm) {
    _motorTarget = cm;
    document.getElementById('motor-slider').value = cm;
    document.getElementById('motor-cm-val').textContent = cm;
    _updateSliderTrack(cm);
    _updateQuickBtns(cm);
    fetch('/motor?cm=' + cm, {method:'POST'}).catch(()=>{});
    _startMotorPolling();
  }

  async function motorHoming() {
    document.getElementById('motor-state-txt').textContent = 'CALIBRANDO...';
    document.getElementById('motor-dot').style.background = '#f4a261';
    document.getElementById('motor-dot').style.boxShadow  = '0 0 6px #f4a261';
    await fetch('/homing', {method:'POST'}).catch(()=>{});
    _startMotorPolling();
  }

  function _updateSliderTrack(val) {
    const pct = (val / 20) * 100;
    document.getElementById('motor-slider').style.background =
      `linear-gradient(to right, var(--orange) ${pct}%, rgba(255,255,255,0.15) ${pct}%)`;
  }

  function _updateQuickBtns(val) {
    document.querySelectorAll('.quick-cm-btn').forEach(b => {
      b.classList.toggle('active', parseInt(b.textContent) === val);
    });
  }

  function _startMotorPolling() {
    if (_motorPolling) return;
    _motorPolling = setInterval(_pollMotor, 400);
  }

  async function _pollMotor() {
    try {
      const d = await (await fetch('/motorstatus')).json();
      const dotEl  = document.getElementById('motor-dot');
      const stEl   = document.getElementById('motor-state-txt');
      const posEl  = document.getElementById('motor-pos-txt');

      posEl.textContent = d.motorCm + ' cm';

      if (d.motorState === 'Listo') {
        dotEl.style.background = '#00FF88';
        dotEl.style.boxShadow  = '0 0 6px #00FF88';
        stEl.textContent = 'LISTO';
        clearInterval(_motorPolling); _motorPolling = null;
      } else if (d.motorState === 'Homing') {
        dotEl.style.background = '#f4a261';
        dotEl.style.boxShadow  = '0 0 6px #f4a261';
        stEl.textContent = 'CALIBRANDO...';
      } else if (d.motorState === 'Moviendo') {
        dotEl.style.background = 'var(--orange)';
        dotEl.style.boxShadow  = '0 0 6px var(--orange)';
        stEl.textContent = 'MOVIENDO → ' + d.motorTarget + ' cm';
        // Actualizar slider mientras mueve
        document.getElementById('motor-slider').value = d.motorCm;
        document.getElementById('motor-cm-val').textContent = d.motorCm;
        _updateSliderTrack(d.motorCm);
        _updateQuickBtns(d.motorCm);
      } else if (d.motorState === 'Error') {
        dotEl.style.background = 'var(--danger)';
        dotEl.style.boxShadow  = '0 0 6px var(--danger)';
        stEl.textContent = 'ERROR — pulsa CALIBRAR';
        clearInterval(_motorPolling); _motorPolling = null;
      }
    } catch(e) {}
  }

  // Arrancar polling del motor al cargar la página (para ver el homing inicial)
  window.addEventListener('DOMContentLoaded', () => {
    _updateSliderTrack(0);
    _startMotorPolling();
  });
</script>
</body>
</html>
)rawliteral";
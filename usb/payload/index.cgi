#!/bin/sh
echo "Content-type: text/html; charset=utf-8"
echo "Cache-Control: no-cache, no-store, must-revalidate"
echo ""
cat << 'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Clinton SHADOW DVR Console | CE-R4S</title>
  <script src="/jmuxer.min.js"></script>
  <style>
    /* CSS Variables for Dark (#101010) and Light (#CCCCCC) */
    :root {
      --bg: #101010;
      --card-bg: #1a1a1a;
      --card-sub-bg: #222222;
      --border: #2e2e2e;
      --text: #f3f4f6;
      --text-muted: #9ca3af;
      --accent: #00f0ff;
      --accent-glow: rgba(0, 240, 255, 0.25);
      --success: #10b981;
      --warning: #f59e0b;
      --danger: #ef4444;
      --primary: #3b82f6;
      --term-bg: #0c0c0c;
      --term-border: #242424;
      --term-text: #00ff9d;
      --chip-bg: #262626;
      --chip-hover: #333333;
      --shadow: 0 8px 24px rgba(0, 0, 0, 0.5);
    }

    @media (prefers-color-scheme: light) {
      :root {
        --bg: #CCCCCC;
        --card-bg: #E6E6E6;
        --card-sub-bg: #DCDCDC;
        --border: #B2B2B2;
        --text: #111827;
        --text-muted: #4B5563;
        --accent: #0088cc;
        --accent-glow: rgba(0, 136, 204, 0.2);
        --term-bg: #1c1c1c;
        --term-border: #999999;
        --term-text: #00ff9d;
        --chip-bg: #D4D4D4;
        --chip-hover: #C2C2C2;
        --shadow: 0 6px 16px rgba(0, 0, 0, 0.12);
      }
    }

    /* Manual Theme Overrides */
    [data-theme="dark"] {
      --bg: #101010 !important;
      --card-bg: #1a1a1a !important;
      --card-sub-bg: #222222 !important;
      --border: #2e2e2e !important;
      --text: #f3f4f6 !important;
      --text-muted: #9ca3af !important;
      --accent: #00f0ff !important;
      --accent-glow: rgba(0, 240, 255, 0.25) !important;
      --term-bg: #0c0c0c !important;
      --term-border: #242424 !important;
      --term-text: #00ff9d !important;
      --chip-bg: #262626 !important;
      --chip-hover: #333333 !important;
      --shadow: 0 8px 24px rgba(0, 0, 0, 0.5) !important;
    }

    [data-theme="light"] {
      --bg: #CCCCCC !important;
      --card-bg: #E6E6E6 !important;
      --card-sub-bg: #DCDCDC !important;
      --border: #B2B2B2 !important;
      --text: #111827 !important;
      --text-muted: #4B5563 !important;
      --accent: #0088cc !important;
      --accent-glow: rgba(0, 136, 204, 0.2) !important;
      --term-bg: #1c1c1c !important;
      --term-border: #999999 !important;
      --term-text: #00ff9d !important;
      --chip-bg: #D4D4D4 !important;
      --chip-hover: #C2C2C2 !important;
      --shadow: 0 6px 16px rgba(0, 0, 0, 0.12) !important;
    }

    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      background-color: var(--bg);
      color: var(--text);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
      min-height: 100vh;
      padding: 1.25rem;
      transition: background-color 0.25s ease, color 0.25s ease;
    }
    .container { max-width: 1280px; margin: 0 auto; }
    
    /* Header */
    header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 1rem 1.5rem;
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: 0.85rem;
      box-shadow: var(--shadow);
      margin-bottom: 1.5rem;
      flex-wrap: wrap;
      gap: 1rem;
    }
    .logo-box { display: flex; align-items: center; gap: 0.85rem; }
    .status-dot {
      width: 12px; height: 12px;
      background: var(--success);
      border-radius: 50%;
      box-shadow: 0 0 10px var(--success);
      animation: pulse 2s infinite;
      flex-shrink: 0;
    }
    @keyframes pulse {
      0%, 100% { opacity: 1; transform: scale(1); }
      50% { opacity: 0.6; transform: scale(0.9); }
    }
    h1 {
      font-size: 1.35rem;
      font-weight: 700;
      letter-spacing: -0.02em;
      line-height: 1.2;
      margin: 0;
    }
    .subtitle { font-size: 0.8rem; color: var(--accent); letter-spacing: 0.05em; text-transform: uppercase; font-weight: 600; margin-top: 0.35rem; }
    
    .header-actions { display: flex; align-items: center; gap: 0.75rem; flex-wrap: wrap; }
    
    .uptime-badge {
      background: var(--card-sub-bg);
      border: 1px solid var(--border);
      color: var(--text-muted);
      padding: 0.45rem 0.85rem;
      border-radius: 0.5rem;
      font-size: 0.8rem;
      font-weight: 600;
      line-height: 1.25;
      display: inline-flex;
      align-items: center;
      gap: 0.45rem;
      white-space: nowrap;
      user-select: none;
    }
    .uptime-badge svg {
      color: var(--accent);
      flex-shrink: 0;
    }
    
    /* Theme Selector Pill */
    .theme-selector {
      display: flex;
      background: var(--card-sub-bg);
      border: 1px solid var(--border);
      border-radius: 0.5rem;
      padding: 3px;
      gap: 2px;
    }
    .theme-btn {
      background: transparent;
      border: none;
      color: var(--text-muted);
      padding: 0.35rem 0.65rem;
      border-radius: 0.35rem;
      font-size: 0.75rem;
      font-weight: 600;
      cursor: pointer;
      display: flex;
      align-items: center;
      gap: 0.3rem;
      transition: all 0.2s;
    }
    .theme-btn.active {
      background: var(--card-bg);
      color: var(--text);
      box-shadow: 0 2px 4px rgba(0,0,0,0.15);
    }
    .theme-btn:hover:not(.active) {
      color: var(--text);
    }

    .btn-action {
      background: var(--card-sub-bg);
      border: 1px solid var(--border);
      color: var(--text);
      padding: 0.45rem 0.85rem;
      border-radius: 0.5rem;
      font-size: 0.8rem;
      font-weight: 600;
      line-height: 1.25;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      gap: 0.4rem;
      transition: all 0.2s;
    }
    .btn-action:hover {
      background: var(--chip-hover);
      border-color: var(--accent);
    }
    .btn-danger {
      background: rgba(239, 68, 68, 0.15);
      border-color: rgba(239, 68, 68, 0.4);
      color: var(--danger);
    }
    .btn-danger:hover {
      background: rgba(239, 68, 68, 0.3);
      border-color: var(--danger);
    }

    /* Grid */
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 1.25rem; margin-bottom: 1.5rem; }
    
    /* Cards */
    .card {
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: 0.85rem;
      padding: 1.25rem;
      box-shadow: var(--shadow);
    }
    .card-title {
      font-size: 0.9rem;
      font-weight: 600;
      color: var(--text-muted);
      margin-bottom: 0.85rem;
      display: flex;
      align-items: center;
      justify-content: space-between;
    }
    .card-value {
      font-size: 1.65rem;
      font-weight: 700;
      letter-spacing: -0.02em;
      margin-bottom: 0.35rem;
    }
    .progress-bar-bg {
      background: var(--card-sub-bg);
      border: 1px solid var(--border);
      height: 8px;
      border-radius: 4px;
      overflow: hidden;
      margin-top: 0.5rem;
    }
    .progress-bar {
      height: 100%;
      background: linear-gradient(90deg, var(--accent), var(--primary));
      border-radius: 4px;
      width: 0%;
      transition: width 0.5s ease;
    }

    /* Video Feeds */
    .camera-section { margin-bottom: 1.5rem; }
    .camera-section-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 0.85rem;
      gap: 0.75rem;
      flex-wrap: wrap;
    }
    .camera-section-title {
      font-size: 1rem;
      font-weight: 700;
      color: var(--text);
      display: flex;
      align-items: center;
      gap: 0.45rem;
      flex-wrap: wrap;
    }
    .camera-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(320px, 1fr)); gap: 1.25rem; }
    .camera-card {
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: 0.85rem;
      overflow: hidden;
      box-shadow: var(--shadow);
      display: flex;
      flex-direction: column;
    }
    .camera-header {
      padding: 0.75rem 1rem;
      display: flex;
      justify-content: space-between;
      align-items: center;
      border-bottom: 1px solid var(--border);
      background: var(--card-sub-bg);
      gap: 0.5rem;
    }
    .camera-name { font-weight: 600; font-size: 0.95rem; }
    .camera-viewport {
      position: relative;
      background: #000;
      width: 100%;
      aspect-ratio: 4 / 3;
      display: flex;
      align-items: center;
      justify-content: center;
      overflow: hidden;
    }
    .camera-img {
      width: 100%;
      height: 100%;
      object-fit: fill;
      display: block;
    }
    video.camera-img {
      width: 100%;
      height: 100%;
      object-fit: fill;
      background: #000;
      pointer-events: none;
    }
    .camera-info-bar {
      padding: 0.65rem 1rem;
      background: var(--card-bg);
      display: flex;
      flex-direction: column;
      gap: 0.45rem;
      border-top: 1px solid var(--border);
    }
    .camera-info-row {
      display: flex;
      align-items: center;
      justify-content: space-between;
      font-size: 0.78rem;
      color: var(--text-muted);
    }
    .rtsp-pill {
      font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
      font-size: 0.75rem;
      background: var(--card-sub-bg);
      padding: 0.35rem 0.65rem;
      border-radius: 0.4rem;
      border: 1px solid var(--border);
      color: var(--text);
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 0.4rem;
      transition: all 0.2s;
    }
    .rtsp-pill:hover {
      border-color: var(--accent);
      background: var(--chip-hover);
    }
    .rtsp-pill span:first-child {
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
    }

    /* Web Terminal */
    .terminal-card {
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: 0.85rem;
      padding: 1.25rem;
      box-shadow: var(--shadow);
      margin-bottom: 1.5rem;
    }
    .term-box {
      background: var(--term-bg);
      border: 1px solid var(--term-border);
      border-radius: 0.65rem;
      padding: 1rem;
      font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
      color: var(--term-text);
      font-size: 0.85rem;
      min-height: 220px;
      max-height: 380px;
      overflow-y: auto;
      white-space: pre-wrap;
      word-break: break-all;
      margin-bottom: 0.75rem;
      line-height: 1.5;
    }
    .term-input-box {
      display: flex;
      gap: 0.5rem;
    }
    .term-input {
      flex: 1;
      background: var(--card-sub-bg);
      border: 1px solid var(--border);
      border-radius: 0.5rem;
      color: var(--text);
      padding: 0.6rem 0.85rem;
      font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
      font-size: 0.85rem;
      outline: none;
    }
    .term-input:focus { border-color: var(--accent); }
    .term-btn {
      background: var(--primary);
      color: #fff;
      border: none;
      padding: 0.6rem 1.25rem;
      border-radius: 0.5rem;
      font-weight: 600;
      cursor: pointer;
      font-size: 0.85rem;
    }
    .term-btn:hover { opacity: 0.9; }

    /* Available Commands Section */
    .cmd-table-card {
      margin-top: 1rem;
      background: var(--card-sub-bg);
      border: 1px solid var(--border);
      border-radius: 0.65rem;
      padding: 0.85rem 1rem;
    }
    .cmd-category-title {
      font-size: 0.75rem;
      text-transform: uppercase;
      font-weight: 700;
      color: var(--text-muted);
      margin-bottom: 0.4rem;
      letter-spacing: 0.05em;
    }
    .cmd-chips-group {
      display: flex;
      flex-wrap: wrap;
      gap: 0.45rem;
      margin-bottom: 0.75rem;
    }
    .cmd-chips-group:last-child { margin-bottom: 0; }
    .cmd-chip {
      background: var(--chip-bg);
      border: 1px solid var(--border);
      color: var(--text);
      font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
      font-size: 0.75rem;
      padding: 0.25rem 0.55rem;
      border-radius: 0.35rem;
      cursor: pointer;
      transition: all 0.15s ease;
      display: inline-flex;
      align-items: center;
      gap: 0.3rem;
    }
    .cmd-chip:hover {
      background: var(--chip-hover);
      border-color: var(--accent);
      transform: translateY(-1px);
    }
    .cmd-desc {
      font-size: 0.7rem;
      color: var(--text-muted);
      font-family: -apple-system, BlinkMacSystemFont, sans-serif;
    }

    /* Auth Badges & Modal */
    .auth-pill {
      font-size: 0.72rem;
      padding: 0.2rem 0.55rem;
      border-radius: 9999px;
      background: var(--card-sub-bg);
      border: 1px solid var(--border);
      color: var(--text-muted);
      display: inline-flex;
      align-items: center;
      gap: 0.35rem;
      font-weight: 500;
    }
    .auth-pill.auth-ok {
      background: rgba(16, 185, 129, 0.15);
      border-color: rgba(16, 185, 129, 0.35);
      color: var(--success);
      font-weight: 600;
    }
    .auth-logout-btn {
      background: transparent;
      border: none;
      color: var(--accent);
      font-size: 0.7rem;
      cursor: pointer;
      text-decoration: underline;
      margin-left: 0.35rem;
      padding: 0;
    }
    .auth-logout-btn:hover {
      opacity: 0.8;
    }
    .modal-overlay {
      position: fixed;
      top: 0; left: 0; right: 0; bottom: 0;
      background: rgba(0, 0, 0, 0.75);
      backdrop-filter: blur(5px);
      display: flex;
      align-items: center;
      justify-content: center;
      z-index: 9999;
      padding: 1rem;
    }
    .modal-card {
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: 0.85rem;
      width: 100%;
      max-width: 440px;
      box-shadow: 0 20px 40px rgba(0, 0, 0, 0.6);
      overflow: hidden;
      animation: modalIn 0.2s ease-out;
    }
    @keyframes modalIn {
      from { opacity: 0; transform: scale(0.95); }
      to { opacity: 1; transform: scale(1); }
    }
    .modal-header {
      padding: 0.95rem 1.25rem;
      background: var(--card-sub-bg);
      border-bottom: 1px solid var(--border);
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    .modal-close {
      cursor: pointer;
      font-size: 1.1rem;
      color: var(--text-muted);
      padding: 0.2rem 0.5rem;
      border-radius: 0.3rem;
      line-height: 1;
    }
    .modal-close:hover {
      color: var(--text);
      background: var(--chip-hover);
    }
    .modal-body {
      padding: 1.25rem;
    }
    .modal-input {
      width: 100%;
      background: var(--card-sub-bg);
      border: 1px solid var(--border);
      border-radius: 0.45rem;
      color: var(--text);
      padding: 0.6rem 0.75rem;
      font-size: 0.85rem;
      outline: none;
      box-sizing: border-box;
      font-family: inherit;
    }
    .modal-input:focus {
      border-color: var(--accent);
    }

    /* Responsive Mobile Styles */
    @media (max-width: 640px) {
      body {
        padding: 0.65rem;
      }
      header {
        padding: 0.85rem 1rem;
        gap: 0.75rem;
      }
      .logo-box {
        gap: 0.65rem;
      }
      h1 {
        font-size: 1.15rem;
      }
      .subtitle {
        font-size: 0.72rem;
      }
      .header-actions {
        width: 100%;
        display: flex;
        flex-wrap: wrap;
        gap: 0.45rem;
        justify-content: flex-start;
      }
      .uptime-badge {
        padding: 0.35rem 0.6rem;
        font-size: 0.75rem;
      }
      .theme-btn {
        padding: 0.3rem 0.5rem;
        font-size: 0.72rem;
      }
      .btn-action {
        padding: 0.35rem 0.65rem;
        font-size: 0.75rem;
      }
      .grid {
        grid-template-columns: 1fr;
        gap: 0.75rem;
        margin-bottom: 1rem;
      }
      .card {
        padding: 1rem;
      }
      .card-title {
        font-size: 0.82rem;
      }
      .card-value {
        font-size: 1.45rem;
      }
      .camera-section {
        margin-bottom: 1rem;
      }
      .camera-section-header {
        align-items: center;
        gap: 0.4rem;
      }
      .camera-grid {
        grid-template-columns: 1fr;
        gap: 0.85rem;
      }
      .camera-header {
        padding: 0.65rem 0.85rem;
      }
      .camera-name {
        font-size: 0.88rem;
      }
      .camera-info-bar {
        padding: 0.55rem 0.75rem;
      }
      .camera-info-row {
        font-size: 0.72rem;
      }
      .rtsp-pill {
        font-size: 0.72rem;
        padding: 0.35rem 0.5rem;
      }
      .terminal-card {
        padding: 1rem;
        margin-bottom: 1rem;
      }
      .term-box {
        font-size: 0.75rem;
        min-height: 180px;
        max-height: 280px;
        padding: 0.75rem;
      }
      .term-input-box {
        flex-direction: column;
        gap: 0.5rem;
      }
      .term-btn {
        width: 100%;
        padding: 0.55rem;
      }
      .cmd-table-card {
        padding: 0.65rem 0.75rem;
      }
      .cmd-chips-group {
        gap: 0.35rem;
      }
      .cmd-chip {
        font-size: 0.7rem;
        padding: 0.25rem 0.45rem;
      }
    }

    footer {
      text-align: center;
      font-size: 0.8rem;
      color: var(--text-muted);
      padding: 1.5rem 0;
    }
  </style>
</head>
<body>
  <div class="container">
    <!-- Header -->
    <header>
      <div class="logo-box">
        <div class="status-dot"></div>
        <div>
          <h1>Clinton Electronics SHADOW DVR</h1>
          <div class="subtitle">CE-R4S • ASTEL-LT400 • Linux 2.6.15</div>
        </div>
      </div>
      <div class="header-actions">
        <!-- Uptime Badge -->
        <div class="uptime-badge" id="valUptime">
          <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"></circle><polyline points="12 6 12 12 16 14"></polyline></svg>
          <span id="uptimeText">Uptime: Loading...</span>
        </div>
        <!-- Theme Selector -->
        <div class="theme-selector">
          <button class="theme-btn" id="themeAuto" onclick="setTheme('auto')">🌓 Auto</button>
          <button class="theme-btn" id="themeDark" onclick="setTheme('dark')">🌙 Dark</button>
          <button class="theme-btn" id="themeLight" onclick="setTheme('light')">☀️ Light</button>
        </div>
        <button class="btn-action" onclick="refreshImages(true)">🔄 Refresh Feeds</button>
        <button class="btn-action btn-danger" onclick="triggerReboot()">⚡ Reboot System</button>
      </div>
    </header>

    <!-- Telemetry Cards -->
    <div class="grid">
      <div class="card">
        <div class="card-title">
          <span>System Load (1m / 5m / 15m)</span>
          <span id="txtCpuPct">--%</span>
        </div>
        <div class="card-value" id="valLoad">-- / -- / --</div>
        <div class="progress-bar-bg">
          <div class="progress-bar" id="barCpu"></div>
        </div>
      </div>

      <div class="card">
        <div class="card-title">
          <span>Memory Usage (RAM 104 MB)</span>
          <span id="txtMemPct">--%</span>
        </div>
        <div class="card-value" id="valMem">-- / 104 MB</div>
        <div class="progress-bar-bg">
          <div class="progress-bar" id="barMem"></div>
        </div>
      </div>

      <div class="card">
        <div class="card-title">
          <span>Flash Storage (JFFS2 MTD)</span>
          <span id="txtMtdPct">--%</span>
        </div>
        <div class="card-value" id="valMtd">-- / 5.8 MB</div>
        <div class="progress-bar-bg">
          <div class="progress-bar" id="barMtd"></div>
        </div>
      </div>

      <div class="card">
        <div class="card-title">
          <span>Storage Pool (SATA HDD)</span>
          <span id="txtHddPct">--%</span>
        </div>
        <div class="card-value" id="valHdd">-- / 912 GB</div>
        <div class="progress-bar-bg">
          <div class="progress-bar" id="barHdd"></div>
        </div>
      </div>
    </div>

    <!-- Live Cameras Section -->
    <div class="camera-section">
      <div class="camera-section-header">
        <div class="camera-section-title">
          <span>📹 Live Camera Feeds</span>
        </div>
      </div>
      <div class="camera-grid">
        <!-- Channel 0 -->
        <div class="camera-card">
          <div class="camera-header">
            <span class="camera-name">CH 1: Camera 01 (Front Door)</span>
          </div>
          <div class="camera-viewport">
            <canvas class="camera-img" id="cam0" width="640" height="480"></canvas>
            <video class="camera-img" id="vid0" autoplay muted playsinline style="display:none;"></video>
          </div>
          <div class="camera-info-bar">
            <div class="camera-info-row">
              <span>Resolution: D1 (4:3)</span>
            </div>
            <div class="rtsp-pill" onclick="copyRtsp('rtsp://10.1.1.250:554/live/0', this)" title="Click to copy RTSP direct URL">
              <span>📋 rtsp://10.1.1.250:554/live/0</span>
              <span style="font-size: 0.7rem; color: var(--accent);">Copy RTSP</span>
            </div>
          </div>
        </div>

        <!-- Channel 1 -->
        <div class="camera-card">
          <div class="camera-header">
            <span class="camera-name">CH 2: Camera 02 (Side Yard)</span>
          </div>
          <div class="camera-viewport">
            <canvas class="camera-img" id="cam1" width="640" height="480"></canvas>
            <video class="camera-img" id="vid1" autoplay muted playsinline style="display:none;"></video>
          </div>
          <div class="camera-info-bar">
            <div class="camera-info-row">
              <span>Resolution: D1 (4:3)</span>
            </div>
            <div class="rtsp-pill" onclick="copyRtsp('rtsp://10.1.1.250:554/live/1', this)" title="Click to copy RTSP direct URL">
              <span>📋 rtsp://10.1.1.250:554/live/1</span>
              <span style="font-size: 0.7rem; color: var(--accent);">Copy RTSP</span>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- Web Terminal -->
    <div class="terminal-card">
      <div class="card-title">
        <span style="display:flex; align-items:center; gap:0.5rem; flex-wrap:wrap;">
          <span>💻 Embedded System Console (Web Root Shell)</span>
          <span id="shellAuthStatus" class="auth-pill">🔒 Presets Open / Auth for Custom Shell</span>
        </span>
        <span style="font-size: 0.75rem; color: var(--text-muted);">ASTEL-LT BusyBox v1.18.4 • ARMv4l</span>
      </div>
      <div class="term-box" id="termOutput">~ # Clinton CE-R4S Web Console Ready.
~ # Click any preset command below to run instantly; custom shell commands require administrator authentication...</div>
      <div class="term-input-box">
        <input type="text" class="term-input" id="termInput" placeholder="Enter custom shell command (e.g., ps, df -h, netstat -tlpn, uname -a)..." onkeydown="if(event.key==='Enter') sendTermCmd()">
        <button class="term-btn" onclick="sendTermCmd()">Execute</button>
      </div>

      <!-- Available Commands Table -->
      <div class="cmd-table-card">
        <div class="cmd-category-title">⚙️ System Status & Hardware</div>
        <div class="cmd-chips-group">
          <span class="cmd-chip" data-cmd="uname -a">uname -a <span class="cmd-desc">Kernel & Arch</span></span>
          <span class="cmd-chip" data-cmd="uptime">uptime <span class="cmd-desc">Uptime & Load Average</span></span>
          <span class="cmd-chip" data-cmd="free">free <span class="cmd-desc">Memory Statistics</span></span>
          <span class="cmd-chip" data-cmd="df -h">df -h <span class="cmd-desc">Mount Points & Disk Space</span></span>
          <span class="cmd-chip" data-cmd="cat /proc/cpuinfo">cat /proc/cpuinfo <span class="cmd-desc">FA526 CPU Details</span></span>
        </div>

        <div class="cmd-category-title" style="margin-top: 0.6rem;">🔍 Processes & Services</div>
        <div class="cmd-chips-group">
          <span class="cmd-chip" data-cmd="ps">ps <span class="cmd-desc">Full Process List</span></span>
          <span class="cmd-chip" data-cmd="ps | grep -E 'nova|astel|webfsd'">ps | grep -E "nova|astel|webfsd" <span class="cmd-desc">Key Daemons Status</span></span>
          <span class="cmd-chip" data-cmd="netstat -tlpn">netstat -tlpn <span class="cmd-desc">Listening Ports (23, 80, 554, 7000)</span></span>
          <span class="cmd-chip" data-cmd="top -n 1">top -n 1 <span class="cmd-desc">Real-Time CPU Usage</span></span>
        </div>

        <div class="cmd-category-title" style="margin-top: 0.6rem;">🌐 Network & Routing</div>
        <div class="cmd-chips-group">
          <span class="cmd-chip" data-cmd="ifconfig">ifconfig <span class="cmd-desc">Network Interfaces & I/O Stats</span></span>
          <span class="cmd-chip" data-cmd="route -n">route -n <span class="cmd-desc">Routing Table & Gateway</span></span>
          <span class="cmd-chip" data-cmd="arp -a">arp -a <span class="cmd-desc">LAN ARP Cache</span></span>
          <span class="cmd-chip" data-cmd="ping -c 3 10.1.1.1">ping -c 3 10.1.1.1 <span class="cmd-desc">Gateway Ping Check</span></span>
        </div>

        <div class="cmd-category-title" style="margin-top: 0.6rem;">📁 Storage Directories & Logs</div>
        <div class="cmd-chips-group">
          <span class="cmd-chip" data-cmd="ls -la /mtd">ls -la /mtd <span class="cmd-desc">Flash Firmware Directory</span></span>
          <span class="cmd-chip" data-cmd="ls -lh /mtd/bin">ls -lh /mtd/bin <span class="cmd-desc">Custom Binaries (astel_rtspd)</span></span>
          <span class="cmd-chip" data-cmd="ls -la /ide">ls -la /ide <span class="cmd-desc">Internal SATA Storage</span></span>
          <span class="cmd-chip" data-cmd="dmesg | tail -n 25">dmesg | tail -n 25 <span class="cmd-desc">Recent Kernel Messages</span></span>
        </div>
      </div>
    </div>

    <!-- Footer -->
    <footer>
      Clinton Electronics CE-R4S Modern Web Dashboard • Standalone Native ARMv4 Daemon • Port 80
    </footer>
  </div>

  <!-- Auth Modal -->
  <div id="authModal" class="modal-overlay" style="display:none;">
    <div class="modal-card">
      <div class="modal-header">
        <span style="font-weight:700; font-size:0.95rem; display:flex; align-items:center; gap:0.4rem;">
          🔒 Web Shell Administrator Authentication
        </span>
        <span class="modal-close" onclick="closeAuthModal()">✕</span>
      </div>
      <div class="modal-body">
        <div style="font-size:0.8rem; color:var(--text-muted); margin-bottom:0.9rem; line-height:1.45;">
          Executing custom shell commands requires administrator authentication. Preset diagnostic commands run without verification.
        </div>
        <div style="margin-bottom:0.75rem;">
          <label style="font-size:0.78rem; font-weight:600; display:block; margin-bottom:0.3rem;">Username</label>
          <input type="text" id="authUserInput" class="modal-input" placeholder="Enter admin username" value="">
        </div>
        <div style="margin-bottom:0.85rem;">
          <label style="font-size:0.78rem; font-weight:600; display:block; margin-bottom:0.3rem;">Password</label>
          <input type="password" id="authPassInput" class="modal-input" placeholder="Enter admin password" onkeydown="if(event.key==='Enter') submitAuthModal()">
        </div>
        <div id="authErrorMsg" style="color:var(--danger); font-size:0.78rem; margin-bottom:0.65rem; display:none;"></div>
        <div style="display:flex; justify-content:flex-end; gap:0.5rem;">
          <button class="btn-action" onclick="closeAuthModal()">Cancel</button>
          <button class="term-btn" onclick="submitAuthModal()" id="btnSubmitAuth">Verify & Run</button>
        </div>
      </div>
    </div>
  </div>

  <script>
    // Theme Management
    let currentTheme = localStorage.getItem('dvr_theme') || 'auto';

    function applyTheme(theme) {
      document.getElementById('themeAuto').classList.toggle('active', theme === 'auto');
      document.getElementById('themeDark').classList.toggle('active', theme === 'dark');
      document.getElementById('themeLight').classList.toggle('active', theme === 'light');

      if (theme === 'auto') {
        document.documentElement.removeAttribute('data-theme');
      } else {
        document.documentElement.setAttribute('data-theme', theme);
      }
    }

    function setTheme(theme) {
      currentTheme = theme;
      localStorage.setItem('dvr_theme', theme);
      applyTheme(theme);
    }

    // Listen to system theme changes in auto mode
    window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', () => {
      if (currentTheme === 'auto') {
        applyTheme('auto');
      }
    });

    applyTheme(currentTheme);

    // Auth State & UI
    let pendingCustomCmd = null;

    function updateAuthStatusUI() {
      const u = sessionStorage.getItem('dvr_auth_u');
      const el = document.getElementById('shellAuthStatus');
      if (!el) return;
      if (u) {
        el.className = 'auth-pill auth-ok';
        el.innerHTML = '🔓 Authenticated: ' + u + ' <button onclick="logoutShell(event)" class="auth-logout-btn">Logout</button>';
      } else {
        el.className = 'auth-pill';
        el.innerHTML = '🔒 Presets Open / Auth for Custom Shell';
      }
    }

    function openAuthModal(msg) {
      const modal = document.getElementById('authModal');
      const err = document.getElementById('authErrorMsg');
      if (msg) {
        err.textContent = msg;
        err.style.display = 'block';
      } else {
        err.style.display = 'none';
      }
      modal.style.display = 'flex';
      setTimeout(() => document.getElementById('authPassInput').focus(), 100);
    }

    function closeAuthModal() {
      document.getElementById('authModal').style.display = 'none';
      pendingCustomCmd = null;
    }

    function submitAuthModal() {
      const u = document.getElementById('authUserInput').value.trim();
      const p = document.getElementById('authPassInput').value.trim();
      const err = document.getElementById('authErrorMsg');
      const btn = document.getElementById('btnSubmitAuth');
      if (!u) {
        err.textContent = 'Please enter username';
        err.style.display = 'block';
        return;
      }
      btn.disabled = true;
      btn.textContent = 'Verifying...';
      err.style.display = 'none';

      fetch('/api_exec.cgi?action=check_auth&u=' + encodeURIComponent(u) + '&p=' + encodeURIComponent(p))
        .then(r => r.text())
        .then(res => {
          btn.disabled = false;
          btn.textContent = 'Verify & Run';
          if (res.trim() === 'AUTH_OK') {
            sessionStorage.setItem('dvr_auth_u', u);
            sessionStorage.setItem('dvr_auth_p', p);
            updateAuthStatusUI();
            closeAuthModal();
            if (pendingCustomCmd) {
              const cmd = pendingCustomCmd;
              pendingCustomCmd = null;
              document.getElementById('termInput').value = '';
              execCustomWithAuth(cmd, u, p);
            }
          } else {
            err.textContent = '❌ Invalid username or password';
            err.style.display = 'block';
          }
        })
        .catch(e => {
          btn.disabled = false;
          btn.textContent = 'Verify & Run';
          err.textContent = 'Network verification error: ' + e;
          err.style.display = 'block';
        });
    }

    function logoutShell(e) {
      if (e) e.stopPropagation();
      sessionStorage.removeItem('dvr_auth_u');
      sessionStorage.removeItem('dvr_auth_p');
      updateAuthStatusUI();
      const out = document.getElementById('termOutput');
      out.textContent += '\n[Logged out safely. Switched to preset mode.]\n';
      out.scrollTop = out.scrollHeight;
    }

    // Preset Command Runner (100% Free / No Auth Needed)
    function runPreset(cmd) {
      const out = document.getElementById('termOutput');
      out.textContent += '\n~ # ' + cmd + '\n';
      out.scrollTop = out.scrollHeight;

      const b64 = btoa(unescape(encodeURIComponent(cmd)));
      fetch('/api_exec.cgi?action=preset&run=' + encodeURIComponent(b64))
        .then(r => r.text())
        .then(txt => {
          out.textContent += (txt.trim() ? txt : '[Command executed successfully with no output]') + '\n';
          out.scrollTop = out.scrollHeight;
        })
        .catch(e => {
          out.textContent += 'Execution error: ' + e + '\n';
          out.scrollTop = out.scrollHeight;
        });
    }

    // Attach event listeners to all cmd-chips
    document.querySelectorAll('.cmd-chip').forEach(el => {
      el.addEventListener('click', () => {
        const cmd = el.getAttribute('data-cmd');
        if (cmd) runPreset(cmd);
      });
    });

    // Custom Command Execution with Auth Check
    function sendTermCmd() {
      const inp = document.getElementById('termInput');
      const cmd = inp.value.trim();
      if (!cmd) return;

      // If user typed one of the preset commands, run it freely without auth!
      const isPreset = Array.from(document.querySelectorAll('.cmd-chip')).some(c => c.getAttribute('data-cmd') === cmd);
      if (isPreset) {
        inp.value = '';
        runPreset(cmd);
        return;
      }

      const authUser = sessionStorage.getItem('dvr_auth_u');
      const authPass = sessionStorage.getItem('dvr_auth_p');

      if (!authUser || !authPass) {
        pendingCustomCmd = cmd;
        openAuthModal();
        return;
      }

      inp.value = '';
      execCustomWithAuth(cmd, authUser, authPass);
    }

    function execCustomWithAuth(cmd, u, p) {
      const out = document.getElementById('termOutput');
      out.textContent += '\n~ # ' + cmd + '\n';
      out.scrollTop = out.scrollHeight;

      const b64 = btoa(unescape(encodeURIComponent(cmd)));
      fetch('/api_exec.cgi?action=sh&u=' + encodeURIComponent(u) + '&p=' + encodeURIComponent(p) + '&run=' + encodeURIComponent(b64))
        .then(r => r.text())
        .then(txt => {
          if (txt.includes('Authentication failed')) {
            sessionStorage.removeItem('dvr_auth_u');
            sessionStorage.removeItem('dvr_auth_p');
            updateAuthStatusUI();
            pendingCustomCmd = cmd;
            openAuthModal('Session expired. Please re-authenticate.');
          }
          out.textContent += (txt.trim() ? txt : '[Command executed successfully with no output]') + '\n';
          out.scrollTop = out.scrollHeight;
        })
        .catch(e => {
          out.textContent += 'Execution error: ' + e + '\n';
          out.scrollTop = out.scrollHeight;
        });
    }

    // Reboot System
    function triggerReboot() {
      if (!confirm('Are you sure you want to reboot the Clinton DVR? System will come back online in ~20 seconds.')) return;
      fetch('/api_exec.cgi?action=reboot')
        .then(r => r.text())
        .then(msg => {
          alert(msg || 'DVR is rebooting. Please wait ~20 seconds and refresh.');
        });
    }

    // Copy RTSP URL
    function copyRtsp(url, el) {
      if (navigator.clipboard && navigator.clipboard.writeText) {
        navigator.clipboard.writeText(url).then(() => showCopied(el)).catch(() => fallbackCopy(url, el));
      } else {
        fallbackCopy(url, el);
      }
    }
    function fallbackCopy(url, el) {
      const ta = document.createElement('textarea');
      ta.value = url;
      document.body.appendChild(ta);
      ta.select();
      try { document.execCommand('copy'); showCopied(el); } catch(e){}
      document.body.removeChild(ta);
    }
    function showCopied(el) {
      const oldHtml = el.innerHTML;
      el.innerHTML = '<span style="color:var(--success);">✅ Copied RTSP URL to clipboard</span>';
      setTimeout(() => el.innerHTML = oldHtml, 1500);
    }

    // Universal Client-side Hardware H.264 Player (Zero DVR CPU overhead)
    // Supports WebCodecs (Safari / Secure Context) & MSE via JMuxer (Chrome Mobile / HTTP)
    class DvrStreamPlayer {
      constructor(channel, canvasId, videoId) {
        this.channel = channel;
        this.canvas = document.getElementById(canvasId);
        this.ctx = this.canvas ? this.canvas.getContext('2d') : null;
        this.video = document.getElementById(videoId);

        this.mode = null; // 'webcodecs' | 'mse' | 'unsupported'
        this.decoder = null;
        this.jmuxer = null;
        this.isBusy = false;
        this.hasRendered = false;
        this.loopStarted = false;

        this.init();
      }

      drawPlaceholder(msg, isError = false) {
        if (!this.ctx || !this.canvas) return;
        this.ctx.fillStyle = '#10141d';
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        this.ctx.fillStyle = isError ? '#ef4444' : '#64748b';
        this.ctx.font = '15px -apple-system, BlinkMacSystemFont, sans-serif';
        this.ctx.textAlign = 'center';
        this.ctx.fillText(msg, this.canvas.width / 2, this.canvas.height / 2);
      }

      init() {
        // In Chrome on plain HTTP, window.isSecureContext is false, so VideoDecoder is restricted.
        // Safari on LAN allows VideoDecoder. If secure and available, try WebCodecs.
        if (window.isSecureContext && 'VideoDecoder' in window) {
          try {
            this.mode = 'webcodecs';
            this.initWebCodecs();
            if (this.decoder) {
              if (this.canvas) this.canvas.style.display = 'block';
              if (this.video) this.video.style.display = 'none';
              this.drawPlaceholder('Connecting to hardware H.264 stream...');
              this.startLoop();
              return;
            }
          } catch (e) {
            console.warn('WebCodecs init failed on CH' + this.channel + ', falling back to MSE', e);
          }
        }

        // Fallback to MSE via JMuxer (Chrome Mobile / Android / Insecure HTTP)
        this.fallbackToMSE();
      }

      fallbackToMSE() {
        if (typeof JMuxer !== 'undefined' && 'MediaSource' in window && MediaSource.isTypeSupported('video/mp4; codecs="avc1.42001e"')) {
          this.mode = 'mse';
          if (this.decoder) {
            try { this.decoder.close(); } catch (e) {}
            this.decoder = null;
          }
          if (this.canvas) this.canvas.style.display = 'none';
          if (this.video) {
            this.video.style.display = 'block';
            this.initMSE();
            if (!this.loopStarted) this.startLoop();
          }
        } else {
          this.mode = 'unsupported';
          if (this.canvas) this.canvas.style.display = 'block';
          if (this.video) this.video.style.display = 'none';
          this.drawPlaceholder('Please use a modern browser (Chrome / Safari / Edge)', true);
          if (this.badge) this.badge.innerText = '⚠️ Unsupported Browser';
        }
      }

      initWebCodecs() {
        this.decoder = new VideoDecoder({
          output: (frame) => {
            if (this.ctx && this.canvas) {
              this.ctx.drawImage(frame, 0, 0, this.canvas.width, this.canvas.height);
              if (!this.hasRendered) {
                this.hasRendered = true;
                if (this.badge) this.badge.innerText = '⚡ Hardware Stream';
              }
            }
            frame.close();
          },
          error: (err) => {
            console.warn('VideoDecoder error on CH' + this.channel + ', falling back to MSE', err);
            this.fallbackToMSE();
          }
        });

        this.decoder.configure({
          codec: 'avc1.42001e',
          avc: { format: 'annexb' },
          hardwareAcceleration: 'prefer-hardware',
          optimizeForLatency: true
        });
      }

      initMSE() {
        try {
          this.jmuxer = new JMuxer({
            node: this.video,
            mode: 'video',
            flushingTime: 0,
            clearBuffer: true,
            fps: 1,
            debug: false,
            onError: (err) => {
              console.warn('JMuxer error CH' + this.channel, err);
            }
          });

          if (this.video) {
            this.video.addEventListener('loadeddata', () => {
              if (!this.hasRendered) {
                this.hasRendered = true;
                if (this.badge) this.badge.innerText = '⚡ Hardware Stream';
              }
            });
            this.video.addEventListener('error', (e) => {
              console.warn('Video element error CH' + this.channel, e);
            });
          }
        } catch (e) {
          console.error('Failed to init JMuxer:', e);
        }
      }

      async fetchFrame() {
        if (document.hidden) return;
        if (this.isBusy) return;
        this.isBusy = true;
        try {
          const resp = await fetch('/live_' + this.channel + '.h264?t=' + Date.now(), { cache: 'no-store' });
          if (resp.ok) {
            const buf = await resp.arrayBuffer();
            if (buf.byteLength > 100) {
              if (this.mode === 'webcodecs' && this.decoder && this.decoder.state === 'configured') {
                try {
                  const chunk = new EncodedVideoChunk({
                    type: 'key',
                    timestamp: performance.now() * 1000,
                    data: buf
                  });
                  this.decoder.decode(chunk);
                } catch (e) {
                  console.warn('Decode chunk failed on CH' + this.channel + ', falling back to MSE', e);
                  this.fallbackToMSE();
                }
              } else if (this.mode === 'mse' && this.jmuxer) {
                this.jmuxer.feed({
                  video: new Uint8Array(buf),
                  duration: 2000
                });
                if (this.video && this.video.paused) {
                  this.video.play().catch(() => {});
                }
              }
            }
          }
        } catch (err) {
          // Transient network glitch
        } finally {
          this.isBusy = false;
        }
      }

      startLoop() {
        if (this.loopStarted) return;
        this.loopStarted = true;
        this.fetchFrame();
        setInterval(() => this.fetchFrame(), 1000);
      }
    }

    let player0 = null, player1 = null;
    window.addEventListener('DOMContentLoaded', () => {
      player0 = new DvrStreamPlayer(0, 'cam0', 'vid0');
      player1 = new DvrStreamPlayer(1, 'cam1', 'vid1');
    });

    function refreshImages(manual = false) {
      if (player0) player0.fetchFrame();
      if (player1) player1.fetchFrame();
      if (manual) {
        const btn = document.querySelector('button[onclick="refreshImages(true)"]');
        if (btn) {
          const old = btn.innerHTML;
          btn.innerHTML = '✅ Refreshed';
          setTimeout(() => btn.innerHTML = old, 1000);
        }
      }
    }

    // Parse clean uptime text
    function parseUptime(str) {
      if (!str) return 'Loading...';
      const m = str.match(/up\s+([^,]+(?:,\s*\d+:\d+)?),\s*load/i) || str.match(/up\s+([^,]+)/i);
      if (m) {
        let t = m[1].trim();
        const dayMatch = t.match(/(\d+)\s*days?,\s*(\d+):(\d+)/i);
        if (dayMatch) {
          return dayMatch[1] + 'd ' + parseInt(dayMatch[2], 10) + 'h ' + parseInt(dayMatch[3], 10) + 'm';
        }
        const hmMatch = t.match(/^(\d+):(\d+)$/);
        if (hmMatch) {
          const h = parseInt(hmMatch[1], 10), mn = parseInt(hmMatch[2], 10);
          return mn > 0 ? (h + 'h ' + mn + 'm') : (h + 'h');
        }
        const minMatch = t.match(/(\d+)\s*mins?/i);
        if (minMatch) {
          return minMatch[1] + 'm';
        }
        return t;
      }
      return str.split(/load average/i)[0].replace(/^.*up\s+/i, '').replace(/,\s*$/, '').trim() || str;
    }

    // Telemetry Polling
    function updateTelemetry() {
      if (document.hidden) return;
      fetch('/api_status.cgi')
        .then(r => r.json())
        .then(d => {
          // 1. CPU Load
          let l1 = '--', l5 = '--', l15 = '--';
          if (d.load_1m !== undefined && d.load_5m !== undefined) {
            l1 = d.load_1m; l5 = d.load_5m; l15 = d.load_15m;
          } else if (typeof d.load === 'string') {
            const parts = d.load.trim().split(/\s+/);
            if (parts.length >= 3) { l1 = parts[0]; l5 = parts[1]; l15 = parts[2]; }
          }
          document.getElementById('valLoad').textContent = l1 + ' / ' + l5 + ' / ' + l15;
          const load1Val = parseFloat(l1) || 0;
          const cpuPct = Math.min(100, Math.round(load1Val * 100));
          document.getElementById('txtCpuPct').textContent = cpuPct + '%';
          document.getElementById('barCpu').style.width = cpuPct + '%';

          // 2. Uptime in header
          if (d.uptime) {
            const el = document.getElementById('uptimeText');
            if (el) el.textContent = 'Uptime: ' + parseUptime(d.uptime);
          }

          // 3. Memory
          const totalKb = d.mem_total_kb || d.mem_total || 106592;
          const freeKb = d.mem_free_kb || d.mem_free || 0;
          const bufKb = d.mem_buffers_kb || d.mem_buffers || 0;
          const cacheKb = d.mem_cached_kb || d.mem_cached || 0;
          const actualUsedKb = Math.max(0, totalKb - freeKb - bufKb - cacheKb);
          const memPct = Math.min(100, Math.round((actualUsedKb / totalKb) * 100));
          document.getElementById('valMem').textContent = Math.round(actualUsedKb / 1024) + ' / ' + Math.round(totalKb / 1024) + ' MB';
          document.getElementById('txtMemPct').textContent = memPct + '%';
          document.getElementById('barMem').style.width = memPct + '%';

          // 4. MTD Flash
          const mtdUsedMb = ((d.mtd_used_kb || 4972) / 1024).toFixed(1);
          const mtdTotalMb = ((d.mtd_total_kb || 5888) / 1024).toFixed(1);
          const mtdPct = d.mtd_pct || Math.round(((d.mtd_used_kb || 4972) / (d.mtd_total_kb || 5888)) * 100) + '%';
          document.getElementById('valMtd').textContent = mtdUsedMb + ' / ' + mtdTotalMb + ' MB';
          document.getElementById('txtMtdPct').textContent = mtdPct;
          document.getElementById('barMtd').style.width = mtdPct;

          // 5. HDD (WD Blue 1TB, 912GB video pool)
          const hddUsedGb = d.hdd_used_gb !== undefined ? d.hdd_used_gb : 0;
          const hddTotalGb = d.hdd_total_gb !== undefined ? d.hdd_total_gb : 912;
          const hddPct = d.hdd_pct || '1%';
          document.getElementById('valHdd').textContent = hddUsedGb + ' / ' + hddTotalGb + ' GB';
          document.getElementById('txtHddPct').textContent = (d.hdd_health === 'PASSED' ? 'Healthy ' : '') + hddPct;
          document.getElementById('barHdd').style.width = hddPct;
        })
        .catch(err => {
          console.error('Telemetry fetch error:', err);
        });
    }

    // Init
    updateTelemetry();
    updateAuthStatusUI();
    setInterval(updateTelemetry, 10000);
    document.addEventListener('visibilitychange', () => {
      if (!document.hidden) {
        updateTelemetry();
        if (player0) player0.fetchFrame();
        if (player1) player1.fetchFrame();
      }
    });
  </script>
</body>
</html>
EOF


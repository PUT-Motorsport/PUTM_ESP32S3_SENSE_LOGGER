//WYGLAD UGOTOWALO AI I POMAGALO OVERALL DUZO
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>PUT Motorsport</title>
  <style>
    :root {
      --bg-color: #0d0d0d; --sidebar-bg: #161616; --border-color: #333;
      --accent-color: #00aaff; /* ZMIANA NA NIEBIESKI */
      --text-color: #e0e0e0; --term-bg: #000000;
      --header-height: 42px;
    }
    * { box-sizing: border-box; }
    body { font-family: 'Segoe UI', monospace; background: var(--bg-color); color: var(--text-color); margin: 0; display: flex; height: 100vh; overflow: hidden; }
    
    .container { display: flex; width: 100%; height: 100%; }
    
    /* SIDEBAR */
    .sidebar { width: 280px; min-width: 200px; background: var(--sidebar-bg); border-right: 1px solid var(--border-color); display: flex; flex-direction: column; }
    
    .panel-header {
      height: var(--header-height); background: #202020; border-bottom: 1px solid var(--border-color);
      display: flex; justify-content: space-between; align-items: center; padding: 0 15px;
      font-size: 12px; font-weight: bold; text-transform: uppercase; letter-spacing: 1px; color: #e0e0e0;
    }

    .file-list { flex: 1; overflow-y: auto; padding: 5px; }
    .file-item { display: flex; align-items: center; padding: 8px 10px; border-bottom: 1px solid #252525; cursor: pointer; transition: 0.2s; border-radius: 3px; font-size: 13px; }
    .file-item:hover { background: #2a2a2a; }
    .file-icon { margin-right: 8px; font-size: 16px; min-width: 20px; text-align: center; }
    .file-name { flex: 1; overflow: hidden; white-space: nowrap; text-overflow: ellipsis; }
    .file-size { font-size: 10px; color: #666; margin-right: 8px; }
    
    .action-btn { background: none; border: none; color: #777; cursor: pointer; padding: 4px; font-size: 14px; transition: 0.2s; }
    .action-btn:hover { color: #fff; }
    .btn-view:hover { color: #00aaff; }
    .btn-save { color: #00ff88; font-weight: bold; border: 1px solid #005533; border-radius: 4px; padding: 2px 10px; margin-right: 10px; }
    .btn-save:hover { background: #003322; color: #fff; }
    .btn-download:hover { color: var(--accent-color); }
    .btn-delete:hover { color: #ff4444; }

    /* MAIN LAYOUT */
    .main-content { flex: 1; display: flex; background: var(--term-bg); overflow: hidden; }
    .col-terminal { flex: 1; display: flex; flex-direction: column; border-right: 1px solid var(--border-color); min-width: 300px; }
    .col-right { flex: 1; display: flex; flex-direction: column; background: #111; min-width: 300px; }

    #terminal { flex: 1; padding: 15px; font-family: 'Consolas', monospace; font-size: 13px; color: #cccccc; overflow-y: auto; white-space: pre-wrap; line-height: 1.4; }
    
    .input-bar { background: #1a1a1a; padding: 10px; border-top: 1px solid var(--border-color); display: flex; gap: 10px; }
    input[type="text"] { flex: 1; background: #252525; border: 1px solid #333; color: white; padding: 8px; font-family: monospace; border-radius: 3px; }
    input[type="text"]:focus { outline: 1px solid var(--accent-color); }
    button.send-btn { background: #333; color: white; border: none; padding: 0 15px; cursor: pointer; font-weight: bold; border-radius: 3px; font-size: 12px; }
    button.send-btn:hover { background: #444; }

    /* STATUSY */
    .status-indicator { font-size: 10px; padding: 3px 6px; border-radius: 8px; background: #333; color: #aaa; cursor: pointer; user-select: none; font-weight: bold; letter-spacing: 0.5px; transition: 0.2s; }
    .status-running { background: rgba(0, 170, 255, 0.15); color: var(--accent-color); border: 1px solid var(--accent-color); }
    .status-stopped { background: rgba(255, 68, 68, 0.15); color: #ff4444; border: 1px solid #ff4444; }
    #clock { color: var(--accent-color); font-family: monospace; font-size: 14px; font-weight: bold; letter-spacing: 1px; }
    #storage-status { color: #00aaff; font-family: monospace; font-size: 12px; margin-right: 15px; font-weight: bold; }

    /* TABS */
    .tab-bar { 
      height: var(--header-height); background: #202020; border-bottom: 1px solid var(--border-color);
      display: flex; align-items: flex-end; padding-left: 10px;
    }
    .tab-btn {
      padding: 8px 15px; cursor: pointer; background: #1a1a1a; border: 1px solid transparent; 
      border-bottom: none; color: #888; font-size: 12px; font-weight: bold; margin-right: 2px;
      border-radius: 5px 5px 0 0; position: relative; top: 1px;
    }
    .tab-btn:hover { color: #ccc; background: #252525; }
    .tab-btn.active { 
      background: #111; color: var(--accent-color); border-color: var(--border-color); 
      border-bottom: 1px solid #111; z-index: 10;
    }
    .tab-content { display: none; flex: 1; flex-direction: column; overflow: hidden; }
    .tab-content.active { display: flex; }

    /* DASHBOARD - NOWY STYL PRZYCISKOW */
    .dash-section { background:#161616; padding:20px; border-radius:0px; margin-bottom:0px; border-bottom: 1px solid #252525; }
    .dash-title { font-size:11px; color:#666; margin-bottom: 15px; text-transform: uppercase; letter-spacing: 2px; display:block; font-weight:bold; }
    
    .cfg-grid { display: flex; flex-direction: column; gap: 10px; }
    
    /* CUSTOM CHECKBOX - BUTTON STYLE */
    .cfg-item { 
      position: relative;
      background: #202020; 
      border: 1px solid #333; 
      padding: 12px 15px; 
      cursor: pointer; 
      transition: all 0.2s;
      display: flex; 
      align-items: center;
      justify-content: space-between;
    }
    .cfg-item:hover { border-color: #555; background: #252525; }
    
    /* Ukrywamy domyślny input */
    .cfg-item input { display: none; }
    
    /* Nazwa opcji */
    .cfg-label { font-size: 14px; font-weight: bold; color: #888; letter-spacing: 1px; transition: 0.2s; }
    
    /* Status ON/OFF (kwadracik po prawej) */
    .cfg-status { width: 10px; height: 10px; background: #111; border: 1px solid #444; transition: 0.2s; box-shadow: 0 0 5px rgba(0,0,0,0.5); }

    /* STYL GDY ZAZNACZONE (CHECKED) */
    .cfg-item input:checked ~ .cfg-label { color: var(--accent-color); text-shadow: 0 0 8px rgba(0, 170, 255, 0.4); }
    .cfg-item input:checked ~ .cfg-status { background: var(--accent-color); border-color: var(--accent-color); box-shadow: 0 0 10px var(--accent-color); }
    .cfg-item:has(input:checked) { border-color: var(--accent-color); background: rgba(0, 170, 255, 0.05); }

    .btn-apply {
       width: 100%; padding: 12px; background: transparent; color: var(--accent-color); border: 1px solid var(--accent-color); 
       font-weight: bold; font-size: 13px; cursor: pointer; margin-top: 20px;
       text-transform: uppercase; transition: 0.2s; letter-spacing: 1px;
    }
    .btn-apply:hover { background: var(--accent-color); color: #000; box-shadow: 0 0 15px rgba(0,170,255, 0.4); }

    /* VIEWER */
    .viewer-toolbar { 
       padding: 8px 15px; border-bottom: 1px solid var(--border-color); background: #161616;
       display: flex; justify-content: space-between; align-items: center; font-size: 12px;
    }
    textarea#viewer-content { 
        flex: 1; overflow: auto; padding: 15px; font-family: 'Consolas', monospace; font-size: 13px; color: #fff; background: #111; border: none; outline: none; resize: none; line-height: 1.5; white-space: pre; 
    }

    ::-webkit-scrollbar { width: 6px; height: 6px; }
    ::-webkit-scrollbar-thumb { background: #333; border-radius: 3px; }
    ::-webkit-scrollbar-track { background: #000; }
  </style>
</head>
<body>

<div class="container">
  <div class="sidebar">
    <div class="panel-header">
      <span>Files</span>
      <span id="current-path" style="color:#666; text-transform:none; margin-left:10px; font-family:monospace; flex:1; text-align:right; overflow:hidden; text-overflow:ellipsis;">/</span>
      <button class="action-btn" onclick="loadDir('..')" title="Up" style="margin-left:5px">⬆</button>
    </div>
    <div class="file-list" id="file-list"><div style="padding:20px; text-align:center; color:#555">Loading...</div></div>
  </div>

  <div class="main-content">
    
    <div class="col-terminal">
      <div class="panel-header">
        <span>Terminal</span>
        <div style="display:flex; align-items:center; gap:15px;">
           <div id="storage-status">SD: ...</div>
           <div id="server-status" class="status-indicator status-running" onclick="dsServer()">SRV: ON</div>
        </div>
      </div>
      <div id="terminal"></div>
      <div class="input-bar">
        <input type="text" id="cmd-input" placeholder="Command..." autocomplete="off">
        <button class="send-btn" onclick="sendCommand()">SEND</button>
      </div>
    </div>

    <div class="col-right">
      <div class="tab-bar">
        <div class="tab-btn active" onclick="switchTab('tab-dash')" id="btn-tab-dash">DASHBOARD</div>
        <div class="tab-btn" onclick="switchTab('tab-editor')" id="btn-tab-editor">EDITOR</div>
      </div>

      <div id="tab-dash" class="tab-content active" style="padding:0; overflow-y:auto;">
        
        <div class="dash-section">
           <span class="dash-title">Status</span>
           <div id="clock" style="font-size:36px; text-shadow: 0 0 10px rgba(0,170,255,0.3);">--:--:--</div>
        </div>

        <div class="dash-section">
           <span class="dash-title">Data Logging</span>
           <div class="cfg-grid">
            <label class="cfg-item">
              <input type="checkbox" id="chk_volt" checked> 
              <span class="cfg-label">VOLTAGE</span>
              <span class="cfg-status"></span>
            </label>

            <label class="cfg-item">
              <input type="checkbox" id="chk_kwh" checked>
              <span class="cfg-label">KWH</span>
              <span class="cfg-status"></span>
            </label>

            <label class="cfg-item">
              <input type="checkbox" id="chk_vel" checked>
              <span class="cfg-label">VELOCITY</span>
              <span class="cfg-status"></span>
            </label>
          </div>
           <button class="btn-apply" onclick="applyConfig()">Save & Apply</button>
        </div>

        <div style="padding:20px; color:#444; font-size:12px;">
          PUT Motorsport Logger v1.0
        </div>
      </div>

      <div id="tab-editor" class="tab-content">
        <div class="viewer-toolbar">
          <span id="viewer-filename" style="font-family:monospace; color:#bbb;">No file open</span>
          <div style="display:flex; gap:10px;">
            <button class="action-btn btn-save" onclick="saveFile()" title="Ctrl + S">SAVE</button>
            <button class="action-btn" onclick="closeViewer()" title="Close">✕</button>
          </div>
        </div>
        <textarea id="viewer-content" spellcheck="false" placeholder="// Open a file to start editing..."></textarea>
      </div>

    </div>
  </div>
</div>

<script> 
let currentPath = "/";
let autoScroll = true;
let serverTime = null;
let openFilePath = "";

function switchTab(tabId) {
    document.querySelectorAll('.tab-content').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.tab-btn').forEach(el => el.classList.remove('active'));
    document.getElementById(tabId).classList.add('active');
    document.getElementById('btn-' + tabId).classList.add('active');
}

function applyConfig() {
    let mask = 0;
    // Tylko 3 opcje, konkretnie tak jak chciales
    if(document.getElementById('chk_volt').checked) mask += 1; // Bit 0
    if(document.getElementById('chk_kwh').checked)  mask += 2; // Bit 1
    if(document.getElementById('chk_vel').checked)  mask += 4; // Bit 2

    const btn = document.querySelector('.btn-apply');
    const oldText = btn.innerText;
    btn.innerText = "SAVING...";
    
    fetch('/set_cfg?mask=' + mask)
        .then(r => {
            if(r.ok) { 
                btn.style.background = "#00aaff"; 
                btn.style.color = "#000";
                btn.innerText = "SAVED!";
            } else {
                btn.style.background = "#ff4444"; 
                btn.innerText = "ERROR";
            }
            setTimeout(() => { 
                btn.innerText = oldText; 
                btn.style.background = ""; 
                btn.style.color = "var(--accent-color)";
            }, 1500);
        });
}

function updateClockDisplay() {
    if(serverTime) {
        serverTime.setSeconds(serverTime.getSeconds() + 1);
        const h = String(serverTime.getHours()).padStart(2, '0');
        const m = String(serverTime.getMinutes()).padStart(2, '0');
        const s = String(serverTime.getSeconds()).padStart(2, '0');
        document.getElementById('clock').innerText = `${h}:${m}:${s}`;
    }
}
setInterval(updateClockDisplay, 1000);

function syncClock() {
    fetch('/time').then(r => r.json()).then(data => {
            if(data.time) {
                const parts = data.time.split(':');
                const now = new Date();
                now.setHours(parseInt(parts[0]));
                now.setMinutes(parseInt(parts[1]));
                now.setSeconds(parseInt(parts[2]));
                serverTime = now;
            }
        }).catch(e => {});
}
setInterval(syncClock, 10000);
syncClock();

function viewFile(path, size) { 
  openFilePath = path;
  switchTab('tab-editor');
  document.getElementById('viewer-filename').innerText = path;
  const textarea = document.getElementById('viewer-content');
  textarea.value = "Loading...";
  textarea.disabled = true;

  fetch('/read?path=' + encodeURIComponent(path))
    .then(r => r.text())
    .then(text => {
      textarea.value = text;
      textarea.disabled = false;
    })
    .catch(e => {
      textarea.value = "Error reading file.";
    });
}

function saveFile() {
  if(!openFilePath) return;
  const content = document.getElementById('viewer-content').value;
  const btn = document.querySelector('.btn-save');
  const originalText = btn.innerText;
  btn.innerText = "SAVING...";
  fetch('/save?path=' + encodeURIComponent(openFilePath), {
    method: 'POST', body: content
  }).then(r => {
    if(r.ok) {
        btn.innerText = "SAVED!";
        setTimeout(() => { btn.innerText = originalText; }, 1500);
    } else {
        alert("Error saving.");
        btn.innerText = originalText;
    }
  }).catch(e => { btn.innerText = originalText; });
}

function closeViewer() {
  openFilePath = "";
  document.getElementById('viewer-filename').innerText = "No file open";
  document.getElementById('viewer-content').value = "";
  switchTab('tab-dash');
}

function fetchLogs() { 
  fetch('/logs').then(r => r.text()).then(data => {
      const term = document.getElementById('terminal');
      const isAtBottom = (term.scrollHeight - term.scrollTop - term.clientHeight) < 50;
      term.innerText = data;
      if (isAtBottom || autoScroll) term.scrollTop = term.scrollHeight;
    }).catch(e => {});
}
setInterval(fetchLogs, 1000);

function loadDir(path) { 
  // Logika "W górę" (..)
  if(path === '..') { 
    if(currentPath == '/') return;
    const parts = currentPath.split('/').filter(p => p);
    parts.pop();
    path = '/' + parts.join('/');
    if(path == '') path = '/';
  } else if (!path.startsWith('/')) { 
    path = (currentPath === '/' ? '' : currentPath) + '/' + path;
  }
  localStorage.setItem('savedPath', path); 
  fetch('/list?dir=' + encodeURIComponent(path)) 
    .then(r => r.json()) 
    .then(files => { 
      currentPath = path;
      document.getElementById('current-path').innerText = path;
      const list = document.getElementById('file-list');
      list.innerHTML = '';
      if(!files || files.length === 0) {
        list.innerHTML = '<div style="padding:20px; text-align:center; color:#444">Folder is empty</div>';
        return;
      }
      files.sort((a, b) => (a.type === b.type) ? 0 : (a.type === 'dir' ? -1 : 1));
      files.forEach(f => {
        const item = document.createElement('div');
        item.className = 'file-item';
        const icon = f.type === 'dir' ? '📁' : '📄'; 
        const size = f.type === 'dir' ? '' : formatBytes(f.size);
        const fullPath = (currentPath === '/' ? '' : currentPath) + '/' + f.name;
        let html = `
          <div class="file-icon">${icon}</div>
          <div class="file-name">${f.name}</div>
          <div class="file-size">${size}</div>
          <div style="display:flex; gap:2px">
        `;
        if (f.type === 'file') {
            html += `<button class="action-btn btn-view" title="Edit" onclick="event.stopPropagation(); viewFile('${fullPath}', ${f.size})">∆</button>`;
            html += `<button class="action-btn btn-download" title="Download" onclick="event.stopPropagation(); download('${fullPath}')">⬇</button>`;
        }
        html += `<button class="action-btn btn-delete" title="Delete" onclick="event.stopPropagation(); deleteItem('${fullPath}')">✕</button>`;
        html += `</div>`;
        item.innerHTML = html; 
        if(f.type === 'dir') item.onclick = () => loadDir(f.name); 
        list.appendChild(item); 
      });
    })
    .catch(err => { 
        document.getElementById('file-list').innerHTML = '<div style="color:red; padding:10px">JSON Read Error</div>';
    });
}
function sendCommand() { 
  const input = document.getElementById('cmd-input');
  const val = input.value;
  if(!val) return; 
  const term = document.getElementById('terminal'); 
  term.innerText += `\n> ${val}`;
  term.scrollTop = term.scrollHeight;
  fetch('/cmd?val=' + encodeURIComponent(val)); 
  input.value = ''; 
}
document.getElementById('cmd-input').addEventListener("keypress", function(event) {
  if (event.key === "Enter") sendCommand();
});
function dsServer() {
  if(confirm("Disconnect Server?")) fetch('/cmd?val=AT:DISCONNECT');
}
function download(path) { window.location.href = '/download?path=' + encodeURIComponent(path); }
function deleteItem(path) { 
  if(confirm('Delete: ' + path + '?')) fetch('/delete?path=' + encodeURIComponent(path)).then(() => loadDir(currentPath));
}
function formatBytes(bytes) { 
    if (!+bytes) return '0 B';
    const k = 1024; const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return `${parseFloat((bytes / Math.pow(k, i)).toFixed(2))} ${sizes[i]}`;
}

function loadConfig() {
    fetch('/get_cfg')
        .then(r => r.text())
        .then(val => {
            const mask = parseInt(val);
            if (!isNaN(mask)) {
                // Ustawiamy checkboxy na podstawie bitów
                document.getElementById('chk_volt').checked = (mask & 1) > 0;
                document.getElementById('chk_kwh').checked = (mask & 2) > 0;
                document.getElementById('chk_vel').checked = (mask & 4) > 0;
            }
        })
        .catch(e => console.log('Config load error'));
}
function updateStorageInfo() {
    fetch('/storage').then(r => r.json()).then(data => {
       document.getElementById('storage-status').innerText = `SD: ${formatBytes(data.used)} / ${formatBytes(data.total)}`;
    }).catch(e => {});
}
updateStorageInfo();
setInterval(updateStorageInfo, 15000);
const startPath = localStorage.getItem('savedPath') || '/';
loadDir(startPath);
loadConfig();
</script>
</body>
</html>
)rawliteral";
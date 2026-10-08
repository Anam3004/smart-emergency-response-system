// ════════════════════════════════════════════════════════════
//  app.js — Smart Emergency Response System
// ════════════════════════════════════════════════════════════

const API = 'http://localhost:3000/api';

// ── State ────────────────────────────────
let session = { username: '', role: '', unitName: '' };
let lastDispatch = null;

// ── API helpers ──────────────────────────
async function get(path) {
  const r = await fetch(API + path);
  return r.json();
}
async function post(path, body) {
  const r = await fetch(API + path, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body)
  });
  return r.json();
}

// ── Page / Tab navigation ─────────────────
function showPage(id) {
  document.querySelectorAll('.page').forEach(p => p.classList.remove('active'));
  document.getElementById(id).classList.add('active');
}

function showTab(tabId) {
  document.querySelectorAll('.tab-panel').forEach(p => p.classList.remove('active'));
  document.querySelectorAll('.nav-item').forEach(n => n.classList.remove('active'));

  const panel = document.getElementById('tab-' + tabId);
  if (panel) panel.classList.add('active');

  const nav = document.querySelector(`[data-tab="${tabId}"]`);
  if (nav) {
    nav.classList.add('active');
    document.getElementById('topbar-title').textContent =
      nav.querySelector('.nav-label').textContent;
  }
  loadTab(tabId);
}

function loadTab(tabId) {
  switch (tabId) {
    case 'overview':    renderOverview();    break;
    case 'dispatch':    renderDispatch();    break;
    case 'map':         renderMap();         break;
    case 'units':       renderAllUnits();    break;
    case 'my-units':    renderMyUnits();     break;
    case 'emergencies': renderEmergencies(); break;
  }
}

// ── UI utilities ──────────────────────────
function alert$(id, msg, type = 'info') {
  const el = document.getElementById(id);
  if (!el) return;
  el.innerHTML = `<div class="alert alert-${type}">${msg}</div>`;
  setTimeout(() => { if (el) el.innerHTML = ''; }, 5000);
}
function loading(el) {
  el.innerHTML = `<div class="empty-state"><span class="spinner"></span>Loading...</div>`;
}
function badge(val) {
  return `<span class="badge badge-${val}">${val}</span>`;
}
function sevBar(s) {
  const pct = (s / 10) * 100;
  return `${s}/10 <span class="sev-bar"><span class="sev-bar-fill" style="width:${pct}%"></span></span>`;
}

// ════════════════════════════════════════
//  AUTH
// ════════════════════════════════════════

function showLogin() {
  document.getElementById('form-login').style.display    = '';
  document.getElementById('form-register').style.display = 'none';
}
function showRegister() {
  document.getElementById('form-login').style.display    = 'none';
  document.getElementById('form-register').style.display = '';

  // Reset ALL fields to default every time register is opened
  document.getElementById('reg-user').value  = '';
  document.getElementById('reg-pass').value  = '';
  document.getElementById('reg-role').value  = 'dispatcher';
  document.getElementById('reg-err').textContent = '';
  document.getElementById('reg-err').style.color = 'var(--red)';

  onRoleChange(); // refresh unit dropdown for default role
}

// Unit options per role — must match resources.txt owner values
const UNIT_OPTIONS = {
  hospital: ['HospitalA', 'HospitalB'],
  firedept: ['FireStationA', 'FireStationB'],
  police:   ['PolicePrecinct1', 'PolicePrecinct2'],
  dispatcher: []
};

// When role changes in register form,
// show/hide unit dropdown and populate options
function onRoleChange() {
  const role      = document.getElementById('reg-role').value;
  const unitField = document.getElementById('unit-field');
  const unitSel   = document.getElementById('reg-unit');
  const units     = UNIT_OPTIONS[role] || [];

  if (units.length === 0) {
    unitField.style.display = 'none';
  } else {
    unitField.style.display = '';
    unitSel.innerHTML = units.map(u =>
      `<option value="${u}">${u}</option>`
    ).join('');
  }
}

async function doLogin() {
  const username = document.getElementById('login-user').value.trim();
  const password = document.getElementById('login-pass').value;
  const errEl    = document.getElementById('login-err');

  errEl.style.color = 'var(--red)';
  if (!username || !password) { errEl.textContent = 'All fields required.'; return; }
  errEl.textContent = '';

  try {
    const data = await post('/login', { username, password });
    if (data.success) {
      session = {
        username: data.username,
        role:     data.role,
        unitName: data.unitName   // e.g. "HospitalA"
      };
      bootApp();
    } else {
      errEl.textContent = 'Invalid credentials.';
    }
  } catch (e) {
    errEl.textContent = 'Cannot connect to server. Is it running?';
  }
}

async function doRegister() {
  const username = document.getElementById('reg-user').value.trim();
  const role     = document.getElementById('reg-role').value;
  const password = document.getElementById('reg-pass').value;
  const errEl    = document.getElementById('reg-err');

  // Unit name: "HQ" for dispatcher, selected unit for others
  const units    = UNIT_OPTIONS[role] || [];
  const unitName = units.length > 0
    ? document.getElementById('reg-unit').value
    : 'HQ';

  if (!username || !password) {
    errEl.style.color = 'var(--red)';
    errEl.textContent = 'All fields required.';
    return;
  }

  try {
    const data = await post('/register', { username, role, unitName, password });
    if (data.success) {
      errEl.style.color = 'var(--green)';
      errEl.textContent = `Account created for ${unitName}! You can now log in.`;
      setTimeout(showLogin, 1500);
    } else {
      errEl.style.color = 'var(--red)';
      errEl.textContent = 'Username already taken.';
    }
  } catch (e) {
    errEl.style.color = 'var(--red)';
    errEl.textContent = 'Cannot connect to server.';
  }
}

function doLogout() {
  session      = { username: '', role: '', unitName: '' };
  lastDispatch = null;
  showPage('page-login');
  document.getElementById('login-user').value  = '';
  document.getElementById('login-pass').value  = '';
  document.getElementById('login-err').textContent = '';
  showLogin();
}

// ════════════════════════════════════════
//  APP BOOT
// ════════════════════════════════════════

const ROLE_CONFIG = {
  dispatcher: {
    badgeClass: 'badge-dispatcher',
    label: 'Dispatcher',
    tabs: [
      { id: 'overview',    icon: '◈', label: 'Overview'   },
      { id: 'dispatch',    icon: '⚡', label: 'Dispatch'   },
      { id: 'map',         icon: '◎', label: 'City Map'   },
      { id: 'units',       icon: '▣', label: 'All Units'  },
    ]
  },
  hospital: {
    badgeClass: 'badge-hospital',
    label: 'Hospital',
    tabs: [
      { id: 'my-units',    icon: '◈', label: 'My Units'     },
      { id: 'emergencies', icon: '⚡', label: 'Emergencies'  },
      { id: 'map',         icon: '◎', label: 'City Map'     },
    ]
  },
  firedept: {
    badgeClass: 'badge-firedept',
    label: 'Fire Dept',
    tabs: [
      { id: 'my-units',    icon: '◈', label: 'My Units'     },
      { id: 'emergencies', icon: '⚡', label: 'Emergencies'  },
      { id: 'map',         icon: '◎', label: 'City Map'     },
    ]
  },
  police: {
    badgeClass: 'badge-police',
    label: 'Police',
    tabs: [
      { id: 'my-units',    icon: '◈', label: 'My Units'     },
      { id: 'emergencies', icon: '⚡', label: 'Emergencies'  },
      { id: 'map',         icon: '◎', label: 'City Map'     },
    ]
  }
};

function bootApp() {
  const cfg = ROLE_CONFIG[session.role] || ROLE_CONFIG.dispatcher;

  document.getElementById('sb-username').textContent = session.username;
  document.getElementById('sb-role').textContent     = cfg.label;
  document.getElementById('sb-unit').textContent     = session.unitName;

  const badgeEl = document.getElementById('sb-badge');
  badgeEl.textContent = cfg.label.toUpperCase();
  badgeEl.className   = 'sidebar-role-badge ' + cfg.badgeClass;

  document.getElementById('sidebar-nav').innerHTML = cfg.tabs.map(t => `
    <div class="nav-item" data-tab="${t.id}" onclick="showTab('${t.id}')">
      <span class="nav-icon">${t.icon}</span>
      <span class="nav-label">${t.label}</span>
    </div>
  `).join('');

  document.getElementById('main-content').innerHTML = cfg.tabs.map(t => `
    <div class="tab-panel" id="tab-${t.id}">
      <div id="content-${t.id}"></div>
    </div>
  `).join('');

  showPage('page-app');
  showTab(cfg.tabs[0].id);
}

// ════════════════════════════════════════
//  TAB: OVERVIEW (dispatcher)
// ════════════════════════════════════════
async function renderOverview() {
  const el = document.getElementById('content-overview');
  loading(el);

  const [emgs, ress] = await Promise.all([get('/emergencies'), get('/resources')]);

  const pending   = emgs.filter(e => e.status === 'PENDING').length;
  const assigned  = emgs.filter(e => e.status === 'ASSIGNED').length;
  const completed = emgs.filter(e => e.status === 'COMPLETED').length;
  const enRoute   = ress.filter(r => r.status === 'EN_ROUTE' || r.status === 'REACHED').length;

  el.innerHTML = `
    <div class="section-title">System Overview</div>
    <div class="grid-4" style="margin-bottom:24px">
      <div class="stat-box amber"><div class="stat-label">Pending</div><div class="stat-value">${pending}</div></div>
      <div class="stat-box blue"><div class="stat-label">Assigned</div><div class="stat-value">${assigned}</div></div>
      <div class="stat-box green"><div class="stat-label">Completed</div><div class="stat-value">${completed}</div></div>
      <div class="stat-box red"><div class="stat-label">Units Active</div><div class="stat-value">${enRoute}</div></div>
    </div>
    <div class="section-title">Recent Emergencies</div>
    <div class="card" style="padding:0">${renderEmgTable(emgs.slice(0, 8))}</div>
  `;
}

// ════════════════════════════════════════
//  TAB: DISPATCH (dispatcher)
// ════════════════════════════════════════
async function renderDispatch() {
  const el    = document.getElementById('content-dispatch');
  const nodes = await get('/graph');

  const nodeOpts = nodes.map(n =>
    `<option value="${n.id}">${n.name}</option>`
  ).join('');

  el.innerHTML = `
    <div id="dispatch-alert"></div>
    <div id="dispatch-banner" class="dispatch-banner"></div>

    <div class="grid-2">
      <div class="card">
        <div class="card-title">⚡ Report New Emergency</div>
        <div class="field">
          <label>Emergency Type</label>
          <select id="d-type">
            <option value="FIRE">🔥 FIRE</option>
            <option value="MEDICAL">🏥 MEDICAL</option>
            <option value="CRIME">🚔 CRIME</option>
          </select>
        </div>
        <div class="field">
          <label>Location</label>
          <select id="d-node">${nodeOpts}</select>
        </div>
        <div class="field">
          <label>Severity (1 = minor, 10 = critical)</label>
          <input type="number" id="d-sev" min="1" max="10" value="5"/>
        </div>
        <button class="btn btn-danger" onclick="addEmergency()">
          Add to Priority Queue →
        </button>
      </div>

      <div class="card">
        <div class="card-title">▶ Dispatch Highest Priority</div>
        <button class="btn btn-primary" onclick="processEmergency()">
          Dispatch Next Emergency →
        </button>
      </div>
    </div>

    <div class="section-title">Pending Queue</div>
    <div class="card" id="queue-table" style="padding:0">
      <div class="empty-state"><span class="spinner"></span>Loading...</div>
    </div>
  `;

  loadQueue();
}

async function loadQueue() {
  const el = document.getElementById('queue-table');
  if (!el) return;
  const emgs = await get('/emergencies?status=PENDING');
  el.innerHTML = emgs.length
    ? renderEmgTable(emgs)
    : '<div class="empty-state">No pending emergencies in queue</div>';
}

async function addEmergency() {
  const type = document.getElementById('d-type').value;
  const node = +document.getElementById('d-node').value;
  const sev  = +document.getElementById('d-sev').value;

  if (sev < 1 || sev > 10) {
    alert$('dispatch-alert', 'Severity must be between 1 and 10.', 'error');
    return;
  }

  const r = await post('/emergency/add', { type, node, severity: sev });
  if (r.success) {
    alert$('dispatch-alert', `Queued — ${type} at node ${node} | Severity ${sev}/10`, 'success');
    loadQueue();
  } else {
    alert$('dispatch-alert', 'Failed to add emergency.', 'error');
  }
}

async function processEmergency() {
  const r      = await post('/emergency/process', {});
  const banner = document.getElementById('dispatch-banner');

  if (!r.success) {
    alert$('dispatch-alert', r.message || 'No emergencies or no units available.', 'error');
    if (banner) banner.classList.remove('visible');
    return;
  }

  lastDispatch = r;

  if (banner) {
    banner.classList.add('visible');
    banner.innerHTML = `
      <div class="db-title">✔ DISPATCH SUCCESSFUL</div>
      <div class="db-grid">
        <div class="db-item"><div class="db-key">Emergency</div><div class="db-val">${r.emergencyId}</div></div>
        <div class="db-item"><div class="db-key">Type</div><div class="db-val">${badge(r.emergencyType)}</div></div>
        <div class="db-item"><div class="db-key">Severity</div><div class="db-val">${sevBar(r.severity)}</div></div>
        <div class="db-item"><div class="db-key">Unit</div><div class="db-val">${r.assignedUnitId} (${r.assignedUnitType})</div></div>
        <div class="db-item"><div class="db-key">Dijkstra Route</div><div class="db-val">${(r.pathNames||[]).join(' → ')}</div></div>
        <div class="db-item"><div class="db-key">ETA</div><div class="db-val">${r.totalDistance} min</div></div>
      </div>
    `;
  }

  alert$('dispatch-alert', `Dispatched ${r.assignedUnitId} → ${r.emergencyId}`, 'success');
  loadQueue();
  if (document.getElementById('city-map')) drawMap();
}

// ════════════════════════════════════════
//  TAB: EMERGENCIES (department roles)
//  FIX: filters to show only this unit's assigned emergencies
//  + all PENDING emergencies (awareness)
// ════════════════════════════════════════
async function renderEmergencies() {
  const el = document.getElementById('content-emergencies');

  el.innerHTML = `
    <div style="display:flex;align-items:center;margin-bottom:16px">
      <div class="section-title" style="margin:0;flex:1">
        Emergencies — ${session.unitName}
      </div>
      <button class="refresh-btn" onclick="renderEmergencies()">↻ Refresh</button>
    </div>
    <div class="card" id="emg-table" style="padding:0">
      <div class="empty-state"><span class="spinner"></span>Loading...</div>
    </div>
  `;

  // Dispatcher sees everything — no filter needed
  if (session.role === 'dispatcher') {
    const allEmgs = await get('/emergencies');
    document.getElementById('emg-table').innerHTML = allEmgs.length
      ? renderEmgTable(allEmgs)
      : '<div class="empty-state">No emergencies on record</div>';
    return;
  }

  // Departments see ONLY emergencies assigned to their own units
  const myType = ROLE_TYPE[session.role];
  const [allEmgs, myRess] = await Promise.all([
    get('/emergencies'),
    get(`/resources?type=${myType}&owner=${session.unitName}`)
  ]);

  // Build a set of unit IDs that belong to this department
  const myUnitIds = new Set(myRess.map(r => r.id));

  // Only show emergencies where assignedUnit is one of this dept's units
  // PENDING emergencies are NOT shown — they haven't been assigned to anyone yet
  const filtered = allEmgs.filter(e =>
    e.assignedUnit && myUnitIds.has(e.assignedUnit)
  );

  document.getElementById('emg-table').innerHTML = filtered.length
    ? renderEmgTable(filtered)
    : `<div class="empty-state">
         No emergencies assigned to ${session.unitName} yet
       </div>`;
}

function renderEmgTable(emgs) {
  if (!emgs.length) return '<div class="empty-state">No emergencies</div>';
  return `
    <table class="data-table">
      <thead>
        <tr>
          <th>ID</th><th>Type</th><th>Location</th>
          <th>Severity</th><th>Status</th><th>Assigned To</th>
        </tr>
      </thead>
      <tbody>
        ${emgs.map(e => `
          <tr>
            <td>${e.id}</td>
            <td>${badge(e.type)}</td>
            <td>${e.locationName || 'Node ' + e.locationNode}</td>
            <td>${sevBar(e.severity)}</td>
            <td>${badge(e.status)}</td>
            <td style="font-family:var(--mono);font-size:11px;color:var(--muted)">
              ${e.assignedUnit || '—'}
            </td>
          </tr>
        `).join('')}
      </tbody>
    </table>
  `;
}

// ════════════════════════════════════════
//  TAB: ALL UNITS (dispatcher only)
//  FIX: shows location name instead of node number
// ════════════════════════════════════════
async function renderAllUnits() {
  const el = document.getElementById('content-units');

  el.innerHTML = `
    <div style="display:flex;align-items:center;margin-bottom:16px">
      <div class="section-title" style="margin:0;flex:1">All Units</div>
      <button class="refresh-btn" onclick="renderAllUnits()">↻ Refresh</button>
    </div>
    <div class="card" id="units-table" style="padding:0">
      <div class="empty-state"><span class="spinner"></span>Loading...</div>
    </div>
  `;

  // Load resources (now includes currentLocation from C++)
  const ress = await get('/resources');

  document.getElementById('units-table').innerHTML = ress.length
    ? renderUnitTable(ress, false)
    : '<div class="empty-state">No units found</div>';
}

// ════════════════════════════════════════
//  TAB: MY UNITS (department roles)
//  Shows only this unit's resources + action buttons
//  NEW: includes Add Resource section at bottom
// ════════════════════════════════════════

const ROLE_TYPE = {
  hospital: 'AMBULANCE',
  firedept: 'FIRETRUCK',
  police:   'POLICE'
};

async function renderMyUnits() {
  const el     = document.getElementById('content-my-units');
  const myType = ROLE_TYPE[session.role];   // locked — hospital=AMBULANCE etc.

  // Fetch city nodes for the add-resource location dropdown
  const nodes    = await get('/graph');
  const nodeOpts = nodes.map(n =>
    `<option value="${n.id}">${n.name}</option>`
  ).join('');

  el.innerHTML = `
    <div id="units-alert"></div>

    <!-- ── Active units table ── -->
    <div style="display:flex;align-items:center;margin-bottom:16px">
      <div class="section-title" style="margin:0;flex:1">
        My Units — ${badge(myType)}
        <span style="color:var(--muted);font-size:10px;margin-left:8px">
          ${session.unitName}
        </span>
      </div>
      <button class="refresh-btn" onclick="renderMyUnits()">↻ Refresh</button>
    </div>

    <div class="card" id="my-units-table" style="padding:0;margin-bottom:24px">
      <div class="empty-state">
        <span class="spinner"></span>Loading...
      </div>
    </div>

    <!-- ── Add new unit (type is locked to this department) ── -->
    <div class="section-title">Add New Unit — ${session.unitName}</div>
    <div class="card">
      <div class="card-title">+ Register New ${myType}</div>
      <div id="add-unit-alert"></div>

      <div class="form-row">
        <div class="field">
          <label>Unit ID</label>
          <input type="text" id="new-unit-id"
            placeholder="${
              myType === 'AMBULANCE'  ? 'AMB_03'   :
              myType === 'FIRETRUCK'  ? 'TRUCK_03'  : 'COP_03'
            }"/>
        </div>

        <div class="field">
          <label>Base Location</label>
          <select id="new-unit-node">${nodeOpts}</select>
        </div>

        <!-- Type is NOT shown — it is hardcoded to myType on submit -->
        <button class="btn btn-primary" style="margin-top:auto"
          onclick="addNewUnit('${myType}')">
          Add ${myType} →
        </button>
      </div>
    </div>
  `;

  await loadMyUnits(myType);
}

async function loadMyUnits(myType) {
  const el = document.getElementById('my-units-table');
  if (!el) return;

  // Get resources filtered by type + owner
  // Get ALL emergencies (not just assigned) so we can match assignedUnit
  const [ress, allEmgs] = await Promise.all([
    get(`/resources?type=${myType}&owner=${session.unitName}`),
    get('/emergencies')
  ]);

  if (!ress.length) {
    el.innerHTML = `
      <div class="empty-state">
        No units registered under ${session.unitName}
      </div>`;
    return;
  }

  el.innerHTML = renderUnitTable(ress, true, allEmgs);
}

// renderUnitTable
// FIX: uses r.currentLocation (name) instead of Node ${r.currentNode}
function renderUnitTable(ress, showActions, emgs = []) {
  // Build lookup: unitId → full emergency object
  // (not just id — so we can show emergency id AND guard N/A)
  const unitToEmg = {};
  emgs.forEach(e => {
    if (e.assignedUnit && e.status !== 'COMPLETED') {
      unitToEmg[e.assignedUnit] = e;   // store full object
    }
  });

  return `
    <table class="data-table">
      <thead>
        <tr>
          <th>Unit ID</th>
          <th>Type</th>
          <th>Owner</th>
          <th>Location</th>
          <th>Status</th>
          ${showActions ? '<th>Actions</th>' : ''}
        </tr>
      </thead>
      <tbody>
        ${ress.map(r => {
          const linkedEmg  = unitToEmg[r.id];        // full emergency object or undefined
          const emgId      = linkedEmg ? linkedEmg.id : null;
          const locDisplay = r.currentLocation || ('Node ' + r.currentNode);

          // Only show action buttons when we have a valid emergency id
          // This prevents the 'N/A' bug that caused emergency status to not update
          let actionCell = '';
          if (showActions) {
            if (r.status === 'EN_ROUTE' && emgId) {
              actionCell = `
                <button class="action-btn success"
                  onclick="updateStatus('${r.id}','REACHED','${emgId}')">
                  Mark Reached
                </button>`;
            } else if (r.status === 'REACHED' && emgId) {
              actionCell = `
                <button class="action-btn success"
                  onclick="updateStatus('${r.id}','COMPLETED','${emgId}')">
                  Mark Completed
                </button>`;
            } else if (r.status === 'AVAILABLE') {
              actionCell = `
                <span style="color:var(--muted);font-size:11px;
                  font-family:var(--mono)">Standby</span>`;
            } else if (r.status === 'EN_ROUTE' && !emgId) {
              // EN_ROUTE but emergency not found — show warning
              actionCell = `
                <span style="color:var(--amber);font-size:11px;
                  font-family:var(--mono)">Syncing...</span>`;
            }
          }

          return `
            <tr>
              <td>${r.id}</td>
              <td>${badge(r.type)}</td>
              <td style="font-family:var(--mono);font-size:11px;
                color:var(--amber)">${r.owner || '—'}</td>
              <td style="font-family:var(--mono);font-size:11px">
                ${locDisplay}
              </td>
              <td>${badge(r.status)}</td>
              ${showActions ? `<td>${actionCell}</td>` : ''}
            </tr>
          `;
        }).join('')}
      </tbody>
    </table>
  `;
}

async function updateStatus(unitId, newStatus, emergencyId) {
  // Guard — should never happen now but safety net
  if (!emergencyId || emergencyId === 'N/A') {
    alert$('units-alert',
      'Cannot update — emergency ID not found. Try refreshing.',
      'error'
    );
    return;
  }

  const r = await post('/resource/status', {
    unitId,
    newStatus,
    emergencyId
  });

  if (r.success) {
    alert$(
      'units-alert',
      `${unitId} → ${badge(newStatus)}
       ${newStatus === 'COMPLETED'
         ? `<span style="color:var(--muted);font-size:10px">
              — ${emergencyId} closed, unit now AVAILABLE
            </span>`
         : ''}`,
      'success'
    );

    // Reload units table
    await loadMyUnits(ROLE_TYPE[session.role]);

    // If the emergencies tab is currently visible, refresh it too
    // so the status change is immediately reflected there
    const emgPanel = document.getElementById('tab-emergencies');
    if (emgPanel && emgPanel.classList.contains('active')) {
      await renderEmergencies();
    }

  } else {
    alert$('units-alert', `Failed to update ${unitId}.`, 'error');
  }
}

async function addNewUnit(type) {
  const id    = document.getElementById('new-unit-id').value.trim().toUpperCase();
  const node  = +document.getElementById('new-unit-node').value;
  const owner = session.unitName;

  if (!id) {
    alert$('add-unit-alert', 'Unit ID is required.', 'error');
    return;
  }

  const r = await post('/resource/add', { id, type, node, owner });

  if (r.success) {
    alert$('add-unit-alert', `${id} added to ${owner} at ${document.getElementById('new-unit-node').selectedOptions[0].text}`, 'success');
    document.getElementById('new-unit-id').value = '';
    await loadMyUnits(type);
  } else {
    alert$('add-unit-alert', `Failed — unit ID may already exist.`, 'error');
  }
}

async function relocateUnit() {
  const id   = document.getElementById('reloc-id').value.trim().toUpperCase();
  const node = +document.getElementById('reloc-node').value;

  if (!id) {
    alert$('add-unit-alert', 'Unit ID is required.', 'error');
    return;
  }

  const r = await post('/resource/updatenode', { id, node });

  if (r.success) {
    alert$('add-unit-alert', `${id} relocated to ${document.getElementById('reloc-node').selectedOptions[0].text}`, 'success');
    document.getElementById('reloc-id').value = '';
    await loadMyUnits(ROLE_TYPE[session.role]);
  } else {
    alert$('add-unit-alert', `Failed to relocate ${id}.`, 'error');
  }
}

// ════════════════════════════════════════
//  TAB: CITY MAP
//  FIX: department roles now see their active route highlighted
//  They check if any of their units are assigned and fetch that route
// ════════════════════════════════════════
async function renderMap() {
  const el = document.getElementById('content-map');

  el.innerHTML = `
    <div class="section-title">City Graph — Dijkstra Route Visualizer</div>
    <div class="map-container">
      <svg id="city-map" viewBox="0 0 820 320" xmlns="http://www.w3.org/2000/svg"></svg>
    </div>
    <div class="card" id="map-legend"
      style="padding:12px 16px;font-family:var(--mono);font-size:11px;color:var(--muted)">
      Loading...
    </div>
  `;

  // For non-dispatcher: check if any of their units are on an active assignment
  // and load that route into lastDispatch before drawing
  if (session.role !== 'dispatcher') {
    await loadUnitRoute();
  }

  await drawMap();
}

// Loads the active route for the current unit's assigned emergency
// Sets lastDispatch so drawMap() highlights it
async function loadUnitRoute() {
  const myType = ROLE_TYPE[session.role];
  if (!myType) return;

  const [myRess, allEmgs] = await Promise.all([
    get(`/resources?type=${myType}&owner=${session.unitName}`),
    get('/emergencies')
  ]);

  const myUnitIds = new Set(myRess.map(r => r.id));

  // Find an active emergency assigned to one of this unit's resources
  const activeEmg = allEmgs.find(e =>
    e.status === 'ASSIGNED' && myUnitIds.has(e.assignedUnit)
  );

  if (!activeEmg) {
    // No active assignment — clear lastDispatch so no stale route shows
    lastDispatch = null;
    return;
  }

  // Find which resource is assigned
  const assignedRes = myRess.find(r => r.id === activeEmg.assignedUnit);
  if (!assignedRes) return;

  // Fetch the Dijkstra path from unit's location to emergency location
  const pathData = await get(
    `/path?from=${assignedRes.currentNode}&to=${activeEmg.locationNode}`
  );

  // Set lastDispatch — drawMap() uses this to highlight the route
  lastDispatch = {
    pathNodes:     pathData.pathNodes   || [],
    pathNames:     pathData.pathNames   || [],
    totalDistance: pathData.distance    || 0,
    emergencyNode: activeEmg.locationNode,
    unitStartNode: assignedRes.currentNode,
    emergencyId:   activeEmg.id,
    unitId:        assignedRes.id
  };
}

async function drawMap() {
  const svg = document.getElementById('city-map');
  if (!svg) return;

  const [nodes, edges] = await Promise.all([get('/graph'), get('/edges')]);

  if (!nodes.length) {
    svg.innerHTML = `<text x="50%" y="50%" text-anchor="middle"
      fill="#4a5568" font-family="monospace" font-size="13">
      No graph data — check data/graph.txt</text>`;
    return;
  }

  const PAD = 60, W = 820, H = 320;
  const xs = nodes.map(n => n.x), ys = nodes.map(n => n.y);
  const minX = Math.min(...xs), maxX = Math.max(...xs);
  const minY = Math.min(...ys), maxY = Math.max(...ys);
  const scaleX = maxX === minX ? 1 : (W - PAD * 2) / (maxX - minX);
  const scaleY = maxY === minY ? 1 : (H - PAD * 2) / (maxY - minY);

  const cx = n => PAD + (n.x - minX) * scaleX;
  const cy = n => PAD + (n.y - minY) * scaleY;

  const nodeMap = {};
  nodes.forEach(n => (nodeMap[n.id] = n));

  const pathNodeSet = new Set(lastDispatch ? lastDispatch.pathNodes : []);
  const pathEdgeSet = new Set();

  if (lastDispatch?.pathNodes) {
    const p = lastDispatch.pathNodes;
    for (let i = 0; i < p.length - 1; i++) {
      pathEdgeSet.add(`${Math.min(p[i], p[i+1])}-${Math.max(p[i], p[i+1])}`);
    }
  }

  let svgHtml = '';

  // Draw edges
  edges.forEach(e => {
    const a = nodeMap[e.from], b = nodeMap[e.to];
    if (!a || !b) return;
    const key    = `${Math.min(e.from, e.to)}-${Math.max(e.from, e.to)}`;
    const isPath = pathEdgeSet.has(key);
    const midX   = (cx(a) + cx(b)) / 2;
    const midY   = (cy(a) + cy(b)) / 2;

    svgHtml += `
      <line x1="${cx(a)}" y1="${cy(a)}" x2="${cx(b)}" y2="${cy(b)}"
        stroke="${isPath ? '#22c55e' : '#1e2330'}"
        stroke-width="${isPath ? 3 : 1.5}"
        ${isPath ? 'stroke-dasharray="6 3"' : ''}/>
      <text x="${midX}" y="${midY - 5}" fill="#4a5568" font-size="10"
        font-family="Share Tech Mono,monospace" text-anchor="middle">${e.weight}</text>
    `;
  });

  // Draw nodes
  nodes.forEach(n => {
    const x = cx(n), y = cy(n);
    const isEmg   = lastDispatch && n.id === lastDispatch.emergencyNode;
    const isStart = lastDispatch && n.id === lastDispatch.unitStartNode;
    const isPath  = pathNodeSet.has(n.id);

    let fill = '#111318', stroke = '#2a3040', textColor = '#4a5568';
    if      (isEmg)   { fill = '#7f1d1d'; stroke = '#ef4444'; textColor = '#ef4444'; }
    else if (isStart) { fill = '#14532d'; stroke = '#22c55e'; textColor = '#22c55e'; }
    else if (isPath)  { fill = '#1e3a5f'; stroke = '#3b82f6'; textColor = '#60a5fa'; }

    svgHtml += `
      <circle cx="${x}" cy="${y}" r="18"
        fill="${fill}" stroke="${stroke}" stroke-width="1.5"/>
      <text x="${x}" y="${y + 4}" fill="${textColor}" font-size="11"
        font-family="Share Tech Mono,monospace"
        text-anchor="middle" font-weight="bold">${n.id}</text>
      <text x="${x}" y="${y + 30}" fill="#c8d0e0" font-size="10"
        font-family="Barlow,sans-serif"
        text-anchor="middle" font-weight="600">${n.name}</text>
    `;
  });

  svg.innerHTML = svgHtml;

  // Legend
  const legend = document.getElementById('map-legend');
  if (legend) {
    let routeInfo;
    if (lastDispatch) {
      routeInfo = session.role === 'dispatcher'
        ? `Last dispatch: ${lastDispatch.pathNames.join(' → ')} (${lastDispatch.totalDistance} min)`
        : `Your active route: ${lastDispatch.pathNames.join(' → ')} (${lastDispatch.totalDistance} min) — ${lastDispatch.unitId} → ${lastDispatch.emergencyId}`;
    } else {
      routeInfo = session.role === 'dispatcher'
        ? 'No dispatch yet — dispatch an emergency to see the route'
        : `No active emergency assigned to ${session.unitName}`;
    }

    legend.innerHTML = `
      <span style="color:var(--green)">■</span> Active route &nbsp;&nbsp;
      <span style="color:var(--red)">●</span> Emergency &nbsp;&nbsp;
      <span style="color:var(--green)">●</span> Unit start &nbsp;&nbsp;
      <span style="color:var(--blue)">●</span> Path node &nbsp;&nbsp;&nbsp;&nbsp;
      <span style="color:var(--amber)">${routeInfo}</span>
    `;
  }
}

// ── Enter key on login/register ──────────
document.addEventListener('keydown', e => {
  if (e.key !== 'Enter') return;
  if (!document.getElementById('page-login').classList.contains('active')) return;
  const loginVisible = document.getElementById('form-login').style.display !== 'none';
  loginVisible ? doLogin() : doRegister();
});
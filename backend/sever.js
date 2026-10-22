const express = require('express');
const { spawn } = require('child_process');
const path      = require('path');
const cors      = require('cors');

const app  = express();
const PORT = 3000;

app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, '..', 'frontend')));

const EXE = path.join(
  __dirname,
  'cpp',
  process.platform === 'win32' ? 'emergency.exe' : 'emergency'
);

// ─────────────────────────────────────────
// runCpp(args)
// BUG FIX: cwd was pointing to cpp/ but data/ is in backend/
// Corrected to __dirname (backend/) so data/graph.txt resolves correctly
// ─────────────────────────────────────────
function runCpp(args) {
  return new Promise((resolve, reject) => {
    const proc = spawn(EXE, args, { cwd: __dirname });  // ← FIXED (was cpp/)

    let stdout = '';
    let stderr = '';

    proc.stdout.on('data', chunk => { stdout += chunk.toString(); });
    proc.stderr.on('data', chunk => { stderr += chunk.toString(); });

    proc.on('close', code => {
      const raw = stdout.trim();
      if (!raw) return reject(new Error(`Exe exited ${code}. stderr: ${stderr}`));
      try {
        resolve(JSON.parse(raw));
      } catch (e) {
        reject(new Error(`Invalid JSON from C++: "${raw}"`));
      }
    });

    proc.on('error', err => reject(new Error(`Failed to start exe: ${err.message}`)));
  });
}

// ── Health check ─────────────────────────
app.get('/api/ping', async (req, res) => {
  try   { res.json(await runCpp(['ping'])); }
  catch (err) { res.status(500).json({ error: err.message }); }
});

// ── Login ─────────────────────────────────
// Returns: { success, role, unitName, username }
app.post('/api/login', async (req, res) => {
  const { username, password } = req.body;
  if (!username || !password)
    return res.status(400).json({ error: 'username and password required' });
  try   { res.json(await runCpp(['login', username, password])); }
  catch (err) { res.status(500).json({ error: err.message }); }
});

// ── Register ──────────────────────────────
// Body: { username, role, unitName, password }
app.post('/api/register', async (req, res) => {
  const { username, role, unitName, password } = req.body;
  if (!username || !role || !unitName || !password)
    return res.status(400).json({ error: 'All fields required' });
  try   { res.json(await runCpp(['register', username, role, unitName, password])); }
  catch (err) { res.status(500).json({ error: err.message }); }
});

// ── Add Emergency ─────────────────────────
app.post('/api/emergency/add', async (req, res) => {
  const { type, node, severity } = req.body;
  if (!type || node === undefined || !severity)
    return res.status(400).json({ error: 'type, node, severity required' });
  try   { res.json(await runCpp(['add', type, String(node), String(severity)])); }
  catch (err) { res.status(500).json({ error: err.message }); }
});

// ── Process Next Emergency ─────────────────
app.post('/api/emergency/process', async (req, res) => {
  try   { res.json(await runCpp(['process'])); }
  catch (err) { res.status(500).json({ error: err.message }); }
});

// ── Get Emergencies ───────────────────────
// GET /api/emergencies
// GET /api/emergencies?status=PENDING
app.get('/api/emergencies', async (req, res) => {
  const { status } = req.query;
  const args = status ? ['emergencies', status] : ['emergencies'];
  try   { res.json(await runCpp(args)); }
  catch (err) { res.status(500).json({ error: err.message }); }
});

// ── Get Resources ─────────────────────────
// GET /api/resources
// GET /api/resources?type=AMBULANCE
// GET /api/resources?type=AMBULANCE&owner=HospitalA  ← NEW: unit filter
app.get('/api/resources', async (req, res) => {
  const { type, owner } = req.query;
  const args = ['resources'];
  if (type)  args.push(type);
  if (owner) args.push(owner);
  try   { res.json(await runCpp(args)); }
  catch (err) { res.status(500).json({ error: err.message }); }
});

// ── Update Unit Status ────────────────────
app.post('/api/resource/status', async (req, res) => {
  const { unitId, newStatus, emergencyId } = req.body;
  if (!unitId || !newStatus || !emergencyId)
    return res.status(400).json({ error: 'unitId, newStatus, emergencyId required' });
  try   { res.json(await runCpp(['status', unitId, newStatus, emergencyId])); }
  catch (err) { res.status(500).json({ error: err.message }); }
});

// ── Add New Resource Unit ──────────────────
// Body: { id, type, node, owner }
app.post('/api/resource/add', async (req, res) => {
  const { id, type, node, owner } = req.body;
  if (!id || !type || node === undefined || !owner)
    return res.status(400).json({ error: 'id, type, node, owner required' });
  try   { res.json(await runCpp(['addunit', id, type, String(node), owner])); }
  catch (err) { res.status(500).json({ error: err.message }); }
});

// ── Relocate Unit ─────────────────────────
app.post('/api/resource/updatenode', async (req, res) => {
  const { id, node } = req.body;
  if (!id || node === undefined)
    return res.status(400).json({ error: 'id and node required' });
  try   { res.json(await runCpp(['updatenode', id, String(node)])); }
  catch (err) { res.status(500).json({ error: err.message }); }
});

// ── City Graph Nodes ──────────────────────
app.get('/api/graph', async (req, res) => {
  try   { res.json(await runCpp(['graph'])); }
  catch (err) { res.status(500).json({ error: err.message }); }
});

// ── City Graph Edges ──────────────────────
app.get('/api/edges', async (req, res) => {
  try   { res.json(await runCpp(['edges'])); }
  catch (err) { res.status(500).json({ error: err.message }); }
});

// ── Path Between Nodes ────────────────────
// Returns pathNames, pathNodes (int[]), distance
app.get('/api/path', async (req, res) => {
  const { from, to } = req.query;
  if (from === undefined || to === undefined)
    return res.status(400).json({ error: 'from and to required' });
  try   { res.json(await runCpp(['path', String(from), String(to)])); }
  catch (err) { res.status(500).json({ error: err.message }); }
});

// ── Start ─────────────────────────────────
app.listen(PORT, () => {
  console.log('');
  console.log('╔══════════════════════════════════════╗');
  console.log('║  Smart Emergency Response System     ║');
  console.log('║  http://localhost:' + PORT + '/              ║');
  console.log('╚══════════════════════════════════════╝');
  console.log('');
});
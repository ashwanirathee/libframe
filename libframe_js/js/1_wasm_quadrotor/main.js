import * as THREE from "three";
import { OrbitControls } from "three/addons/controls/OrbitControls.js";

const statusEl = document.getElementById("status");
const stateEl = document.getElementById("state");

const throttleEl = document.getElementById("throttle");
const rollEl = document.getElementById("roll");
const pitchEl = document.getElementById("pitch");
const yawrateEl = document.getElementById("yawrate");
const resetBtn = document.getElementById("reset");

// --- Three.js scene ---
const scene = new THREE.Scene();
scene.add(new THREE.AxesHelper(1.0));

const grid = new THREE.GridHelper(20, 20);
scene.add(grid);

const camera = new THREE.PerspectiveCamera(60, window.innerWidth / window.innerHeight, 0.01, 200);
camera.position.set(3, 2, 3);

const renderer = new THREE.WebGLRenderer({ antialias: true });
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

const controls = new OrbitControls(camera, renderer.domElement);
controls.target.set(0, 0.5, 0);
controls.update();

// simple quad "body"
const quad = new THREE.Group();
scene.add(quad);

const body = new THREE.Mesh(new THREE.BoxGeometry(0.3, 0.06, 0.15), new THREE.MeshNormalMaterial());
quad.add(body);

const armGeom = new THREE.BoxGeometry(0.6, 0.03, 0.03);
const arm = new THREE.Mesh(armGeom, new THREE.MeshNormalMaterial());
quad.add(arm);

const arm2 = arm.clone();
arm2.rotation.y = Math.PI / 2;
quad.add(arm2);

// --- Trail (path) ---
const MAX_POINTS = 6000;
const trailPositions = new Float32Array(MAX_POINTS * 3);
const trailGeom = new THREE.BufferGeometry();
trailGeom.setAttribute("position", new THREE.BufferAttribute(trailPositions, 3));
trailGeom.setDrawRange(0, 0);

const trailLine = new THREE.Line(trailGeom, new THREE.LineBasicMaterial());
scene.add(trailLine);

let trailCount = 0;
const minDist = 0.01;
const lastTrail = new THREE.Vector3();
let hasLastTrail = false;

function clearTrail() {
  trailCount = 0;
  hasLastTrail = false;
  trailGeom.setDrawRange(0, 0);
  trailGeom.attributes.position.needsUpdate = true;
}

function addTrailPoint(x, y, z) {
  const p = new THREE.Vector3(x, y, z);
  if (hasLastTrail && p.distanceTo(lastTrail) < minDist) return;

  lastTrail.copy(p);
  hasLastTrail = true;

  // if full, shift left (simple). If you want faster, we can switch to a ring buffer.
  if (trailCount >= MAX_POINTS) {
    trailPositions.copyWithin(0, 3, MAX_POINTS * 3);
    trailCount = MAX_POINTS - 1;
  }

  const i = trailCount * 3;
  trailPositions[i + 0] = x;
  trailPositions[i + 1] = y;
  trailPositions[i + 2] = z;

  trailCount++;
  trailGeom.setDrawRange(0, trailCount);
  trailGeom.attributes.position.needsUpdate = true;
  trailGeom.computeBoundingSphere();
}

// --- Follow camera ---
let followEnabled = true; // set false if you want to orbit freely

// camera offset in world coordinates (behind + above)
const followOffset = new THREE.Vector3(0, 2.0, 6.0);
const desiredCamPos = new THREE.Vector3();

function updateCameraFollow() {
  if (!followEnabled || !latest) return;

  // keep OrbitControls target on the quad for nice orbiting
  controls.target.lerp(quad.position, 0.2);

  // optionally also move camera toward an offset position
  desiredCamPos.copy(quad.position).add(followOffset);
  camera.position.lerp(desiredCamPos, 0.08);

  controls.update();
}

// toggle follow with "F"
window.addEventListener("keydown", (e) => {
  if (e.key.toLowerCase() === "f") {
    followEnabled = !followEnabled;
  }
});

// --- Worker (Pyodide sim) ---
const worker = new Worker("./../js/1_wasm_quadrotor/sim_worker.js", { type: "module" });

let latest = null; // {t, p:[x,y,z], q:[w,x,y,z], v:[...], w:[...]}
let workerReady = false;

worker.onmessage = (e) => {
  const msg = e.data;
  if (msg.type === "status") {
    statusEl.textContent = msg.text;
  } else if (msg.type === "ready") {
    workerReady = true;
    statusEl.textContent = "Ready.";
  } else if (msg.type === "state") {
    latest = msg.state;
    stateEl.textContent =
      `t = ${latest.t.toFixed(3)}\n` +
      `p = [${latest.p.map((x) => x.toFixed(3)).join(", ")}]\n` +
      `v = [${latest.v.map((x) => x.toFixed(3)).join(", ")}]\n` +
      `q = [${latest.q.map((x) => x.toFixed(3)).join(", ")}]\n` +
      `w = [${latest.w.map((x) => x.toFixed(3)).join(", ")}]\n`;
  } else if (msg.type === "error") {
    statusEl.textContent = `Error: ${msg.error}`;
    console.error(msg.error);
  }
};

// send controls at ~30Hz (not every frame)
function sendControls() {
  if (!workerReady || paused) return;

  worker.postMessage({
    type: "controls",
    controls: {
      throttle: parseFloat(throttleEl.value),
      roll: parseFloat(rollEl.value),
      pitch: parseFloat(pitchEl.value),
      yaw_rate: parseFloat(yawrateEl.value),
    },
  });
}


setInterval(sendControls, 33);

resetBtn.onclick = () => {
  worker.postMessage({ type: "reset" });
  clearTrail();
  latest = null;
};

let paused = false;
const pauseBtn = document.getElementById("pause");

function setPaused(p) {
  paused = p;
  pauseBtn.textContent = paused ? "Resume" : "Pause";
  worker.postMessage({ type: paused ? "pause" : "resume" });
}

pauseBtn.onclick = () => setPaused(!paused);

// optional: spacebar toggles pause
window.addEventListener("keydown", (e) => {
  if (e.code === "Space") {
    e.preventDefault();
    setPaused(!paused);
  }
});
 
function animate() {
  requestAnimationFrame(animate);

  if (latest && !paused) {
    quad.position.set(latest.p[0], latest.p[1], latest.p[2]);
    const [qw, qx, qy, qz] = latest.q;
    quad.quaternion.set(qx, qy, qz, qw).normalize();

    addTrailPoint(latest.p[0], latest.p[1], latest.p[2]);
    updateCameraFollow();
  } else if (latest && paused) {
    // still allow orbiting while paused
    controls.update();
  }

  renderer.render(scene, camera);
}
animate();

window.addEventListener("resize", () => {
  camera.aspect = window.innerWidth / window.innerHeight;
  camera.updateProjectionMatrix();
  renderer.setSize(window.innerWidth, window.innerHeight);
});
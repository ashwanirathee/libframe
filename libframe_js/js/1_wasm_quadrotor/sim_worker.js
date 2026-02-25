// sim_worker.js (ES module worker)
import "https://cdn.jsdelivr.net/pyodide/v0.25.1/full/pyodide.js";

let pyodide = null;
let stepFn = null, getStateFn = null, setControlsFn = null, resetFn = null;

let paused = false;           // <-- NEW
let loopRunning = false;      // <-- NEW (optional safety)

function postStatus(text) {
  postMessage({ type: "status", text });
}

async function init() {
  postStatus("Loading Pyodide…");
  pyodide = await loadPyodide();
  await pyodide.loadPackage("numpy");

  postStatus("Fetching quad.py…");
  const resp = await fetch("./../../../libframe_py/src/7_wasm_example.py");
  if (!resp.ok) throw new Error(`fetch quad.py failed: ${resp.status}`);
  const code = await resp.text();

  postStatus("Initializing Python sim…");
  await pyodide.runPythonAsync(code);

  stepFn = pyodide.globals.get("step_sim");
  getStateFn = pyodide.globals.get("get_state");
  setControlsFn = pyodide.globals.get("set_controls");
  resetFn = pyodide.globals.get("reset_sim");

  postMessage({ type: "ready" });

  const dt = 1 / 200;
  const sendEvery = 1 / 60;
  let acc = 0;
  let last = performance.now() / 1000;
  let lastSend = last;

  function loop() {
    // schedule next tick first (keeps the loop alive even if paused)
    setTimeout(loop, 0);

    const now = performance.now() / 1000;
    let frame = now - last;
    last = now;
    if (frame > 0.1) frame = 0.1;

    // If paused: don’t advance sim; also don’t let acc explode.
    if (paused) {
      acc = 0;
      // still allow state streaming (optional). If you want NO updates while paused, comment this block.
      if (now - lastSend >= sendEvery) {
        const stateProxy = getStateFn();
        const state = stateProxy.toJs({ dict_converter: Object.fromEntries });
        stateProxy.destroy?.();
        postMessage({ type: "state", state });
        lastSend = now;
      }
      return;
    }

    acc += frame;
    while (acc >= dt) {
      stepFn(dt);
      acc -= dt;
    }

    if (now - lastSend >= sendEvery) {
      const stateProxy = getStateFn();
      const state = stateProxy.toJs({ dict_converter: Object.fromEntries });
      stateProxy.destroy?.();
      postMessage({ type: "state", state });
      lastSend = now;
    }
  }

  if (!loopRunning) {
    loopRunning = true;
    loop();
  }
}

self.onmessage = (e) => {
  const msg = e.data;
  try {
    if (msg.type === "controls") setControlsFn(msg.controls);
    if (msg.type === "reset") {
      resetFn();
      // keep paused state as-is; if you prefer reset to also pause/unpause, change here
    }
    if (msg.type === "pause") paused = true;     // <-- NEW
    if (msg.type === "resume") paused = false;   // <-- NEW
    if (msg.type === "set_paused") paused = !!msg.paused; // <-- optional convenience
  } catch (err) {
    postMessage({ type: "error", error: String(err) });
  }
};

init().catch((err) => postMessage({ type: "error", error: String(err) }));
// Smart Wardrobe frontend. All recommendation logic lives in the C++ backend.
// Communicates with the native C++ HTTP server (or relative origin if hosted on the same host/port).
const API_BASE = "http://localhost:18080";

// Controlled values (must match the backend's accepted values)
const SUBTYPES = {
  Top: ["T-Shirt", "Shirt", "Hoodie", "Sweater"],
  Bottom: ["Jeans", "Cargo", "Trousers", "Sweatpants"],
  Shoes: ["Sneakers", "Boots", "Formal Shoes"],
};
const COLORS = {
  Black: "#1c1f24", White: "#f7f7f4", Grey: "#9a9fa8", Beige: "#d9c9a8", Brown: "#7a5232",
  "Light Blue": "#9ec5ea", Blue: "#2f6fd0", Navy: "#1d2d5c", Red: "#c8372d", Green: "#3d8a52",
};
const OCCASIONS = ["Casual", "Smart Casual", "Formal", "Active", "Party"];
const SEASONS = ["All Season", "Summer", "Winter", "Monsoon"];
const ALL_SUBTYPES = Object.values(SUBTYPES).flat();

const $ = (s) => document.querySelector(s);
const h = (tag, props = {}, ...kids) => {
  const el = document.createElement(tag);
  for (const [k, v] of Object.entries(props)) {
    if (k === "class") el.className = v;
    else if (k.startsWith("on")) el.addEventListener(k.slice(2), v);
    else if (v !== false && v != null) el.setAttribute(k, v);
  }
  el.append(...kids.flat().filter((k) => k != null && k !== false));
  return el;
};
const fill = (sel, opts, first) => {
  sel.replaceChildren(...(first ? [h("option", { value: "" }, first)] : []), ...opts.map((o) => h("option", { value: o }, o)));
};

let items = [];
let editingId = null;

// ---------- API ----------
async function api(path, opts = {}) {
  let res;
  try {
    res = await fetch(API_BASE + path, { headers: { "Content-Type": "application/json" }, ...opts });
  } catch {
    throw new Error("Cannot reach the backend. Check that the C++ server is running.");
  }
  let data = null;
  try { data = await res.json(); } catch { /* empty body */ }
  if (!res.ok) throw new Error((data && data.error) || `Request failed (${res.status}).`);
  return data;
}
const toList = (v) => (Array.isArray(v) ? v : String(v || "").split("|").filter(Boolean));
const normalize = (c) => ({
  id: c.id, type: c.type, subtype: c.subtype, color: c.color,
  occasions: toList(c.occasions ?? c.occasion), seasons: toList(c.seasons ?? c.season),
});
async function loadItems() {
  const d = await api("/api/clothes");
  items = (Array.isArray(d) ? d : d.clothes || d.items || []).map(normalize);
}

// ---------- helpers ----------
let toastTimer;
function toast(msg) {
  const t = $("#toast");
  t.textContent = msg; t.hidden = false;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => (t.hidden = true), 2800);
}
function showError(id, msg) { const e = $(id); e.textContent = msg; e.hidden = !msg; }
// Finds the color word inside text such as "White T-Shirt"
function colorOf(text) {
  const names = Object.keys(COLORS).sort((a, b) => b.length - a.length);
  const hit = names.find((n) => String(text).toLowerCase().includes(n.toLowerCase()));
  return COLORS[hit] || "#ccd2da";
}
const strip = (...colors) => h("div", { class: "strip" }, colors.map((c) => h("i", { style: `background:${c}` })));
const itemName = (i) => `${i.color} ${i.subtype}`;

// ---------- navigation ----------
function show(view) {
  document.querySelectorAll("main > section").forEach((s) => (s.hidden = s.id !== "view-" + view));
  document.querySelectorAll("#nav button").forEach((b) => b.classList.toggle("on", b.dataset.view === view));
  if (view !== "add") resetForm();
  if (view === "dashboard") renderDashboard();
  if (view === "wardrobe") renderWardrobe();
}
document.addEventListener("click", (e) => {
  const v = e.target.dataset && (e.target.dataset.view || e.target.dataset.go);
  if (v) show(v);
});

// ---------- dashboard ----------
function renderDashboard() {
  const count = (t) => items.filter((i) => i.type === t).length;
  $("#stats").replaceChildren(
    h("div", { class: "stat big" }, h("b", {}, items.length), "Items in wardrobe"),
    ...["Top", "Bottom", "Shoes"].map((t) => h("div", { class: "stat" }, h("b", {}, count(t)), t === "Top" ? "Tops" : t === "Bottom" ? "Bottoms" : "Shoes"))
  );
  const recent = [...items].sort((a, b) => b.id - a.id).slice(0, 4);
  $("#recent").replaceChildren(...(recent.length ? recent.map(itemCard) : [h("p", { class: "empty" }, "Nothing here yet. Add your first clothing item.")]));
  const favs = getFavs();
  $("#favs").replaceChildren(...(favs.length ? favs.map((o) => outfitCard(o)) : [h("p", { class: "empty" }, "Star an outfit on the Outfits page to keep it here.")]));
}

// ---------- wardrobe ----------
function itemCard(i, withActions = false) {
  return h("article", { class: "card" }, strip(COLORS[i.color] || "#ccd2da"),
    h("div", { class: "body" },
      h("div", { class: "name" }, itemName(i)),
      h("div", {}, `${i.type} \u2022 ${i.subtype}`),
      h("div", { class: "muted" }, `${i.occasions.join(", ")} \u2022 ${i.seasons.join(", ")}`),
      withActions && h("div", { class: "actions" },
        h("button", { onclick: () => startEdit(i) }, "Edit"),
        h("button", { class: "danger", onclick: () => removeItem(i) }, "Delete"))));
}
function renderWardrobe() {
  const q = $("#f-search").value.trim().toLowerCase();
  const f = { type: $("#f-type").value, subtype: $("#f-subtype").value, color: $("#f-color").value };
  const occ = $("#f-occasion").value, sea = $("#f-season").value;
  const list = items.filter((i) =>
    (!f.type || i.type === f.type) && (!f.subtype || i.subtype === f.subtype) && (!f.color || i.color === f.color) &&
    (!occ || i.occasions.includes(occ)) && (!sea || i.seasons.includes(sea)) &&
    (!q || [i.type, i.subtype, i.color, ...i.occasions, ...i.seasons].join(" ").toLowerCase().includes(q)));
  $("#count").textContent = `Showing ${list.length} of ${items.length} items`;
  $("#wardrobe").replaceChildren(...(list.length ? list.map((i) => itemCard(i, true)) : [h("p", { class: "empty" }, items.length ? "No items match these filters." : "Your wardrobe is empty. Add clothing to get started.")]));
}
async function removeItem(i) {
  if (!confirm(`Delete ${itemName(i)}?`)) return;
  try {
    await api(`/api/clothes/${i.id}`, { method: "DELETE" });
    await loadItems(); renderWardrobe(); toast("Item deleted");
  } catch (e) { toast(e.message); }
}

// ---------- add / edit form ----------
function chips(box, name, opts) {
  box.replaceChildren(...opts.map((o) => h("label", {}, h("input", { type: "checkbox", name, value: o }), h("span", {}, o))));
}
const checked = (name) => [...document.querySelectorAll(`input[name=${name}]:checked`)].map((c) => c.value);
const setChecked = (name, vals) => document.querySelectorAll(`input[name=${name}]`).forEach((c) => (c.checked = vals.includes(c.value)));

function resetForm() {
  editingId = null;
  $("#form-title").textContent = "Add clothing";
  $("#save-btn").textContent = "Add to wardrobe";
  $("#cancel-btn").hidden = true;
  showError("#form-error", "");
  $("#i-type").value = "Top"; fill($("#i-subtype"), SUBTYPES.Top);
  $("#i-color").selectedIndex = 0;
  setChecked("occ", []); setChecked("sea", []);
}
function startEdit(i) {
  show("add");
  editingId = i.id;
  $("#form-title").textContent = "Edit clothing";
  $("#save-btn").textContent = "Save changes";
  $("#cancel-btn").hidden = false;
  $("#i-type").value = i.type; fill($("#i-subtype"), SUBTYPES[i.type] || []);
  $("#i-subtype").value = i.subtype; $("#i-color").value = i.color;
  setChecked("occ", i.occasions); setChecked("sea", i.seasons);
}
async function saveItem() {
  const body = {
    type: $("#i-type").value, subtype: $("#i-subtype").value, color: $("#i-color").value,
    occasions: checked("occ"), seasons: checked("sea"),
  };
  if (!body.occasions.length || !body.seasons.length) return showError("#form-error", "Select at least one occasion and one season.");
  try {
    await api(editingId ? `/api/clothes/${editingId}` : "/api/clothes", { method: editingId ? "PUT" : "POST", body: JSON.stringify(body) });
    toast(editingId ? "Changes saved" : "Added to wardrobe");
    await loadItems(); show("wardrobe");
  } catch (e) { showError("#form-error", e.message); }
}

// ---------- outfits ----------
const favKey = (o) => [o.top, o.bottom, o.shoes].join("|");
function getFavs() { try { return JSON.parse(localStorage.getItem("sw-favs")) || []; } catch { return []; } }
function toggleFav(o) {
  const favs = getFavs(), k = favKey(o), has = favs.some((f) => favKey(f) === k);
  try { localStorage.setItem("sw-favs", JSON.stringify(has ? favs.filter((f) => favKey(f) !== k) : [...favs, o])); } catch { /* storage unavailable */ }
  return !has;
}
function outfitCard(o) {
  const why = Array.isArray(o.reason) ? o.reason : o.reason ? [o.reason] : [];
  const star = h("button", { class: "fav" + (getFavs().some((f) => favKey(f) === favKey(o)) ? " on" : ""), "aria-label": "Favourite outfit", title: "Favourite" }, "\u2605");
  star.addEventListener("click", () => { star.classList.toggle("on", toggleFav(o)); });
  return h("article", { class: "card outfit" }, strip(colorOf(o.top), colorOf(o.bottom), colorOf(o.shoes)),
    h("div", { class: "body" },
      h("div", { class: "score" }, h("span", {}, o.score), star),
      h("ul", { class: "parts" }, [["Top", o.top], ["Bottom", o.bottom], ["Shoes", o.shoes]].map(([k, v]) => h("li", {}, h("span", {}, k), v))),
      why.length && h("ul", { class: "why" }, why.map((w) => h("li", {}, w)))));
}
async function generate() {
  showError("#r-error", "");
  const btn = $("#gen-btn"); btn.disabled = true; btn.textContent = "Generating...";
  try {
    const d = await api("/api/outfits/generate", { method: "POST", body: JSON.stringify({ occasion: $("#r-occasion").value, season: $("#r-season").value }) });
    const list = d.outfits || [];
    $("#outfits").replaceChildren(...(list.length ? list.map(outfitCard) : [h("p", { class: "empty" }, "No valid outfits for this occasion and season. Add a top, bottom and shoes that match, or try another selection.")]));
  } catch (e) { showError("#r-error", e.message); }
  btn.disabled = false; btn.textContent = "Generate outfits";
}

// ---------- init ----------
async function init() {
  fill($("#i-type"), Object.keys(SUBTYPES)); fill($("#i-subtype"), SUBTYPES.Top); fill($("#i-color"), Object.keys(COLORS));
  chips($("#i-occasions"), "occ", OCCASIONS); chips($("#i-seasons"), "sea", SEASONS);
  fill($("#f-type"), Object.keys(SUBTYPES), "All types"); fill($("#f-subtype"), ALL_SUBTYPES, "All subtypes");
  fill($("#f-color"), Object.keys(COLORS), "All colors"); fill($("#f-occasion"), OCCASIONS, "All occasions"); fill($("#f-season"), SEASONS, "All seasons");
  fill($("#r-occasion"), OCCASIONS); fill($("#r-season"), SEASONS);

  $("#i-type").addEventListener("change", (e) => fill($("#i-subtype"), SUBTYPES[e.target.value]));
  $("#f-type").addEventListener("change", (e) => fill($("#f-subtype"), SUBTYPES[e.target.value] || ALL_SUBTYPES, "All subtypes"));
  ["#f-search", "#f-type", "#f-subtype", "#f-color", "#f-occasion", "#f-season"].forEach((s) => $(s).addEventListener("input", renderWardrobe));
  $("#save-btn").addEventListener("click", saveItem);
  $("#cancel-btn").addEventListener("click", () => show("wardrobe"));
  $("#gen-btn").addEventListener("click", generate);

  const hp = $("#health");
  try { await api("/api/health"); hp.textContent = "Backend connected"; hp.className = "health ok"; }
  catch { hp.textContent = "Backend offline"; hp.className = "health bad"; }
  try { await loadItems(); } catch (e) { toast(e.message); }
  show("dashboard");
}
init();

const input = document.getElementById("query");
const output = document.getElementById("output");
const suggestions = document.getElementById("suggestions");

function search() {
  const q = input.value.trim();
  if (!q) return;

  fetch(`http://localhost:8080/search?q=${q}`)
    .then(res => res.json())
    .then(data => {
      output.innerHTML = "";
      if (!data.results.length) {
        output.innerHTML = "<p>No results found</p>";
        return;
      }

      data.results.forEach(r => {
        const card = document.createElement("div");
        card.className = "result-card";

        card.innerHTML = `
          <h3>${r.document.split("/").pop()}</h3>
          <p><b>Frequency:</b> ${r.frequency}</p>
          <p><b>Positions:</b> ${r.positions.join(", ")}</p>
          <p><b>Offsets:</b> ${r.offsets.join(", ")}</p>
        `;
        output.appendChild(card);
      });
    });
}

// Autocomplete
input.addEventListener("input", () => {
  const q = input.value.trim();
  if (!q) {
    suggestions.innerHTML = "";
    return;
  }

  fetch(`http://localhost:8080/autocomplete?prefix=${q}`)
    .then(res => res.json())
    .then(data => {
      suggestions.innerHTML = "";
      data.suggestions.forEach(word => {
        const div = document.createElement("div");
        div.className = "suggestion";
        div.innerText = word;
        div.onclick = () => {
          input.value = word;
          suggestions.innerHTML = "";
        };
        suggestions.appendChild(div);
      });
    });
});

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Little Ticker Config</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    body {
      font-family: Arial, Helvetica, sans-serif;
      text-align: center;
      margin: 0;
      padding: 0;
      background-color: #f2f2f2;
    }

    .topnav {
      background-color: #005f99;
      overflow: hidden;
      padding: 20px 0;
    }

    .topnav h1 {
      color: white;
      margin: 0;
      font-size: 24px;
    }

    .content {
      padding: 20px;
    }

    .card-grid {
      display: flex;
      justify-content: center;
      flex-wrap: wrap;
    }

    .card {
      background-color: white;
      box-shadow: 2px 2px 12px rgba(0, 0, 0, 0.3);
      padding: 20px;
      border-radius: 8px;
      width: 300px;
      max-width: 90vw;
      margin: 20px;
      text-align: left;
      box-sizing: border-box;
    }

    label {
      font-weight: bold;
      display: block;
      margin-bottom: 4px;
    }

    input[type=number],
    select,
    input[type=text] {
      font-size: 16px;
      box-sizing: border-box;
      width: 100%;
      padding: 10px;
      margin-top: 6px;
      margin-bottom: 12px;
      border: 1px solid #ccc;
      border-radius: 4px;
    }

    input[type=submit] {
      background-color: #4CAF50;
      color: white;
      border: none;
      padding: 12px;
      border-radius: 4px;
      width: 100%;
      font-size: 16px;
      cursor: pointer;
    }

    input[type=submit]:hover {
      background-color: #45a049;
    }

    #status {
      margin-top: 10px;
      font-weight: bold;
      color: #333;
    }
  </style>
</head>
<body>
  <div class="topnav">
    <h1>Little Ticker</h1>
  </div>
  <div class="content">
    <div class="card-grid">
      <div class="card">
        <form id="configForm">
          <p>
            <label for="ticker">Ticker Symbol</label>
            <input type="number" id="ticker" name="ticker" min="0" max="255"><br>

            <label for="currency">Currency</label>
            <input type="number" id="currency" name="currency"><br>

            <label for="timeframe">Chart Time Frame</label>
            <select id="timeframe" name="timeframe">
              <option value=0>1-Day</option>
              <option value=1>1-Week</option>
              <option value=2>1-Month</option>
            </select><br>

            <input type="submit" value="Save Config">
          </p>
          <p id="status"></p>
        </form>
      </div>
    </div>
  </div>

  <script>
    const baseUrl = location.origin;

    // Load current config on page load
    window.onload = async () => {
      try {
        const res = await fetch(`${baseUrl}/config`);
        const data = await res.json();
        document.getElementById("ticker").value = data.ticker;
        document.getElementById("currency").value = data.currency;
        document.getElementById("timeframe").value = data.timeframe;
      } catch (e) {
        document.getElementById("status").textContent = "⚠️ Failed to load config.";
      }
    };

    // Handle form submission
    document.getElementById("configForm").onsubmit = async (e) => {
      e.preventDefault();

      const payload = {
        ticker: parseInt(document.getElementById("ticker").value),
        currency: parseInt(document.getElementById("currency").value),
        timeframe: document.getElementById("timeframe").value
      };

      try {
        const res = await fetch(`${baseUrl}/set`, {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify(payload)
        });

        const result = await res.json();
        document.getElementById("status").textContent =
          result.status === "ok" ? "✅ Config saved" : "⚠️ Save failed";
      } catch (err) {
        document.getElementById("status").textContent = "⚠️ Save failed.";
      }
    };
  </script>
</body>
</html>
)rawliteral";
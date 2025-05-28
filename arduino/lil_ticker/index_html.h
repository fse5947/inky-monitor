const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Little Ticker Config</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <link href="https://cdn.jsdelivr.net/npm/select2@4.1.0-rc.0/dist/css/select2.min.css" rel="stylesheet" />
  <script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
  <script src="https://cdn.jsdelivr.net/npm/select2@4.1.0-rc.0/dist/js/select2.min.js"></script>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      margin: 0;
      padding: 0;
      background-color: #f2f2f2;
    }
    .topnav {
      background-color: #005f99;
      padding: 20px 0;
    }
    .topnav h1 {
      color: white;
      margin: 0;
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
      text-align: left;
    }
    label {
      font-weight: bold;
      display: block;
      margin-bottom: 6px;
    }
    select {
      width: 100%;
      padding: 10px;
      margin-bottom: 12px;
      font-size: 16px;
      border-radius: 4px;
      border: 1px solid #ccc;
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
    }

    .select2-container {
      margin-bottom: 16px;
    }

    .select2-selection__rendered img {
      margin-right: 6px;
      vertical-align: middle;
      height: 20px;
    }

    .select2-container--default .select2-selection--single,
    .select2-results__option {
      font-family: Arial, Helvetica, sans-serif;
      font-size: 16px;
      font-weight: normal;
      color: #333;
    }

    /* Match dropdown appearance */
    .select2-container--default .select2-selection--single {
      background-color: #f8f8f8;
      border: 1px solid #ccc;
      border-radius: 4px;
      height: 42px;
      padding: 6px 12px;
      display: flex;
      align-items: center;
    }

    /* Match text alignment and spacing */
    .select2-container--default .select2-selection--single .select2-selection__rendered {
      line-height: 28px;
      padding-left: 0;
      display: flex;
      align-items: center;
      font-size: 16px;
      font-family: Arial, Helvetica, sans-serif;
    }

    /* Adjust the dropdown arrow */
    .select2-container--default .select2-selection--single .select2-selection__arrow {
      height: 100%;
      right: 6px;
    }

    /* Dropdown list option alignment */
    .select2-results__option {
      padding: 6px 10px;
      display: flex;
      align-items: center;
    }

    /* Option logos spacing */
    .select2-results__option img {
      margin-right: 8px;
      height: 20px;
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
          <label for="ticker">Ticker Symbol</label>
            <select id="ticker" name="ticker">
              <option value="0" data-image="https://logo.clearbit.com/apple.com">AAPL</option>
              <option value="1" data-image="https://logo.clearbit.com/google.com">GOOG</option>
              <option value="2" data-image="https://logo.clearbit.com/tesla.com">TSLA</option>
            </select>

          <label for="currency">Currency</label>
            <select id="currency" name="currency">
              <option value="0" data-image="https://assets.kraken.com/marketing/web/icons-uni-webp/s_aud.webp?i=kds">AUD</option>
              <option value="1" data-image="https://assets.kraken.com/marketing/web/icons-uni-webp/s_cad.webp?i=kds">CAD</option>
              <option value="2" data-image="https://assets.kraken.com/marketing/web/icons-uni-webp/s_chf.webp?i=kds">CHF</option>
              <option value="3" data-image="https://assets.kraken.com/marketing/web/icons-uni-webp/s_eur.webp?i=kds">EUR</option>
              <option value="4" data-image="https://assets.kraken.com/marketing/web/icons-uni-webp/s_gbp.webp?i=kds">GBP</option>
              <option value="5" data-image="https://assets.kraken.com/marketing/web/icons-uni-webp/s_jpy.webp?i=kds">JPY</option>
              <option value="6" data-image="https://assets.kraken.com/marketing/web/icons-uni-webp/s_usd.webp?i=kds">USD</option>
            </select><br>

          <label for="timeframe">Chart Time Frame</label>
            <select id="timeframe" name="timeframe">
              <option value="0">1-Day</option>
              <option value="1">1-Week</option>
              <option value="2">1-Month</option>
            </select>

          <input type="submit" value="Save Configuration">
          <p id="status"></p>
        </form>
      </div>
    </div>
  </div>

  <script>
    function formatWithImage(option) {
      if (!option.id) return option.text;
      const image = $(option.element).data('image');
      if (!image) return option.text;
      return $(`
      <span style="display: flex; align-items: center;">
        <img src="${image}" style="height: 18px; width: 18px; margin-right: 8px; object-fit: contain; vertical-align: middle;" />
        ${option.text}
      </span>
    ` );
    }

    $(document).ready(function() {
      $('#ticker').select2({
        templateResult: formatWithImage,
        templateSelection: formatWithImage
      });

      $('#currency').select2({
        templateResult: formatWithImage,
        templateSelection: formatWithImage
      });

      $('#timeframe').select2({
        minimumResultsForSearch: Infinity // hides the search box
      });
    });

    const baseUrl = location.origin;

    // Load config
    window.onload = async () => {
      try {
        const res = await fetch(`${baseUrl}/config`);
        const data = await res.json();
        $('#ticker').val(data.ticker).trigger('change');
        $('#currency').val(data.currency).trigger('change');
        document.getElementById("timeframe").value = data.timeframe;
      } catch (e) {
        document.getElementById("status").textContent = "⚠️ Failed to load config.";
      }
    };

    // Submit config
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

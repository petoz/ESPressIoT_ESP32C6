#ifndef WEBSTATIC_H
#define WEBSTATIC_H

#include <Arduino.h>

static const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESPressIoT Dashboard</title>
    <link rel="stylesheet" href="style.css">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script> 
</head>
<body>
    <div class="container">
        <header>
            <h1>ESPressIoT</h1>
            <div id="status-indicator" class="status-offline">Connecting...</div>
        </header>

        <div class="card">
            <h2>Current Status</h2>
            <div class="status-grid">
                <div class="status-item">
                    <span class="label">Temperature</span>
                    <span class="value" id="temp-value">-- °C</span>
                </div>
                <div class="status-item">
                    <span class="label">Heater Power</span>
                    <span class="value" id="power-value">-- %</span>
                </div>
                <div class="status-item">
                    <span class="label">Target</span>
                    <span class="value" id="target-value">-- °C</span>
                </div>
                <div class="status-item">
                    <span class="label">ECO Timer</span>
                    <span class="value" id="eco-value">--</span>
                </div>
            </div>
        </div>

        <div class="card">
            <h2>Temperature History</h2>
            <div class="chart-container">
                <canvas id="tempChart"></canvas>
            </div>
        </div>

        <div class="card">
            <h2>Controls</h2>
            <div class="control-group">
                <button id="btn-heater-on" onclick="control('heater_on')">Heater ON</button>
                <button id="btn-heater-off" onclick="control('heater_off')" class="danger">Heater OFF</button>
            </div>
            <div class="control-group">
                <label>Set Target Temp:</label>
                <input type="number" id="target-input" step="0.5" value="96.5">
                <button onclick="updateConfig()">Set</button>
            </div>
        </div>
        
        <footer>
            <a href="/config.html">Advanced Configuration</a>
        </footer>
    </div>
    <script src="script.js"></script>
</body>
</html>

)rawliteral";

static const char config_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>EspressIoT Configuration</title>
    <link rel="stylesheet" href="style.css">
</head>

<body>
    <div class="container">
        <header>
            <h1>Configuration</h1>
            <a href="/" class="button">Back to Dashboard</a>
        </header>

        <div class="card">
            <h2>General Settings</h2>
            <form action="/set_config" method="GET">
                <div class="control-group">
                    <label>Target Temperature:</label>
                    <input type="text" name="tset" id="tset">
                </div>
                <div class="control-group">
                    <label>Threshold for adaptive PID:</label>
                    <input type="text" name="tband" id="tband">
                </div>
                <div class="control-group">
                    <label>ECO Time (minutes, 0=disabled):</label>
                    <input type="text" name="eco_time" id="eco_time">
                </div>
                <!-- TODO: MQTT sections -->
                <div class="control-group">
                    <label>Enable MQTT:</label>
                    <select name="mqtt_enabled" id="mqtt_enabled">
                        <option value="1">Enabled</option>
                        <option value="0">Disabled</option>
                    </select>
                </div>
                <div class="control-group">
                    <label>MQTT Topic:</label>
                    <input type="text" name="mqtt_topic" id="mqtt_topic">
                </div>
                <div class="control-group">
                    <label>Reference Resistor (Ohms):</label>
                    <input type="text" name="rref" id="rref">
                </div>
                <button type="submit">Submit</button>
            </form>
            <div class="action-buttons">
                <a href="/loadconf"><button>Load Config</button></a>
                <a href="/saveconf"><button>Save Config</button></a>
                <a href="/resetconf"><button>Reset Config to Default</button></a>
                <a href="/update"><button>Update Firmware</button></a>
            </div>
        </div>

        <div class="card">
            <h2>PID Parameters</h2>
            <form action="/set_config" method="GET">
                <h3>Normal PID</h3>
                <div class="control-group">
                    <label>P:</label> <input type="text" name="pgain" id="pgain">
                </div>
                <div class="control-group">
                    <label>I:</label> <input type="text" name="igain" id="igain">
                </div>
                <div class="control-group">
                    <label>D:</label> <input type="text" name="dgain" id="dgain">
                </div>

                <h3>Adaptive PID</h3>
                <div class="control-group">
                    <label>P:</label> <input type="text" name="apgain" id="apgain">
                </div>
                <div class="control-group">
                    <label>I:</label> <input type="text" name="aigain" id="aigain">
                </div>
                <div class="control-group">
                    <label>D:</label> <input type="text" name="adgain" id="adgain">
                </div>
                <button type="submit">Submit PID</button>
            </form>
        </div>

        <div class="card">
            <h2>Tuning</h2>
            <form action="/set_tuning" method="GET">
                <div class="control-group">
                    <label>Tuning Threshold (°C):</label>
                    <input type="text" name="tunethres" id="tunethres">
                </div>
                <div class="control-group">
                    <label>Tuning Power (heater):</label>
                    <input type="text" name="tunestep" id="tunestep">
                </div>
                <button type="submit">Submit Tuning Params</button>
            </form>
            <hr>
            <div class="action-buttons">
                <a href="/tuningmode"><button style="background-color:#98B4D4">Toggle PID Tuning Mode</button></a>
                <a href="/tuningstats"><button>Stats</button></a>
            </div>
        </div>

        <footer>
            <div id="version-info">
                <div><small id="fw_version">Version: --</small></div>
                <div><small id="git_commit">Commit: --</small></div>
                <div><small id="build_time">Build: --</small></div>
            </div>
        </footer>
    </div>

    <script>
        // Fetch config on load
        fetch('/api/config')
            .then(response => response.json())
            .then(data => {
                document.getElementById('tset').value = data.tset;
                document.getElementById('tband').value = data.tband;
                document.getElementById('eco_time').value = data.eco_time;
                document.getElementById('mqtt_enabled').value = data.mqtt_enabled;
                document.getElementById('mqtt_topic').value = data.mqtt_topic;
                document.getElementById('rref').value = data.rref;

                document.getElementById('pgain').value = data.pgain;
                document.getElementById('igain').value = data.igain;
                document.getElementById('dgain').value = data.dgain;

                document.getElementById('apgain').value = data.apgain;
                document.getElementById('aigain').value = data.aigain;
                document.getElementById('adgain').value = data.adgain;

                document.getElementById('tunethres').value = data.tunethres;
                document.getElementById('tunestep').value = data.tunestep;

                document.getElementById('fw_version').textContent = 'Version: ' + data.fw_version;
                document.getElementById('git_commit').textContent = 'Commit: ' + data.git_commit;
                document.getElementById('build_time').textContent = 'Build: ' + data.build_time;
            })
            .catch(err => console.error('Error fetching config:', err));
    </script>
</body>

</html>
)rawliteral";

const char style_css[] PROGMEM = R"rawliteral(
:root {
  --bg-color: #121212;
  --card-bg: #1e1e1e;
  --text-primary: #e0e0e0;
  --text-secondary: #a0a0a0;
  --accent: #bb86fc;
  --danger: #cf6679;
  --success: #03dac6;
}

body {
  font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
  background-color: var(--bg-color);
  color: var(--text-primary);
  margin: 0;
  padding: 20px;
}

.container {
  max-width: 800px;
  margin: 0 auto;
}

header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}

.card {
  background-color: var(--card-bg);
  border-radius: 12px;
  padding: 20px;
  margin-bottom: 20px;
  box-shadow: 0 4px 6px rgba(0, 0, 0, 0.3);
}

.status-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  /* Changed to 2 columns for 4 items */
  gap: 15px;
  text-align: center;
}

.status-item {
  display: flex;
  flex-direction: column;
}

.label {
  font-size: 0.9em;
  color: var(--text-secondary);
}

.value {
  font-size: 1.5em;
  font-weight: bold;
  color: var(--accent);
}

.chart-container {
  position: relative;
  height: 300px;
  width: 100%;
}

button {
  background-color: var(--accent);
  color: #000;
  border: none;
  padding: 10px 20px;
  border-radius: 6px;
  cursor: pointer;
  font-weight: bold;
  transition: transform 0.1s;
  margin-right: 10px;
}

button:active {
  transform: scale(0.98);
}

button.danger {
  background-color: var(--danger);
}

input {
  background: #333;
  border: 1px solid #444;
  color: white;
  padding: 8px;
  border-radius: 4px;
}

.status-offline {
  color: var(--danger);
}

.status-online {
  color: var(--success);
}

/*# sourceMappingURL=style.css.map */

)rawliteral";

const char script_js[] PROGMEM = R"rawliteral(
const ctx = document.getElementById('tempChart').getContext('2d');
const tempChart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Temperature (°C)',
            data: [],
            borderColor: '#bb86fc',
            tension: 0.4
        },
        {
            label: 'Target (°C)',
            data: [],
            borderColor: '#03dac6',
            borderDash: [5, 5],
            fill: false
        }]
    },
    options: {
        responsive: true,
        maintainAspectRatio: false,
        scales: {
            x: { display: false },
            y: { beginAtZero: false } // Auto scale
        },
        animation: false // Disable animation for performance
    }
});

function updateStatus() {
    fetch('/api/status')
        .then(response => response.json())
        .then(data => {
            document.getElementById('status-indicator').textContent = "Online";
            document.getElementById('status-indicator').className = "status-online";

            document.getElementById('temp-value').textContent = data.mesauredTemperature.toFixed(2) + ' °C';
            document.getElementById('target-value').textContent = data.targetTemperature.toFixed(1) + ' °C';
            document.getElementById('temp-value').textContent = data.mesauredTemperature.toFixed(2) + ' °C';
            document.getElementById('target-value').textContent = data.targetTemperature.toFixed(1) + ' °C';
            document.getElementById('power-value').textContent = (data.heaterPower / 10).toFixed(0) + ' %';
            
            // Update ECO
            const ecoVal = document.getElementById('eco-value');
            if (data.ecoTimeRemaining >= 0) {
                 const minutes = Math.floor(data.ecoTimeRemaining / 60000);
                 const seconds = Math.floor((data.ecoTimeRemaining % 60000) / 1000);
                 ecoVal.textContent = minutes + "m " + seconds + "s";
                 ecoVal.style.color = "var(--accent)";
            } else {
                 ecoVal.textContent = "Disabled";
                 ecoVal.style.color = "var(--text-secondary)";
            }

            // Update inputs
            if (document.activeElement !== document.getElementById('target-input')) {
                document.getElementById('target-input').value = data.targetTemperature;
            }

            // Update Chart
            const now = new Date().toLocaleTimeString();
            if (tempChart.data.labels.length > 180) { // Keep last 180 points
                tempChart.data.labels.shift();
                tempChart.data.datasets[0].data.shift();
                tempChart.data.datasets[1].data.shift();
            }
            tempChart.data.labels.push(now);
            tempChart.data.datasets[0].data.push(data.mesauredTemperature);
            tempChart.data.datasets[1].data.push(data.targetTemperature);
            tempChart.update();
        })
        .catch(err => {
            document.getElementById('status-indicator').textContent = "Offline";
            document.getElementById('status-indicator').className = "status-offline";
        });
}

function control(action) {
    fetch('/' + action).then(updateStatus);
}

function updateConfig() {
    const newVal = document.getElementById('target-input').value;
    fetch('/set_config?tset=' + newVal).then(updateStatus);
}

setInterval(updateStatus, 2000);
updateStatus();

)rawliteral";

#endif

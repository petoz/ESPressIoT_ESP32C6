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
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
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
    box-shadow: 0 4px 6px rgba(0,0,0,0.3);
}

.status-grid {
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    gap: 10px;
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

.status-offline { color: var(--danger); }
.status-online { color: var(--success); }
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
            document.getElementById('power-value').textContent = (data.heaterPower / 10).toFixed(0) + ' %';

            // Update inputs
            if (document.activeElement !== document.getElementById('target-input')) {
                document.getElementById('target-input').value = data.targetTemperature;
            }

            // Update Chart
            const now = new Date().toLocaleTimeString();
            if (tempChart.data.labels.length > 60) { // Keep last 60 points
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

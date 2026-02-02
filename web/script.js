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

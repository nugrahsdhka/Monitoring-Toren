// Konfigurasi HiveMQ
const host = "wss://b991b24530db4275a23f9fa3f4fe29f6.s1.eu.hivemq.cloud:8884/mqtt";
const client = mqtt.connect(host, {
    username: "TugasBesarAndesis12",
    password: "TugasBesarAndesis12"
});

const logTable = document.getElementById('log-table');

const ctx = document.getElementById('levelChart').getContext('2d');
const levelChart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Level Air (%)',
            data: [],
            borderColor: '#10b981',
            backgroundColor: 'rgba(16, 185, 129, 0.1)',
            fill: true,
            tension: 0.4
        }]
    },
    options: { responsive: true, scales: { y: { beginAtZero: true, max: 100 } } }
});

let lastData = null;

client.on('connect', () => {
    document.getElementById('status-badge').textContent = "Connected";
    document.getElementById('status-badge').className = "px-3 py-1 rounded-full text-xs font-mono bg-emerald-900 border border-emerald-500 text-emerald-400";
    client.subscribe("sensor/tangki");
});

client.on('message', (topic, message) => {
    const data = JSON.parse(message.toString());
    lastData = data; 
    const now = new Date().toLocaleTimeString();

    document.getElementById('status-tangki').textContent = data.status;
    document.getElementById('level-persen').textContent = data.level + "%";
    document.getElementById('jarak-air').textContent = data.jarak + " cm";

    if (levelChart.data.labels.length > 20) {
        levelChart.data.labels.shift();
        levelChart.data.datasets[0].data.shift();
    }
    levelChart.data.labels.push(now);
    levelChart.data.datasets[0].data.push(parseInt(data.level));
    levelChart.update();

    const row = `
        <tr class="fade-in">
            <td class="py-3 font-mono text-emerald-500/60">${now}</td>
            <td class="py-3 font-medium">${data.status}</td>
            <td class="py-3 font-mono">${data.level}%</td>
        </tr>`;
    logTable.insertAdjacentHTML('afterbegin', row);
});


setInterval(() => {
    if (lastData) {
        const now = new Date().toLocaleTimeString();
        
        if (levelChart.data.labels.length > 20) {
            levelChart.data.labels.shift();
            levelChart.data.datasets[0].data.shift();
        }
        levelChart.data.labels.push(now);
        levelChart.data.datasets[0].data.push(parseInt(lastData.level));
        levelChart.update();
    }
}, 1000);
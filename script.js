// Konfigurasi HiveMQ (Pastikan menggunakan URL WebSocket)
const host = "wss://b991b24530db4275a23f9fa3f4fe29f6.s1.eu.hivemq.cloud:8884/mqtt";
const client = mqtt.connect(host, {
    username: "TugasBesarAndesis12",
    password: "TugasBesarAndesis12"
});

const logTable = document.getElementById('log-table');

client.on('connect', () => {
    document.getElementById('status-badge').textContent = "Connected";
    document.getElementById('status-badge').className = "px-3 py-1 rounded-full text-xs font-mono bg-emerald-900 border border-emerald-500 text-emerald-400";
    client.subscribe("sensor/tangki");
});

client.on('message', (topic, message) => {
    const data = JSON.parse(message.toString());
    const now = new Date().toLocaleTimeString();

    // Update UI
    document.getElementById('status-tangki').textContent = data.status;
    document.getElementById('level-persen').textContent = data.level + "%";
    document.getElementById('jarak-air').textContent = data.jarak + " cm";

    // Insert to Log
    const row = `
        <tr class="fade-in">
            <td class="py-3 font-mono text-emerald-500/60">${now}</td>
            <td class="py-3 font-medium">${data.status}</td>
            <td class="py-3 font-mono">${data.level}%</td>
        </tr>`;
    logTable.insertAdjacentHTML('afterbegin', row);
});
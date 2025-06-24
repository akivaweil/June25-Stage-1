/*
 * WebSocket_Manager.cpp - WebSocket Serial Monitor for ESP32-S3
 * 
 * SETUP INSTRUCTIONS:
 * 1. Copy this file (WebSocket_Manager.cpp) and WebSocket_Manager.h to your project's src/ and include/ folders
 * 2. In your main.cpp:
 *    - #include "StateMachine/WebSocket_Manager.h" // Adjusted path
 *    - Call initWebSocket() in setup() after WiFi is connected
 *    - Call handleWebSocket() in loop()
 *    - Use SerialWS_print(), SerialWS_println(), SerialWS_printf() instead of Serial.print functions
 * 
 * USAGE:
 * - Navigate to ESP32's IP address in browser
 * - View real-time serial output via websocket connection
 * - Messages appear in both serial monitor and web browser
 * - Automatic reconnection and message buffering for seamless experience
 * 
 * BROWSER COMPATIBILITY:
 * - Works with Chrome, Firefox, Safari, Edge
 * - Requires WebSocket support (all modern browsers)
 * - Handles reconnections gracefully with message history
 */

#include "StateMachine/WebSocket_Manager.h" // Adjusted path
#include <stdarg.h>

//* ************************************************************************
//* ************************ WEBSOCKET VARIABLES *************************
//* ************************************************************************

AsyncWebServer server(WEB_SERVER_PORT);
AsyncWebSocket ws("/ws");

// Message buffer for reconnection support
String messageBuffer[MESSAGE_BUFFER_SIZE];
int bufferIndex = 0;
int bufferCount = 0;
bool bufferFull = false;

// Heartbeat and connection monitoring
unsigned long lastHeartbeat = 0;
unsigned long lastClientCheck = 0;

// Motor movement WebSocket pause system
bool webSocketPaused = false;
String pausedMessageBuffer[50];  // Smaller buffer for paused messages
int pausedBufferIndex = 0;
int pausedBufferCount = 0;
bool pausedBufferFull = false;

//* ************************************************************************
//* ************************ HTML PAGE CONTENT ****************************
//* ************************************************************************

const char* html_page = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-S3 Serial Monitor</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #0c0c0c 0%, #1a1a1a 100%);
            color: #e1e1e1;
            min-height: 100vh;
            padding: 20px;
            overflow-x: hidden;
        }
        
        .container {
            max-width: 1400px;
            margin: 0 auto;
            animation: fadeIn 0.8s ease-out;
        }
        
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(20px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        .header {
            text-align: center;
            margin-bottom: 30px;
            padding: 30px 0;
            background: linear-gradient(45deg, #2d3748, #1a202c);
            border-radius: 20px;
            border: 1px solid #8b5cf6;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3), 0 0 10px rgba(139, 92, 246, 0.15);
            position: relative;
            overflow: hidden;
        }
        
        .header::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.1), transparent);
            animation: shimmer 8s infinite;
        }
        
        @keyframes shimmer {
            0% { left: -100%; }
            100% { left: 100%; }
        }
        
        h1 {
            color: #60a5fa;
            font-size: 2.5rem;
            font-weight: 700;
            margin-bottom: 10px;
            text-shadow: 0 0 20px rgba(96, 165, 250, 0.3);
            position: relative;
            z-index: 1;
        }
        
        .status-container {
            display: flex;
            justify-content: center;
            margin-bottom: 25px;
        }
        
        .status {
            display: inline-flex;
            align-items: center;
            gap: 10px;
            padding: 12px 24px;
            border-radius: 25px;
            border: 1px solid #a855f7;
            font-weight: 600;
            font-size: 1rem;
            transition: all 0.3s ease;
            position: relative;
            overflow: hidden;
        }
        
        .status::before {
            content: '';
            position: absolute;
            top: 50%;
            left: 50%;
            width: 0;
            height: 0;
            border-radius: 50%;
            transition: all 0.3s ease;
            transform: translate(-50%, -50%);
        }
        
        .connected {
            background: linear-gradient(45deg, #10b981, #047857);
            color: white;
            border: 2px solid #34d399;
            box-shadow: 0 5px 15px rgba(16, 185, 129, 0.3), 0 0 15px rgba(52, 211, 153, 0.4);
        }
        
        .connected::before {
            background: rgba(255, 255, 255, 0.2);
            width: 100px;
            height: 100px;
        }
        
        .disconnected {
            background: linear-gradient(45deg, #ef4444, #dc2626);
            color: white;
            border: 2px solid #f87171;
            box-shadow: 0 5px 15px rgba(239, 68, 68, 0.3), 0 0 15px rgba(248, 113, 113, 0.4);
        }
        
        .status-indicator {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            animation: pulse 2s infinite;
        }
        
        .connected .status-indicator {
            background: #34f0a1;
        }
        
        .disconnected .status-indicator {
            background: #fca5a5;
        }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; transform: scale(1); }
            50% { opacity: 0.7; transform: scale(1.2); }
        }
        
        .terminal-container {
            background: linear-gradient(145deg, #1e293b, #0f172a);
            border-radius: 15px;
            border: 1px solid #8b5cf6;
            box-shadow: 
                0 20px 40px rgba(0, 0, 0, 0.4),
                inset 0 1px 0 rgba(255, 255, 255, 0.1),
                0 0 15px rgba(139, 92, 246, 0.15);
            overflow: hidden;
            position: relative;
        }
        
        .terminal-header {
            background: linear-gradient(90deg, #374151, #1f2937);
            padding: 15px 20px;
            display: flex;
            align-items: center;
            gap: 10px;
            border-bottom: 1px solid #8b5cf6;
        }
        
        .terminal-dot {
            width: 12px;
            height: 12px;
            border-radius: 50%;
        }
        
        .dot-red { background: #ef4444; }
        .dot-yellow { background: #f59e0b; }
        .dot-green { background: #10b981; }
        
        .terminal-title {
            margin-left: 15px;
            color: #9ca3af;
            font-size: 0.9rem;
            font-weight: 500;
        }
        
        .serial-output {
            background: #0f172a;
            padding: 20px;
            height: 500px;
            overflow-y: auto;
            font-family: 'JetBrains Mono', 'Fira Code', 'Courier New', monospace;
            font-size: 14px;
            line-height: 1.2;
            white-space: pre-wrap;
            color: #e2e8f0;
            border: 1px solid #8b5cf6;
            box-shadow: inset 0 0 10px rgba(139, 92, 246, 0.2);
            position: relative;
        }
        
        .serial-output::-webkit-scrollbar {
            width: 8px;
        }
        
        .serial-output::-webkit-scrollbar-track {
            background: rgba(255, 255, 255, 0.05);
            border-radius: 4px;
        }
        
        .serial-output::-webkit-scrollbar-thumb {
            background: linear-gradient(45deg, #4f46e5, #7c3aed);
            border-radius: 4px;
        }
        
        .serial-output::-webkit-scrollbar-thumb:hover {
            background: linear-gradient(45deg, #6366f1, #8b5cf6);
        }
        
        .controls {
            padding: 20px;
            background: linear-gradient(90deg, #1e293b, #334155);
            border-top: 1px solid #8b5cf6;
            display: flex;
            justify-content: center;
            gap: 15px;
            flex-wrap: wrap;
        }
        
        button {
            background: linear-gradient(45deg, #3b82f6, #1d4ed8);
            color: white;
            border: 1px solid #8b5cf6;
            padding: 12px 24px;
            border-radius: 8px;
            cursor: pointer;
            font-size: 14px;
            font-weight: 600;
            transition: all 0.3s ease;
            position: relative;
            overflow: hidden;
            box-shadow: 0 4px 15px rgba(59, 130, 246, 0.3), 0 0 8px rgba(139, 92, 246, 0.1);
        }
        
        button::before {
            content: '';
            position: absolute;
            top: 50%;
            left: 50%;
            width: 0;
            height: 0;
            background: rgba(255, 255, 255, 0.2);
            border-radius: 50%;
            transition: all 0.3s ease;
            transform: translate(-50%, -50%);
        }
        
        button:hover {
            transform: translateY(-2px);
            border-color: #a855f7;
            box-shadow: 0 6px 20px rgba(59, 130, 246, 0.4), 0 0 25px rgba(168, 85, 247, 0.4);
        }
        
        button:hover::before {
            width: 300px;
            height: 300px;
        }
        
        button:active {
            transform: translateY(0);
        }
        
        .btn-clear {
            background: linear-gradient(45deg, #ef4444, #dc2626);
            border: 2px solid #f87171;
            box-shadow: 0 4px 15px rgba(239, 68, 68, 0.3), 0 0 15px rgba(248, 113, 113, 0.2);
        }
        
        .btn-clear:hover {
            border-color: #fca5a5;
            box-shadow: 0 6px 20px rgba(239, 68, 68, 0.4), 0 0 25px rgba(252, 165, 165, 0.4);
        }
        
        .btn-download {
            background: linear-gradient(45deg, #10b981, #047857);
            border: 2px solid #34d399;
            box-shadow: 0 4px 15px rgba(16, 185, 129, 0.3), 0 0 15px rgba(52, 211, 153, 0.2);
        }
        
        .btn-download:hover {
            border-color: #6ee7b7;
            box-shadow: 0 6px 20px rgba(16, 185, 129, 0.4), 0 0 25px rgba(110, 231, 183, 0.4);
        }
        
        .timestamp {
            color: #22d3ee;
            font-weight: 600;
        }
        
        .system-message {
            color: #34d399;
            font-weight: 500;
        }
        
        .error-message {
            color: #f87171;
            font-weight: 500;
        }
        
        .stats {
            display: flex;
            justify-content: space-around;
            margin-top: 20px;
            padding: 20px;
            background: linear-gradient(45deg, #1e293b, #334155);
            border-radius: 15px;
            border: 1px solid #8b5cf6;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2), 0 0 10px rgba(139, 92, 246, 0.15);
        }
        
        .stat-item {
            text-align: center;
            padding: 15px;
            border-radius: 10px;
            border: 1px solid #a855f7;
            background: rgba(139, 92, 246, 0.1);
            transition: all 0.3s ease;
        }
        
        .stat-item:hover {
            border-color: #c084fc;
            background: rgba(139, 92, 246, 0.2);
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(139, 92, 246, 0.3);
        }
        
        .stat-value {
            font-size: 1.5rem;
            font-weight: 700;
            color: #60a5fa;
            display: block;
        }
        
        .stat-label {
            font-size: 0.9rem;
            color: #9ca3af;
            margin-top: 5px;
        }
        
        .ota-progress {
            position: fixed;
            top: 20px;
            right: 20px;
            background: linear-gradient(45deg, #1e293b, #334155);
            border: 2px solid #8b5cf6;
            border-radius: 10px;
            padding: 15px 20px;
            min-width: 200px;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.5), 0 0 20px rgba(139, 92, 246, 0.3);
            z-index: 1000;
            display: none;
            animation: slideIn 0.3s ease-out;
        }
        
        @keyframes slideIn {
            from { transform: translateX(100%); opacity: 0; }
            to { transform: translateX(0); opacity: 1; }
        }
        
        .ota-progress h3 {
            margin: 0 0 10px 0;
            color: #60a5fa;
            font-size: 1.1rem;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        
        .ota-progress-bar {
            width: 100%;
            height: 8px;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 4px;
            overflow: hidden;
            margin-bottom: 8px;
        }
        
        .ota-progress-fill {
            height: 100%;
            background: linear-gradient(45deg, #10b981, #34d399);
            border-radius: 4px;
            transition: width 0.3s ease;
            box-shadow: 0 0 10px rgba(52, 211, 153, 0.5);
        }
        
        .ota-progress-text {
            text-align: center;
            color: #e2e8f0;
            font-size: 0.9rem;
            font-weight: 600;
        }
        
        @media (max-width: 768px) {
            .container {
                padding: 10px;
            }
            
            h1 {
                font-size: 2rem;
            }
            
            .controls {
                flex-direction: column;
                gap: 10px;
            }
            
            button {
                width: 100%;
            }
            
            .stats {
                flex-direction: column;
                gap: 15px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>ESP32-S3 Serial Monitor</h1>
        </div>
        
        <!-- OTA Upload Progress Box -->
        <div id="ota-progress" class="ota-progress">
            <h3>🔄 OTA Upload</h3>
            <div class="ota-progress-bar">
                <div id="ota-progress-fill" class="ota-progress-fill" style="width: 0%"></div>
            </div>
            <div id="ota-progress-text" class="ota-progress-text">0%</div>
        </div>
        
        <div class="status-container">
            <div id="status" class="status disconnected">
                <div class="status-indicator"></div>
                <span>Disconnected</span>
            </div>
        </div>
        
        <div class="terminal-container">
            <div class="terminal-header">
                <div class="terminal-dot dot-red"></div>
                <div class="terminal-dot dot-yellow"></div>
                <div class="terminal-dot dot-green"></div>
                <div class="terminal-title">Serial Output</div>
            </div>
            <div id="output" class="serial-output"></div>
            <div class="controls">
                <button onclick="clearOutput()" class="btn-clear">🗑️ Clear Output</button>
                <button onclick="toggleAutoScroll()">📜 Auto-scroll: <span id="autoscroll-status">ON</span></button>
                <button onclick="downloadLogs()" class="btn-download">💾 Download Logs</button>
                <button onclick="manualReconnect()">🔄 Reconnect</button>
            </div>
        </div>
        
        <div class="stats">
            <div class="stat-item">
                <span id="message-count" class="stat-value">0</span>
                <div class="stat-label">Messages</div>
            </div>
            <div class="stat-item">
                <span id="connection-time" class="stat-value">--</span>
                <div class="stat-label">Connected</div>
            </div>
            <div class="stat-item">
                <span id="data-received" class="stat-value">0 KB</span>
                <div class="stat-label">Data Received</div>
            </div>
        </div>
    </div>

    <script>
        let ws;
        let autoScroll = true;
        let logs = [];
        let messageCount = 0;
        let connectTime = null;
        let dataReceived = 0;
        let reconnectAttempts = 0;
        let maxReconnectAttempts = 10;
        let reconnectDelay = 3000;
        let isReceivingHistory = false;
        
        function connect() {
            // Show connecting status
            const statusEl = document.getElementById('status');
            statusEl.className = 'status disconnected';
            statusEl.innerHTML = '<div class="status-indicator"></div><span>Connecting...</span>';
            
            ws = new WebSocket('ws://' + window.location.hostname + '/ws');
            
            ws.onopen = function() {
                statusEl.className = 'status connected';
                statusEl.innerHTML = '<div class="status-indicator"></div><span>Connected</span>';
                addMessage('🔗 WebSocket connected successfully', 'system');
                connectTime = new Date();
                reconnectAttempts = 0; // Reset reconnection counter
                updateStats();
            };
            
            ws.onmessage = function(event) {
                const message = event.data;
                
                // Check if message is JSON (OTA progress or other JSON messages)
                try {
                    const jsonData = JSON.parse(message);
                    if (jsonData.type === 'ota_progress') {
                        updateOTAProgress(jsonData.percentage);
                        return;
                    } else if (jsonData.type === 'ota_start') {
                        showOTAProgress(jsonData.update_type);
                        return;
                    } else if (jsonData.type === 'ota_end') {
                        hideOTAProgress(jsonData.status, jsonData.message);
                        return;
                    }
                } catch (e) {
                    // Not JSON, continue with normal message processing
                }
                
                // Handle special history markers
                if (message === "=== RECONNECTED - Message History ===") {
                    isReceivingHistory = true;
                    addMessage('📜 Receiving message history from ESP32...', 'system');
                    return;
                } else if (message === "=== End of Message History - Live Feed Resumes ===") {
                    isReceivingHistory = false;
                    addMessage('✅ Message history complete - live feed resumed', 'system');
                    return;
                }
                
                // Add timestamp prefix for history messages
                if (isReceivingHistory) {
                    addMessage(message, 'history');
                } else {
                    addMessage(message, 'serial');
                }
                
                dataReceived += message.length;
                updateStats();
            };
            
            ws.onclose = function() {
                statusEl.className = 'status disconnected';
                connectTime = null;
                isReceivingHistory = false;
                
                if (reconnectAttempts < maxReconnectAttempts) {
                    reconnectAttempts++;
                    statusEl.innerHTML = `<div class="status-indicator"></div><span>Reconnecting... (${reconnectAttempts}/${maxReconnectAttempts})</span>`;
                    addMessage(`⚠️ Connection lost - attempting to reconnect (${reconnectAttempts}/${maxReconnectAttempts})...`, 'system');
                    
                    // Exponential backoff for reconnection delay
                    const delay = Math.min(reconnectDelay * Math.pow(1.5, reconnectAttempts - 1), 30000);
                    setTimeout(connect, delay);
                } else {
                    statusEl.innerHTML = '<div class="status-indicator"></div><span>Disconnected - Max attempts reached</span>';
                    addMessage('❌ Maximum reconnection attempts reached. Please refresh the page.', 'error');
                }
            };
            
            ws.onerror = function(error) {
                addMessage('❌ WebSocket error occurred', 'error');
            };
        }
        
        function addMessage(message, type) {
            const output = document.getElementById('output');
            const timestamp = new Date().toLocaleTimeString();
            messageCount++;
            
            let formattedMessage = '';
            if (type === 'system') {
                formattedMessage = `<span class="timestamp">[${timestamp}]</span> <span class="system-message">${message}</span>\n`;
            } else if (type === 'error') {
                formattedMessage = `<span class="timestamp">[${timestamp}]</span> <span class="error-message">${message}</span>\n`;
            } else if (type === 'history') {
                formattedMessage = `<span class="timestamp">[HISTORY]</span> ${message}\n`;
            } else {
                formattedMessage = `<span class="timestamp">[${timestamp}]</span> ${message}\n`;
            }
            
            output.innerHTML += formattedMessage;
            logs.push(`[${timestamp}] ${message}`);
            
            if (autoScroll) {
                output.scrollTop = output.scrollHeight;
            }
            
            updateStats();
        }
        
        function updateStats() {
            document.getElementById('message-count').textContent = messageCount.toLocaleString();
            
            if (connectTime) {
                const elapsed = Math.floor((new Date() - connectTime) / 1000);
                const minutes = Math.floor(elapsed / 60);
                const seconds = elapsed % 60;
                document.getElementById('connection-time').textContent = `${minutes}:${seconds.toString().padStart(2, '0')}`;
            } else {
                document.getElementById('connection-time').textContent = '--';
            }
            
            const kb = (dataReceived / 1024).toFixed(1);
            document.getElementById('data-received').textContent = `${kb} KB`;
        }
        
        function clearOutput() {
            document.getElementById('output').innerHTML = '';
            logs = [];
            messageCount = 0;
            dataReceived = 0;
            updateStats();
            addMessage('🧹 Output cleared', 'system');
        }
        
        function toggleAutoScroll() {
            autoScroll = !autoScroll;
            document.getElementById('autoscroll-status').textContent = autoScroll ? 'ON' : 'OFF';
            addMessage(`📜 Auto-scroll ${autoScroll ? 'enabled' : 'disabled'}`, 'system');
        }
        
        function downloadLogs() {
            const blob = new Blob([logs.join('\n')], { type: 'text/plain' });
            const url = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = 'esp32_serial_logs_' + new Date().toISOString().slice(0,19).replace(/:/g, '-') + '.txt';
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            window.URL.revokeObjectURL(url);
            addMessage('💾 Logs downloaded successfully', 'system');
        }
        
        // Add manual reconnect function
        function manualReconnect() {
            reconnectAttempts = 0;
            if (ws) {
                ws.close();
            }
            addMessage('🔄 Manual reconnection initiated...', 'system');
            setTimeout(connect, 1000);
        }
        
        // OTA Progress Functions
        function showOTAProgress(updateType) {
            const progressBox = document.getElementById('ota-progress');
            const progressText = document.getElementById('ota-progress-text');
            const progressFill = document.getElementById('ota-progress-fill');
            
            progressBox.style.display = 'block';
            progressText.textContent = '0%';
            progressFill.style.width = '0%';
            
            addMessage(`🔄 OTA upload started - updating ${updateType}`, 'system');
        }
        
        function updateOTAProgress(percentage) {
            const progressText = document.getElementById('ota-progress-text');
            const progressFill = document.getElementById('ota-progress-fill');
            
            progressText.textContent = `${percentage.toFixed(1)}%`;
            progressFill.style.width = `${percentage}%`;
        }
        
        function hideOTAProgress(status, errorMessage) {
            const progressBox = document.getElementById('ota-progress');
            
            if (status === 'completed') {
                // Show 100% briefly before hiding
                updateOTAProgress(100);
                addMessage('✅ OTA upload completed successfully', 'system');
                setTimeout(() => {
                    progressBox.style.display = 'none';
                }, 2000);
            } else if (status === 'error') {
                addMessage(`❌ OTA upload failed: ${errorMessage || 'Unknown error'}`, 'error');
                setTimeout(() => {
                    progressBox.style.display = 'none';
                }, 3000);
            }
        }
        
        // Update connection time every second
        setInterval(updateStats, 1000);
        
        // Start connection
        connect();
    </script>
</body>
</html>
)rawliteral";

//* ************************************************************************
//* ************************ WEBSOCKET EVENT HANDLERS *******************
//* ************************************************************************

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                     AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", 
                   client->id(), client->remoteIP().toString().c_str());
      
      // Send welcome message
      client->text("WebSocket connected to ESP32-S3");
      
      // Send buffered messages to the newly connected client
      sendBufferedMessagesToClient(client);
      break;
      
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
      
    case WS_EVT_DATA:
      // Handle incoming data if needed
      break;
      
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

//* ************************************************************************
//* ************************ WEBSOCKET SETUP FUNCTIONS ******************
//* ************************************************************************

void initWebSocket() {
  // Only initialize if WiFi is connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected - skipping WebSocket setup");
    return;
  }

  //! Step 1: Configure WebSocket
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  //! Step 2: Setup web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", html_page);
  });

  //! Step 3: Start web server
  server.begin();
  
  Serial.println("\n=== WebSocket Serial Monitor Ready ==="); // Use Serial here as SerialWS might not be fully ready
  Serial.printf("Web interface available at: http://%s\n", WiFi.localIP().toString().c_str());
  Serial.println("Use browser to view real-time serial output");
}

void initWebServer() {
  initWebSocket(); // WebSocket setup includes web server setup
}

//* ************************************************************************
//* ************************ WEBSOCKET RUNTIME FUNCTIONS ****************
//* ************************************************************************

void handleWebSocket() {
  //! Clean up disconnected clients and send heartbeats
  if (WiFi.status() == WL_CONNECTED) { // Only handle if WiFi is up
    ws.cleanupClients(); // Clean up disconnected clients
    checkConnectionHealth(); // Includes ws.cleanupClients() and additional checks
    sendHeartbeat();
  }
}

void handleWebServer() {
  // AsyncWebServer doesn't require a handler in the loop like ESP8266WebServer
  // WebSocket events are handled by handleWebSocket()
}


void sendToWebSocket(const String& message) {
  //! Send message to all connected WebSocket clients and add to buffer
  addToMessageBuffer(message);
  
  if (webSocketPaused) {
    // If paused, add to paused buffer instead of sending immediately
    addToPausedBuffer(message);
    return;
  }
  
  if (ws.count() > 0) {
    ws.textAll(message);
  }
}

//* ************************************************************************
//* ************************ CUSTOM SERIAL FUNCTIONS ********************
//* ************************************************************************

void SerialWS_print(const String& message) {
  //! Print to both Serial and WebSocket
  Serial.print(message);
  sendToWebSocket(message);
}

void SerialWS_println(const String& message) {
  //! Print line to both Serial and WebSocket
  Serial.println(message);
  sendToWebSocket(message + "\n");
}

void SerialWS_printf(const char* format, ...) {
  //! Printf to both Serial and WebSocket
  va_list args;
  va_start(args, format);
  
  // Calculate required buffer size
  va_list args_copy;
  va_copy(args_copy, args);
  int len = vsnprintf(nullptr, 0, format, args_copy);
  va_end(args_copy);
  
  // Create buffer and format string
  char* buffer = new char[len + 1];
  vsnprintf(buffer, len + 1, format, args);
  va_end(args);
  
  // Send to both Serial and WebSocket
  Serial.print(buffer);
  sendToWebSocket(String(buffer));
  
  delete[] buffer;
}

//* ************************************************************************
//* ************************ CHARACTER OVERLOADS *************************
//* ************************************************************************

void SerialWS_print(char c) {
  Serial.print(c);
  sendToWebSocket(String(c));
}

void SerialWS_println(char c) {
  Serial.println(c);
  sendToWebSocket(String(c) + "\n");
}

void SerialWS_print(const char* str) {
  Serial.print(str);
  sendToWebSocket(String(str));
}

void SerialWS_println(const char* str) {
  Serial.println(str);
  sendToWebSocket(String(str) + "\n");
}

//* ************************************************************************
//* ************************ INTEGER OVERLOADS ****************************
//* ************************************************************************

void SerialWS_print(int value) {
  Serial.print(value);
  sendToWebSocket(String(value));
}

void SerialWS_println(int value) {
  Serial.println(value);
  sendToWebSocket(String(value) + "\n");
}

void SerialWS_print(unsigned int value) {
  Serial.print(value);
  sendToWebSocket(String(value));
}

void SerialWS_println(unsigned int value) {
  Serial.println(value);
  sendToWebSocket(String(value) + "\n");
}

void SerialWS_print(long value) {
  Serial.print(value);
  sendToWebSocket(String(value));
}

void SerialWS_println(long value) {
  Serial.println(value);
  sendToWebSocket(String(value) + "\n");
}

void SerialWS_print(unsigned long value) {
  Serial.print(value);
  sendToWebSocket(String(value));
}

void SerialWS_println(unsigned long value) {
  Serial.println(value);
  sendToWebSocket(String(value) + "\n");
}

//* ************************************************************************
//* ************************ FLOAT OVERLOADS ******************************
//* ************************************************************************

void SerialWS_print(float value) {
  Serial.print(value);
  sendToWebSocket(String(value));
}

void SerialWS_println(float value) {
  Serial.println(value);
  sendToWebSocket(String(value) + "\n");
}

void SerialWS_print(double value) {
  Serial.print(value);
  sendToWebSocket(String(value));
}

void SerialWS_println(double value) {
  Serial.println(value);
  sendToWebSocket(String(value) + "\n");
}

void SerialWS_print(float value, int digits) {
  Serial.print(value, digits);
  sendToWebSocket(String(value, digits));
}

void SerialWS_println(float value, int digits) {
  Serial.println(value, digits);
  sendToWebSocket(String(value, digits) + "\n");
}

void SerialWS_print(double value, int digits) {
  Serial.print(value, digits);
  sendToWebSocket(String(value, digits));
}

void SerialWS_println(double value, int digits) {
  Serial.println(value, digits);
  sendToWebSocket(String(value, digits) + "\n");
}

//* ************************************************************************
//* ************************ BOOLEAN OVERLOADS ****************************
//* ************************************************************************

void SerialWS_print(bool value) {
  Serial.print(value);
  sendToWebSocket(value ? "1" : "0");
}

void SerialWS_println(bool value) {
  Serial.println(value);
  sendToWebSocket((value ? "1" : "0") + String("\n"));
}

//* ************************************************************************
//* ************************ BASE/HEX OVERLOADS ***************************
//* ************************************************************************

void SerialWS_print(int value, int base) {
  Serial.print(value, base);
  sendToWebSocket(String(value, base));
}

void SerialWS_println(int value, int base) {
  Serial.println(value, base);
  sendToWebSocket(String(value, base) + "\n");
}

void SerialWS_print(unsigned int value, int base) {
  Serial.print(value, base);
  sendToWebSocket(String(value, base));
}

void SerialWS_println(unsigned int value, int base) {
  Serial.println(value, base);
  sendToWebSocket(String(value, base) + "\n");
}

void SerialWS_print(long value, int base) {
  Serial.print(value, base);
  sendToWebSocket(String(value, base));
}

void SerialWS_println(long value, int base) {
  Serial.println(value, base);
  sendToWebSocket(String(value, base) + "\n");
}

void SerialWS_print(unsigned long value, int base) {
  Serial.print(value, base);
  sendToWebSocket(String(value, base));
}

void SerialWS_println(unsigned long value, int base) {
  Serial.println(value, base);
  sendToWebSocket(String(value, base) + "\n");
}

//* ************************************************************************
//* ************************ MESSAGE BUFFER FUNCTIONS ********************
//* ************************************************************************

void addToMessageBuffer(const String& message) {
  //! Add message to circular buffer for reconnecting clients
  messageBuffer[bufferIndex] = message;
  bufferIndex = (bufferIndex + 1) % MESSAGE_BUFFER_SIZE;
  
  if (!bufferFull && bufferIndex == 0) {
    bufferFull = true; // Buffer just became full
  }
  
  if (!bufferFull) {
    bufferCount++;
  } else {
    // If buffer is full, count remains MESSAGE_BUFFER_SIZE
    bufferCount = MESSAGE_BUFFER_SIZE;
  }
}

void sendBufferedMessagesToClient(AsyncWebSocketClient *client) {
  //! Send buffered messages to newly connected client
  if (client == nullptr || !client->canSend()) {
    return;
  }
  
  // Send a reconnection notice first
  client->text("=== RECONNECTED - Message History ===");
  
  int startIndex;
  int messagesToSend;
  
  if (bufferFull) {
    startIndex = bufferIndex; // Start from the oldest message (current bufferIndex if full)
    messagesToSend = MESSAGE_BUFFER_SIZE;
  } else {
    startIndex = 0;
    messagesToSend = bufferCount;
  }
  
  // Send buffered messages in chronological order
  for (int i = 0; i < messagesToSend; i++) {
    int msgIndex = (startIndex + i) % MESSAGE_BUFFER_SIZE;
    if (messageBuffer[msgIndex].length() > 0) {
      client->text(messageBuffer[msgIndex]);
      delay(10); // Small delay to prevent overwhelming the client
    }
  }
  
  // Send end of history marker
  client->text("=== End of Message History - Live Feed Resumes ===");
}

//* ************************************************************************
//* ************************ HEARTBEAT FUNCTIONS *************************
//* ************************************************************************

void sendHeartbeat() {
  //! Send heartbeat to all connected WebSocket clients
  if (ws.count() > 0 && millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    // Send ping to all connected clients
    for (size_t i = 0; i < ws.count(); i++) {
      AsyncWebSocketClient *client = ws.client(i);
      if (client && client->canSend()) {
         // PINGs are automatically handled by the library to check for PONGs
        client->ping();
      }
    }
    lastHeartbeat = millis();
  }
}

void checkConnectionHealth() {
  //! Monitor connection health and clean up stale connections
  if (millis() - lastClientCheck > 10000) { // Check every 10 seconds
    ws.cleanupClients(); // This will close clients that haven't responded to pings
    lastClientCheck = millis();
  }
}

//* ************************************************************************
//* ************************ OTA STATUS FUNCTIONS ************************
//* ************************************************************************

void sendOTAStartEvent(const String& updateType) {
  //! Send OTA start event as JSON to WebSocket clients
  if (ws.count() > 0) {
    String startJson = "{\"type\":\"ota_start\",\"update_type\":\"" + updateType + "\"}";
    ws.textAll(startJson);
  }
}

void sendOTAProgress(unsigned int progress, unsigned int total) {
  //! Send OTA upload progress as JSON to WebSocket clients
  if (ws.count() > 0 && total > 0) { // Avoid division by zero
    float percentage = (float)progress / (float)total * 100.0f;
    // Format percentage to one decimal place
    char p_str[10];
    dtostrf(percentage, 4, 1, p_str); // Convert float to string with 1 decimal place
    String progressJson = "{\"type\":\"ota_progress\",\"percentage\":" + String(p_str) + "}";
    ws.textAll(progressJson);
  }
}

void sendOTAEndEvent() {
  //! Send OTA end event (success) as JSON to WebSocket clients
  if (ws.count() > 0) {
    String endJson = "{\"type\":\"ota_end\",\"status\":\"completed\"}";
    ws.textAll(endJson);
  }
}

void sendOTAErrorEvent(const String& errorMessage) {
  //! Send OTA error event as JSON to WebSocket clients
  if (ws.count() > 0) {
    String errorJson = "{\"type\":\"ota_end\",\"status\":\"error\",\"message\":\"" + errorMessage + "\"}";
    ws.textAll(errorJson);
  }
}


//* ************************************************************************
//* ************************ MOTOR MOVEMENT FUNCTIONS ********************
//* ************************************************************************

void pauseWebSocketTransmission() {
  //! Pause WebSocket transmission during motor movements to prevent missed steps
  webSocketPaused = true;
  Serial.println("WebSocket transmission paused for motor movement"); // Log to physical serial
}

void resumeWebSocketTransmission() {
  //! Resume WebSocket transmission and send any buffered messages
  webSocketPaused = false;
  Serial.println("WebSocket transmission resumed"); // Log to physical serial
  
  // Send any messages that were buffered during pause
  if (ws.count() > 0 && pausedBufferCount > 0) {
    Serial.printf("Sending %d buffered messages from motor pause period\n", pausedBufferCount);
    
    int startIndex = pausedBufferFull ? pausedBufferIndex : 0;
    int messagesToSend = pausedBufferFull ? 50 : pausedBufferCount;
    
    for (int i = 0; i < messagesToSend; i++) {
      int msgIndex = (startIndex + i) % 50;
      if (pausedMessageBuffer[msgIndex].length() > 0) {
        ws.textAll(pausedMessageBuffer[msgIndex]); // Send to WebSocket
        //addToMessageBuffer(pausedMessageBuffer[msgIndex]); // Already added by SerialWS_ functions
        delay(5); // Small delay to prevent overwhelming clients
      }
    }
  }
  
  // Clear paused buffer
  pausedBufferIndex = 0;
  pausedBufferCount = 0;
  pausedBufferFull = false;
  for (int i = 0; i < 50; i++) {
    pausedMessageBuffer[i] = "";
  }
}

bool isWebSocketPaused() {
  //! Check if WebSocket transmission is currently paused
  return webSocketPaused;
}

void addToPausedBuffer(const String& message) {
  //! Add message to paused buffer during motor movements
  // This buffer stores messages that SerialWS_ functions attempt to send while paused.
  // These messages are already in the main messageBuffer.
  // This separate buffer is just to track what to flush upon resume.
  pausedMessageBuffer[pausedBufferIndex] = message;
  pausedBufferIndex = (pausedBufferIndex + 1) % 50; // 50 is the size of pausedMessageBuffer
  
  if (!pausedBufferFull && pausedBufferIndex == 0) {
    pausedBufferFull = true;
  }
  
  if (!pausedBufferFull) {
    pausedBufferCount++;
  } else {
      pausedBufferCount = 50; // Max count is the buffer size
  }
} 
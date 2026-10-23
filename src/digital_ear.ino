#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Sound_detection_inferencing.h>
#include <driver/i2s.h>

const char* ssid     = "Vivo Y400";
const char* password = "012345678";

#define I2S_WS   4
#define I2S_SD   6
#define I2S_SCK  5
#define I2S_PORT I2S_NUM_0

AsyncWebServer server(80);

SemaphoreHandle_t dataMutex;
String  g_label = "starting";
float   g_scores[2] = {0, 0};
const char* CLASS_NAMES[2] = {"anomaly", "normal"};

void i2s_init() {
  i2s_config_t cfg = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = 16000,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 8,
    .dma_buf_len          = 512,
    .use_apll             = false,
    .tx_desc_auto_clear   = false,
    .fixed_mclk           = 0
  };
  i2s_pin_config_t pins = {
    .bck_io_num   = I2S_SCK,
    .ws_io_num    = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = I2S_SD
  };
  i2s_driver_install(I2S_PORT, &cfg, 0, NULL);
  i2s_set_pin(I2S_PORT, &pins);
}

void capture_audio(float* buf, size_t n) {
  int16_t raw[512];
  size_t bytes_read, idx = 0;
  while (idx < n) {
    size_t to_read = min((size_t)512, n - idx);
    i2s_read(I2S_PORT, raw, to_read * 2, &bytes_read, portMAX_DELAY);
    size_t got = bytes_read / 2;
 if (got == 0) continue; // stall guard
    
    for (size_t i = 0; i < got && idx < n; i++, idx++)
      buf[idx] = (float)raw[i] / 32768.0f;
  }
}

void classifierTask(void* param) {
  static float buf[EI_CLASSIFIER_RAW_SAMPLE_COUNT];
  for (;;) {
    capture_audio(buf, EI_CLASSIFIER_RAW_SAMPLE_COUNT);
   
float sum = 0;
int16_t mn = 32767, mx = -32768;
for (int i = 0; i < EI_CLASSIFIER_RAW_SAMPLE_COUNT; i++) {
  sum += fabsf(buf[i]);
  int16_t s = (int16_t)(buf[i] * 32768.0f);
  if (s < mn) mn = s;
  if (s > mx) mx = s;
}
Serial.printf("Energy: %.5f | min: %d max: %d\n", 
  sum / EI_CLASSIFIER_RAW_SAMPLE_COUNT, mn, mx);
    signal_t signal;
    numpy::signal_from_buffer(buf, EI_CLASSIFIER_RAW_SAMPLE_COUNT, &signal);

    ei_impulse_result_t result;
    if (run_classifier(&signal, &result, false) != EI_IMPULSE_OK) continue;

    float best = 0;
    int   best_idx = 0;
    float tmp_scores[2];
    for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
      tmp_scores[i] = result.classification[i].value;
      if (tmp_scores[i] > best) { best = tmp_scores[i]; best_idx = i; }
    }
    String tmp_label = (best < 0.55f) ? "uncertain" : String(CLASS_NAMES[best_idx]);

    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
      g_label = tmp_label;
      for (int i = 0; i < 2; i++) g_scores[i] = tmp_scores[i];
      xSemaphoreGive(dataMutex);
    }

    Serial.println("-> " + tmp_label + " (" + String(best * 100, 1) + "%)");
  }
}

const char HTML[] PROGMEM = R"html(
<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Digital Ear</title>
<style>
*{margin:0;padding:0;box-sizing:border-box;}
body{font-family:'Segoe UI',sans-serif;background:#0f1117;color:#e0e0e0;min-height:100vh;}
header{background:#1a1d27;padding:16px 24px;border-bottom:1px solid #2a2d3a;display:flex;align-items:center;gap:12px;}
header h1{font-size:20px;font-weight:600;color:#fff;}
.dot{width:10px;height:10px;border-radius:50%;background:#22c55e;animation:pulse 1.2s infinite;}
@keyframes pulse{0%,100%{opacity:1;}50%{opacity:0.3;}}
.container{padding:24px;max-width:600px;margin:auto;}
.card{background:#1a1d27;border:1px solid #2a2d3a;border-radius:12px;padding:20px;margin-bottom:16px;}
.label-big{font-size:42px;font-weight:800;text-align:center;padding:12px 0;}
.anomaly{color:#ef4444;}
.normal{color:#22c55e;}
.starting{color:#94a3b8;}
.uncertain{color:#94a3b8;}
.alert{display:none;background:#450a0a;border:1px solid #ef4444;border-radius:10px;padding:16px;color:#fca5a5;font-weight:700;font-size:18px;text-align:center;margin-bottom:16px;animation:blink 0.8s infinite;}
.alert.show{display:block;}
@keyframes blink{0%,100%{opacity:1;}50%{opacity:0.5;}}
.bar-row{display:flex;align-items:center;gap:10px;margin-bottom:12px;}
.bar-label{width:130px;font-size:13px;color:#94a3b8;text-align:right;}
.bar-track{flex:1;height:12px;background:#2a2d3a;border-radius:6px;overflow:hidden;}
.bar-fill{height:100%;border-radius:6px;transition:width 0.5s ease;}
.bar-pct{width:44px;font-size:13px;}
.sub{text-align:center;font-size:13px;color:#94a3b8;margin-top:4px;}
.ts{font-size:11px;color:#475569;text-align:center;margin-top:10px;}
</style></head><body>
<header><div class="dot"></div><h1>🎧 Digital Ear — Live Monitor</h1></header>
<div class="container">
<div class="alert" id="alertBox">⚠️ ANOMALY DETECTED!</div>
<div class="card">
  <div class="sub">Detected sound</div>
  <div class="label-big starting" id="mainLabel">STARTING...</div>
  <div class="sub" id="mainScore"></div>
  <div class="ts" id="ts">Waiting...</div>
</div>
<div class="card">
  <div style="font-size:13px;color:#94a3b8;margin-bottom:16px;">Confidence scores</div>
  <div class="bar-row"><div class="bar-label">Anomaly</div><div class="bar-track"><div class="bar-fill" id="b0" style="background:#ef4444;width:0%"></div></div><div class="bar-pct" id="p0">0%</div></div>
  <div class="bar-row"><div class="bar-label">Normal</div><div class="bar-track"><div class="bar-fill" id="b1" style="background:#22c55e;width:0%"></div></div><div class="bar-pct" id="p1">0%</div></div>
</div>
<div style="font-size:11px;color:#475569;text-align:center;">ESP32-S3 · Edge Impulse · Digital Ear</div>
</div>
<script>
function update(){
  fetch('/data').then(r=>r.json()).then(d=>{
    const lbl=document.getElementById('mainLabel');
    lbl.textContent=d.label.toUpperCase().replace('_',' ');
    lbl.className='label-big '+d.label;
    document.getElementById('mainScore').textContent=(d.score*100).toFixed(1)+'% confidence';
    document.getElementById('ts').textContent='Updated: '+new Date().toLocaleTimeString();
    d.label==='anomaly'
      ? document.getElementById('alertBox').classList.add('show')
      : document.getElementById('alertBox').classList.remove('show');
    d.scores.forEach((s,i)=>{
      const p=(s*100).toFixed(1);
      document.getElementById('b'+i).style.width=p+'%';
      document.getElementById('p'+i).textContent=p+'%';
    });
  }).catch(()=>{});
}
setInterval(update, 800);
update();
</script>
</body></html>
)html";

String buildJSON() {
  String lbl;
  float s0, s1;

  if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
    lbl = g_label;
    s0  = g_scores[0];
    s1  = g_scores[1];
    xSemaphoreGive(dataMutex);
  } else {
    return "{\"label\":\"starting\",\"score\":0,\"scores\":[0,0]}";
  }

  float best = max(s0, s1);
  return "{\"label\":\"" + lbl + "\",\"score\":" + String(best, 3) +
         ",\"scores\":[" + String(s0, 3) + "," + String(s1, 3) + "]}";
}

void setup() {
  Serial.begin(115200);
  dataMutex = xSemaphoreCreateMutex();
  i2s_init();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConnected! Open: http://" + WiFi.localIP().toString());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", HTML);
  });
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "application/json", buildJSON());
  });
  server.begin();
  Serial.println("HTTP server started");
int16_t test_buf[64];
size_t bytes_read = 0;
i2s_read(I2S_PORT, test_buf, sizeof(test_buf), &bytes_read, pdMS_TO_TICKS(1000));
Serial.printf("I2S test: bytes_read=%d, first=%d\n", bytes_read, test_buf[0]);
  xTaskCreatePinnedToCore(classifierTask, "classifier", 32768, NULL, 1, NULL, 1);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}


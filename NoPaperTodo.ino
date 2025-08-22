#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <M5Unified.h>
#include <M5GFX.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <vector>

// 配置文件管理
std::vector<String> g_config_files;
String g_current_config_file = "/config.json";  // 默认配置文件

// 配置结构体
struct WiFiNetwork {
  String ssid;
  String password;
};

struct NotionConfig {
  String api_key;
  String database_id;
  String status_complete_id;
  String status_property;
  String title_property;
  String status_todo;
  String status_progress;
  String status_update_property;
};

struct DisplayConfig {
  int max_items;
  unsigned long sleep_timeout;
  int line_height;
};

struct Config {
  std::vector<WiFiNetwork> wifi_networks;
  NotionConfig notion;
  DisplayConfig display;
};

// 全局配置对象
Config g_config;

// SSL证书
const char* rootCACertificate = 
"-----BEGIN CERTIFICATE-----\n"
"MIIFVzCCAz+gAwIBAgINAgPlk28xsBNJiGuiFzANBgkqhkiG9w0BAQwFADBHMQsw\n"
"CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU\n"
"MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAw\n"
"MDAwWjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp\n"
"Y2VzIExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwggIiMA0GCSqGSIb3DQEBAQUA\n"
"A4ICDwAwggIKAoICAQC2EQKLHuOhd5s73L+UPreVp0A8of2C+X0yBoJx9vaMf/vo\n"
"27xqLpeXo4xL+Sv2sfnOhB2x+cWX3u+58qPpvBKJXqeqUqv4IyfLpLGcY9vXmX7w\n"
"Cl7raKb0xlpHDU0QM+NOsROjyBhsS+z8CZDfnWQpJSMHobTSPS5g4M/SCYe7zUjw\n"
"TcLCeoiKu7rPWRnWr4+wB7CeMfGCwcDfLqZtbBkOtdh+JhpFAz2weaSUKK0Pfybl\n"
"qAj+lug8aJRT7oM6iCsVlgmy4HqMLnXWnOunVmSPlk9orj2XwoSPwLxAwAtcvfaH\n"
"szVsrBhQf4TgTM2S0yDpM7xSma8ytSmzJSq0SPly4cpk9+aCEI3oncKKiPo4Zor8\n"
"Y/kB+Xj9e1x3+naH+uzfsQ55lVe0vSbv1gHR6xYKu44LtcXFilWr06zqkUspzBmk\n"
"MiVOKvFlRNACzqrOSbTqn3yDsEB750Orp2yjj32JgfpMpf/VjsPOS+C12LOORc92\n"
"wO1AK/1TD7Cn1TsNsYqiA94xrcx36m97PtbfkSIS5r762DL8EGMUUXLeXdYWk70p\n"
"aDPvOmbsB4om3xPXV2V4J95eSRQAogB/mqghtqmxlbCluQ0WEdrHbEg8QOB+DVrN\n"
"VjzRlwW5y0vtOUucxD/SVRNuJLDWcfr0wbrM7Rv1/oFB2ACYPTrIrnqYNxgFlQID\n"
"AQABo0IwQDAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4E\n"
"FgQU5K8rJnEaK0gnhS9SZizv8IkTcT4wDQYJKoZIhvcNAQEMBQADggIBAJ+qQibb\n"
"C5u+/x6Wki4+omVKapi6Ist9wTrYggoGxval3sBOh2Z5ofmmWJyq+bXmYOfg6LEe\n"
"QkEzCzc9zolwFcq1JKjPa7XSQCGYzyI0zzvFIoTgxQ6KfF2I5DUkzps+GlQebtuy\n"
"h6f88/qBVRRiClmpIgUxPoLW7ttXNLwzldMXG+gnoot7TiYaelpkttGsN/H9oPM4\n"
"7HLwEXWdyzRSjeZ2axfG34arJ45JK3VmgRAhpuo+9K4l/3wV3s6MJT/KYnAK9y8J\n"
"ZgfIPxz88NtFMN9iiMG1D53Dn0reWVlHxYciNuaCp+0KueIHoI17eko8cdLiA6Ef\n"
"MgfdG+RCzgwARWGAtQsgWSl4vflVy2PFPEz0tv/bal8xa5meLMFrUKTX5hgUvYU/\n"
"Z6tGn6D/Qqc6f1zLXbBwHSs09dR2CQzreExZBfMzQsNhFRAbd03OIozUhfJFfbdT\n"
"6u9AWpQKXCBfTkBdYiJ23//OYb2MI3jSNwLgjt7RETeJ9r/tSQdirpLsQBqvFAnZ\n"
"0E6yove+7u7Y/9waLd64NnHi/Hm3lCXRSHNboTXns5lndcEZOitHTtNCjv0xyBZm\n"
"2tIMPNuzjsmhDYAPexZ3FL//2wmUspO8IFgV6dtxQ/PeEMMA3KgqlbbC1j+Qa3bb\n"
"bP6MvPJwNQzcmRk13NfIRmPVNnGuV/u3gm3c\n"
"-----END CERTIFICATE-----\n";

// SD卡配置 (Paper S3 专用引脚)
#define SD_SPI_CS_PIN   47
#define SD_SPI_SCK_PIN  39
#define SD_SPI_MOSI_PIN 38
#define SD_SPI_MISO_PIN 40

// 字体文件路径
const char* FONT_FILE_PATH = "/CnFont.vlw";

// 布局常量（将从配置文件加载的会被覆盖）
const int START_X = 30;
const int START_Y = 120;
const int STATUS_LABEL_Y = 900;

// 全局变量
struct TodoItem {
  String title;
  String id;
  int y_start;
  int y_end;
};

// LGFX按钮组件
LGFX_Button g_confirm_btn;
LGFX_Button g_cancel_btn;
LGFX_Button g_config_switch_btn;

// 确认弹框状态
struct ConfirmDialog {
  bool show = false;
  TodoItem* task = nullptr;
  String task_title = "";
};

// 配置选择界面状态
struct ConfigSelectionDialog {
  bool show = false;
  int selected_index = 0;
};

// Server模式状态
struct ServerMode {
  bool active = false;
  unsigned long start_time = 0;
  String server_ip = "";
};

std::vector<TodoItem> g_todo_items;
unsigned long g_last_touch_time = 0;
String g_status_text = "";
ConfirmDialog g_confirm_dialog;
ConfigSelectionDialog g_config_selection_dialog;
ServerMode g_server_mode;
WebServer g_web_server(80);
bool g_touch_point_valid = false;
m5::touch_detail_t g_last_touch_point;

// 函数声明
void updateStatusLabel(String text);
bool initializeSDCard();
bool loadChineseFont();
bool displaySplashScreen();
bool scanConfigFiles();
bool loadConfig(String config_file = "");
bool connectWifi();
bool testInternetConnection();
bool fetchNotionTodos();
bool completeNotionTask(String page_id);
void redrawTodoScreen();
void enterDeepSleep();
void showConfirmDialog(TodoItem* task);
void hideConfirmDialog();
void showConfigSelectionDialog();
void hideConfigSelectionDialog();
void switchToConfig(int config_index);
void startServerMode();
void stopServerMode();
void showServerModeScreen();
void setupWebServerRoutes();
void handleRoot();
void handleUpload();
void handleFileUpload();
void handleListConfigs();
void handleEditConfig();
void handleSaveConfig();
void handleDeleteConfig();
void handleRenameConfig();
void initializeButtons();

void setup() {
  Serial.begin(115200);
  Serial.println("Starting M5Paper S3 Notion Todo...");
  
  // 初始化M5设备
  auto cfg = M5.config();
  M5.begin(cfg);
  
  // 设置显示屏
  M5.Display.setRotation(2);  // 旋转180度，让挂钩在顶部
  M5.Display.fillScreen(0xFFFFFF);
  M5.Display.setTextColor(0x000000);
  M5.Display.setTextSize(1);
  
  // 初始化SD卡并显示启动封面
  bool sd_initialized = initializeSDCard();
  if (sd_initialized) {
    displaySplashScreen();  // 显示启动封面
  }
  
  updateStatusLabel("Booting...");

  
  // 处理SD卡后的初始化
  if (sd_initialized) {
    updateStatusLabel("SD Card OK");
    
    // 扫描配置文件
    scanConfigFiles();
    
    // 加载配置文件
    if (loadConfig()) {
      updateStatusLabel("Config Loaded");
    } else {
      updateStatusLabel("Config load failed!");
      // 如果配置文件加载失败，程序无法继续运行
      delay(500);
      return;
    }
    
    // 加载中文字体
    if (loadChineseFont()) {
      updateStatusLabel("Chinese Font Loaded");
    } else {
      updateStatusLabel("Font load failed, using default");
    }
  } else {
    updateStatusLabel("SD Card failed!");
    // 如果SD卡失败，程序无法加载配置文件，无法继续
    delay(2000);
    return;
  }
  
  // 连接WiFi
  if (connectWifi()) {
    // 测试网络连接
    updateStatusLabel("Network testing...");
    if (testInternetConnection()) {
      updateStatusLabel("Connected, syncing todos...");
      if (fetchNotionTodos()) {
        redrawTodoScreen();
      } else {
        g_todo_items.clear();
        redrawTodoScreen();
        updateStatusLabel("Sync failed");
      }
    } else {
      updateStatusLabel("Connection failed");
      g_todo_items.clear();
      redrawTodoScreen();
    }
  } else {
    g_todo_items.clear();
    redrawTodoScreen();
    updateStatusLabel("Connection failed");
  }
  
  // 初始化按钮
  initializeButtons();
  
  g_last_touch_time = millis();
  updateStatusLabel("");
  Serial.println("Setup complete.");
}

void loop() {
  unsigned long current_time = millis();
  
  // Server模式下处理HTTP请求
  if (g_server_mode.active) {
    g_web_server.handleClient();
    // 移除频繁的屏幕刷新，只在启动时显示一次界面
  }
  
  // 检查是否需要进入睡眠模式（Server模式下不进入睡眠）
  if (!g_server_mode.active && current_time - g_last_touch_time > g_config.display.sleep_timeout) {
    updateStatusLabel("Sleep mode...");
    redrawTodoScreen();
    delay(2000);  // 显示2秒钟
    enterDeepSleep();
  }
  
  // 确认弹框不再需要自动完成逻辑
  
  // 处理触摸事件
  M5.update();
  auto touch_detail = M5.Touch.getDetail();
  
  if (touch_detail.isPressed()) {
    g_last_touch_time = current_time;
    
    // 检查触摸点是否发生变化
    bool point_changed = false;
    if (!g_touch_point_valid || 
        touch_detail.x != g_last_touch_point.x || 
        touch_detail.y != g_last_touch_point.y) {
      point_changed = true;
      g_last_touch_point = touch_detail;
      g_touch_point_valid = true;
    }
    
    if (point_changed) {
      int touch_x = touch_detail.x;
      int touch_y = touch_detail.y;
      
      Serial.printf("Touch at: %d, %d\n", touch_x, touch_y);
      
      // 检查是否点击了右上角电量区域 (340-540, 30-70)
      if (touch_x >= 340 && touch_x <= 540 && touch_y >= 30 && touch_y <= 70) {
        Serial.println("Battery area touched, entering sleep mode...");
        updateStatusLabel("Manual sleep...");
        delay(1000);
        enterDeepSleep();
      }
      
      if (g_config_selection_dialog.show) {
        // 配置选择对话框模式下，检查点击
        
        // 检查是否点击了Server模式按钮
        if (touch_x >= 70 && touch_x <= 220 && touch_y >= 540 && touch_y <= 580) {
          Serial.println("Server mode button pressed");
          startServerMode();
        }
        // 检查是否点击了配置列表
        else if (touch_x >= 70 && touch_x <= 470 && touch_y >= 280 && touch_y <= 520) {
          // 点击在配置列表区域内
          int clicked_index = (touch_y - 280) / 40;
          if (clicked_index >= 0 && clicked_index < g_config_files.size()) {
            Serial.println("Config selected: " + g_config_files[clicked_index]);
            switchToConfig(clicked_index);
          }
        } else {
          // 点击在对话框外，关闭对话框
          Serial.println("Clicked outside config dialog, closing");
          hideConfigSelectionDialog();
        }
      } else if (g_confirm_dialog.show) {
        // 确认弹框模式下，检查按钮点击
        if (g_confirm_btn.contains(touch_x, touch_y)) {
          Serial.println("Confirm button pressed");
          // 执行完成任务逻辑
          updateStatusLabel("Completing task...");
          
          if (completeNotionTask(g_confirm_dialog.task->id)) {
            updateStatusLabel("Task completed!");
            fetchNotionTodos();
            //redrawTodoScreen();
            delay(500);
            updateStatusLabel("");
          } else {
            updateStatusLabel("Task completion failed");
            delay(500);
            redrawTodoScreen();
          }
          hideConfirmDialog();
        } else if (g_cancel_btn.contains(touch_x, touch_y)) {
          Serial.println("Cancel button pressed");
          hideConfirmDialog();
        }
      } else if (g_server_mode.active) {
        // Server模式下，点击任意位置退出
        Serial.println("Exiting server mode");
        stopServerMode();
      } else {
        // 正常模式下，检查各种点击
        
        // 检查是否点击了配置切换按钮
        if (g_config_files.size() > 1 && 
            touch_x >= 440 && touch_x <= 520 && 
            touch_y >= STATUS_LABEL_Y && touch_y <= STATUS_LABEL_Y + 40) {
          Serial.println("Config switch button pressed");
          showConfigSelectionDialog();
        }
        // 检查是否点击了某个待办事项
        else {
          for (auto& item : g_todo_items) {
            if (item.y_start > 0 && 
                touch_y >= item.y_start && touch_y < item.y_end) {
              Serial.println("Todo item selected: " + item.title);
              showConfirmDialog(&item);
              break;
            }
          }
        }
      }
    }
  } else if (touch_detail.isReleased()) {
    g_touch_point_valid = false;
  }
  
  delay(50);
}

void updateStatusLabel(String text) {
  g_status_text = text;
  M5.Display.fillRect(START_X, STATUS_LABEL_Y, 500, 50, 0xFFFFFF);
  M5.Display.setTextColor(0x000000);
  M5.Display.setTextDatum(top_left); // 确保状态文字使用左对齐
  M5.Display.drawString(g_status_text, START_X, STATUS_LABEL_Y);
  Serial.println("Status: " + text);
}

bool connectWifi() {
  WiFi.mode(WIFI_STA);
  
  if (g_config.wifi_networks.empty()) {
    Serial.println("No WiFi networks configured");
    return false;
  }
  
  // 尝试连接配置文件中的每个WiFi网络
  for (const auto& network : g_config.wifi_networks) {
    updateStatusLabel("Connect WiFi: " + network.ssid + "...");
    Serial.println("Trying to connect to: " + network.ssid);
    
    WiFi.begin(network.ssid.c_str(), network.password.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
      delay(500);
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi connected to: " + network.ssid);
      Serial.println("IP: " + WiFi.localIP().toString());
      return true;
    }
    
    // 断开连接准备尝试下一个网络
    WiFi.disconnect();
    delay(500);
  }
  
  Serial.println("Failed to connect to any configured WiFi network");
  return false;
}

bool fetchNotionTodos() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return false;
  }
  
  g_todo_items.clear();
  
  for (int attempt = 0; attempt <= 3; attempt++) {
    WiFiClientSecure client;
    
    // 尝试不同的SSL配置方法
    client.setInsecure(); // 临时跳过证书验证来测试连接
    // client.setCACert(rootCACertificate);  // 如果上面的方法工作，再启用证书验证
    
    HTTPClient http;
    String url = "https://api.notion.com/v1/databases/" + g_config.notion.database_id + "/query";
    
    Serial.println("Connecting to: " + url);
    
    if (!http.begin(client, url)) {
      Serial.println("HTTP begin failed");
      continue;
    }
    
    // 设置请求头
    http.addHeader("Authorization", "Bearer " + g_config.notion.api_key);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Notion-Version", "2022-06-28");
    http.addHeader("User-Agent", "M5Stack-Arduino/1.0");
    
    // 设置超时
    http.setTimeout(30000);  // 增加超时时间到30秒
    http.setConnectTimeout(10000); // 连接超时10秒
    
    // 发送POST请求
    String payload = "{}";
    Serial.println("Sending POST request...");
    int httpResponseCode = http.POST(payload);
    
    Serial.printf("HTTP Response Code: %d\n", httpResponseCode);
    
    if (httpResponseCode == 200) {
      String response = http.getString();
      Serial.println("Notion API response received");
      
      // 解析JSON响应
      DynamicJsonDocument doc(8192);
      DeserializationError error = deserializeJson(doc, response);
      
      if (!error) {
        JsonArray results = doc["results"];
        
        for (JsonObject item : results) {
          JsonObject properties = item["properties"];
          
          // 检查状态
          if (properties.containsKey(g_config.notion.status_property)) {
            JsonObject status_obj = properties[g_config.notion.status_property];
            if (status_obj.containsKey("status")) {
              String status_name = status_obj["status"]["name"];
              
              if (status_name == g_config.notion.status_todo || status_name == g_config.notion.status_progress) {
                // 获取标题
                if (properties.containsKey(g_config.notion.title_property)) {
                  JsonArray title_array = properties[g_config.notion.title_property]["title"];
                  if (title_array.size() > 0) {
                    String title = title_array[0]["plain_text"];
                    String id = item["id"];
                    
                    // 限制标题长度
                    if (title.length() > 50) {
                      title = title.substring(0, 47) + "...";
                    }
                    
                    TodoItem todo;
                    todo.title = title;
                    todo.id = id;
                    todo.y_start = 0;
                    todo.y_end = 0;
                    
                    g_todo_items.push_back(todo);
                  }
                }
              }
            }
          }
        }
        
        http.end();
        Serial.printf("Successfully fetched %d todos\n", g_todo_items.size());
        return true;
      } else {
        Serial.println("JSON parsing failed: " + String(error.c_str()));
      }
    } else {
      Serial.printf("HTTP request failed with code: %d\n", httpResponseCode);
    }
    
    http.end();
    
    if (attempt < 5) {
      Serial.printf("Retry attempt %d/3\n", attempt + 1);
      delay(500);
    }
  }
  
  return false;
}

bool completeNotionTask(String page_id) {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }
  
  for (int attempt = 0; attempt <= 3; attempt++) {
    WiFiClientSecure client;
    client.setInsecure(); // 临时跳过证书验证
    
    HTTPClient http;
    String url = "https://api.notion.com/v1/pages/" + page_id;
    
    Serial.println("Updating task: " + url);
    
    if (!http.begin(client, url)) {
      Serial.println("HTTP begin failed for task update");
      continue;
    }
    
    http.addHeader("Authorization", "Bearer " + g_config.notion.api_key);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Notion-Version", "2022-06-28");
    http.addHeader("User-Agent", "M5Stack-Arduino/1.0");
    
    // 构造更新状态的JSON
    DynamicJsonDocument doc(512);
    doc["properties"][g_config.notion.status_update_property]["status"]["id"] = g_config.notion.status_complete_id;
    
    String payload;
    serializeJson(doc, payload);
    
    Serial.println("Update payload: " + payload);
    
    http.setTimeout(30000);
    http.setConnectTimeout(10000);
    int httpResponseCode = http.PATCH(payload);
    
    if (httpResponseCode == 200) {
      http.end();
      Serial.println("Task completed successfully");
      return true;
    } else {
      Serial.printf("Failed to complete task, code: %d\n", httpResponseCode);
    }
    
    http.end();
    
    if (attempt < 5) {
      delay(1000);
    }
  }
  
  return false;
}

void redrawTodoScreen() {
  M5.Display.fillScreen(0xFFFFFF);
  M5.Display.setTextColor(0x000000);
  
  // 重置文字对齐方式为默认左上角对齐
  M5.Display.setTextDatum(top_left);
  
  // 绘制标题 - 调整字体大小
  M5.Display.setTextSize(1);  // 从2改为1，字体变小
  M5.Display.drawString("Todo Items", START_X, 40);
  
  // 绘制电池信息 - 完全按照您的示例代码
  try {
    bool isCharging = M5.Power.isCharging();                // 直接使用M5.Power.isCharging()
    int32_t batteryLevel = M5.Power.getBatteryLevel();       // 使用int32_t类型
    
    String battery_text = String(batteryLevel) + "%";
    if (isCharging) {
      battery_text = battery_text+"++";
    } else {
      battery_text = "Battery: " + battery_text;
    }
    
    M5.Display.setTextSize(1);
    M5.Display.drawString(battery_text, 340, 40);
    
    // 调试信息 - 和您的示例一样的格式
    Serial.printf("Battery Charging: %s, Level: %d%%\n", isCharging ? "Yes" : "No", batteryLevel);
    
  } catch (...) {
    Serial.println("Could not draw battery level");
  }
  
  // 绘制分割线
  M5.Display.drawLine(START_X, 100, 540 - START_X, 100, 0x000000);
  
  // 重置状态标签区域
  M5.Display.fillRect(START_X, STATUS_LABEL_Y, 500, 50, 0xFFFFFF);
  
  // 清除所有任务项的坐标信息
  for (auto& item : g_todo_items) {
    item.y_start = 0;
    item.y_end = 0;
  }
  
  // 显示待办事项
  M5.Display.setTextSize(1);
  
  if (g_todo_items.empty()) {
    M5.Display.drawString("- No pending todos -", START_X, START_Y);
  } else {
    int max_display = min((int)g_todo_items.size(), g_config.display.max_items);
    for (int i = 0; i < max_display; i++) {
      int y_pos = START_Y + (i * g_config.display.line_height);
      String display_text = " - " + g_todo_items[i].title;
      
      M5.Display.drawString(display_text, START_X, y_pos);
      
      // 保存触摸区域信息
      g_todo_items[i].y_start = y_pos;
      g_todo_items[i].y_end = y_pos + g_config.display.line_height;
    }
  }
  
  // 恢复状态文本
  if (g_status_text.length() > 0) {
    M5.Display.drawString(g_status_text, START_X, STATUS_LABEL_Y);
  }
  
  // 绘制配置切换按钮（仅在有多个配置文件时显示）
  if (g_config_files.size() > 1) {
    // 绘制配置按钮在右下角
    M5.Display.drawRoundRect(400, STATUS_LABEL_Y, 120, 40, 8, 0x000000);
    
    M5.Display.setTextColor(0x000000);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(middle_center);
    M5.Display.drawString("Config", 460, STATUS_LABEL_Y + 20);
    
    // 恢复文字对齐方式
    M5.Display.setTextDatum(top_left);
  }
}

// SD卡初始化函数 (完全按照您提供的示例代码)
bool initializeSDCard() {
  // SD Card Initialization (严格按照您的示例)
  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    // Print a message if SD card initialization failed or if the SD card does not exist.
    Serial.println("SD card not detected");
    M5.Display.print("SD card not detected");
    return false;
  } else {
    Serial.println("SD card detected");
    M5.Display.print("SD card detected");
    return true;
  }
}

// 中文字体加载函数 (严格按照您提供的示例代码)
bool loadChineseFont() {
  Serial.println("Loading Chinese font...");
  
  // 检查字体文件是否存在 (完全按照您的示例)
  if (SD.open(FONT_FILE_PATH, FILE_READ, false)) {
    Serial.println("vlw detected");
  } else {
    Serial.println("vlw not detected");
    return false;
  }
  
  // 加载字体 (完全按照您的示例方法)
  if (M5.Display.loadFont(SD, FONT_FILE_PATH)) {
    Serial.println("Chinese font loaded successfully");
    return true;
  } else {
    Serial.println("vlw not loaded");
    return false;
  }
}

// 显示启动封面函数
bool displaySplashScreen() {
  Serial.println("Displaying splash screen...");
  
  // 检查cover.png文件是否存在
  if (!SD.exists("/cover.png")) {
    Serial.println("cover.png not found");
    return false;
  }
  
  // 检查文件大小
  File file = SD.open("/cover.png", FILE_READ);
  if (!file) {
    Serial.println("Cannot open cover.png");
    return false;
  }
  
  size_t fileSize = file.size();
  file.close();
  Serial.printf("Image file size: %d bytes\n", fileSize);
  
  // 如果文件太大，跳过显示
  if (fileSize > 500000) {  // 限制为500KB
    Serial.println("Image file too large, skipping");
    return false;
  }
  
  // 临时保存当前旋转设置
  uint8_t current_rotation = M5.Display.getRotation();
  
  // 尝试显示图片，添加异常处理
  bool success = false;
  
  try {
    // 设置旋转为1（顺时针90度）来显示960x540的图片
    M5.Display.setRotation(1);
    
    // 使用M5GFX显示PNG图片
    success = M5.Display.drawPngFile(SD, "/cover.png", 0, 0);
    
  } catch (...) {
    Serial.println("Exception caught while loading image");
    success = false;
  }
  
  // 恢复原来的旋转设置
  M5.Display.setRotation(current_rotation);
  
  if (success) {
    Serial.println("Splash screen displayed successfully");
    return true;
  } else {
    Serial.println("PNG failed, trying BMP format...");
    
    // 尝试BMP格式作为备选
    if (SD.exists("/cover.bmp")) {
      try {
        M5.Display.setRotation(1);
        success = M5.Display.drawBmpFile(SD, "/cover.bmp", 0, 0);
        M5.Display.setRotation(current_rotation);
        
        if (success) {
          Serial.println("BMP splash screen displayed successfully");
          return true;
        }
      } catch (...) {
        Serial.println("BMP loading also failed");
      }
    }
    
    Serial.println("Failed to display splash screen");
    return false;
  }
}

// 网络连接测试函数
bool testInternetConnection() {
  Serial.println("Testing internet connection...");
  
  WiFiClientSecure client;
  client.setInsecure(); // 跳过证书验证进行快速测试
  
  HTTPClient http;
  
  // 测试连接到一个简单的HTTP服务
  if (http.begin(client, "https://httpbin.org/get")) {
    http.setTimeout(10000);
    http.setConnectTimeout(5000);
    http.addHeader("User-Agent", "M5Stack-Test/1.0");
    
    int httpCode = http.GET();
    Serial.printf("Test connection response: %d\n", httpCode);
    
    http.end();
    
    if (httpCode > 0) {
      Serial.println("Internet connection OK");
      return true;
    } else {
      Serial.println("Internet connection failed");
      return false;
    }
  } else {
    Serial.println("HTTP begin failed for test");
    return false;
  }
}

// 扫描SD卡中的配置文件
bool scanConfigFiles() {
  Serial.println("Scanning for config files on SD card...");
  
  g_config_files.clear();
  
  File root = SD.open("/");
  if (!root) {
    Serial.println("Failed to open root directory");
    return false;
  }
  
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String fileName = file.name();
      // 查找所有以.json结尾的文件
      if (fileName.endsWith(".json")) {
        String fullPath = "/" + fileName;
        g_config_files.push_back(fullPath);
        Serial.println("Found config file: " + fullPath);
      }
    }
    file.close();
    file = root.openNextFile();
  }
  root.close();
  
  // 如果没有找到配置文件，添加默认的config.json
  if (g_config_files.empty()) {
    g_config_files.push_back("/config.json");
    Serial.println("No config files found, using default config.json");
  } else {
    // 排序配置文件列表
    std::sort(g_config_files.begin(), g_config_files.end());
  }
  
  Serial.printf("Total config files found: %d\n", g_config_files.size());
  return true;
}

// 加载配置文件
bool loadConfig(String config_file) {
  if (config_file.isEmpty()) {
    config_file = g_current_config_file;
  } else {
    g_current_config_file = config_file;
  }
  Serial.println("Loading configuration from SD card: " + config_file);
  
  if (!SD.exists(config_file)) {
    Serial.println("Config file not found: " + config_file);
    return false;
  }
  
  File configFile = SD.open(config_file, FILE_READ);
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }
  
  size_t fileSize = configFile.size();
  if (fileSize > 2048) { // 限制配置文件大小
    Serial.println("Config file too large");
    configFile.close();
    return false;
  }
  
  String configContent = configFile.readString();
  configFile.close();
  
  Serial.println("Config file content loaded");
  
  // 解析JSON
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, configContent);
  
  if (error) {
    Serial.println("Failed to parse config JSON: " + String(error.c_str()));
    return false;
  }
  
  // 清空现有配置
  g_config.wifi_networks.clear();
  
  // 加载WiFi配置
  if (doc.containsKey("wifi") && doc["wifi"].containsKey("networks")) {
    JsonArray networks = doc["wifi"]["networks"];
    for (JsonObject network : networks) {
      WiFiNetwork wifi;
      wifi.ssid = network["ssid"].as<String>();
      wifi.password = network["password"].as<String>();
      g_config.wifi_networks.push_back(wifi);
      Serial.println("Loaded WiFi: " + wifi.ssid);
    }
  }
  
  // 加载Notion配置
  if (doc.containsKey("notion")) {
    JsonObject notion = doc["notion"];
    g_config.notion.api_key = notion["api_key"].as<String>();
    g_config.notion.database_id = notion["database_id"].as<String>();
    g_config.notion.status_complete_id = notion["status_complete_id"].as<String>();
    
    if (notion.containsKey("properties")) {
      JsonObject props = notion["properties"];
      g_config.notion.status_property = props["status_property"].as<String>();
      g_config.notion.title_property = props["title_property"].as<String>();
      g_config.notion.status_todo = props["status_todo"].as<String>();
      g_config.notion.status_progress = props["status_progress"].as<String>();
      g_config.notion.status_update_property = props["status_update_property"].as<String>();
    }
    
    Serial.println("Loaded Notion config");
  }
  
  // 加载显示配置
  if (doc.containsKey("display")) {
    JsonObject display = doc["display"];
    g_config.display.max_items = display["max_items"] | 10;
    g_config.display.sleep_timeout = display["sleep_timeout"] | 180000;
    g_config.display.line_height = display["line_height"] | 65;
    
    Serial.println("Loaded display config");
  }
  
  Serial.println("Configuration loaded successfully");
  return true;
}

// 进入深度睡眠函数
void enterDeepSleep() {
  Serial.println("Entering deep sleep...");
  
  // 进入深度睡眠
  esp_deep_sleep_start();
}

// 初始化LGFX按钮
void initializeButtons() {
  // 弹框尺寸: 70, 355, 400, 250 (x, y, w, h)
  // 弹框中心: x = 70 + 400/2 = 270
  // 按钮间距: 120像素
  
  // 初始化确认按钮 (x, y, w, h, outline_color, fill_color, text_color, label, text_size_x, text_size_y)
  g_confirm_btn.initButton(&M5.Display, 180, 550, 140, 50, 
                          0x000000, 0xFFFFFF, 0x000000, "Confirm", 1, 1);
  
  // 初始化取消按钮
  g_cancel_btn.initButton(&M5.Display, 360, 550, 140, 50, 
                         0x000000, 0x000000, 0xFFFFFF, "Cancel", 1, 1);
}

// 显示确认弹框
void showConfirmDialog(TodoItem* task) {
  g_confirm_dialog.show = true;
  g_confirm_dialog.task = task;
  g_confirm_dialog.task_title = task->title;
  
  // 限制标题长度
  if (g_confirm_dialog.task_title.length() > 25) {
    g_confirm_dialog.task_title = g_confirm_dialog.task_title.substring(0, 25) + "...";
  }
  
  // 绘制弹框背景（不改变整个屏幕背景）
  M5.Display.fillRoundRect(70, 355, 400, 250, 15, 0xFFFFFF);
  M5.Display.drawRoundRect(70, 355, 400, 250, 15, 0x000000);
  
  // 绘制标题 - 居中显示
  M5.Display.setTextColor(0x000000);
  M5.Display.setTextSize(1);
  M5.Display.setTextDatum(middle_center); // 设置文字居中对齐
  M5.Display.drawString("Confirm task completion?", 270, 380);
  
  // 绘制任务标题 - 居中显示
  M5.Display.drawString(g_confirm_dialog.task_title, 270, 420);
  
  // 绘制按钮
  g_confirm_btn.drawButton();
  g_cancel_btn.drawButton();
  
  Serial.println("Confirm dialog shown for: " + task->title);
}

// 隐藏确认弹框
void hideConfirmDialog() {
  g_confirm_dialog.show = false;
  g_confirm_dialog.task = nullptr;
  g_confirm_dialog.task_title = "";
  
  // 重新绘制整个界面
  redrawTodoScreen();
  
  Serial.println("Confirm dialog hidden");
}

// 显示配置选择对话框
void showConfigSelectionDialog() {
  g_config_selection_dialog.show = true;
  g_config_selection_dialog.selected_index = 0;
  
  // 找到当前配置文件的索引
  for (int i = 0; i < g_config_files.size(); i++) {
    if (g_config_files[i] == g_current_config_file) {
      g_config_selection_dialog.selected_index = i;
      break;
    }
  }
  
  // 绘制配置选择对话框背景
  M5.Display.fillRoundRect(50, 200, 440, 400, 15, 0xFFFFFF);
  M5.Display.drawRoundRect(50, 200, 440, 400, 15, 0x000000);
  
  // 绘制标题
  M5.Display.setTextColor(0x000000);
  M5.Display.setTextSize(1);
  M5.Display.setTextDatum(middle_center);
  M5.Display.drawString("Select Config File", 270, 230);
  
  // 绘制配置文件列表
  M5.Display.setTextDatum(top_left);
  for (int i = 0; i < g_config_files.size() && i < 8; i++) {
    int y_pos = 280 + (i * 40);
    
    // 高亮当前选择的配置
    if (i == g_config_selection_dialog.selected_index) {
      M5.Display.fillRect(70, y_pos - 5, 400, 30, 0xD0D0D0);
    }
    
    // 提取文件名（去掉路径和扩展名）
    String fileName = g_config_files[i];
    fileName.replace("/", "");
    fileName.replace(".json", "");
    
    // 添加当前配置标记
    if (g_config_files[i] == g_current_config_file) {
      fileName += " (Current)";
    }
    
    M5.Display.setTextColor(0x000000);
    M5.Display.drawString(fileName, 80, y_pos);
  }
  
  // 绘制Server模式按钮
  M5.Display.drawRoundRect(70, 540, 200, 40, 8, 0x000000);
  M5.Display.setTextDatum(middle_center);
  M5.Display.drawString("Server Mode", 160, 560);
  
  // 绘制提示信息
  M5.Display.drawString("Click outside to cancel", 270, 710);
  
  Serial.println("Config selection dialog shown");
}

// 隐藏配置选择对话框
void hideConfigSelectionDialog() {
  g_config_selection_dialog.show = false;
  
  // 重新绘制整个界面
  redrawTodoScreen();
  
  Serial.println("Config selection dialog hidden");
}

// 切换到指定配置
void switchToConfig(int config_index) {
  if (config_index < 0 || config_index >= g_config_files.size()) {
    Serial.println("Invalid config index: " + String(config_index));
    return;
  }
  
  String new_config_file = g_config_files[config_index];
  if (new_config_file == g_current_config_file) {
    Serial.println("Already using config: " + new_config_file);
    hideConfigSelectionDialog();
    return;
  }
  
  Serial.println("Switching to config: " + new_config_file);
  updateStatusLabel("Switching config...");
  
  // 加载新配置
  if (loadConfig(new_config_file)) {
    updateStatusLabel("Config switched, syncing...");
    
    // 重新连接WiFi（如果需要）
    if (WiFi.status() != WL_CONNECTED) {
      connectWifi();
    }
    
    // 重新获取待办事项
    if (fetchNotionTodos()) {
      redrawTodoScreen();
      updateStatusLabel("Config switched successfully");
    } else {
      g_todo_items.clear();
      redrawTodoScreen();
      updateStatusLabel("Data sync failed");
    }
    
    // 隐藏对话框
    hideConfigSelectionDialog();
    
    delay(500);
    updateStatusLabel("");
  } else {
    updateStatusLabel("Config load failed");
    delay(500);
    hideConfigSelectionDialog();
  }
}

// 启动Server模式
void startServerMode() {
  if (g_server_mode.active) {
    Serial.println("Server mode already active");
    return;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    updateStatusLabel("WiFi connection required for server");
    delay(500);
    hideConfigSelectionDialog();
    return;
  }
  
  g_server_mode.active = true;
  g_server_mode.start_time = millis();
  g_server_mode.server_ip = WiFi.localIP().toString();
  
  setupWebServerRoutes();
  g_web_server.begin();
  
  Serial.println("Server mode started at: " + g_server_mode.server_ip);
  hideConfigSelectionDialog();
  showServerModeScreen();
}

// 停止Server模式
void stopServerMode() {
  if (!g_server_mode.active) {
    return;
  }
  
  g_web_server.stop();
  g_server_mode.active = false;
  g_server_mode.server_ip = "";
  
  Serial.println("Server mode stopped");
  redrawTodoScreen();
}

// 显示Server模式界面
void showServerModeScreen() {
  M5.Display.fillScreen(0xFFFFFF);
  M5.Display.setTextColor(0x000000);
  M5.Display.setTextSize(1);
  
  // 标题
  M5.Display.setTextDatum(middle_center);
  M5.Display.drawString("Server Mode Running", 270, 100);
  
  // IP地址信息
  M5.Display.drawString("Server Address:", 270, 200);
  M5.Display.drawString("http://" + g_server_mode.server_ip, 270, 240);
  
  // 使用说明
  M5.Display.setTextDatum(top_left);
  M5.Display.drawString("Instructions:", 50, 300);
  M5.Display.drawString("1. Visit the above address", 50, 350);
  M5.Display.drawString("2. Select config file and upload", 50, 400);
  M5.Display.drawString("3. File will be saved to SD card", 50, 450);
  M5.Display.drawString("4. Click anywhere to exit", 50, 500);
  
  // 状态信息（不显示动态运行时间）
  M5.Display.setTextDatum(middle_center);
  M5.Display.drawString("Server running, waiting for upload...", 270, 600);
  
  // 退出提示 - 加大按钮和文字
  M5.Display.drawRoundRect(125, 780, 300, 80, 15, 0x000000);
  M5.Display.setTextSize(1);
  M5.Display.drawString("Exit Server Mode", 270, 840);
}

// 设置Web服务器路由
void setupWebServerRoutes() {
  g_web_server.on("/", handleRoot);
  g_web_server.on("/upload", HTTP_POST, handleUpload, handleFileUpload);
  g_web_server.on("/configs", handleListConfigs);
  g_web_server.on("/edit", handleEditConfig);
  g_web_server.on("/save", HTTP_POST, handleSaveConfig);
  g_web_server.on("/delete", HTTP_POST, handleDeleteConfig);
  g_web_server.on("/rename", HTTP_POST, handleRenameConfig);
  
  g_web_server.onNotFound([]() {
    g_web_server.send(404, "text/plain", "File not found");
  });
}

// 处理根路径请求，显示上传页面
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>M5Stack Config File Management</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; padding: 20px; background-color: #f5f5f5; }";
  html += ".container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); margin-bottom: 20px; }";
  html += "h1 { color: #333; text-align: center; margin-bottom: 30px; }";
  html += "h2 { color: #555; border-bottom: 2px solid #007bff; padding-bottom: 10px; }";
  html += ".upload-area { border: 2px dashed #ccc; border-radius: 10px; padding: 40px; text-align: center; margin: 20px 0; }";
  html += ".upload-area:hover { border-color: #007bff; background-color: #f8f9fa; }";
  html += "input[type='file'] { margin: 20px 0; padding: 10px; width: 100%; border: 1px solid #ddd; border-radius: 5px; }";
  html += "input[type='submit'], .btn { background-color: #007bff; color: white; padding: 12px 20px; border: none; border-radius: 5px; cursor: pointer; font-size: 14px; text-decoration: none; display: inline-block; margin: 5px; }";
  html += "input[type='submit']:hover, .btn:hover { background-color: #0056b3; }";
  html += ".btn-danger { background-color: #dc3545; } .btn-danger:hover { background-color: #c82333; }";
  html += ".btn-warning { background-color: #ffc107; color: #212529; } .btn-warning:hover { background-color: #e0a800; }";
  html += ".info { background-color: #e7f3ff; padding: 15px; border-radius: 5px; margin: 20px 0; border-left: 4px solid #007bff; }";
  html += ".config-item { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 15px; margin: 10px 0; }";
  html += ".config-name { font-weight: bold; font-size: 16px; margin-bottom: 10px; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>🔧 M5Stack Config File Management</h1>";
  
  // 上传新文件部分
  html += "<h2>📤 Upload New Config File</h2>";
  html += "<div class='info'>";
  html += "<strong>Instructions:</strong><br>";
  html += "• Upload .json config files<br>";
  html += "• Files will be saved to SD card root<br>";
  html += "• Any .json filename is supported";
  html += "</div>";
  html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
  html += "<div class='upload-area'>";
  html += "<p>📁 Select config file to upload</p>";
  html += "<input type='file' name='configfile' accept='.json' required>";
  html += "</div>";
  html += "<input type='submit' value='🚀 Upload File'>";
  html += "</form>";
  html += "</div>";
  
  // 现有配置文件管理部分
  html += "<div class='container'>";
  html += "<h2>📋 Existing Config Files</h2>";
  html += "<a href='/configs' class='btn'>🔄 Refresh List</a>";
  html += "</div>";
  
  html += "</body></html>";
  
  g_web_server.send(200, "text/html", html);
}

// 处理上传完成
void handleUpload() {
  g_web_server.send(200, "text/html", 
    "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Upload Success</title>"
    "<style>body{font-family:Arial;max-width:600px;margin:50px auto;padding:20px;text-align:center;}"
    ".success{background:#d4edda;color:#155724;padding:20px;border-radius:10px;margin:20px 0;}"
    "a{display:inline-block;background:#007bff;color:white;padding:10px 20px;text-decoration:none;border-radius:5px;margin-top:20px;}"
    "</style></head><body>"
    "<div class='success'><h2>✅ File uploaded successfully!</h2><p>Config file saved to SD card</p></div>"
    "<a href='/'>Return to upload page</a></body></html>");
}

// 处理文件上传
void handleFileUpload() {
  HTTPUpload& upload = g_web_server.upload();
  static File uploadFile;
  
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    Serial.println("Upload start: " + filename);
    
    // 验证文件名
    if (!filename.endsWith(".json")) {
      Serial.println("Invalid filename: " + filename);
      return;
    }
    
    // 确保文件名以 "/" 开头
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    
    Serial.println("Saving to: " + filename);
    uploadFile = SD.open(filename, FILE_WRITE);
    if (!uploadFile) {
      Serial.println("Failed to open file for writing");
      return;
    }
  } 
  else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
  } 
  else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
      Serial.println("Upload complete: " + String(upload.totalSize) + " bytes");
      
      // 重新扫描配置文件
      scanConfigFiles();
    } else {
      Serial.println("Upload failed");
    }
  }
}

// 列出现有配置文件
void handleListConfigs() {
  // 重新扫描配置文件
  scanConfigFiles();
  
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Config File List</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; padding: 20px; background-color: #f5f5f5; }";
  html += ".container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += "h1 { color: #333; text-align: center; margin-bottom: 30px; }";
  html += ".config-item { background-color: #f8f9fa; border: 1px solid #dee2e6; border-radius: 5px; padding: 15px; margin: 10px 0; }";
  html += ".config-name { font-weight: bold; font-size: 16px; margin-bottom: 10px; }";
  html += ".btn { background-color: #007bff; color: white; padding: 8px 15px; border: none; border-radius: 3px; cursor: pointer; font-size: 12px; text-decoration: none; display: inline-block; margin: 2px; }";
  html += ".btn:hover { background-color: #0056b3; }";
  html += ".btn-danger { background-color: #dc3545; } .btn-danger:hover { background-color: #c82333; }";
  html += ".btn-warning { background-color: #ffc107; color: #212529; } .btn-warning:hover { background-color: #e0a800; }";
  html += ".btn-success { background-color: #28a745; } .btn-success:hover { background-color: #218838; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>📋 Config File List</h1>";
  html += "<a href='/' class='btn'>🏠 Back to Home</a>";
  
  if (g_config_files.empty()) {
    html += "<p>No config files found</p>";
  } else {
    for (const String& configFile : g_config_files) {
      String fileName = configFile;
      fileName.replace("/", "");
      
      html += "<div class='config-item'>";
      html += "<div class='config-name'>" + fileName;
      if (configFile == g_current_config_file) {
        html += " <span style='color: #28a745;'>(Currently Active)</span>";
      }
      html += "</div>";
      html += "<a href='/edit?file=" + configFile + "' class='btn'>✏️ Edit</a>";
      html += "<button onclick='renameFile(\"" + configFile + "\")' class='btn btn-warning'>📝 Rename</button>";
      html += "<button onclick='deleteFile(\"" + configFile + "\")' class='btn btn-danger'>🗑️ Delete</button>";
      html += "</div>";
    }
  }
  
  html += "</div>";
  html += "<script>";
  html += "function deleteFile(file) {";
  html += "  if (confirm('Are you sure you want to delete config file ' + file.replace('/', '') + '?')) {";
  html += "    fetch('/delete', { method: 'POST', headers: {'Content-Type': 'application/x-www-form-urlencoded'}, body: 'file=' + encodeURIComponent(file) })";
  html += "    .then(() => location.reload());";
  html += "  }";
  html += "}";
  html += "function renameFile(file) {";
  html += "  var newName = prompt('Enter new filename (without .json extension):', file.replace('/', '').replace('.json', ''));";
  html += "  if (newName && newName.trim() !== '') {";
  html += "    if (!newName.endsWith('.json')) newName += '.json';";
  html += "    fetch('/rename', { method: 'POST', headers: {'Content-Type': 'application/x-www-form-urlencoded'}, body: 'oldfile=' + encodeURIComponent(file) + '&newfile=' + encodeURIComponent('/' + newName) })";
  html += "    .then(() => location.reload());";
  html += "  }";
  html += "}";
  html += "</script>";
  html += "</body></html>";
  
  g_web_server.send(200, "text/html", html);
}

// 编辑配置文件
void handleEditConfig() {
  String filename = g_web_server.arg("file");
  if (filename.isEmpty()) {
    g_web_server.send(400, "text/plain", "Missing file parameter");
    return;
  }
  
  if (!SD.exists(filename)) {
    g_web_server.send(404, "text/plain", "File not found");
    return;
  }
  
  File file = SD.open(filename, FILE_READ);
  if (!file) {
    g_web_server.send(500, "text/plain", "Cannot open file");
    return;
  }
  
  String content = file.readString();
  file.close();
  
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Edit Config File</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; max-width: 900px; margin: 20px auto; padding: 20px; background-color: #f5f5f5; }";
  html += ".container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += "h1 { color: #333; text-align: center; margin-bottom: 30px; }";
  html += "textarea { width: 100%; height: 400px; font-family: 'Courier New', monospace; font-size: 14px; border: 1px solid #ddd; border-radius: 5px; padding: 10px; }";
  html += ".btn { background-color: #007bff; color: white; padding: 12px 20px; border: none; border-radius: 5px; cursor: pointer; font-size: 14px; text-decoration: none; display: inline-block; margin: 5px; }";
  html += ".btn:hover { background-color: #0056b3; }";
  html += ".btn-success { background-color: #28a745; } .btn-success:hover { background-color: #218838; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>✏️ Edit Config File: " + filename.substring(1) + "</h1>";
  html += "<form method='POST' action='/save'>";
  html += "<input type='hidden' name='filename' value='" + filename + "'>";
  html += "<textarea name='content' placeholder='Enter JSON config content...'>" + content + "</textarea><br>";
  html += "<input type='submit' value='💾 Save File' class='btn btn-success'>";
  html += "<a href='/configs' class='btn'>↩️ Back to List</a>";
  html += "</form>";
  html += "</div></body></html>";
  
  g_web_server.send(200, "text/html", html);
}

// 保存配置文件
void handleSaveConfig() {
  String filename = g_web_server.arg("filename");
  String content = g_web_server.arg("content");
  
  if (filename.isEmpty() || content.isEmpty()) {
    g_web_server.send(400, "text/plain", "Missing parameters");
    return;
  }
  
  // 验证JSON格式
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, content);
  if (error) {
    g_web_server.send(400, "text/html", 
      "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Save Failed</title></head><body>"
      "<h1>❌ JSON Format Error</h1><p>Error: " + String(error.c_str()) + "</p>"
      "<a href='javascript:history.back()'>Back to Edit</a></body></html>");
    return;
  }
  
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    g_web_server.send(500, "text/plain", "Cannot create file");
    return;
  }
  
  file.print(content);
  file.close();
  
  // 重新扫描配置文件
  scanConfigFiles();
  
  g_web_server.send(200, "text/html", 
    "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Save Success</title>"
    "<style>body{font-family:Arial;max-width:600px;margin:50px auto;padding:20px;text-align:center;}"
    ".success{background:#d4edda;color:#155724;padding:20px;border-radius:10px;margin:20px 0;}"
    "a{display:inline-block;background:#007bff;color:white;padding:10px 20px;text-decoration:none;border-radius:5px;margin:5px;}"
    "</style></head><body>"
    "<div class='success'><h2>✅ Config file saved successfully!</h2></div>"
    "<a href='/configs'>Back to Config List</a>"
    "<a href='/edit?file=" + filename + "'>Continue Editing</a></body></html>");
}

// 删除配置文件
void handleDeleteConfig() {
  String filename = g_web_server.arg("file");
  if (filename.isEmpty()) {
    g_web_server.send(400, "text/plain", "Missing file parameter");
    return;
  }
  
  if (!SD.exists(filename)) {
    g_web_server.send(404, "text/plain", "File not found");
    return;
  }
  
  // 防止删除当前正在使用的配置文件
  if (filename == g_current_config_file) {
    g_web_server.send(400, "text/plain", "Cannot delete current config file");
    return;
  }
  
  if (SD.remove(filename)) {
    // 重新扫描配置文件
    scanConfigFiles();
    g_web_server.send(200, "text/plain", "File deleted successfully");
  } else {
    g_web_server.send(500, "text/plain", "Failed to delete file");
  }
}

// 重命名配置文件
void handleRenameConfig() {
  String oldfile = g_web_server.arg("oldfile");
  String newfile = g_web_server.arg("newfile");
  
  if (oldfile.isEmpty() || newfile.isEmpty()) {
    g_web_server.send(400, "text/plain", "Missing parameters");
    return;
  }
  
  if (!SD.exists(oldfile)) {
    g_web_server.send(404, "text/plain", "Source file not found");
    return;
  }
  
  if (SD.exists(newfile)) {
    g_web_server.send(400, "text/plain", "Target file already exists");
    return;
  }
  
  // 读取原文件内容
  File sourceFile = SD.open(oldfile, FILE_READ);
  if (!sourceFile) {
    g_web_server.send(500, "text/plain", "Cannot open source file");
    return;
  }
  
  String content = sourceFile.readString();
  sourceFile.close();
  
  // 写入新文件
  File targetFile = SD.open(newfile, FILE_WRITE);
  if (!targetFile) {
    g_web_server.send(500, "text/plain", "Cannot create target file");
    return;
  }
  
  targetFile.print(content);
  targetFile.close();
  
  // 删除原文件
  if (SD.remove(oldfile)) {
    // 如果重命名的是当前配置文件，更新当前配置文件路径
    if (oldfile == g_current_config_file) {
      g_current_config_file = newfile;
    }
    
    // 重新扫描配置文件
    scanConfigFiles();
    g_web_server.send(200, "text/plain", "File renamed successfully");
  } else {
    // 如果删除原文件失败，也要删除新文件
    SD.remove(newfile);
    g_web_server.send(500, "text/plain", "Failed to rename file");
  }
}
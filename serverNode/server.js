import { SerialPort } from "serialport";
import { ReadlineParser } from "@serialport/parser-readline";
import express from "express";
import cors from "cors";

import path from "path";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
app.use(cors());
app.use(express.static(__dirname));

// Start the web server even if serial connection fails
const webServer = app.listen(3000, () => {
  console.log("Server running on http://127.0.0.1:3000");
});

app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "dashboard.html"));
});

let sensorData = { status: "Initializing" };

// Try to connect to the serial port
function connectToSerial() {
  try {
    const port = new SerialPort(
      {
        path: "COM5", // Change to your Arduino port
        baudRate: 9600,
      },
      (err) => {
        if (err) {
          console.error("Error opening port:", err.message);
          sensorData = { status: "Serial connection error: " + err.message };
          // Try to reconnect after 5 seconds
          setTimeout(connectToSerial, 5000);
          return;
        }

        console.log("Serial port opened successfully");
        sensorData = { status: "Connected" };

        const parser = port.pipe(new ReadlineParser({ delimiter: "\n" }));

        parser.on("data", (data) => {
          console.log("Received:", data);
          try {
            // Parse the incoming data from Arduino
            const lines = data.trim().split("\n");

            // Process each line separately
            lines.forEach((line) => {
              if (line.includes("Temperature:")) {
                const tempMatch = line.match(/Temperature:\s*([\d.]+)/);
                if (tempMatch && tempMatch[1]) {
                  sensorData["Temperature"] = tempMatch[1];
                }
              } else if (line.includes("Flame Sensor Value:")) {
                const flameMatch = line.match(/Flame Sensor Value:\s*(\d+)/);
                if (flameMatch && flameMatch[1]) {
                  sensorData["Flame"] = flameMatch[1];
                }
              } else if (line.includes("Sound Sensor:")) {
                const soundMatch = line.match(/Sound Sensor:\s*(\d+)/);
                if (soundMatch && soundMatch[1]) {
                  sensorData["Sound"] = soundMatch[1];
                }
              }
            });

            // Check if alarm is active based on Arduino output
            if (
              data.includes("FLAME") ||
              data.includes("SHOCK") ||
              data.includes("TILT") ||
              data.includes("LIGHTS") ||
              data.includes("CLOSED DOR") ||
              data.includes("HIGH TEMP") ||
              data.includes("HIGH SOUND")
            ) {
              sensorData["alarmActive"] = "true";

              // Identify which alarm is triggered
              if (data.includes("FLAME")) sensorData["alarmTrigger"] = "FLAME";
              else if (data.includes("SHOCK"))
                sensorData["alarmTrigger"] = "SHOCK";
              else if (data.includes("TILT"))
                sensorData["alarmTrigger"] = "TILT";
              else if (data.includes("LIGHTS"))
                sensorData["alarmTrigger"] = "LIGHTS";
              else if (data.includes("CLOSED DOR"))
                sensorData["alarmTrigger"] = "DOOR";
              else if (data.includes("HIGH TEMP"))
                sensorData["alarmTrigger"] = "TEMPERATURE";
              else if (data.includes("HIGH SOUND"))
                sensorData["alarmTrigger"] = "SOUND";
            } else if (data.includes("No Trigger")) {
              sensorData["alarmActive"] = "false";
              sensorData["alarmTrigger"] = "";
            }
          } catch (parseErr) {
            console.error("Error parsing data:", parseErr);
          }
        });

        port.on("error", (err) => {
          console.error("Serial port error:", err.message);
          sensorData = { status: "Error: " + err.message };
          // Try to reconnect after error
          setTimeout(connectToSerial, 5000);
        });

        port.on("close", () => {
          console.log("Serial port closed");
          sensorData = { status: "Disconnected" };
          // Try to reconnect after close
          setTimeout(connectToSerial, 5000);
        });
      }
    );
  } catch (err) {
    console.error("Failed to initialize serial port:", err.message);
    sensorData = { status: "Initialization error: " + err.message };
    // Try again after 5 seconds
    setTimeout(connectToSerial, 5000);
  }
}

// Start the connection process
connectToSerial();

// Endpoint API for sensor data
app.get("/data", (req, res) => {
  res.json(sensorData);
});

// Add endpoint to manually attempt reconnection
app.get("/reconnect", (req, res) => {
  connectToSerial();
  res.json({ message: "Attempting to reconnect to serial port" });
});

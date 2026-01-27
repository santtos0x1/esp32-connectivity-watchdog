## Network Stability Monitor(NS Monitor) (Connectivity Watchdog)

This project is a robust *ESP32-based* solution designed for the Telecommunications sector. Its primary objective is to monitor, in real-time, the quality and availability of internet connections in residential or corporate environments.

The system acts as an active sentinel, performing the following operations:

- Latency Monitoring: Executes frequent ICMP "pings" to global reference servers, such as *Google (8.8.8.8)* and *Cloudflare (1.1.1.1)*, measuring response times in milliseconds (ms).

- Availability Analysis: Detects internet service interruptions and calculates precise downtime statistics throughout the day.

- Automated Reporting: Utilizes the *MQTT protocol* to transmit performance metrics and outage alerts to a central broker, enabling seamless integration with dashboards (such as Grafana or Node-RED).

- Data Persistence: Manages credentials and critical states via *NVS* (Non-Volatile Storage), ensuring the device automatically resumes monitoring after power cycles.
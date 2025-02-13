# EvenESP Coex

A single ESP32S3 controller using BLE and WiFi STA coexistence.


# EvenESP Dual

A dual ESP32S3 controller for the Even Realities G1 glasses written in C with ESP-IDF.

One (1) ESP32S3 will handle the BLE communication with the Even Realities G1 glasses, while the other (1) ESP32S3 will handle the WiFi communication with the internet.

The two ESP32S3s will communicate with each other through a UART connection.

## TODO

- [x] Get BLE working
- [ ] Get BLE working to two devices simultaneously
- [ ] Get WiFi working
- [ ] Get communication between the two ESP32S3s working

## Work

- [ ] MicroPython for the wifi ESP32S3?

```mermaid
graph TD
    A[Start] --> B[Initialize Wi-Fi]
    B --> C[Initialize BLE]
    C --> D[Create Wi-Fi Task]
    D --> E[Create BLE Task]
    E --> F[Create API Request Task]
    F --> G[Create Send Data Back to BLE Task]
    G --> H[Tasks Running Concurrently]

    H --> I[Wi-Fi Task: Connect to Wi-Fi]
    I --> J[Wi-Fi Task: Wait for Connection]
    J --> K[BLE Task: Connect to BLE Device]
    K --> L[BLE Task: Wait for Data from BLE]
    L --> M[BLE Task: Parse Data]

    M --> N[API Request Task: Send API Request]
    N --> O[API Request Task: Wait for API Response]
    O --> P[API Request Task: Parse Response]
    P --> Q[Send Data Back to BLE Task: Send Response to BLE]
    Q --> L

    L --> F
    F --> G

```
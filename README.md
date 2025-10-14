# W5500 TCP Server on LPC54114
A lightweight **TCP server implementation** using the **WIZnet W5500 Ethernet controller** and **NXP LPC54114** microcontroller.  
This project provides a reliable real-time data streaming framework over TCP, ideal for embedded applications that need continuous data transmission (e.g., medical devices, sensors, or signal acquisition systems).

---

## 🚀 Features
- Lightweight full TCP server using W5500 hardware TCP/IP stack  
- Automatic **connection / disconnection detection**  
- Supports IPv4 with static IP configuration
- Real-time data streaming to and from the microcontroller and client
- Echo mode: sends back received data to the client
- Easily adaptable to other microcontrollers with minimal changes 
- Stable SPI communication up to 80 MHz  
- Link status monitoring (PHY check)  
- Easy to integrate into larger embedded systems  

---

## 🧠 Overview
The project sets up W5500 as a standalone **TCP server** running over IPv4 with a configurable static IP.  
When a client connects, the microcontroller starts streaming data in real-time and also operates in echo mode, sending back received data.
If the connection is lost, the system automatically detects it and resets the socket.

---

## 🧩 Hardware Setup

| Signal   | W5500 Pin  | LPC54114 Pin  | Notes |
|:---------|:-----------|:--------------|:------|
| SPI      | SPI        | FLEXCOMM6     |  |
| ETH_LINK | LINKLED    | P0_2          | for link or socket events |
| VCC      | 3.3 V      | 3.3 V         | Power supply |
| GND      | GND        | GND           | Common ground |

---

## ⚙️ Software Requirements
- **Toolchain:** MCUXpresso IDE or ARM GCC  
- **MCU:** NXP LPC54114  
- **Ethernet Controller:** WIZnet W5500  
- **Interface:** SPI (max 80 MHz)  
- **Library dependencies:** none (all driver code included)

---

## 📁 Project Important Parts

```
W5500_LPC54114_TCP_Server/
├── source/
│   ├── Main.c
├── drivers/
│   ├── W5500_config.h       # IPv4 Address, MAC Address, TCP Port
│   ├── W5500.h
│   ├── W5500.c
├── LICENSE
└── README.md
```

---

## 🥪 How to Test

1. **Build and flash** the firmware on LPC54114.  
2. Connect W5500 to Ethernet.  
3. On PC, open a TCP client (e.g. `netcat` or Python socket):  
   ```bash
   nc <device_ip> <port>
   ```
4. When connected, you should start receiving live stream data continuously.  
5. If you disconnect, the server automatically resets the socket and waits for new connections.

---
## 📊 Example Output
```
[TCP server] Hello #a
[TCP server] Hello #b
[TCP server] Received: Sent data
[TCP server] Hello #c
```

---

## ⚡ Future Improvements
- Multi-client support using multiple W5500 sockets  
- DMA-based SPI transfers  
- Optional UDP mode for broadcast streaming  
- Command/control channel for configuration  

---

## 📝 License
Released under the **MIT License**.  
You’re free to use, modify, and distribute this code with attribution.

---

## 👤 Author
**Mohammadreza Hosseini**  
Embedded Systems Engineer (Hardware & Firmware)  
[mrh9977@gmail.com](mailto:mrh9977@gmail.com)  
[GitHub](https://github.com/mohammadrezahosseini99)  
[LinkedIn](https://www.linkedin.com/in/mohammadreza-hosseinii)  



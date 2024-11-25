# CryptoDongle - Cryptocurrency Price Display

A firmware project for the LILYGO T-Dongle S3 that displays real-time cryptocurrency prices and 24-hour price changes. Built with ESP-IDF framework.

## Features

- **WiFi Connectivity**
  - Connects to configured WiFi network
  - Fallback to open networks if configured network is unavailable
  - Creates AP mode for configuration if no network is available

- **Cryptocurrency Support**
  - BTC (Bitcoin)
  - ETH (Ethereum)
  - SOL (Solana)
  - ADA (Cardano)
  - XRP (Ripple)
  - LTC (Litecoin)
  - ETC (Ethereum Classic)
  - DOGE (Dogecoin)
  - HNT (Helium)

- **Display Features**
  - Current price in USD/EUR
  - 24-hour price change percentage
  - Auto-refresh at configurable intervals

- **Web Configuration**
  - WiFi network settings
  - Cryptocurrency selection
  - Currency selection (USD/EUR)
  - Refresh interval configuration

## Hardware Requirements

- LILYGO T-Dongle S3
- USB-C cable for programming

## Development Setup

### Prerequisites

- PlatformIO IDE
- ESP-IDF Framework
- Arduino Framework for ESP32

### Installation

1. Clone the repository:
```bash
git clone https://github.com/yourusername/cryptodongle.git
cd cryptodongle
```

2. Install dependencies using PlatformIO:
```bash
pio pkg install
```

3. Build the project:
```bash
pio run
```

4. Upload to your device:
```bash
pio run --target upload
```

### Project Structure

```
project/
├── platformio.ini          # PlatformIO configuration
├── README.md              # This file
└── src/
    ├── main.c             # Main application entry
    ├── wifi_manager.c     # WiFi connection handling
    ├── wifi_manager.h
    ├── webserver.c        # Configuration web server
    ├── webserver.h
    ├── crypto_price.c     # Cryptocurrency price fetching
    └── crypto_price.h
```

## Usage

1. **First Time Setup**
   - Power on the device
   - Connect to the "CryptoDongle" WiFi network
   - Visit http://192.168.4.1 in your browser
   - Configure your WiFi settings and preferences
   - Device will restart and connect to your network

2. **Normal Operation**
   - Device will display the selected cryptocurrency price
   - Price updates according to configured interval
   - 24-hour change displayed below the price

3. **Reconfiguration**
   - Reset the device to enter AP mode
   - Follow the first-time setup steps again

## API Usage

The project uses the CoinGecko API to fetch cryptocurrency prices. No API key is required for basic usage.

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

Distributed under the MIT License. See `LICENSE` for more information.

## Acknowledgments

- LILYGO for the T-Dongle S3 hardware
- CoinGecko for the cryptocurrency price API
- ESP-IDF framework developers
- Arduino community

## Contact

Your Name - [@yourusername](https://twitter.com/yourusername)

Project Link: [https://github.com/yourusername/cryptodongle](https://github.com/yourusername/cryptodongle)

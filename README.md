# MultiUserQuiz 🎯

A command-line based multi-user quiz application built in C++ that supports real-time quiz sessions with multiple participants. The application consists of a server component and client applications that communicate via WebSocket connections.

## ✨ Features

- **Command-line Interface**: Terminal-based quiz application for both server and client
- **Real-time Multi-user Support**: Multiple clients can connect and participate simultaneously
- **Excel Question Bank Import**: Import quiz questions from Excel files using xlnt library
- **Configuration Management**: INI file-based configuration system
- **WebSocket Communication**: Real-time client-server communication using WebSocket and Asio
- **Multiple Quiz Modes**: Support for different quiz formats and game modes
- **Cross-platform**: Built with CMake for cross-platform compatibility
- **Static Linking**: All dependencies are statically linked, no external runtime dependencies

## 🏗️ Project Structure

```
MultiUserQuiz/
├── ClientApp/
│   └── MultiUserQuizClient.cpp    # Client application source
├── ServerApp/                     # Server application source files
├── Common/                        # Shared code between client and server
├── Config/                        # Configuration files
├── Questions/                     # Quiz question banks
├── CMakeLists.txt                # CMake build configuration
└── README.md                     # Project documentation
```

## 🛠️ Tech Stack & Dependencies

### Core Technologies
- **C++**: Primary programming language
- **CMake**: Cross-platform build system
- **Command Line Interface**: Terminal-based user interaction

### External Libraries
- **xlnt**: Excel file reading and parsing for question bank import
- **WebSocket**: Real-time communication protocol implementation
- **Asio**: Asynchronous I/O library for network communication
- **INI Parser**: Configuration file parsing library

All libraries are statically linked, ensuring no external runtime dependencies.

## 🚀 Getting Started

### Prerequisites

- C++ compiler with C++11 support or higher
- CMake (version 3.10 or higher)
- Terminal/Command prompt access

### Building the Application

1. **Clone the repository**
   ```bash
   git clone https://github.com/AbhijithNair73/MultiUserQuiz.git
   cd MultiUserQuiz
   ```

2. **Create build directory**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake**
   ```bash
   cmake ..
   ```

4. **Build the project**
   ```bash
   cmake --build .
   ```

### Running the Application

#### Server Setup
1. **Start the quiz server**
   ```bash
   ./MultiUserQuizServer
   ```

2. **Configure server settings**
   - Modify configuration files in the Config directory
   - Set up question banks using Excel files
   - Configure network settings (port, host, etc.)

#### Client Connection
1. **Launch client application**
   ```bash
   ./MultiUserQuizClient
   ```

2. **Connect to server**
   - Enter server IP address and port
   - Provide participant name/identifier
   - Wait for quiz session to begin

## 📖 Usage

### Quiz Modes

The application supports multiple quiz modes:

- **Standard Quiz**: Traditional question-answer format
- **Timed Quiz**: Questions with time limits
- **Competitive Mode**: Real-time scoring and leaderboards
- **Practice Mode**: Individual learning sessions

### Question Bank Management

1. **Excel Import**
   - Prepare questions in Excel format
   - Use xlnt library integration for seamless import
   - Support for multiple question types

2. **Configuration**
   - Modify INI configuration files
   - Set quiz parameters and rules
   - Configure scoring algorithms

### Server Administration

- **Session Management**: Create and manage quiz sessions
- **Participant Monitoring**: Track connected clients
- **Real-time Control**: Start/pause/end quiz sessions
- **Results Export**: Save quiz results and statistics

## 🔧 Configuration

### INI Configuration Files

The application uses INI files for configuration management:

```ini
[Server]
port=8080
max_clients=50
timeout=30

[Quiz]
questions_per_session=20
time_per_question=30
scoring_method=standard

[Database]
question_bank_path=./Questions/
default_quiz_set=general_knowledge.xlsx
```

### Excel Question Format

Structure your Excel files with the following columns:
- Question Text
- Option A
- Option B  
- Option C
- Option D
- Correct Answer
- Category
- Difficulty

## 🏃‍♂️ Development

### Building from Source

The project uses CMake for cross-platform building:

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Code Structure

- **ClientApp/MultiUserQuizClient.cpp**: Main client application logic
- **ServerApp/**: Server-side implementation
- **Common/**: Shared utilities and data structures
- **Config/**: Configuration management
- **Questions/**: Question bank storage

## 🤝 Contributing

1. **Fork the repository**
2. **Create a feature branch**
   ```bash
   git checkout -b feature/new-feature
   ```
3. **Make your changes**
4. **Build and test**
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```
5. **Commit your changes**
   ```bash
   git commit -m 'Add new feature'
   ```
6. **Push to the branch**
   ```bash
   git push origin feature/new-feature
   ```
7. **Open a Pull Request**

## 🐛 Troubleshooting

### Common Issues

**Build Errors**
```bash
# Clean build directory
rm -rf build/
mkdir build && cd build
cmake ..
```

**Connection Issues**
- Verify server is running and accessible
- Check firewall settings
- Ensure correct IP address and port configuration

**Excel Import Problems**
- Verify Excel file format matches expected structure
- Check file permissions and paths
- Ensure xlnt library is properly linked

## 📋 System Requirements

### Minimum Requirements
- Operating System: Windows 10, macOS 10.14, or Linux (Ubuntu 18.04+)
- RAM: 512 MB
- Storage: 50 MB available space
- Network: TCP/IP connectivity for multi-user sessions

### Supported Platforms
- Windows (Visual Studio, MinGW)
- macOS (Xcode, GCC)
- Linux (GCC, Clang)

## 📄 License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**Abhijith Nair**
- GitHub: [@AbhijithNair73](https://github.com/AbhijithNair73)

## 🙏 Acknowledgments

- **xlnt library** for Excel file processing capabilities
- **Asio library** for robust asynchronous networking
- **WebSocket** implementation for real-time communication
- **CMake** for cross-platform build system

## 📞 Support

For issues and questions:

1. Check the [Issues](https://github.com/AbhijithNair73/MultiUserQuiz/issues) page
2. Create a new issue with detailed description
3. Include build environment and error messages

---

**Terminal-based Collaborative Learning! 🖥️**

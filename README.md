# SecureDropBox

This is a secure file storage application built with Qt.

**Note:** This project is primarily developed and tested on Linux. Other platforms are not officially supported.

## Build Instructions

### Prerequisites
- Qt6
- libsodium
- zlib
- g++
- make

On a Debian-based system, you can install these with:
`sudo apt-get install -y qt6-base-dev libsodium-dev zlib1g-dev g++ make`

### Building
1.  `qmake6`
2.  `make`

The executable will be created in the root of the project as `SecureDropBox`.

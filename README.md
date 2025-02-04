# RTSP Client for Streaming Video  

## Overview  

This project is an **RTSP client** that receives and decodes an RTSP stream using **FFmpeg** and displays it using **SDL2**.  
It supports different transport protocols (**TCP, UDP, HTTP**) and allows authentication via a **username and password**.

## Features  

- Supports **RTSP streaming** over **TCP, UDP, and HTTP**  
- Uses **FFmpeg** for decoding video streams  
- **SDL2** is used for rendering video frames  
- Dynamic RTSP URL construction with **optional authentication**  
- Works with various **IP cameras and streaming servers**  

## Dependencies  

Ensure you have the following dependencies installed before building the project:  

### Required Libraries  
- **FFmpeg** (`libavcodec`, `libavformat`, `libavutil`)  
- **SDL2** (for rendering video)  

### Install on Ubuntu/Debian  
```sh
sudo apt update
sudo apt install libsdl2-dev libavcodec-dev libavformat-dev libavutil-dev libswscale-dev
```

## Building the Project  

Make sure you have cmake (≥v3.10) installed for build configuration

### 1. **Create a Build Directory**  
```sh
mkdir build && cd build
```
This keeps build files separate from the source code.  

### 2. **Configure the Build with CMake**  
```sh
cmake ..
```
- Detects dependencies.  
- Sets up the project.  
- Defines `install/` as the custom installation directory.  

### 3. **Compile the Project**  
```sh
make 
```
- Compiles `RTSPClient.bin`.  

### 4. **Install the Executable**  
```sh
make install
```
- Installs **RTSPClient.bin** to `install/bin/`.  
- Installs **README.md** to `install/share/`.  

## How It Works  

1. The program takes **IP address, transport type, and stream path** as arguments.  
2. It prompts the user for an **optional username and password** (for authentication).  
3. The **RTSP URL** is dynamically constructed.  
4. The program initializes **FFmpeg** to connect and decode the stream.  
5. **SDL2** is used to render the video in a window.  
6. The user can **exit** by clicking the close button.  

## Code Breakdown  

### 1. **RTSP URL Construction**  
- If authentication is provided, the URL format is:  
  ```
  rtsp://username:password@ip_address/stream_path
  ```
- Otherwise:  
  ```
  rtsp://ip_address/stream_path
  ```

### 2. **FFmpeg Integration**  
- `avformat_open_input()` – Opens the RTSP stream  
- `avformat_find_stream_info()` – Retrieves stream details  
- `avcodec_find_decoder()` – Finds the video decoder  
- `avcodec_open2()` – Opens the decoder  
- `av_read_frame()` – Reads frames from the RTSP stream  

### 3. **SDL2 Rendering**  
- `SDL_CreateWindow()` – Creates a window for video display  
- `SDL_CreateRenderer()` – Sets up rendering  
- `SDL_CreateTexture()` – Creates a texture for YUV frames  
- `SDL_UpdateYUVTexture()` – Updates texture with frame data  
- `SDL_RenderCopy()` – Renders the frame  

### 4. **Event Handling**  
- Listens for **SDL_QUIT** event (when the window is closed)  
- Decodes and **renders video frames in real-time**  

## Known Issues  

- Some RTSP streams may require additional FFmpeg options for compatibility.  
- Network latency may impact real-time performance.  
- The current implementation does not support **audio decoding**.  

## Future Improvements  

- **Add support for audio playback**.  
- **Implement reconnection logic** for handling network failures.  

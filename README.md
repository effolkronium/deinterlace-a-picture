# deinterlace-a-picture
## Requirements
### MacOS:
```bash
brew install jpeg
```
### Linux:
```bash
sudo apt-get install libjpeg-dev
```
## Building
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
## Running
```bash
./deinterlace-a-picture input.jpg output.jpg
```

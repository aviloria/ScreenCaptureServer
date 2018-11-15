# ScreenCaptureServer
A Windows server tool for sharing desktop screenshots and video streams via HTTP
Video streams are performed using Motion-JPEG format ([M-JPEG](https://en.wikipedia.org/wiki/Motion_JPEG))

This project was developed using Microsft Visual Studio 2015.
The server runs properly on Windows 7. Not tested on higher versions.

## Running the server
```
Syntax
  ScreenCaptureServer.exe \[options\]
where 'options' is a combination of:
  -i:<_strInterface_>      Server interface for accepting connections
                         Default value: ANY
  -p:<_nPort_>             Server port number for accepting connections
                         Default value: 8080
  -c:<_nMaxConnections_>   Maximum simultanous connections
                         Default value: 10
```

**Command line examples**:
```
  ScreenCaptureServer.exe -p:80 -c:5
```
  
## Connecting to the server
**Snapshot mode:**
```
http://<server-address>:<server-port>/getImage
  
This request can include aditional parameters:
  * width=<nWidth>      Destination image width
  * height=<nHeight>    Destination image height
  * x0=<nX0>            Snapshot window position (horizontal offset)
  * y0=<nY0>            Snapshot window position (vertical offset)
  * cx=<nCX>            Snapshot window width
  * cy=<nCY>            Snapshot window height
```
  
**Snapshot mode:**
```
http://<server-address>:<server-port>/getVideo
  
This request can include aditional parameters:
  * width=<nWidth>      Destination image width
  * height=<nHeight>    Destination image height
  * x0=<nX0>            Snapshot window position (horizontal offset)
  * y0=<nY0>            Snapshot window position (vertical offset)
  * cx=<nCX>            Snapshot window width
  * cy=<nCY>            Snapshot window height
  * fps=<nFPS>          Aprox. frame rate (in frames per second)
```
  
**Examples:**
```
  http://127.0.0.1:8080/getImage?width=640&x0=100&y0=100&cx=400&cy=400
  http://127.0.0.1:8080/getVideo?width=640&fps=10
```

# ScreenCaptureServer
A Windows server tool for sharing desktop screenshots and video streams via HTTP.<br/>
Video streams are performed using Motion-JPEG format ([M-JPEG](https://en.wikipedia.org/wiki/Motion_JPEG))<br/><br/>

This project was developed using Microsft Visual Studio 2015.<br/><br/>

The server runs properly on Windows 7.<br/>
Not tested on higher versions.<br/><br/>

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
  * width=<nWidth>      Destination image width. Default value: same as nCX
  * height=<nHeight>    Destination image height. Default value: same as nCY
  * x0=<nX0>            Snapshot window position (horizontal offset). Default value: 0
  * y0=<nY0>            Snapshot window position (vertical offset). Default value: 0
  * cx=<nCX>            Snapshot window width. Default value: Original screen width resolution
  * cy=<nCY>            Snapshot window height. Default value: Original screen height resolution
```
All these parameters are optional<br/>
Provided _nWidth_ and _nHeight_ values can be modified in forder to keep aspect ratio, fitting to the best values.<br/><br/>

**Snapshot mode:**
```
http://<server-address>:<server-port>/getVideo
  
This request can include aditional parameters:
  * width=<nWidth>      Destination image width. Default value: same as nCX
  * height=<nHeight>    Destination image height. Default value: same as nCY
  * x0=<nX0>            Snapshot window position (horizontal offset). Default value: 0
  * y0=<nY0>            Snapshot window position (vertical offset). Default value: 0
  * cx=<nCX>            Snapshot window width. Default value: Original screen width resolution
  * cy=<nCY>            Snapshot window height. Default value: Original screen height resolution
  * fps=<nFPS>          Aprox. frame rate (in frames per second). Default value: 25
```
All these parameters are optional<br/>
Provided _nWidth_ and _nHeight_ values can be modified in forder to keep aspect ratio, fitting to the best values.<br/><br/>
  
**Examples:**
```
  http://127.0.0.1:8080/getImage?width=640&x0=100&y0=100&cx=400&cy=400
  http://127.0.0.1:8080/getVideo?width=640&fps=10
```

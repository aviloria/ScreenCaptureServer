# ScreenCaptureServer
A Windows tool for sharing desktop screenshots and video-streams via <a href="https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol">HTTP</a>.<br/>
Screenshots are served in <a href="https://en.wikipedia.org/wiki/JPEG">JPEG</a> format.<br/>
Video-streams are served using <a href="https://en.wikipedia.org/wiki/Motion_JPEG">Motion-JPEG</a> format.<br/>

This project was developed using Microsft Visual Studio 2015.<br/>
The server runs properly on Windows 7, Windows 8.1 and Windows 10.

## Running the server
```
Syntax
  ScreenCaptureServer.exe [options]
where 'options' is a combination of:
  -i:<strInterface>      Server interface for accepting connections
                         Default value: ANY
  -p:<nPort>             Server port number for accepting connections
                         Default value: 8080
  -c:<nMaxConnections>   Maximum simultanous connections
                         Default value: 10
```

### Command line examples
To run the server using port 80 (instead of port 8080 -default-) and allowing 5 simultaneous connections maximum (default value is 10), type:
```
  ScreenCaptureServer.exe -p:80 -c:5
```
  
## Connecting to the server
### HealthCheck mode
Use this mode for checking server connection and getting current native screen size.
```
http://<server-address>:<server-port>/healthCheck
```

This request should return an HTTP 200 code including the following JSON payload information:
```
{
  ip : <server-ip>
  hostname : <server-hostname>
  width : <desktop-width>
  height : <desktop-height>
  caps : <capability-list>
}
```

### Snapshot mode
```
http://<server-address>:<server-port>/getImage
  
This request can include aditional parameters:
  * width=<nWidth>      Destination image width. Default value: same as nCX
  * height=<nHeight>    Destination image height. Default value: same as nCY
  * x0=<nX0>            Snapshot window position (horizontal offset). Default value: 0
  * y0=<nY0>            Snapshot window position (vertical offset). Default value: 0
  * cx=<nCX>            Snapshot window width. Default value: Original screen width resolution
  * cy=<nCY>            Snapshot window height. Default value: Original screen height resolution
  * cap=<CAPTURER>      Capturer method to be used
  
Where CAPTURER can be one of these methods:
  * GDI                 Use GDI method to capture the desktop image (Default).
  * GDI+                Use GDI+ method to capture the desktop image.
  * D3D9                Use DirectX9 method to capture the desktop image.
  * D3D11               Use DirectX11 method to capture the desktop image.
                        Only for Windows 8 and higher. On Windows 7, it's available only with Aero themes.
  * RDP                 Use Remote Desktop Mirror Driver method to capture the desktop image (Windows 7 only)
```
All these parameters are optional<br/>
Provided <i>nWidth</i> and <i>nHeight</i> values can be modified in forder to keep aspect ratio, fitting to the best values.

### Video-stream mode
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
  * cap=<CAPTURER>      Capturer method to be used
  
Where CAPTURER can be one of these methods:
  * GDI                 Use GDI method to capture the desktop image (Default).
  * GDI+                Use GDI+ method to capture the desktop image.
  * D3D9                Use DirectX9 method to capture the desktop image.
  * D3D11               Use DirectX11 method to capture the desktop image.
                        Only for Windows 8 and higher. On Windows 7, it's available only with Aero themes.
  * RDP                 Use Remote Desktop Mirror Driver method to capture the desktop image (Windows 7 only)
```
All these parameters are optional<br/>
Provided <i>nWidth</i> and <i>nHeight</i> values can be modified in forder to keep aspect ratio, fitting to the best values.<br/>

This stream can be easily embeded in a HTML page using [&lt;embed&gt; tag](https://www.w3schools.com/tags/tag_embed.asp), i.e.:
```
<embed src="http://<server-address>:<server-port>/getVideo" />
```

### Examples
```
  http://127.0.0.1:8080/getImage?width=640&x0=100&y0=100&cx=400&cy=400
  http://127.0.0.1:8080/getVideo?width=640&fps=10&cap=D3D11
```

# Video Element {#video}

The video element uses VLC to display video from any source that VLC supports. This includes RTSP, necessary for most IP surveillance cameras.

This is still experimental and buggy. The video functionality is **not** included in the binaries, compile it yourself if you want to try it. See the .pro file for details.

Major bugs:
- If VLC does not find its plugins below the path to the binary, QVisu will silently crash with some unintelligible error.
- With video support, QVisu will not start from Qt Creator
- Even worse: VLC seems to block the event queue, so nothing happens in the other elements while the video is running

Two streams in parallel have not been tested yet. 

## Syntax

<pre>
&lt;element type="video" position="x,y" width="w" height="h"&gt;
	&lt;url&gt;URL&lt;/url&gt;
&lt;/element&gt;
</pre>

Elements in **bold** are mandatory.

| element/tag           | comment                                                                                                                       |
|-----------------------|-------------------------------------------------------------------------------------------------------------------------------|
| width, height         | Can be chosen arbitrarily                                                                                                     |
| url                   | URL of the stream. Include credentials in the URL, as in rtsp://user:password@camera:port/video.h264                          |


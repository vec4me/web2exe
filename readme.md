# web2exe
Turn any HTML file into a **standalone Windows executable** w/ *full support* for WebGPU, WebGL, WebSockets, WebRTC, WASM, etc.

## "Why make this?"
The sad reality is that software *isn't portable*, especially not games. And despite the browser being an *awful ecosystem*, it *solves this shortcoming* and even makes my life as a game developer easier. *(That should concern more people.)* But anyways, I wanted the **best of both portability and power**, and the browser became a good candidate for that after WebGPU mainstreamed, the only thing left was to make distributing feel more *"official,"* because not once have I seen a AAA title playable through a URL.

Send someone a game installer, and they're satisfied, send someone a **standalone executable under a megabyte** and they're mortified *(in a good way)*.

> But Jeff, why not just use GLFW w/ Vulkan and *call it a day*?

Because, that's **only a tenth of what I need**, like *peer-to-peer networking, audio, input, screen capture, gyroscopes*, etc. and while there are libraries for those, I've done this long enough to know that *only myself* should be making *encapsulation boundaries*, not some *random guy from Timbuktu*. Well, I guess in this case it'd be Google making those boundaries, but they're probably less constrained.

## Dependencies
- **xwin**
- **wget**

## Building & Usage
Running it *for the first time* will download the **Windows & WebView2 SDK** through `xwin`, and `wget` respectively and are saved to a temporary directory for the sake of speed and not flooding the filesystem with garbage.

Here's an *example* of building an HTML game to a binary:
```bash
$ ./web2exe game.html    # This builds your game into an executable.
$ wine game.exe
```

**The repository may live wherever you wish, and `web2exe` can be invoked from outside the folder.**

![](lineage/Screenshot%202025-10-20%20113851.png)

## Caveats
- Running the executable from a ZIP archive **will flag Windows Defender**.
- External `file://` assets **will not be bundled**, this is a *literal translation* of the webpage into an executable.
- This only compiles to **32-bit** *for the sake of achieving the smallest sizes*.

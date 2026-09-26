# Gallery HDR viewer

HDR progressively enhances the existing Lightbox2 viewer. All thumbnails, previous/next arrows, keyboard navigation, captions, image sizing, and close behavior use the original lightbox. There is no separate dialog or launch button.

On a display reporting HDR, an EXR-backed image first shows its standard PNG, then receives an HDR canvas overlay after decoding and GPU submission finish. The canvas covers only the image content, preserving the existing white frame and navigation. A compact HDR badge button sits immediately left of the matching close button in the existing caption row. Its highlighted state indicates HDR is on; pressing it toggles HDR/SDR without adding another row. No HDR controls appear during loading, on ordinary images, on SDR displays, or after an enhancement failure.

HDR loads preprocessed `.hdr.gz` assets (6–11 MB each), generated from the original EXRs. They preserve exactly the same sanitized RGB half-float samples the original viewer uploaded, including values above white; resolution and orientation are unchanged. The format stores RGB in six byte planes, omits the always-opaque alpha channel, and uses gzip compression. Native decompression and a small unpacking loop replace the large JavaScript EXR decoder on the normal path. If an optimized asset fails to load or decode, the worker lazily loads EXRLoader and falls back to the original EXR.

Download/decode runs concurrently with GPU setup. A 64 MiB LRU cache keeps recently decoded images for the page session; revisiting them skips both downloading and decoding. Navigating or closing terminates unfinished worker work and releases GPU resources. Late asynchronous results cannot replace another image. The PNG remains underneath for fallback. A small “Loading HDR…” overlay at the bottom of the image stays visible until HDR is ready, then disappears; failure, navigation, or closing also clears it without changing image dimensions. No HDR downloads occur on a display that reports SDR.

## Regenerating optimized assets

After adding or replacing an EXR, make sure its gallery link has the correct `data-exr`, then run:

```sh
node scripts/build-hdr-assets.mjs
node scripts/test-hdr-assets.mjs
```

The builder generates files under `assets/gallery/hdr/` and updates `data-hdr` links in `gallery.html`. Content hashes in filenames prevent stale optimized images after an EXR changes. Publish the generated files together with the HTML and scripts; GitHub Pages requires no server processing. Original EXRs remain available for fallback. Older generated files are not automatically deleted.

The viewer uses WebGPU `rgba16float` with `toneMapping.mode = extended`. It preserves scene-linear highlights, keeps the original 0 EV exposure and applies the extended sRGB transfer function, and presents opaque output. The SDR comparison uses the identical EXR and exposure with standard canvas tone mapping (clipping above white), so it isolates display range rather than comparing against a differently graded PNG. These EXRs have no chromaticities metadata; sRGB/Rec.709 primaries are assumed. Future EXRs with other primaries need a color conversion.

Unsupported browsers, loading failures, and device loss leave the existing PNG in the ordinary lightbox; diagnostics go to the console. The display capability query is advisory; actual brightness depends on the browser, OS, display mode, and current HDR headroom. Use HTTPS in production or localhost for development.

## Verification

Serve the repository with `python3 -m http.server 8765 --bind 127.0.0.1`, then open `/gallery.html` in current Chrome or Edge on an HDR display.

- Click an EXR-backed thumbnail. Verify its frame, size, orientation, caption, arrows, and close behavior match ordinary images.
- Verify the PNG appears while the EXR loads and the HDR button appears beside Close once enhancement finishes.
- Use the HDR toggle to compare HDR/SDR at the fixed 0 EV exposure; WaterColor and Intel Sponza contain conspicuous highlights. Verify keyboard arrows still navigate images and Escape closes the lightbox when the toggle is focused.
- Navigate between HDR and ordinary images with mouse and keyboard, including while decoding. Verify there is no stale HDR overlay or control panel.
- Check that unsupported browsers and SDR displays keep the ordinary lightbox without HDR controls.
- Move the window between HDR and SDR screens; enhancement should follow the display capability.

All five current assets (Orange Juice, Dam Break, WaterColor, Kroken, Intel Sponza camera 1) passed exact half-float pixel comparisons against the original EXR rendering path. Combined downloads fell from 117.45 MB to 45.79 MB (61% smaller). Orange Juice is slightly larger in the preprocessed format but avoids EXR decode time. Local Node measurements were 276–523 ms for EXR decoding versus 27–51 ms for native gzip decompression and unpacking; these are CPU measurements, not browser or network timings.

The format tests also cover malformed headers, truncated data, sanitization, opaque alpha, and preserved highlights. Actual HDR brightness depends on the display and needs visual verification on HDR hardware.

## Dependencies and references

`vendor/three` contains Three.js r180's EXRLoader, core, fflate, and MIT license, downloaded from the official repository. Only EXRLoader's two import paths were changed to local paths. The loader and fflate retain their embedded third-party license notices. No build step or runtime CDN is required.

- https://github.com/mrdoob/three.js/tree/r180
- https://threejs.org/docs/pages/EXRLoader.html
- https://developer.chrome.com/blog/new-in-webgpu-129

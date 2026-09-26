import { decodeHDR } from './hdr-format.js';

async function loadOptimized(url) {
  const response = await fetch(url);
  if (!response.ok) throw new Error(`HDR download failed (${response.status}).`);
  let buffer = await response.arrayBuffer();
  const bytes = new Uint8Array(buffer);
  // Some hosts decompress .gz responses automatically via Content-Encoding.
  if (bytes[0] === 0x1f && bytes[1] === 0x8b) {
    const stream = new Blob([buffer]).stream().pipeThrough(new DecompressionStream('gzip'));
    buffer = await new Response(stream).arrayBuffer();
  }
  return decodeHDR(buffer);
}

async function loadEXR(url) {
  // Large EXR dependencies are only needed if an optimized asset is unavailable.
  const [{ EXRLoader }, response] = await Promise.all([
    import('./vendor/three/EXRLoader.js'), fetch(url)
  ]);
  if (!response.ok) throw new Error(`EXR download failed (${response.status}).`);
  const { width, height, data } = new EXRLoader().parse(await response.arrayBuffer());
  if (data.length !== width * height * 4) throw new Error('This EXR needs RGB channels.');
  for (let i = 0; i < data.length; i++) {
    if (i % 4 === 3) data[i] = 0x3c00;
    else if ((data[i] & 0x8000) || (data[i] & 0x7c00) === 0x7c00) data[i] = 0;
  }
  return { width, height, data };
}

self.onmessage = async ({ data: source }) => {
  try {
    let decoded;
    if (source.hdr) {
      try { decoded = await loadOptimized(source.hdr); }
      catch (error) { console.warn('Optimized HDR unavailable; using EXR.', error); }
    }
    decoded ??= await loadEXR(source.exr);
    self.postMessage(decoded, [decoded.data.buffer]);
  } catch (error) {
    self.postMessage({ error: error.message });
  }
};

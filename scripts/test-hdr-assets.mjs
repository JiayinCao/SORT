import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { gunzipSync } from 'node:zlib';
import { EXRLoader } from './vendor/three/EXRLoader.js';
import { encodeHDR, decodeHDR } from './hdr-format.js';

const root = new URL('../', import.meta.url);
const html = await readFile(new URL('gallery.html', root), 'utf8');
let totalRaw = 0;
let totalOptimized = 0;
for (const [tag] of html.matchAll(/<a\b[^>]*\bdata-exr="[^"]+"[^>]*>/g)) {
  const exrPath = /data-exr="([^"]+)"/.exec(tag)[1];
  const hdrPath = /data-hdr="([^"]+)"/.exec(tag)?.[1];
  assert(hdrPath, `Missing optimized asset: ${exrPath}`);
  const exrBytes = await readFile(new URL(exrPath, root));
  const startEXR = performance.now();
  const original = new EXRLoader().parse(exrBytes.buffer.slice(exrBytes.byteOffset, exrBytes.byteOffset + exrBytes.byteLength));
  const exrTime = performance.now() - startEXR;
  const packed = await readFile(new URL(hdrPath, root));
  const startHDR = performance.now();
  const raw = gunzipSync(packed);
  const decoded = decodeHDR(raw.buffer.slice(raw.byteOffset, raw.byteOffset + raw.byteLength));
  const hdrTime = performance.now() - startHDR;
  assert.equal(decoded.width, original.width);
  assert.equal(decoded.height, original.height);
  assert.equal(decoded.data.length, original.data.length);
  for (let i = 0; i < original.data.length; i++) {
    let expected = original.data[i];
    if (i % 4 === 3) expected = 0x3c00;
    else if ((expected & 0x8000) || (expected & 0x7c00) === 0x7c00) expected = 0;
    if (decoded.data[i] !== expected) assert.fail(`${exrPath}: changed half-float sample at ${i}`);
  }
  totalRaw += exrBytes.length;
  totalOptimized += packed.length;
  console.log(`${exrPath}: exact pixel match; local decode ${exrTime.toFixed(0)} → ${hdrTime.toFixed(0)} ms`);
}
const samples = new Uint16Array([0x8000, 0x7c00, 0x7e00, 0, 0x3c00, 0x4900, 0x0001, 0]);
const encoded = encodeHDR({ width: 2, height: 1, data: samples });
assert.deepEqual([...decodeHDR(encoded.buffer).data], [0, 0, 0, 0x3c00, 0x3c00, 0x4900, 0x0001, 0x3c00]);
assert.throws(() => decodeHDR(new ArrayBuffer(2)), /Truncated/);
assert.throws(() => decodeHDR(encoded.buffer.slice(0, -1)), /Invalid/);
const invalidVersion = encoded.slice();
new DataView(invalidVersion.buffer).setUint32(4, 99, true);
assert.throws(() => decodeHDR(invalidVersion.buffer), /Unsupported/);
console.log(`Total download: ${(totalRaw / 1e6).toFixed(2)} → ${(totalOptimized / 1e6).toFixed(2)} MB (${Math.round((1 - totalOptimized / totalRaw) * 100)}% smaller).`);

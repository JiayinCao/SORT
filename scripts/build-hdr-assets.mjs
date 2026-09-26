// Run from any directory: node scripts/build-hdr-assets.mjs
import { readFile, writeFile, mkdir } from 'node:fs/promises';
import { createHash } from 'node:crypto';
import { gzipSync } from 'node:zlib';
import { EXRLoader } from './vendor/three/EXRLoader.js';
import { encodeHDR } from './hdr-format.js';

const root = new URL('../', import.meta.url);
const htmlURL = new URL('gallery.html', root);
let html = await readFile(htmlURL, 'utf8');
await mkdir(new URL('assets/gallery/hdr/', root), { recursive: true });
for (const match of [...html.matchAll(/<a\b[^>]*\bdata-exr="([^"]+)"[^>]*>/g)]) {
  const exr = await readFile(new URL(match[1], root));
  const start = performance.now();
  const decoded = new EXRLoader().parse(exr.buffer.slice(exr.byteOffset, exr.byteOffset + exr.byteLength));
  const packed = gzipSync(encodeHDR(decoded), { level: 9 });
  const hash = createHash('sha256').update(packed).digest('hex').slice(0, 12);
  const name = match[1].split('/').pop().replace(/\.exr$/i, '');
  const path = `./assets/gallery/hdr/${name}.${hash}.hdr.gz`;
  await writeFile(new URL(path, root), packed);
  const tag = match[0].replace(/\sdata-hdr="[^"]*"/g, '').replace('<a ', `<a data-hdr="${path}" `);
  html = html.replace(match[0], tag);
  console.log(`${name}: ${(exr.length / 1e6).toFixed(2)} → ${(packed.length / 1e6).toFixed(2)} MB (${Math.round((1 - packed.length / exr.length) * 100)}% smaller; ${Math.round(performance.now() - start)} ms build)`);
}
await writeFile(htmlURL, html);

// Run from any directory: node scripts/build-hdr-assets.mjs
import { readFile, writeFile, mkdir } from 'node:fs/promises';
import { createHash } from 'node:crypto';
import { gzipSync, gunzipSync } from 'node:zlib';
import { EXRLoader } from './vendor/three/EXRLoader.js';
import { encodeHDR } from './hdr-format.js';

const root = new URL('../', import.meta.url);
for (const page of ['gallery.html', 'index.html']) {
  const htmlURL = new URL(page, root);
  let html = await readFile(htmlURL, 'utf8');
  for (const match of [...html.matchAll(/<[\w-]+\b[^>]*\bdata-exr="([^"]+)"[^>]*>/g)]) {
    const exr = await readFile(new URL(match[1], root));
    const start = performance.now();
    const decoded = new EXRLoader().parse(exr.buffer.slice(exr.byteOffset, exr.byteOffset + exr.byteLength));
    const encoded = encodeHDR(decoded);
    const existingPath = /\bdata-hdr="([^"]+)"/.exec(match[0])?.[1];
    if (existingPath) {
      try {
        // Preserve matching assets across Node/zlib versions with different gzip output.
        const existing = gunzipSync(await readFile(new URL(existingPath, root)));
        if (existing.equals(Buffer.from(encoded))) {
          console.log(`${match[1]}: existing HDR pixels are current`);
          continue;
        }
      } catch { /* Rebuild missing or damaged assets. */ }
    }
    const packed = gzipSync(encoded, { level: 9 });
    const hash = createHash('sha256').update(packed).digest('hex').slice(0, 12);
    const name = match[1].split('/').pop().replace(/\.exr$/i, '');
    const directory = new URL('../hdr/', new URL(match[1], root));
    await mkdir(directory, { recursive: true });
    const path = `./${directory.pathname.slice(root.pathname.length)}${name}.${hash}.hdr.gz`;
    await writeFile(new URL(path, root), packed);
    const tag = match[0].replace(/\sdata-hdr="[^"]*"/g, '').replace(/^(<[\w-]+)/, `$1 data-hdr="${path}"`);
    html = html.replace(match[0], tag);
    console.log(`${name}: ${(exr.length / 1e6).toFixed(2)} → ${(packed.length / 1e6).toFixed(2)} MB (${Math.round((1 - packed.length / exr.length) * 100)}% smaller; ${Math.round(performance.now() - start)} ms build)`);
  }
  await writeFile(htmlURL, html);
}

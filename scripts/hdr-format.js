// SORT HDR v1: 16-byte header, then six byte planes of lossless RGB half floats.
// Rows stay bottom-up, matching EXRLoader. Alpha is always opaque in the viewer.
const MAGIC = 0x52444853; // "SHDR" in little-endian order.
const HEADER_SIZE = 16;

export function encodeHDR({ width, height, data }) {
  if (data.length !== width * height * 4) throw new Error('Expected RGBA half floats.');
  const pixels = width * height;
  const bytes = new Uint8Array(HEADER_SIZE + pixels * 6);
  const header = new DataView(bytes.buffer);
  header.setUint32(0, MAGIC, true);
  header.setUint32(4, 1, true);
  header.setUint32(8, width, true);
  header.setUint32(12, height, true);
  for (let i = 0; i < pixels; i++) {
    for (let c = 0; c < 3; c++) {
      let value = data[i * 4 + c];
      // Match the original EXR worker's treatment of negative/non-finite samples.
      if ((value & 0x8000) || (value & 0x7c00) === 0x7c00) value = 0;
      bytes[HEADER_SIZE + c * 2 * pixels + i] = value & 255;
      bytes[HEADER_SIZE + (c * 2 + 1) * pixels + i] = value >>> 8;
    }
  }
  return bytes;
}

export function decodeHDR(buffer) {
  if (buffer.byteLength < HEADER_SIZE) throw new Error('Truncated HDR header.');
  const header = new DataView(buffer);
  if (header.getUint32(0, true) !== MAGIC || header.getUint32(4, true) !== 1) {
    throw new Error('Unsupported HDR format.');
  }
  const width = header.getUint32(8, true);
  const height = header.getUint32(12, true);
  const pixels = width * height;
  if (!width || !height || buffer.byteLength !== HEADER_SIZE + pixels * 6) {
    throw new Error('Invalid HDR dimensions or payload.');
  }
  const bytes = new Uint8Array(buffer, HEADER_SIZE);
  const data = new Uint16Array(pixels * 4);
  for (let i = 0; i < pixels; i++) {
    for (let c = 0; c < 3; c++) {
      data[i * 4 + c] = bytes[c * 2 * pixels + i] | (bytes[(c * 2 + 1) * pixels + i] << 8);
    }
    data[i * 4 + 3] = 0x3c00;
  }
  return { width, height, data };
}

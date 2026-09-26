// Progressive HDR enhancement for the existing Lightbox2 viewer.
// EXRs are scene-linear RGB with assumed sRGB/Rec.709 primaries.
const shader = `
struct Vertex { @builtin(position) position: vec4f, @location(0) uv: vec2f }
@vertex fn vertex(@builtin(vertex_index) i: u32) -> Vertex {
  let p = array<vec2f, 3>(vec2f(-1,-1), vec2f(3,-1), vec2f(-1,3));
  var out: Vertex;
  out.position = vec4f(p[i], 0, 1);
  // EXRLoader returns bottom-up rows; WebGPU's viewport starts at the top.
  out.uv = (p[i] + 1) * 0.5;
  return out;
}
@group(0) @binding(0) var source: texture_2d<f32>;
@group(0) @binding(1) var sourceSampler: sampler;
@fragment fn fragment(in: Vertex) -> @location(0) vec4f {
  let linear = max(textureSample(source, sourceSampler, in.uv).rgb, vec3f(0));
  // rgba16float with colorSpace sRGB needs the sRGB transfer function, including
  // values above 1. Do not tone-map or clamp these highlights before presentation.
  let encoded = select(1.055 * pow(linear, vec3f(1.0 / 2.4)) - 0.055,
                       linear * 12.92, linear <= vec3f(0.0031308));
  return vec4f(encoded, 1);
}`;

function installHDR() {
  const lightbox = window.lightbox;
  if (!lightbox || !navigator.gpu) return;
  const root = lightbox.$lightbox[0];
  const image = lightbox.$image[0];
  const outer = lightbox.$outerContainer[0];
  const sources = new Map([...document.querySelectorAll('[data-exr]')].map(link =>
    [link.href, {
      exr: new URL(link.dataset.exr, document.baseURI).href,
      hdr: link.dataset.hdr ? new URL(link.dataset.hdr, document.baseURI).href : null
    }]));
  const displayHDR = matchMedia('(dynamic-range: high)');
  const canvas = document.createElement('canvas');
  canvas.className = 'lb-hdr-canvas';
  canvas.hidden = true;
  canvas.setAttribute('aria-hidden', 'true'); // Existing image retains its alt text.
  lightbox.$container[0].append(canvas);
  const controls = document.createElement('button');
  controls.type = 'button';
  controls.className = 'lb-hdr-controls';
  controls.hidden = true;
  controls.textContent = 'HDR';
  controls.setAttribute('aria-label', 'High dynamic range');
  controls.setAttribute('aria-pressed', 'true');
  controls.title = 'HDR on — click to turn off';
  root.querySelector('.lb-closeContainer').prepend(controls);
  const loading = document.createElement('span');
  loading.className = 'lb-hdr-loading';
  loading.hidden = true;
  loading.setAttribute('role', 'status');
  loading.setAttribute('aria-live', 'polite');
  loading.innerHTML = '<span class="lb-hdr-spinner" aria-hidden="true"></span><span>Loading HDR…</span>';
  outer.append(loading);
  let hdrEnabled = true;
  let active = null;
  let isOpen = false;
  // Bounded LRU of decoded pixels: reopening recent images skips download/decode.
  const cache = new Map();
  const cacheLimit = 64 * 1024 * 1024;
  let cacheBytes = 0;

  async function loadPixels(state, source) {
    const key = source.hdr || source.exr;
    if (cache.has(key)) {
      const decoded = cache.get(key);
      cache.delete(key);
      cache.set(key, decoded);
      return decoded;
    }
    const decoded = await new Promise((resolve, reject) => {
      state.worker = new Worker(new URL('./hdr-exr-worker.js', import.meta.url), { type: 'module' });
      const finish = () => { state.worker?.terminate(); state.worker = null; };
      state.worker.onmessage = ({ data }) => {
        finish();
        data.error ? reject(new Error(data.error)) : resolve(data);
      };
      state.worker.onerror = () => { finish(); reject(new Error('HDR decoding failed.')); };
      state.worker.postMessage(source);
    });
    if (active === state && decoded.data.byteLength <= cacheLimit) {
      while (cacheBytes + decoded.data.byteLength > cacheLimit) {
        const oldest = cache.keys().next().value;
        cacheBytes -= cache.get(oldest).data.byteLength;
        cache.delete(oldest);
      }
      cache.set(key, decoded);
      cacheBytes += decoded.data.byteLength;
    }
    return decoded;
  }

  function release() {
    const state = active;
    active = null;
    canvas.hidden = controls.hidden = loading.hidden = true;
    if (!state) return;
    state.worker?.terminate();
    state.context?.unconfigure();
    state.texture?.destroy();
    state.device?.destroy();
  }
  function fail(state, error) {
    if (active !== state) return;
    console.warn('HDR enhancement unavailable:', error);
    release(); // The ordinary image is always in place underneath.
  }
  function alignCanvas() {
    if (!active?.ready) return;
    const bounds = image.getBoundingClientRect();
    const parent = outer.getBoundingClientRect();
    const css = getComputedStyle(image);
    const left = parseFloat(css.borderLeftWidth) || 0;
    const top = parseFloat(css.borderTopWidth) || 0;
    Object.assign(canvas.style, {
      left: `${bounds.left - parent.left + left}px`,
      top: `${bounds.top - parent.top + top}px`,
      width: `${bounds.width - left - (parseFloat(css.borderRightWidth) || 0)}px`,
      height: `${bounds.height - top - (parseFloat(css.borderBottomWidth) || 0)}px`
    });
  }
  function render() {
    const s = active;
    if (!s?.ready) return;
    try {
      s.context.configure({ device: s.device, format: 'rgba16float', colorSpace: 'srgb',
        alphaMode: 'opaque', toneMapping: { mode: hdrEnabled ? 'extended' : 'standard' } });
      const encoder = s.device.createCommandEncoder();
      const pass = encoder.beginRenderPass({ colorAttachments: [{
        view: s.context.getCurrentTexture().createView(), loadOp: 'clear', storeOp: 'store',
        clearValue: { r: 0, g: 0, b: 0, a: 1 }
      }] });
      pass.setPipeline(s.pipeline);
      pass.setBindGroup(0, s.bindGroup);
      pass.draw(3);
      pass.end();
      s.device.queue.submit([encoder.finish()]);
      controls.setAttribute('aria-pressed', String(hdrEnabled));
      controls.title = hdrEnabled ? 'HDR on — click to turn off' : 'HDR off — click to turn on';
    } catch (error) { fail(s, error); }
  }
  async function enhance() {
    if (!isOpen || !displayHDR.matches || active) return;
    const current = lightbox.album[lightbox.currentImageIndex];
    if (!current) return;
    const href = new URL(current.link, document.baseURI).href;
    const source = sources.get(href);
    // Wait for Lightbox to finish showing this image, including arrow navigation.
    if (!source || image.src !== href) return;
    const s = {};
    active = s;
    loading.hidden = false;
    hdrEnabled = true;
    try {
      // Download/decode alongside GPU setup, instead of waiting for shader compilation.
      // Handle rejection immediately even if GPU initialization fails first.
      const pixels = loadPixels(s, source).then(value => ({ value }), error => ({ error }));
      const adapter = await navigator.gpu.requestAdapter();
      if (active !== s) return;
      if (!adapter) throw new Error('No compatible GPU.');
      const device = await adapter.requestDevice();
      if (active !== s) { device.destroy(); return; }
      s.device = device;
      device.lost.then(() => fail(s, 'GPU device lost.'));
      device.addEventListener('uncapturederror', event => fail(s, event.error));
      s.context = canvas.getContext('webgpu');
      if (!s.context) throw new Error('No WebGPU canvas.');
      s.context.configure({ device, format: 'rgba16float', colorSpace: 'srgb',
        alphaMode: 'opaque', toneMapping: { mode: 'extended' } });
      if (s.context.getConfiguration?.().toneMapping?.mode !== 'extended') {
        throw new Error('Extended HDR output is unsupported.');
      }
      const module = device.createShaderModule({ code: shader });
      s.pipeline = await device.createRenderPipelineAsync({ layout: 'auto',
        vertex: { module, entryPoint: 'vertex' },
        fragment: { module, entryPoint: 'fragment', targets: [{ format: 'rgba16float' }] }
      });
      if (active !== s) return;
      const result = await pixels;
      if (active !== s) return;
      if (result.error) throw result.error;
      const decoded = result.value;
      const { width, height, data } = decoded;
      if (Math.max(width, height) > device.limits.maxTextureDimension2D) throw new Error('EXR exceeds GPU limits.');
      canvas.width = width;
      canvas.height = height;
      s.texture = device.createTexture({ size: [width, height], format: 'rgba16float',
        usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST });
      device.queue.writeTexture({ texture: s.texture }, data, { bytesPerRow: width * 8 }, [width, height]);
      s.bindGroup = device.createBindGroup({ layout: s.pipeline.getBindGroupLayout(0), entries: [
        { binding: 0, resource: s.texture.createView() },
        { binding: 1, resource: device.createSampler({ magFilter: 'linear', minFilter: 'linear' }) }
      ] });
      s.ready = true;
      alignCanvas();
      render();
      await device.queue.onSubmittedWorkDone();
      if (active !== s) return;
      canvas.hidden = controls.hidden = false;
      loading.hidden = true;
      lightbox.sizeOverlay();
    } catch (error) { fail(s, error); }
  }

  // Keep Lightbox's existing sizing, captions, transitions, arrows and close UI.
  // Hooks cover both thumbnail clicks and previous/next keyboard navigation.
  const changeImage = lightbox.changeImage;
  lightbox.changeImage = function(...args) {
    release();
    isOpen = true;
    return changeImage.apply(this, args);
  };
  const showImage = lightbox.showImage;
  lightbox.showImage = function(...args) {
    const result = showImage.apply(this, args);
    enhance();
    return result;
  };
  const end = lightbox.end;
  lightbox.end = function(...args) {
    isOpen = false;
    release();
    return end.apply(this, args);
  };
  controls.addEventListener('click', () => { hdrEnabled = !hdrEnabled; render(); });
  new ResizeObserver(alignCanvas).observe(image);
  displayHDR.addEventListener('change', () => {
    release();
    enhance();
  });
}

// Lightbox builds its DOM in a jQuery ready callback; register after it.
if (window.jQuery) window.jQuery(installHDR);

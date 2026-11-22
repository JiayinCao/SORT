// Display the home-page EXR using the gallery's packed pixels and HDR transfer.
import { shader } from './hdr-shader.js';

const image = document.querySelector('.hdr-home > img[data-exr][data-hdr]');
if (image && navigator.gpu) installHDR(image);

function installHDR(image) {
  const displayHDR = matchMedia('(dynamic-range: high)');
  const canvas = document.createElement('canvas');
  canvas.hidden = true;
  canvas.setAttribute('aria-hidden', 'true');
  // Keep the credit last in paint order, including Safari's GPU compositing path.
  image.insertAdjacentElement('afterend', canvas);
  let active = null;

  function release() {
    const state = active;
    active = null;
    canvas.hidden = true;
    state?.worker?.terminate();
    state?.context?.unconfigure();
    state?.texture?.destroy();
    state?.device?.destroy();
  }

  function fail(state, error) {
    if (active !== state) return;
    console.warn('Home-page HDR enhancement unavailable:', error);
    release(); // Keep the PNG banner visible on any failure.
  }

  function loadPixels(state) {
    return new Promise((resolve, reject) => {
      state.worker = new Worker(new URL('./hdr-exr-worker.js', import.meta.url), { type: 'module' });
      const finish = () => { state.worker?.terminate(); state.worker = null; };
      state.worker.onmessage = ({ data }) => {
        finish();
        data.error ? reject(new Error(data.error)) : resolve(data);
      };
      state.worker.onerror = () => { finish(); reject(new Error('HDR decoding failed.')); };
      state.worker.postMessage({
        exr: new URL(image.dataset.exr, document.baseURI).href,
        hdr: new URL(image.dataset.hdr, document.baseURI).href
      });
    });
  }

  async function enhance() {
    if (!displayHDR.matches || active) return;
    const state = {};
    active = state;
    try {
      // Handle decode failures immediately while GPU initialization is pending.
      const pixels = loadPixels(state).then(value => ({ value }), error => ({ error }));
      const adapter = await navigator.gpu.requestAdapter();
      if (active !== state) return;
      if (!adapter) throw new Error('No compatible GPU.');
      const device = await adapter.requestDevice();
      if (active !== state) { device.destroy(); return; }
      state.device = device;
      device.lost.then(() => fail(state, 'GPU device lost.'));
      device.addEventListener('uncapturederror', event => fail(state, event.error));
      state.context = canvas.getContext('webgpu');
      if (!state.context) throw new Error('No WebGPU canvas.');
      state.context.configure({ device, format: 'rgba16float', colorSpace: 'srgb',
        alphaMode: 'opaque', toneMapping: { mode: 'extended' } });
      if (state.context.getConfiguration?.().toneMapping?.mode !== 'extended') {
        throw new Error('Extended HDR output is unsupported.');
      }
      const module = device.createShaderModule({ code: shader });
      const pipeline = await device.createRenderPipelineAsync({ layout: 'auto',
        vertex: { module, entryPoint: 'vertex' },
        fragment: { module, entryPoint: 'fragment', targets: [{ format: 'rgba16float' }] }
      });
      if (active !== state) return;
      const result = await pixels;
      if (active !== state) return;
      if (result.error) throw result.error;
      const { width, height, data } = result.value;
      if (Math.max(width, height) > device.limits.maxTextureDimension2D) {
        throw new Error('EXR exceeds GPU limits.');
      }
      canvas.width = width;
      canvas.height = height;
      state.texture = device.createTexture({ size: [width, height], format: 'rgba16float',
        usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST });
      device.queue.writeTexture({ texture: state.texture }, data, { bytesPerRow: width * 8 }, [width, height]);
      const bindGroup = device.createBindGroup({ layout: pipeline.getBindGroupLayout(0), entries: [
        { binding: 0, resource: state.texture.createView() },
        { binding: 1, resource: device.createSampler({ magFilter: 'linear', minFilter: 'linear' }) }
      ] });
      const encoder = device.createCommandEncoder();
      const pass = encoder.beginRenderPass({ colorAttachments: [{
        view: state.context.getCurrentTexture().createView(), loadOp: 'clear', storeOp: 'store',
        clearValue: { r: 0, g: 0, b: 0, a: 1 }
      }] });
      pass.setPipeline(pipeline);
      pass.setBindGroup(0, bindGroup);
      pass.draw(3);
      pass.end();
      device.queue.submit([encoder.finish()]);
      await device.queue.onSubmittedWorkDone();
      if (active === state) canvas.hidden = false;
    } catch (error) { fail(state, error); }
  }

  displayHDR.addEventListener('change', () => { release(); enhance(); });
  window.addEventListener('pagehide', release);
  window.addEventListener('pageshow', enhance);
  enhance();
}

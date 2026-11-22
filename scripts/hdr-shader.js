// Shared scene-linear EXR presentation for gallery and home-page HDR.
export const shader = `
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

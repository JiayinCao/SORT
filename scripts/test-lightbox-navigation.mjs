import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';

const source = readFileSync(new URL('../lightbox/js/lightbox.js', import.meta.url), 'utf8');

function setup() {
  const handlers = new Map();
  const loads = [];
  const document = {};
  const window = {};
  let imageAttributes = {};
  let resized = 0;
  const chain = {
    ready() {}, on(name, callback) { handlers.set(name, callback); return this; },
    off(name) { handlers.delete(name); return this; },
    stop() { return this; }, fadeIn() { return this; }, fadeOut() { return this; },
    hide() { return this; }, addClass() { return this; }, removeClass() { return this; },
    width(value) { return value === undefined ? 800 : this; },
    height(value) { return value === undefined ? 600 : this; },
    attr(value) { imageAttributes = value; return this; },
    find() { return this; }
  };
  const $ = target => ({
    ...chain,
    is: () => ['INPUT', 'TEXTAREA', 'SELECT'].includes(target?.tagName)
  });
  $.extend = Object.assign;
  $.proxy = (fn, context) => fn.bind(context);
  class Image {
    constructor() { this.width = 64; this.height = 32; loads.push(this); }
  }
  const sandbox = { jQuery: $, document, window, Image };
  vm.runInNewContext(source, sandbox);
  const lightbox = sandbox.lightbox;
  lightbox.album = [0, 1, 2].map(i => ({ link: `${i}.png`, alt: `${i}` }));
  lightbox.options.wrapAround = true;
  lightbox.$lightbox = $(null);
  lightbox.$overlay = $(null);
  lightbox.$outerContainer = $(null);
  lightbox.$image = $(null);
  lightbox.containerPadding = lightbox.imageBorderWidth = { left: 0, right: 0, top: 0, bottom: 0 };
  lightbox.sizeContainer = () => resized++;
  const key = (keyCode, extra = {}) => {
    const event = { keyCode, target: { tagName: 'BODY' }, prevented: false,
      preventDefault() { this.prevented = true; }, stopPropagation() {}, ...extra };
    handlers.get('keydown.lightbox')?.(event);
    return event;
  };
  return { lightbox, handlers, loads, key,
    attributes: () => imageAttributes, resized: () => resized };
}

test('document keydown remains active through loading and wraps in both directions', () => {
  const { lightbox, handlers, key } = setup();
  lightbox.changeImage(0);
  lightbox.enableKeyboardNav();
  assert.equal(handlers.size, 1, 'rebinding must not duplicate navigation');
  assert.equal(key(37).prevented, true);
  assert.equal(lightbox.currentImageIndex, 2);
  key(39);
  assert.equal(lightbox.currentImageIndex, 0);
  key(39, { target: { tagName: 'BUTTON' } });
  key(39);
  assert.equal(lightbox.currentImageIndex, 2, 'presses during loading must be handled');
});

test('late image loads cannot replace the newest selection', () => {
  const state = setup();
  state.lightbox.changeImage(0);
  state.key(39);
  state.loads[1].onload();
  assert.equal(state.attributes().src, '1.png');
  assert.equal(state.resized(), 1);
  state.loads[0].onload();
  assert.equal(state.attributes().src, '1.png');
  assert.equal(state.resized(), 1);
});

test('Escape cancels pending loads and removes document keyboard handling', () => {
  const state = setup();
  state.lightbox.changeImage(0);
  state.key(27);
  assert.equal(state.handlers.size, 0);
  state.loads[0].onload();
  assert.equal(state.resized(), 0, 'a completed load must not reopen the viewer');
});

test('browser shortcuts and editable fields retain their arrow keys', () => {
  const state = setup();
  state.lightbox.changeImage(0);
  for (const extra of [
    { altKey: true }, { ctrlKey: true }, { metaKey: true },
    { target: { tagName: 'INPUT' } }, { target: { tagName: 'TEXTAREA' } },
    { target: { tagName: 'SELECT' } }, { target: { isContentEditable: true } }
  ]) {
    assert.equal(state.key(39, extra).prevented, false);
    assert.equal(state.lightbox.currentImageIndex, 0);
  }
});

import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';

const source = readFileSync(new URL('./gallery-swipe.js', import.meta.url), 'utf8');

function setup(pointerEvents = true) {
  const handlers = new Map();
  const viewportHandlers = new Map();
  const changes = [];
  const surface = { style: {}, addEventListener(type, handler) { handlers.set(type, handler); } };
  const lightbox = {
    $container: [surface], album: ['a', 'b', 'c'], currentImageIndex: 0,
    options: { wrapAround: true },
    end() {},
    changeImage(index) { this.currentImageIndex = index; changes.push(index); }
  };
  const viewport = { scale: 1, addEventListener(type, handler) { viewportHandlers.set(type, handler); } };
  vm.runInNewContext(source, { window: {
    jQuery: callback => callback(), lightbox, visualViewport: viewport,
    PointerEvent: pointerEvents ? function() {} : undefined
  } });
  const event = (type, values = {}) => {
    const result = { cancelable: true, defaultPrevented: false, stopped: false,
      preventDefault() { this.defaultPrevented = true; },
      stopImmediatePropagation() { this.stopped = true; }, ...values };
    handlers.get(type)(result);
    return result;
  };
  const pointer = (type, x, y, extra = {}) => event(type, {
    pointerId: 1, pointerType: 'touch', clientX: x, clientY: y, timeStamp: 100, ...extra
  });
  const swipe = (dx, dy = 0) => {
    pointer('pointerdown', 150, 100, { timeStamp: 0 });
    pointer('pointermove', 150 + dx, 100 + dy, { timeStamp: 100 });
    return pointer('pointerup', 150 + dx, 100 + dy, { timeStamp: 200 });
  };
  const touch = (x, y = 100, identifier = 1) => ({ clientX: x, clientY: y, identifier });
  return { lightbox, changes, surface, viewport, viewportHandlers, event, pointer, swipe, touch };
}

test('touch swipes navigate one image and wrap in either direction', () => {
  const state = setup();
  state.swipe(-80);
  assert.equal(state.lightbox.currentImageIndex, 1);
  state.swipe(80);
  assert.equal(state.lightbox.currentImageIndex, 0);
  state.swipe(80);
  assert.equal(state.lightbox.currentImageIndex, 2);
  state.swipe(-80);
  assert.equal(state.lightbox.currentImageIndex, 0);
  assert.deepEqual(state.changes, [1, 0, 2, 0]);
});

test('a swipe suppresses its generated click, then permits a new tap and keyboard clicks', () => {
  const state = setup();
  state.swipe(-80);
  const click = state.event('click', { detail: 1 });
  assert.equal(click.defaultPrevented, true);
  assert.equal(click.stopped, true);
  assert.equal(state.event('click', { detail: 1 }).defaultPrevented, false);
  state.swipe(-80);
  assert.equal(state.event('click', { detail: 0 }).defaultPrevented, false);
  state.pointer('pointerdown', 100, 100);
  state.pointer('pointerup', 100, 100);
  assert.equal(state.event('click', { detail: 1 }).defaultPrevented, false);
});

test('vertical gestures, short drags, slow gestures and mouse drags do not navigate', () => {
  const state = setup();
  state.swipe(15);
  state.swipe(60, 80);
  state.pointer('pointerdown', 150, 100, { timeStamp: 0 });
  state.pointer('pointerup', 50, 100, { timeStamp: 2000 });
  state.pointer('pointerdown', 150, 100, { pointerType: 'mouse' });
  state.pointer('pointerup', 50, 100, { pointerType: 'mouse' });
  assert.deepEqual(state.changes, []);
});

test('pinch and cancelled gestures do not navigate or cancel the pinch', () => {
  const state = setup();
  state.pointer('pointerdown', 150, 100);
  state.pointer('pointerdown', 200, 100, { pointerId: 2 });
  assert.equal(state.pointer('pointermove', 50, 100).defaultPrevented, false);
  state.pointer('pointerup', 50, 100);
  state.pointer('pointerup', 250, 100, { pointerId: 2 });
  state.pointer('pointerdown', 150, 100);
  state.pointer('pointercancel', 50, 100);
  state.pointer('pointerup', 50, 100);
  assert.deepEqual(state.changes, []);
});

test('zoomed images allow native panning and swipe works again after zooming out', () => {
  const state = setup();
  assert.equal(state.surface.style.touchAction, 'pan-y pinch-zoom');
  state.viewport.scale = 2;
  state.viewportHandlers.get('resize')();
  assert.equal(state.surface.style.touchAction, 'manipulation');
  state.swipe(-80);
  assert.deepEqual(state.changes, []);
  state.viewport.scale = 1;
  state.viewportHandlers.get('resize')();
  state.swipe(-80);
  assert.deepEqual(state.changes, [1]);
});

test('swipes respect the non-wrapping album boundary', () => {
  const state = setup();
  state.lightbox.options.wrapAround = false;
  state.swipe(80);
  assert.deepEqual(state.changes, []);
  state.swipe(-80);
  assert.deepEqual(state.changes, [1]);
});

test('closing the viewer cancels an unfinished swipe', () => {
  const state = setup();
  state.pointer('pointerdown', 150, 100);
  state.pointer('pointermove', 60, 100);
  state.lightbox.end();
  state.pointer('pointerup', 60, 100);
  assert.deepEqual(state.changes, []);
});

test('touch-event fallback navigates and ignores multi-touch', () => {
  const state = setup(false);
  state.event('touchstart', { touches: [state.touch(150)], timeStamp: 0 });
  const moved = state.event('touchmove', { touches: [state.touch(60)], timeStamp: 100 });
  assert.equal(moved.defaultPrevented, true);
  state.event('touchend', { touches: [], changedTouches: [state.touch(60)], timeStamp: 200 });
  assert.deepEqual(state.changes, [1]);
  state.event('touchstart', { touches: [state.touch(150)], timeStamp: 300 });
  state.event('touchstart', { touches: [state.touch(150), state.touch(200, 100, 2)], timeStamp: 350 });
  state.event('touchend', { touches: [], changedTouches: [state.touch(60)], timeStamp: 400 });
  assert.deepEqual(state.changes, [1]);
});

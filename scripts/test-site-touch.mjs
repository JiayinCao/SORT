import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';

const source = readFileSync(new URL('./site-touch.js', import.meta.url), 'utf8');

function setup() {
  const handlers = new Map();
  let clicks = 0;
  const target = {
    isContentEditable: false,
    editable: false,
    closest(selector) {
      if (selector === 'input, textarea, select, label') return this.editable ? this : null;
      return this;
    },
    click() { clicks++; }
  };
  vm.runInNewContext(source, { document: {
    addEventListener(type, handler, options) { handlers.set(type, { handler, options }); }
  } });
  const point = (x = 10, y = 20, identifier = 1) => ({ clientX: x, clientY: y, identifier });
  const send = (type, values) => {
    const event = { target, cancelable: true, defaultPrevented: false,
      preventDefault() { this.defaultPrevented = true; }, ...values };
    handlers.get(type).handler(event);
    return event;
  };
  const tap = (time, x = 10, y = 20, extra = {}) => {
    send('touchstart', { touches: [point(x, y)], timeStamp: time });
    return send('touchend', { touches: [], changedTouches: [point(x, y)], timeStamp: time + 40, ...extra });
  };
  return { handlers, target, point, send, tap, clicks: () => clicks };
}

test('a second quick tap cancels zoom and activates its target exactly once', () => {
  const state = setup();
  assert.equal(state.tap(0).defaultPrevented, false);
  assert.equal(state.clicks(), 0);
  assert.equal(state.tap(180).defaultPrevented, true);
  assert.equal(state.clicks(), 1);
  assert.equal(state.tap(360).defaultPrevented, true);
  assert.equal(state.clicks(), 2, 'rapid arrow navigation must not lose taps');
  assert.equal(state.handlers.get('touchend').options.passive, false);
  assert.equal(state.handlers.get('touchend').options.capture, true);
});

test('a child inside a navigation button activates the button', () => {
  const state = setup();
  const child = { isContentEditable: false, closest: selector =>
    selector === 'input, textarea, select, label' ? null : state.target };
  state.tap(0, 10, 20, { target: child });
  state.tap(180, 10, 20, { target: child });
  assert.equal(state.clicks(), 1);
});

test('pinch gestures and their final finger release are not cancelled', () => {
  const state = setup();
  state.tap(0);
  state.send('touchstart', { touches: [state.point()], timeStamp: 100 });
  state.send('touchstart', { touches: [state.point(), state.point(50, 50, 2)], timeStamp: 120 });
  const first = state.send('touchend', { touches: [state.point()], changedTouches: [state.point(50, 50, 2)], timeStamp: 150 });
  const last = state.send('touchend', { touches: [], changedTouches: [state.point()], timeStamp: 180 });
  assert.equal(first.defaultPrevented, false);
  assert.equal(last.defaultPrevented, false);
  assert.equal(state.tap(210).defaultPrevented, false);
  assert.equal(state.clicks(), 0);
});

test('scrolling, long presses and cancelled touches are not double taps', () => {
  const state = setup();
  state.tap(0);
  state.send('touchstart', { touches: [state.point()], timeStamp: 100 });
  state.send('touchmove', { touches: [state.point(70)], timeStamp: 120 });
  assert.equal(state.send('touchend', { touches: [], changedTouches: [state.point()], timeStamp: 140 }).defaultPrevented, false);
  state.send('touchstart', { touches: [state.point()], timeStamp: 200 });
  assert.equal(state.send('touchend', { touches: [], changedTouches: [state.point()], timeStamp: 900 }).defaultPrevented, false);
  state.tap(1000);
  state.send('touchcancel', {});
  assert.equal(state.tap(1100).defaultPrevented, false);
  assert.equal(state.clicks(), 0);
});

test('separate taps and editable controls keep native behavior', () => {
  const state = setup();
  state.tap(0);
  assert.equal(state.tap(600).defaultPrevented, false);
  assert.equal(state.tap(750, 200).defaultPrevented, false);
  state.target.editable = true;
  state.tap(900, 200);
  assert.equal(state.tap(1050, 200).defaultPrevented, false);
  state.target.editable = false;
  state.target.isContentEditable = true;
  state.tap(1200, 200);
  assert.equal(state.tap(1350, 200).defaultPrevented, false);
  assert.equal(state.clicks(), 0);
});

test('already handled or non-cancelable touches do not synthesize another click', () => {
  const state = setup();
  state.tap(0);
  state.tap(180, 10, 20, { defaultPrevented: true });
  state.tap(360);
  state.tap(540, 10, 20, { cancelable: false });
  assert.equal(state.clicks(), 0);
});

// Safari versions with broken touch-action support can still zoom on double tap.
// Cancel only a completed second single-finger tap, preserving pinch and scroll.
(function() {
  var started = null;
  var previousTap = null;
  var tapDuration = 400;
  var doubleTapDelay = 350;
  var moveLimit = 12;
  var doubleTapDistance = 32;

  function reset() {
    started = previousTap = null;
  }

  document.addEventListener('touchstart', function(event) {
    if (event.touches.length !== 1) { reset(); return; }
    var touch = event.touches[0];
    started = { id: touch.identifier, x: touch.clientX, y: touch.clientY, time: event.timeStamp };
  }, { capture: true, passive: true });

  document.addEventListener('touchmove', function(event) {
    if (!started || event.touches.length !== 1) { reset(); return; }
    var touch = event.touches[0];
    if (touch.identifier !== started.id ||
        Math.hypot(touch.clientX - started.x, touch.clientY - started.y) > moveLimit) reset();
  }, { capture: true, passive: true });

  document.addEventListener('touchcancel', reset, { capture: true, passive: true });

  document.addEventListener('touchend', function(event) {
    var tap = started;
    started = null;
    if (!tap || event.touches.length || event.changedTouches.length !== 1) { reset(); return; }
    var touch = event.changedTouches[0];
    var elapsed = event.timeStamp - tap.time;
    if (touch.identifier !== tap.id || elapsed < 0 || elapsed > tapDuration ||
        Math.hypot(touch.clientX - tap.x, touch.clientY - tap.y) > moveLimit) { reset(); return; }
    var target = event.target.closest ? event.target : event.target.parentElement;
    // Leave native editing, selection and form interactions alone.
    if (!target || target.isContentEditable || target.closest('input, textarea, select, label') ||
        event.defaultPrevented || !event.cancelable) { reset(); return; }
    var doubleTap = previousTap && event.timeStamp - previousTap.time <= doubleTapDelay &&
      Math.hypot(touch.clientX - previousTap.x, touch.clientY - previousTap.y) <= doubleTapDistance;
    previousTap = { x: touch.clientX, y: touch.clientY, time: event.timeStamp };
    if (!doubleTap) return;

    // Cancel Safari's zoom and its compatibility click together. Replaying one
    // click preserves the second tap on gallery arrows, buttons and links.
    event.preventDefault();
    var action = target.closest('a, button, summary, [role="button"]') || target;
    if (typeof action.click === 'function') action.click();
  }, { capture: true, passive: false });
}());

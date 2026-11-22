// Touchscreen navigation for the existing Lightbox viewer, including iPad and Surface.
(function() {
  if (!window.jQuery) return;
  window.jQuery(function() {
    var lightbox = window.lightbox;
    var surface = lightbox && lightbox.$container[0];
    if (!surface) return;
    var gesture = null;
    var pointers = new Set();
    var suppressClickUntil = 0;

    function zoomed() {
      return window.visualViewport && window.visualViewport.scale > 1.01;
    }
    function updateTouchAction() {
      // When zoomed in, let one finger pan the page in either direction.
      surface.style.touchAction = zoomed() ? 'manipulation' : 'pan-y pinch-zoom';
      gesture = null;
    }
    updateTouchAction();
    if (window.visualViewport) window.visualViewport.addEventListener('resize', updateTouchAction);

    var close = lightbox.end;
    lightbox.end = function() {
      gesture = null;
      pointers.clear();
      suppressClickUntil = 0;
      return close.apply(this, arguments);
    };

    function begin(id, x, y, time) {
      suppressClickUntil = 0;
      gesture = !zoomed() && lightbox.album.length > 1 ?
        { id: id, x: x, y: y, time: time, axis: null } : null;
    }
    function move(id, x, y, event) {
      if (!gesture || gesture.id !== id) return;
      var dx = Math.abs(x - gesture.x);
      var dy = Math.abs(y - gesture.y);
      if (!gesture.axis && Math.max(dx, dy) > 10) gesture.axis = dx > dy * 1.5 ? 'x' : 'y';
      if (gesture.axis === 'x' && event.cancelable) event.preventDefault();
    }
    function end(id, x, y, event) {
      var swipe = gesture;
      gesture = null;
      if (!swipe || swipe.id !== id || zoomed()) return;
      var dx = x - swipe.x;
      var dy = y - swipe.y;
      if (swipe.axis === 'y' || Math.abs(dx) < 50 || Math.abs(dx) < Math.abs(dy) * 1.5 ||
          event.timeStamp - swipe.time > 1500) return;
      var index = lightbox.currentImageIndex + (dx < 0 ? 1 : -1);
      if (index < 0 || index >= lightbox.album.length) {
        if (!lightbox.options.wrapAround) return;
        index = (index + lightbox.album.length) % lightbox.album.length;
      }
      if (event.cancelable) event.preventDefault();
      suppressClickUntil = Date.now() + 500;
      lightbox.changeImage(index);
    }

    // Pointer-event cancellation does not suppress click. Stop the click generated
    // by a swipe before it reaches the previous/next links underneath the finger.
    surface.addEventListener('click', function(event) {
      if (Date.now() > suppressClickUntil || event.detail === 0) return;
      suppressClickUntil = 0;
      event.preventDefault();
      event.stopImmediatePropagation();
    }, true);

    if (window.PointerEvent) {
      surface.addEventListener('pointerdown', function(event) {
        suppressClickUntil = 0;
        if (event.pointerType !== 'touch') return;
        pointers.add(event.pointerId);
        if (pointers.size !== 1) { gesture = null; return; }
        begin(event.pointerId, event.clientX, event.clientY, event.timeStamp);
      });
      surface.addEventListener('pointermove', function(event) {
        move(event.pointerId, event.clientX, event.clientY, event);
      }, { passive: false });
      surface.addEventListener('pointerup', function(event) {
        pointers.delete(event.pointerId);
        end(event.pointerId, event.clientX, event.clientY, event);
      });
      surface.addEventListener('pointercancel', function(event) {
        pointers.delete(event.pointerId);
        gesture = null;
      });
    } else {
      surface.addEventListener('touchstart', function(event) {
        if (event.touches.length !== 1) { gesture = null; return; }
        var touch = event.touches[0];
        begin(touch.identifier, touch.clientX, touch.clientY, event.timeStamp);
      }, { passive: true });
      surface.addEventListener('touchmove', function(event) {
        if (event.touches.length !== 1) { gesture = null; return; }
        var touch = event.touches[0];
        move(touch.identifier, touch.clientX, touch.clientY, event);
      }, { passive: false });
      surface.addEventListener('touchend', function(event) {
        if (event.touches.length || event.changedTouches.length !== 1) { gesture = null; return; }
        var touch = event.changedTouches[0];
        end(touch.identifier, touch.clientX, touch.clientY, event);
      }, { passive: false });
      surface.addEventListener('touchcancel', function() { gesture = null; });
    }
  });
}());

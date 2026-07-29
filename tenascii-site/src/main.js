import {
  animate,
  createTimeline,
  stagger,
  svg,
  onScroll,
  splitText,
} from 'animejs';
import './style.css';

// Mirrors the hex values in style.css -- kept as plain constants here
// because SVG presentation attributes set via setAttribute() shouldn't
// depend on CSS custom-property resolution.
const COLOR_FORWARD = '#2f9bff';
const COLOR_INVERTED = '#ff3b3b';

// ------------------------------------------------------------------
// Ambient particle field: a persistent field of slow-drifting dots
// behind the whole page, mostly blue with a minority red, echoing the
// forward/inverted duality everywhere, not just in the diagrams.
// Plain canvas + rAF rather than anime.js -- animating ~70 independent
// elements is exactly the case where a direct canvas loop is the
// better tool than driving that many DOM nodes through a tween engine.
// ------------------------------------------------------------------
function particleField() {
  const canvas = document.getElementById('particle-field');
  if (!canvas) return;
  const ctx = canvas.getContext('2d');
  let w, h, dpr;
  let particles = [];

  function resize() {
    dpr = Math.min(window.devicePixelRatio || 1, 2);
    w = window.innerWidth;
    h = document.documentElement.scrollHeight;
    canvas.width = w * dpr;
    canvas.height = h * dpr;
    canvas.style.height = h + 'px';
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
  }

  function seed() {
    const count = Math.round((w * h) / 32000);
    particles = Array.from({ length: Math.min(count, 140) }, () => ({
      x: Math.random() * w,
      y: Math.random() * h,
      r: Math.random() * 1.4 + 0.4,
      vx: (Math.random() - 0.5) * 0.06,
      vy: (Math.random() - 0.5) * 0.06,
      inverted: Math.random() < 0.22,
      phase: Math.random() * Math.PI * 2,
    }));
  }

  resize();
  seed();
  window.addEventListener('resize', () => { resize(); seed(); });

  let t = 0;
  function tick() {
    t += 0.01;
    ctx.clearRect(0, 0, w, h);
    for (const p of particles) {
      p.x += p.vx; p.y += p.vy;
      if (p.x < 0) p.x = w; if (p.x > w) p.x = 0;
      if (p.y < 0) p.y = h; if (p.y > h) p.y = 0;
      const pulse = 0.35 + 0.25 * Math.sin(t + p.phase);
      ctx.beginPath();
      ctx.arc(p.x, p.y, p.r, 0, Math.PI * 2);
      ctx.fillStyle = p.inverted
        ? `rgba(255,59,59,${pulse})`
        : `rgba(47,155,255,${pulse})`;
      ctx.fill();
    }
    requestAnimationFrame(tick);
  }
  requestAnimationFrame(tick);
}

// ------------------------------------------------------------------
// Hero: title reveal, turnstile ring self-draw, particle crossing +
// inverting along a motion path. Plays once on load, not looped --
// this is a one-shot entrance, the ambient loops live further down
// the page in the concept diagrams.
// ------------------------------------------------------------------
function heroIntro() {
  const split = splitText('#hero-title', { chars: true });
  const ringDrawable = svg.createDrawable('#turnstile-ring');
  const motion = svg.createMotionPath('#motion-track');
  const motionB = svg.createMotionPath('#motion-track-b');
  const dot = document.getElementById('hero-dot');
  const dotB = document.getElementById('hero-dot-b');

  const legDuration = 1700;
  const forwardLegStart = 650;
  // the path arcs up and over; the ring sits roughly at its horizontal
  // midpoint, so ~55% through the forward leg is where the dot visually
  // crosses into it
  const flipDelay = forwardLegStart + legDuration * 0.55;

  createTimeline({ defaults: { ease: 'outExpo' } })
    .add(split.chars, {
      opacity: [0, 1],
      y: [26, 0],
      duration: 700,
      delay: stagger(36),
    }, 0)
    .add('.hero-rule', { width: [0, 120], duration: 700, ease: 'inOutQuad' }, 500)
    .add('.hero-tag', { opacity: [0, 1], y: [12, 0], duration: 600 }, 350)
    .add('.hero-cta', { opacity: [0, 1], y: [12, 0], duration: 600 }, 500)
    .add('.hero-ring-caption', { opacity: [0, 1], duration: 600 }, 550)
    .add(ringDrawable, { draw: ['0 0', '0 1'], duration: 1300, ease: 'inOutQuad' }, 250)
    .add(dot, { ...motion, duration: legDuration, ease: 'inOutQuad' }, forwardLegStart)
    .add(dotB, { ...motionB, duration: legDuration * 0.9, ease: 'inOutQuad' }, forwardLegStart + 220)
    .add(dot, {
      ...motion,
      duration: legDuration,
      ease: 'inOutQuad',
      reversed: true,
      onComplete: () => dot.setAttribute('fill', COLOR_FORWARD),
    }, forwardLegStart + legDuration + 100)
    .add('.scroll-cue', { opacity: [0, 1], duration: 500 }, forwardLegStart + 200);

  setTimeout(() => dot.setAttribute('fill', COLOR_INVERTED), flipDelay);
}

// ------------------------------------------------------------------
// Concept card mini-diagrams: small looping demonstrations of each
// mechanic. Started immediately -- they're invisible until their
// parent card fades in via the scroll reveal below, so an early start
// is harmless.
// ------------------------------------------------------------------
function conceptDiagrams() {
  // 01 turnstile: dot crosses in, inverts, retraces back out
  const turnstileDot = document.querySelector('.cd-turnstile-dot');
  if (turnstileDot) {
    createTimeline({ loop: true, defaults: { ease: 'inOutQuad', duration: 850 } })
      .add(turnstileDot, {
        cx: 80,
        onComplete: () => turnstileDot.setAttribute('fill', COLOR_INVERTED),
      }, 0)
      .add(turnstileDot, {
        cx: 10,
        onComplete: () => turnstileDot.setAttribute('fill', COLOR_FORWARD),
      }, 1150);
  }

  // 02 the algorithm: two squads converge from opposite sides
  const pincerA = document.querySelector('.cd-pincer-a');
  const pincerB = document.querySelector('.cd-pincer-b');
  if (pincerA && pincerB) {
    createTimeline({ loop: true, defaults: { ease: 'inOutQuad', duration: 950 } })
      .add(pincerA, { cx: 75, cy: 50 }, 0)
      .add(pincerB, { cx: 85, cy: 50 }, 0)
      .add(pincerA, { cx: 10, cy: 35 }, 1300)
      .add(pincerB, { cx: 150, cy: 65 }, 1300);
  }

  // 03 paradox: forward meets inverted, both vanish, an ember burst marks it
  const paraA = document.querySelector('.cd-para-a');
  const paraB = document.querySelector('.cd-para-b');
  const burst = document.querySelector('.cd-burst');
  if (paraA && paraB && burst) {
    createTimeline({ loop: true, defaults: { ease: 'inOutQuad' } })
      .add([paraA, paraB], { opacity: 1, duration: 1 }, 0)
      .add(paraA, { cx: 78, duration: 700 }, 0)
      .add(paraB, { cx: 82, duration: 700 }, 0)
      .add([paraA, paraB], { opacity: 0, duration: 120 }, 680)
      .add(burst, { opacity: [0, 1, 0], r: [2, 16, 2], duration: 500 }, 650)
      .add(paraA, { cx: 60, opacity: 1, duration: 1 }, 1600)
      .add(paraB, { cx: 100, opacity: 1, duration: 1 }, 1600);
  }
}

// ------------------------------------------------------------------
// Scroll reveals for every [data-reveal] element -- fires once per
// element the first time it scrolls into view (repeat: false), not
// on every pass.
// ------------------------------------------------------------------
function scrollReveals() {
  document.querySelectorAll('[data-reveal]').forEach((el, i) => {
    onScroll({
      target: el,
      enter: 'bottom-=10% top',
      repeat: false,
      onEnter: () => {
        animate(el, {
          opacity: [0, 1],
          y: [22, 0],
          duration: 700,
          delay: (i % 3) * 90,
          ease: 'outExpo',
        });
      },
    });
  });
}

particleField();

document.fonts.ready.then(() => {
  heroIntro();
  conceptDiagrams();
  scrollReveals();
});
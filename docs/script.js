/* ════════════════════════════════════════════════════
   AlgoTrack — script.js  (2026 edition)
   ════════════════════════════════════════════════════ */

/* ── Nav toggle ── */
const navToggle = document.querySelector("[data-nav-toggle]");
const nav       = document.querySelector("[data-nav]");
const header    = document.querySelector("[data-header]");

if (navToggle && nav) {
  navToggle.addEventListener("click", () => {
    const isOpen = nav.classList.toggle("is-open");
    navToggle.setAttribute("aria-expanded", String(isOpen));
  });
  nav.addEventListener("click", (e) => {
    if (e.target instanceof HTMLAnchorElement) {
      nav.classList.remove("is-open");
      navToggle.setAttribute("aria-expanded", "false");
    }
  });
}

/* ── Scrolled header ── */
const updateHeader = () => {
  if (!header) return;
  header.classList.toggle("is-scrolled", window.scrollY > 12);
};
window.addEventListener("scroll", updateHeader, { passive: true });
updateHeader();

/* ── Scroll reveal ── */
const revealItems = document.querySelectorAll(".reveal");
const revealObserver = new IntersectionObserver(
  (entries) => {
    entries.forEach((entry) => {
      if (entry.isIntersecting) {
        entry.target.classList.add("in-view");
        revealObserver.unobserve(entry.target);
      }
    });
  },
  { threshold: 0.1, rootMargin: "0px 0px -48px 0px" }
);
revealItems.forEach((el) => revealObserver.observe(el));

/* ── Stat counter animation ── */
function animateCounter(el, target, duration = 1200) {
  const start = performance.now();
  const update = (now) => {
    const elapsed = now - start;
    const progress = Math.min(elapsed / duration, 1);
    const eased = 1 - Math.pow(1 - progress, 3); // ease-out cubic
    el.textContent = Math.round(eased * target);
    if (progress < 1) requestAnimationFrame(update);
  };
  requestAnimationFrame(update);
}

const counters = document.querySelectorAll(".stat-num[data-count]");
const counterObserver = new IntersectionObserver(
  (entries) => {
    entries.forEach((entry) => {
      if (entry.isIntersecting) {
        const target = parseInt(entry.target.dataset.count, 10);
        animateCounter(entry.target, target);
        counterObserver.unobserve(entry.target);
      }
    });
  },
  { threshold: 0.5 }
);
counters.forEach((el) => counterObserver.observe(el));

/* ── Animated dot-grid background ── */
(function initCanvas() {
  const canvas = document.getElementById("bg-canvas");
  if (!canvas) return;
  const ctx = canvas.getContext("2d");

  let W, H, dots;

  const DOT_SPACING = 38;
  const DOT_R       = 1;
  const ACCENT_RGB  = "20, 184, 166";
  const WAVE_SPEED  = 0.0008;
  const RIPPLE_R    = 180;

  let mouse = { x: -9999, y: -9999 };
  let tick  = 0;

  function resize() {
    W = canvas.width  = window.innerWidth;
    H = canvas.height = window.innerHeight;
    buildDots();
  }

  function buildDots() {
    dots = [];
    const cols = Math.ceil(W / DOT_SPACING) + 1;
    const rows = Math.ceil(H / DOT_SPACING) + 1;
    for (let r = 0; r < rows; r++) {
      for (let c = 0; c < cols; c++) {
        dots.push({ x: c * DOT_SPACING, y: r * DOT_SPACING });
      }
    }
  }

  function draw() {
    ctx.clearRect(0, 0, W, H);
    tick += WAVE_SPEED;

    for (const dot of dots) {
      const dx    = dot.x - mouse.x;
      const dy    = dot.y - mouse.y;
      const dist  = Math.sqrt(dx * dx + dy * dy);
      const near  = Math.max(0, 1 - dist / RIPPLE_R);

      // gentle wave offset
      const wave = Math.sin(dot.x * 0.018 + dot.y * 0.012 + tick * 6) * 0.5 + 0.5;

      const alpha  = 0.08 + wave * 0.1 + near * 0.45;
      const radius = DOT_R + near * 2.2;

      if (near > 0.01) {
        ctx.fillStyle = `rgba(${ACCENT_RGB}, ${Math.min(alpha, 0.7)})`;
      } else {
        ctx.fillStyle = `rgba(255, 255, 255, ${alpha * 0.6})`;
      }

      ctx.beginPath();
      ctx.arc(dot.x, dot.y, radius, 0, Math.PI * 2);
      ctx.fill();
    }

    requestAnimationFrame(draw);
  }

  window.addEventListener("resize", resize, { passive: true });
  window.addEventListener("mousemove", (e) => {
    mouse.x = e.clientX;
    mouse.y = e.clientY;
  }, { passive: true });

  resize();
  draw();
})();

/* ── Screenshot lightbox ── */
(function initLightbox() {
  const lightbox    = document.getElementById("lightbox");
  const lightboxImg = lightbox && lightbox.querySelector(".lightbox-img");
  const closeBtn    = lightbox && lightbox.querySelector(".lightbox-close");

  if (!lightbox || !lightboxImg) return;

  function openLightbox(src, alt) {
    lightboxImg.src = src;
    lightboxImg.alt = alt || "";
    lightbox.classList.add("is-open");
    document.body.style.overflow = "hidden";
  }

  function closeLightbox() {
    lightbox.classList.remove("is-open");
    document.body.style.overflow = "";
    lightboxImg.src = "";
  }

  // Attach to screenshot links
  document.querySelectorAll(".screenshot-link").forEach((link) => {
    link.addEventListener("click", (e) => {
      e.preventDefault();
      const img = link.querySelector("img");
      openLightbox(link.href, img ? img.alt : "");
    });
  });

  closeBtn && closeBtn.addEventListener("click", closeLightbox);
  lightbox.addEventListener("click", (e) => {
    if (e.target === lightbox) closeLightbox();
  });
  document.addEventListener("keydown", (e) => {
    if (e.key === "Escape" && lightbox.classList.contains("is-open")) closeLightbox();
  });
})();

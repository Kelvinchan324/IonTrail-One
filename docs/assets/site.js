const menuButton = document.querySelector('.menu-button');
const nav = document.querySelector('#main-nav');

menuButton?.addEventListener('click', () => {
  const open = nav.classList.toggle('open');
  menuButton.setAttribute('aria-expanded', String(open));
});

nav?.addEventListener('click', (event) => {
  if (event.target.matches('a')) {
    nav.classList.remove('open');
    menuButton?.setAttribute('aria-expanded', 'false');
  }
});

const values = [12, 16, 14, 22, 18, 27, 19, 15, 21, 17, 24, 18, 13, 20, 16, 25, 18, 14];
const spark = document.querySelector('#spark');
const readingValue = document.querySelector('#reading-value');
const readingUnit = document.querySelector('#reading-unit');
const modeButtons = document.querySelectorAll('[data-mode]');

function renderSpark(mode) {
  if (!spark) return;
  spark.replaceChildren(...values.map((value) => {
    const bar = document.createElement('i');
    bar.style.height = `${Math.max(9, mode === 'cps' ? value * 1.2 : value * 2.5)}px`;
    return bar;
  }));
}

modeButtons.forEach((button) => {
  button.addEventListener('click', () => {
    modeButtons.forEach((item) => item.classList.toggle('active', item === button));
    const mode = button.dataset.mode;
    readingValue.textContent = mode === 'cps' ? '0.3' : '18';
    readingUnit.textContent = mode.toUpperCase();
    renderSpark(mode);
  });
});

renderSpark('cpm');


   // === Animação inicial do portifólio ===
document.addEventListener('DOMContentLoaded', function () {
    const carousels = document.querySelectorAll('.carousel');
  
    carousels.forEach(carousel => {
      const imgs = carousel.querySelectorAll('img');
      let current = 0;
  
      function showNextImage() {
        imgs[current].classList.remove('active');
        current = (current + 1) % imgs.length;
        imgs[current].classList.add('active');
      }
  
      setInterval(showNextImage, 3000);
    });
  
    AOS.init({
      duration: 2000,
      once: true
    });
  });
   // === Botão flutuante "Voltar ao topo" ===
   const botaoTopo = document.createElement('button');
   botaoTopo.textContent = '⬆️';
   botaoTopo.style.position = 'fixed';
   botaoTopo.style.bottom = '20px';
   botaoTopo.style.right = '20px';
   botaoTopo.style.padding = '10px';
   botaoTopo.style.border = 'none';
   botaoTopo.style.borderRadius = '50%';
   botaoTopo.style.background = '#0077B5';
   botaoTopo.style.color = '#fff';
   botaoTopo.style.cursor = 'pointer';
   botaoTopo.style.display = 'none';
   botaoTopo.style.zIndex = '9999';
   document.body.appendChild(botaoTopo);
 
   botaoTopo.addEventListener('click', () => {
     window.scrollTo({ top: 0, behavior: 'smooth' });
   });
 
   window.addEventListener('scroll', () => {
     if (window.scrollY > 300) {
       botaoTopo.style.display = 'block';
     } else {
       botaoTopo.style.display = 'none';
     }
   });
  
// Wait for the DOM to be fully loaded
document.addEventListener('DOMContentLoaded', function() {
    // Create the background animation container
    createBackgroundAnimation();
    
    // Create the hero section background animation
    createHeroBackgroundAnimation();
    
    // Check for single items in grids and add appropriate classes
    checkSingleItems();
    
    // Setup education timeline click events
    setupEducationImageViewer();
    
    // Setup certificate card click events
    setupCertificateViewer();
    
    // Setup skill item click events
    setupSkillPercentagePopup();
    
    // Setup view all skills button
    setupViewAllSkills();
    
    // Mobile Navigation Toggle
    const hamburger = document.querySelector('.hamburger');
    const navLinks = document.querySelector('.nav-links');
    
    if (hamburger) {
        hamburger.addEventListener('click', function() {
            navLinks.classList.toggle('active');
            this.classList.toggle('active');
        });
    }
    
    // Close mobile menu when clicking on a link
    const navItems = document.querySelectorAll('.nav-links li a');
    navItems.forEach(item => {
        item.addEventListener('click', function() {
            if (navLinks.classList.contains('active')) {
                navLinks.classList.remove('active');
                hamburger.classList.remove('active');
            }
        });
    });
    
    // Smooth scrolling for navigation links
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function(e) {
            e.preventDefault();
            
            const targetId = this.getAttribute('href');
            if (targetId === '#') return;
            
            const targetElement = document.querySelector(targetId);
            if (targetElement) {
                window.scrollTo({
                    top: targetElement.offsetTop - 70,
                    behavior: 'smooth'
                });
            }
        });
    });
    
    // Add active class to navigation items on scroll
    window.addEventListener('scroll', function() {
        const scrollPosition = window.scrollY;
        
        // Add sticky header on scroll
        const header = document.querySelector('header');
        if (scrollPosition > 100) {
            header.classList.add('sticky');
        } else {
            header.classList.remove('sticky');
        }
        
        // Update active navigation item
        document.querySelectorAll('section').forEach(section => {
            const sectionTop = section.offsetTop - 100;
            const sectionHeight = section.offsetHeight;
            const sectionId = section.getAttribute('id');
            
            if (scrollPosition >= sectionTop && scrollPosition < sectionTop + sectionHeight) {
                document.querySelectorAll('.nav-links li a').forEach(link => {
                    link.classList.remove('active');
                    if (link.getAttribute('href') === '#' + sectionId) {
                        link.classList.add('active');
                    }
                });
            }
        });
    });
    
    // Initialize EmailJS
    emailjs.init("YOUR_PUBLIC_KEY"); // Replace with your actual EmailJS public key
    
    // Form Submission
    const contactForm = document.getElementById('contactForm');
    if (contactForm) {
        contactForm.addEventListener('submit', function(e) {
            e.preventDefault();
            
            // Get form data
            const formData = new FormData(this);
            const formValues = {};
            
            for (let [key, value] of formData.entries()) {
                formValues[key] = value;
            }
            
            // Show loading state
            const submitButton = this.querySelector('button[type="submit"]');
            const originalText = submitButton.innerHTML;
            submitButton.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Sending...';
            submitButton.disabled = true;
            
            // Send email using EmailJS
            emailjs.send('YOUR_SERVICE_ID', 'YOUR_TEMPLATE_ID', {
                from_name: formValues.name,
                from_email: formValues.email,
                subject: formValues.subject,
                message: formValues.message,
                to_email: 'tbrylle7@gmail.com' // Your email address
            })
            .then(function(response) {
                console.log('SUCCESS!', response.status, response.text);
                
                // Show success message
                const successMessage = document.createElement('div');
                successMessage.className = 'success-message';
                successMessage.innerHTML = '<p><i class="fas fa-check-circle"></i> Thank you for your message! I will get back to you soon.</p>';
                
                contactForm.innerHTML = '';
                contactForm.appendChild(successMessage);
            })
            .catch(function(error) {
                console.log('FAILED...', error);
                
                // Show error message
                const errorMessage = document.createElement('div');
                errorMessage.className = 'error-message';
                errorMessage.innerHTML = '<p><i class="fas fa-exclamation-circle"></i> Sorry, there was an error sending your message. Please try again or contact me directly at tbrylle7@gmail.com</p>';
                
                // Reset button
                submitButton.innerHTML = originalText;
                submitButton.disabled = false;
                
                // Insert error message before the form
                contactForm.parentNode.insertBefore(errorMessage, contactForm);
                
                // Remove error message after 5 seconds
                setTimeout(() => {
                    if (errorMessage.parentNode) {
                        errorMessage.parentNode.removeChild(errorMessage);
                    }
                }, 5000);
            });
        });
    }
    
    // Add reveal animations to sections
    const revealElements = document.querySelectorAll('.skill-category, .project-card, .timeline-content, .certification-card');
    
    const revealOnScroll = () => {
        revealElements.forEach(element => {
            const elementTop = element.getBoundingClientRect().top;
            const elementBottom = element.getBoundingClientRect().bottom;
            
            if (elementTop < window.innerHeight - 100 && elementBottom > 0) {
                element.classList.add('revealed');
            }
        });
    };
    
    // Initial check
    revealOnScroll();
    
    // Check on scroll
    window.addEventListener('scroll', revealOnScroll);
});

// Function to setup skill percentage popup
function setupSkillPercentagePopup() {
    const skillItems = document.querySelectorAll('.skill-item');
    const popup = document.querySelector('.skill-percentage-popup');
    const popupName = document.getElementById('skill-popup-name');
    const popupFill = document.querySelector('.popup-percentage-fill');
    const popupText = document.querySelector('.popup-percentage-text');
    const closePopup = document.querySelector('.close-popup');
    
    if (!popup || !popupName || !popupFill || !popupText) return;
    
    skillItems.forEach(item => {
        item.addEventListener('click', function() {
            const skill = this.getAttribute('data-skill');
            const percentage = this.getAttribute('data-percentage');
            
            popupName.textContent = skill;
            popupFill.style.width = '0';
            popupText.textContent = '0%';
            
            popup.classList.add('active');
            
            // Animate the percentage fill after a small delay
            setTimeout(() => {
                popupFill.style.width = percentage + '%';
                popupText.textContent = percentage + '%';
            }, 200);
        });
    });
    
    // Close popup when clicking the close button
    if (closePopup) {
        closePopup.addEventListener('click', function() {
            popup.classList.remove('active');
        });
    }
    
    // Close popup when clicking outside of it
    popup.addEventListener('click', function(e) {
        if (e.target === popup) {
            popup.classList.remove('active');
        }
    });
    
    // Close popup when pressing ESC key
    document.addEventListener('keydown', function(e) {
        if (e.key === 'Escape' && popup.classList.contains('active')) {
            popup.classList.remove('active');
        }
    });
}

// Function to check if grids have only one child and add a class
function checkSingleItems() {
    const projectsGrid = document.querySelector('.projects-grid');
    const certificationsGrid = document.querySelector('.certifications-grid');
    
    // Check projects grid
    if (projectsGrid) {
        const projectCards = projectsGrid.querySelectorAll('.project-card');
        if (projectCards.length === 1) {
            projectsGrid.classList.add('single-item');
        } else {
            projectsGrid.classList.remove('single-item');
        }
    }
    
    // Check certifications grid
    if (certificationsGrid) {
        const certCards = certificationsGrid.querySelectorAll('.certification-card');
        if (certCards.length === 1) {
            certificationsGrid.classList.add('single-item');
            console.log('Added single-item class to certifications grid');
        } else {
            certificationsGrid.classList.remove('single-item');
        }
    }
    
    // Set up a MutationObserver to watch for changes in the grids
    setupGridObservers();
}

// Set up observers to watch for changes in the project and certification grids
function setupGridObservers() {
    const projectsGrid = document.querySelector('.projects-grid');
    const certificationsGrid = document.querySelector('.certifications-grid');
    
    // Observer configuration
    const config = { childList: true };
    
    // Callback function to execute when mutations are observed
    const callback = function(mutationsList, observer) {
        for (const mutation of mutationsList) {
            if (mutation.type === 'childList') {
                const grid = mutation.target;
                
                // Check if this is the projects grid or certifications grid
                if (grid.classList.contains('projects-grid')) {
                    const projectCards = grid.querySelectorAll('.project-card');
                    if (projectCards.length === 1) {
                        grid.classList.add('single-item');
                    } else {
                        grid.classList.remove('single-item');
                    }
                } else if (grid.classList.contains('certifications-grid')) {
                    const certCards = grid.querySelectorAll('.certification-card');
                    if (certCards.length === 1) {
                        grid.classList.add('single-item');
                        console.log('Observer added single-item class to certifications grid');
                    } else {
                        grid.classList.remove('single-item');
                    }
                }
            }
        }
    };
    
    // Create observers
    const projectsObserver = new MutationObserver(callback);
    const certificationsObserver = new MutationObserver(callback);
    
    // Start observing
    if (projectsGrid) {
        projectsObserver.observe(projectsGrid, config);
    }
    
    if (certificationsGrid) {
        certificationsObserver.observe(certificationsGrid, config);
    }
}

// Function to create IT-themed background animation
function createBackgroundAnimation() {
    // Create background animation container
    const bgAnimation = document.createElement('div');
    bgAnimation.className = 'bg-animation';
    document.body.appendChild(bgAnimation);
    
    // IT-related icons and symbols
    const icons = ['<i class="fas fa-laptop-code"></i>', '<i class="fas fa-code"></i>', 
                  '<i class="fas fa-database"></i>', '<i class="fas fa-microchip"></i>',
                  '<i class="fas fa-wifi"></i>', '<i class="fas fa-server"></i>',
                  '<i class="fas fa-network-wired"></i>', '<i class="fas fa-cloud"></i>'];
    
    // Binary text elements
    const binaryTexts = ['01001001', '01010100', '10101010', '11001100', '00110011'];
    
    // Create floating particles
    for (let i = 0; i < 50; i++) {
        createParticle(bgAnimation);
    }
    
    // Create floating icons
    for (let i = 0; i < 15; i++) {
        createFloatingIcon(bgAnimation, icons);
    }
    
    // Create binary text elements
    for (let i = 0; i < 20; i++) {
        createBinaryText(bgAnimation, binaryTexts);
    }
}

// Function to create a single floating particle
function createParticle(container) {
    const particle = document.createElement('div');
    particle.className = 'floating-particle';
    
    // Random position
    const posX = Math.random() * 100;
    const posY = Math.random() * 100;
    
    // Random size
    const size = Math.random() * 3 + 1;
    
    // Random animation duration
    const duration = Math.random() * 20 + 10;
    const delay = Math.random() * 5;
    
    particle.style.left = `${posX}%`;
    particle.style.top = `${posY}%`;
    particle.style.width = `${size}px`;
    particle.style.height = `${size}px`;
    particle.style.animationDuration = `${duration}s`;
    particle.style.animationDelay = `${delay}s`;
    
    container.appendChild(particle);
}

// Function to create a floating icon
function createFloatingIcon(container, icons) {
    const iconElement = document.createElement('div');
    iconElement.className = 'floating-icon';
    
    // Random icon
    const randomIcon = icons[Math.floor(Math.random() * icons.length)];
    iconElement.innerHTML = randomIcon;
    
    // Random position
    const posX = Math.random() * 100;
    const posY = Math.random() * 100;
    
    // Random animation duration
    const duration = Math.random() * 30 + 20;
    const delay = Math.random() * 10;
    
    iconElement.style.left = `${posX}%`;
    iconElement.style.top = `${posY}%`;
    iconElement.style.animationDuration = `${duration}s`;
    iconElement.style.animationDelay = `${delay}s`;
    
    container.appendChild(iconElement);
}

// Function to create binary text elements
function createBinaryText(container, texts) {
    const textElement = document.createElement('div');
    textElement.className = 'binary';
    
    // Random text
    const randomText = texts[Math.floor(Math.random() * texts.length)];
    textElement.innerText = randomText;
    
    // Random position
    const posX = Math.random() * 100;
    const posY = Math.random() * 100;
    
    // Random rotation
    const rotation = Math.random() * 360;
    
    // Random animation duration
    const duration = Math.random() * 15 + 5;
    const delay = Math.random() * 8;
    
    textElement.style.left = `${posX}%`;
    textElement.style.top = `${posY}%`;
    textElement.style.transform = `rotate(${rotation}deg)`;
    textElement.style.animationDuration = `${duration}s`;
    textElement.style.animationDelay = `${delay}s`;
    
    container.appendChild(textElement);
} 

// Function to setup education timeline click events
function setupEducationImageViewer() {
    const timelineContents = document.querySelectorAll('.timeline-content');
    const imageContainer = document.querySelector('.education-image-container');
    const educationImage = document.getElementById('educationImage');
    const closeButton = document.querySelector('.close-button');
    
    if (!timelineContents.length || !imageContainer || !educationImage || !closeButton) return;
    
    // Add click event to each timeline content
    timelineContents.forEach(content => {
        content.addEventListener('click', function() {
            const imageName = this.getAttribute('data-image');
            if (imageName) {
                educationImage.src = `images/${imageName}`;
                educationImage.onload = function() {
                    imageContainer.classList.add('active');
                    document.body.style.overflow = 'hidden'; // Prevent scrolling
                };
                
                // If image fails to load
                educationImage.onerror = function() {
                    console.error('Failed to load image:', imageName);
                    alert('Image could not be loaded');
                };
            }
        });
    });
    
    // Close image when close button is clicked
    closeButton.addEventListener('click', function() {
        imageContainer.classList.remove('active');
        document.body.style.overflow = ''; // Re-enable scrolling
    });
    
    // Close image when clicking outside the image
    imageContainer.addEventListener('click', function(e) {
        if (e.target === imageContainer) {
            imageContainer.classList.remove('active');
            document.body.style.overflow = ''; // Re-enable scrolling
        }
    });
    
    // Close image when ESC key is pressed
    document.addEventListener('keydown', function(e) {
        if (e.key === 'Escape' && imageContainer.classList.contains('active')) {
            imageContainer.classList.remove('active');
            document.body.style.overflow = ''; // Re-enable scrolling
        }
    });
} 

// Function to setup certificate card click events
function setupCertificateViewer() {
    const certificationCards = document.querySelectorAll('.certification-card');
    
    if (!certificationCards.length) return;
    
    certificationCards.forEach(card => {
        card.addEventListener('click', function() {
            const pdfName = this.getAttribute('data-certificate');
            if (pdfName) {
                // Open the PDF in a new tab
                const pdfUrl = `certificates/${pdfName}`;
                window.open(pdfUrl, '_blank');
            }
        });
    });
} 

// Function to create hero section background animation with tech icons
function createHeroBackgroundAnimation() {
    const heroSection = document.querySelector('.hero');
    if (!heroSection) return;
    
    // Create animation container
    const animationContainer = document.createElement('div');
    animationContainer.className = 'hero-bg-animation';
    heroSection.appendChild(animationContainer);
    
    // Tech-related icons
    const techIcons = [
        'fa-code', 'fa-microchip', 'fa-laptop-code', 'fa-database', 
        'fa-wifi', 'fa-server', 'fa-network-wired', 'fa-sitemap',
        'fa-code-branch', 'fa-terminal', 'fa-cogs', 'fa-cloud',
        'fa-mobile-alt', 'fa-desktop', 'fa-keyboard', 'fa-project-diagram',
        'fa-memory', 'fa-usb', 'fa-shield-alt', 'fa-bug'
    ];
    
    // Create 30 floating icons (from top-right to bottom-left)
    for (let i = 0; i < 30; i++) {
        createFloatingTechIcon(animationContainer, techIcons, false);
    }
    
    // Create 30 floating icons (from bottom-left to top-right)
    for (let i = 0; i < 30; i++) {
        createFloatingTechIcon(animationContainer, techIcons, true);
    }
}

// Function to create a single floating tech icon
function createFloatingTechIcon(container, icons, isReverse = false) {
    const icon = document.createElement('i');
    
    // Randomly select an icon
    const randomIcon = icons[Math.floor(Math.random() * icons.length)];
    icon.className = `fas ${randomIcon} ${isReverse ? 'floating-tech-icon-reverse' : 'floating-tech-icon'}`;
    
    if (!isReverse) {
        // Top-right to bottom-left (original)
        // Random starting position (top right quadrant)
        const startX = 70 + Math.random() * 30; // 70-100%
        const startY = Math.random() * 30; // 0-30%
        
        // Set position
        icon.style.top = `${startY}%`;
        icon.style.right = `${100 - startX}%`;
    } else {
        // Bottom-left to top-right (new)
        // Random starting position (bottom left quadrant)
        const startX = Math.random() * 30; // 0-30%
        const startY = 70 + Math.random() * 30; // 70-100%
        
        // Set position
        icon.style.bottom = `${100 - startY}%`;
        icon.style.left = `${startX}%`;
    }
    
    // Random size (reverted to original)
    const size = 16 + Math.floor(Math.random() * 24); // 16-40px
    icon.style.fontSize = `${size}px`;
    
    // Random animation duration
    const duration = 10 + Math.random() * 20; // 10-30s
    icon.style.animationDuration = `${duration}s`;
    
    // Random delay
    const delay = Math.random() * 10; // 0-10s
    icon.style.animationDelay = `${delay}s`;
    
    // Add to container
    container.appendChild(icon);
    
    // Remove and recreate after animation completes
    setTimeout(() => {
        icon.remove();
        createFloatingTechIcon(container, icons, isReverse);
    }, (duration + delay) * 1000);
} 

// Function to setup view all skills buttons
function setupViewAllSkills() {
    const viewAllButtons = document.querySelectorAll('.view-all-skills');
    const categoryPopup = document.querySelector('.category-skills-popup');
    const categoryPopupTitle = document.querySelector('.category-popup-title');
    const categorySkillsList = document.querySelector('.category-skills-list');
    const closePopup = document.querySelector('.close-category-popup');
    
    if (!categoryPopup || !categoryPopupTitle || !categorySkillsList) return;
    
    // Category titles mapping
    const categoryTitles = {
        'programming': 'Programming Languages',
        'frameworks': 'Frameworks & Tools',
        'iot': 'IoT & Hardware'
    };
    
    viewAllButtons.forEach(button => {
        button.addEventListener('click', function() {
            const category = this.getAttribute('data-category');
            const categoryTitle = categoryTitles[category] || 'Skills';
            
            // Set the popup title
            categoryPopupTitle.textContent = categoryTitle;
            
            // Clear previous skills
            categorySkillsList.innerHTML = '';
            
            // Get all skills from this category
            const skillCategory = this.closest('.skill-category');
            const skillItems = skillCategory.querySelectorAll('.skill-item');
            
            // Add each skill to the popup
            skillItems.forEach(item => {
                const skillName = item.getAttribute('data-skill');
                const percentage = item.getAttribute('data-percentage');
                const iconClass = item.querySelector('.skill-icon i').className;
                
                // Create skill item element
                const skillElement = document.createElement('div');
                skillElement.className = 'category-skill-item';
                
                // Create skill name with icon
                const nameElement = document.createElement('div');
                nameElement.className = 'category-skill-name';
                
                // Create icon container
                const iconElement = document.createElement('div');
                iconElement.className = 'category-skill-icon';
                
                // Create icon
                const icon = document.createElement('i');
                icon.className = iconClass;
                iconElement.appendChild(icon);
                
                // Add icon and name to name element
                nameElement.appendChild(iconElement);
                nameElement.appendChild(document.createTextNode(skillName));
                
                // Create percentage element
                const percentageElement = document.createElement('div');
                percentageElement.className = 'category-skill-percentage';
                percentageElement.textContent = percentage + '%';
                
                // Add name and percentage to skill item
                skillElement.appendChild(nameElement);
                skillElement.appendChild(percentageElement);
                
                // Add skill item to list
                categorySkillsList.appendChild(skillElement);
            });
            
            // Show the popup
            categoryPopup.classList.add('active');
        });
    });
    
    // Close popup when clicking the close button
    if (closePopup) {
        closePopup.addEventListener('click', function() {
            categoryPopup.classList.remove('active');
        });
    }
    
    // Close popup when clicking outside of it
    categoryPopup.addEventListener('click', function(e) {
        if (e.target === categoryPopup) {
            categoryPopup.classList.remove('active');
        }
    });
    
    // Close popup when pressing ESC key
    document.addEventListener('keydown', function(e) {
        if (e.key === 'Escape' && categoryPopup.classList.contains('active')) {
            categoryPopup.classList.remove('active');
        }
    });
} 
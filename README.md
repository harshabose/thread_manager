# thread_manager
A versatile and simple C++ multi threading manager

Mutithread your programme with my robust and straightforward C++ Thread Manager. Tailored to handle engineering and mathematical operations, this header-only implementation seamlessly integrates into diverse projects.

**Motivation:** While working on CPU heavy engineering design tasks such as conceptual design of a UAV, often I encountered the need for more performance and speed on my laptop. Generic C++ <threads> library, while powerful, often burden users with heafty responsibilities related to thread safety and efficient memory management. Furthermore, off-the-self public multithreading libraries are often too heavy and require understanding of complex multithreading concepts and are mainly aimed for seasoned developers such as game-devs. Thus, I created a lightweight and simple header file to allow for more performance and speed in my code and not worry about complex multithreading concepts all the time. This file has been a game-changer for me, transforming the way I work and propelling me beyond what I had initially envisioned. Give it a try, experience the difference, and lemmino.

**Seamless Integration:**
No complexity here – a hassle-free header-only solution. Copy, paste, include, and experience the advantages of multithreading without unnecessary intricacies.

**Comprehensive Operation Support:**
The header file supports task injection including static and non-static methods, lambda functions, pure functions, and even recursive multithreading task pusher functions. This flexibility ensures your code aligns perfectly with your project's unique requirements. Furthurmore, specialisations for loop parallasation are also implemented for multithreading 'for' loops

Designed to facilitate both asynchronous and synchronous tasks, with or without return values allowing for more control over task order and achieve synchronization with ease.

**How to Use:**
- **Declaration:** Declare an object instance of the class in your code.
- **Core Configuration:** Define the number of cores explicitly or let the class determine the maximum multithreading configuration for your setup.
- **Single Task Submission:** Utilize the submit_task method to dispatch a single function that returns a result. The method returns a promise object, allowing for synchronisation and retrieval of the function's output at your convenience.
- Single Task Execution without Return: Employ the push_task method for scenarios where a function doesn't return a value. This is perfect for tasks where the focus is on execution rather than output.
- **For Loop Submission (with Return):** Leverage the submit_loop method to send a for loop encapsulated within a function that returns a result. This function returns a vector of promise objects, allowing you to manage and retrieve results seamlessly.
- **For Loop Execution without Return:** Opt for the push_loop method when dispatching a for loop wrapped within a function that doesn't return a value allowing for pure loop parallisation.
- **Additional Functionality:** Explore additional class methods crafted to enhance your experience. Redefine the number of cores in use, wait until all tasks in the queue are completed, and more.

**NOTE:** Some documentation work is left. std::cin.ignore that

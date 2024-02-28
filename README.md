# thread_manager
A versatile and simple C++ multi threading manager

Mutithread your programme with my robust and straightforward C++ Thread Manager. Tailored to handle engineering and mathematical operations, this header-only implementation seamlessly integrates into diverse projects.

**Motivation:** While working on CPU heavy engineering design tasks such as conceptual design of a UAV, often I encountered the need for more performance and speed on my laptop. Generic C++ <threads> library,while powerful, often burden users with intricate responsibilities related to thread safety and efficient memory management. Furthermore, off-the-self public multithreading libraries are often too heavy and require understanding of complex multithreading concepts and are mainly aimed for seasoned developers such as game-devs. Thus, I created a lightweight and simple header file to allow for more performance and speed in my code and not worry about complex multithreading concepts all the time. his file has been a game-changer for me, transforming the way I work and propelling me beyond what I had initially envisioned. Give it a try, experience the difference, and lemmino.

**Seamless Integration:**
No complexity here – a hassle-free header-only solution. Copy, paste, include, and experience the advantages of multithreading without unnecessary intricacies.

**Comprehensive Operation Support:**
The header file supports task injection including static and non-static methods, lambda functions, pure functions, and even recursive multithreading task pusher functions. This flexibility ensures your code aligns perfectly with your project's unique requirements. Furthurmore, specialisations for loop parallasation are also implemented for multithreading 'for' loops

Sequential Execution Focus:
Designed to facilitate both asynchronous and synchronous tasks, with or without return values, the Thread Manager ensures a sequential execution that meets the demands of your project. Maintain control over task order and achieve synchronization with ease.

**How to Use:**
- **Declaration:** Declare an object instance of the class, establishing the foundation for streamlined multithreading in your project.
- **Core Configuration:** Define the number of cores explicitly or let the class effortlessly determine the optimal configuration for you.
- **Single Task Submission:** Utilize the submit_task method to dispatch a single function that returns a result. The method returns a promise object, enabling you to synchronize and retrieve the function's output at your convenience.
- Single Task Execution without Return: Employ the push_task method for scenarios where a function doesn't return a value. This is perfect for tasks where the focus is on execution rather than output.
- **For Loop Submission (with Return):** Leverage the submit_loop method to send a for loop encapsulated within a function that returns a result. This function returns a vector of promise objects, allowing you to manage and retrieve results seamlessly.
- **For Loop Execution without Return:** Opt for the push_loop method when dispatching a for loop wrapped within a function that doesn't return a value. Efficiently execute tasks without the need for output handling.
- **Additional Functionality:** Explore additional class methods crafted to enhance your experience. Redefine the number of cores in use, wait until all tasks in the queue are completed, and more.

# thread_manager
A versatile and simple C++ multi threading manager

Mutithread your programme with my robust and straightforward C++ Thread Manager. Tailored to handle engineering and mathematical operations, this header-only implementation seamlessly integrates into diverse projects.

**Seamless Integration:**
No complexity here – a hassle-free header-only solution. Copy, paste, include, and experience the advantages of multithreading without unnecessary intricacies.

**Comprehensive Operation Support:**
Empower your project with versatile support for operations, including static and non-static methods, lambda functions, pure functions, and even recursive multithreading task pusher functions. This flexibility ensures your code aligns perfectly with your project's unique requirements.

Sequential Execution Focus:
Designed to facilitate both asynchronous and synchronous tasks, with or without return values, the Thread Manager ensures a sequential execution that meets the demands of your project. Maintain control over task order and achieve synchronization with ease.

**How to Use:**
- Declare an object instance of the class in your workspace.
- Define the number of cores or allow the class to find it for you.
- Use **submit_task** method to send single function which returns something. The **submit_task** returns a **promise** object which can be used to wait until the task is done and retreive the return of your function.
- Use **push_task** method to send single function which does does not return.
- use **submit_loop** to send a for loop wrapped within a function which returns something. The **submit_loop** returns a vector of **promise** objects which can be used to wait until the task is done and retreive the return of your.
- Use **push_loop** method to send a for loop wrapped within a function which does not return.
- Other class method include for your use including ability to redfine number of cores to use, wait untill all task in que are done etc.

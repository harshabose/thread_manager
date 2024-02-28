/**
 * @file thead_manager.h
 * @author Bose
 * @brief Multi-threading for C++
 * @version 1.0
 * @date 2024-01-04
 * 
 * @copyright Free to use
 */


#ifndef thread_manager_h
#define thread_manager_h

#include <iostream>
#include <functional>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <type_traits>
#include <future>
#include <condition_variable>
#include <string>

class thread_manager {
public:
    unsigned int numberOfThreads_ui;
    std::string affinity_s;
    bool syncAll_b, NOSLEEP_b;
    bool autoErrorCorrection_b;

    explicit thread_manager (const unsigned int thread_count_ui = 0) : numberOfThreads_ui(this->determine_max_hardware_threads(thread_count_ui)), affinity_s("effeciency"), syncAll_b(false), NOSLEEP_b(false), autoErrorCorrection_b(true), threads(std::make_unique<std::thread[]>(this->determine_max_hardware_threads(thread_count_ui))) {
        this->create_threads();
        std::cout << "Threads live.. waiting for tasks..." << std::endl;
    }

    ~thread_manager() {
        this->wait_for_all_tasks_in_que(true);
        this->kill_threads();
        std::cout << "Threads killed.. Thread Manager exiting..." << std::endl;
    }

    /**
     * @brief Push single task into task queue which returns a pointer to a promise which can be used to wait and get return values. If not return value is expected, return is void.
     * @tparam funcType type of function; can be static, non-static, method or function
     * @tparam argsType type of arguments. It is recommended to specify the types to maintain readability and unexpected behaviour
     * @tparam returnType type of return object. Automatically evaluated by default using std::invoke_result_t at compile-time
     * @param func function object
     * @param args arguments pack
     * @return Task future wrapped in a shared_ptr. It can be used to wait untill the task is complete. If return is not "void", the shared_pointer can be used to get the result of the func
    */
    template<class funcType, class... argsType, class returnType = std::invoke_result_t<std::decay_t<funcType>, std::decay_t<argsType>...>>
    [[nodiscard]] std::future<returnType>submit_task (funcType&& func, argsType&&... args) {
        std::shared_ptr<std::promise<returnType>> task_promise = std::make_shared<std::promise<returnType>>();
        auto current_task = [task = std::bind(std::forward<funcType>(func), std::forward<argsType>(args)...), task_promise] () {
            try {
                if constexpr (std::is_void_v<returnType>) {
                    std::invoke(task);
                    task_promise->set_value();
                } else task_promise->set_value(std::invoke(task));
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                std::cerr << "Setting the exception to the task_promise. It can be accessed using the future (return)" << std::endl;
                try {
                    task_promise->set_exception(std::current_exception());
                }
                catch (const std::exception& do_nothing){}
            }
        };

        this->push_task(current_task);
        return task_promise->get_future();
    }

    /**
     * @brief 
     * 
     * @tparam funcType 
     * @tparam argsType 
     * @param func 
     * @param args 
     */
    template<class funcType, class... argsType> void push_task (funcType&& func, argsType&&... args) {
        size_t retries = 3;
        while (retries > 0) {
            try {
                {
                    std::scoped_lock push_lock(this->task_mutex);
                    this->que.push(std::bind(std::forward<funcType>(func), std::forward<argsType>(args)...));
                }
                if (this->thread_is_waiting_at_cv_b) this->task_available_cv.notify_one();
                break;
            } catch (const std::bad_alloc& e) {     //scoped_lock might fail to allocate memory for the lock
                std::cerr << "Error: " << e.what() << std::endl;
                --retries;
                if (retries == 0) std::cout << "task not created..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                std:: cout << "re-trying to create task..." << std::endl;
            } catch (const std::logic_error& e) {   //if args are not copyable or movable
                std::cerr << "Error: " << e.what() << std::endl;
                std::cout << "task not created..." << std::endl;
            }
        }
    }

    /**
     * @brief Divides the loop into blocks. Each block is to execute a set of iterations of the loop.
     * Each block is sent to a separate thread, enabling parallelization of the loop. The loop range is
     * specified by the start and end indices. This method returns a vector of futures, allowing you to 
     * track and operate on the results of each parallelized block asynchronously with each other. 
     * This method also takes in arguments which can be send to each block. This method internally
     * uses a thread pool to manage parallel execution.
     * 
     * @tparam startIndexType The type of the loop start index.
     * @tparam endIndexType The type of the loop end index.
     * @tparam funcType The type of the function to be parallelized within the loop.
     * @tparam returnType The type of the result returned by the parallelized function.
     * @param startIndex The starting index of the loop.
     * @param endIndex The ending index of the loop.
     * @param func The function to be parallelized within the loop. It should take a start index and an end index as parameters.
     * @param args The arg set which contains the arguments to be passed to each block
     * @param num_blocks The number of blocks to divide the loop into. Defaults to the number of available hardware threads.
     * @return A vector of futures, each representing the result of a parallelized block of the loop.
     * 
     * @note func(size_t start, size_t end, args...)
     * @note optional variables wont work in the func. conflit between variadic args... and optional variables
     * @note num_blocks needs to explicitly declared. Stems from comflict with num_blocks(if optional) and variadic args...
     */
    template<typename startIndexType, typename endIndexType, class funcType, class... argsType, class returnType = std::invoke_result_t<std::decay_t<funcType>, startIndexType, endIndexType, std::decay_t<argsType>...>>
    [[nodiscard]] std::vector<std::future<returnType>> submit_loop_task (const startIndexType startIndex, const endIndexType endIndex, size_t num_blocks, funcType&& func, argsType&&... args) {
        num_blocks = (num_blocks > 0) ? num_blocks : static_cast<size_t>(this->numberOfThreads_ui);

        const std::vector<size_t> loops_per_block = get_loops_per_block<startIndexType, endIndexType>(startIndex, endIndex, num_blocks);
        std::vector<std::future<returnType>> future_vector;
        size_t currentStartIndex = 0, currentEndIndex = 0;
        for (size_t i = 0; i < loops_per_block.size(); i++) {
            currentStartIndex = (i == 0) ? startIndex + (currentEndIndex) : currentEndIndex;
            currentEndIndex = currentStartIndex + loops_per_block[i];
            future_vector.push_back(this->submit_task(std::forward<funcType>(func), currentStartIndex, currentEndIndex, std::forward<argsType>(args)...));
        }
        return future_vector;
    }

    /**
     * @brief Divides the loop into blocks. Each block is to execute a set of iterations of the loop.
     * Each block is sent to a separate thread, enabling parallelization of the loop. The loop range is
     * specified by the start and end indices. This method is pure asynchronous parallelisation. Each
     * blocks are excuted and no indication or control over the progress is given.
     * This method also takes in arguments which can be send to each block. If no arguments are required,
     * an overrided version exists. This method internally uses a thread pool to manage parallel execution.
     * 
     * @tparam startIndexType The type of the loop start index.
     * @tparam endIndexType The type of the loop end index.
     * @tparam funcType The type of the function to be parallelized within the loop.
     * @param startIndex The starting index of the loop.
     * @param endIndex The ending index of the loop.
     * @param func The function to be parallelized within the loop. It should take a start index and an end index as parameters.
     * @param args The arg set which contains the arguments to be passed to each block
     * @param num_blocks The number of blocks to divide the loop into. Defaults to the number of available hardware threads.
     */
    template<typename startIndexType, typename endIndexType, class funcType, class... argsType> void push_loop (const startIndexType startIndex, const endIndexType endIndex, size_t num_blocks, funcType&& func, argsType&&... args) {
        num_blocks = (num_blocks > 0) ? num_blocks : static_cast<size_t>(this->numberOfThreads_ui);

        const std::vector<size_t> loops_per_block = get_loops_per_block<startIndexType, endIndexType>(startIndex, endIndex, num_blocks);
        size_t currentStartIndex = 0, currentEndIndex = 0;
        for (size_t i = 0; i < loops_per_block.size(); i++) {
            currentStartIndex = (i == 0) ? startIndex + (currentEndIndex) : currentEndIndex;
            currentEndIndex = currentStartIndex + loops_per_block[i];
            this->push_task(std::forward<funcType>(func), currentStartIndex, currentEndIndex, std::forward<argsType>(args)...);
        }
    }


    /**
     * @brief 
     * 
     * @param report 
     */
    void wait_for_all_tasks_in_que (const bool report = false) {
        std::unique_lock wait_lock(this->task_mutex);
        this->waiting_to_finish_all_tasks_b = true;
        this->tasks_complete_cv.wait(wait_lock, [this]{return (this->number_of_tasks_running_t == 0) && this->que.empty();});
        this->waiting_to_finish_all_tasks_b = false;
        if (report) std::cout << "All tasks in queue are completed..." << std::endl;
    }

    /**
     * @brief 
     * 
     */
    void kill_threads () {
        {
            std::scoped_lock kill_lock(this->task_mutex);
            this->worker_working_b = false;
        }
        this->task_available_cv.notify_all();
        for (unsigned int i = 0; i < this->numberOfThreads_ui; i++) {
            if (this->threads[i].joinable()) this->threads[i].join();
        }
    }

    size_t get_number_of_threads () const {
        return this->que.size();
    }

    /**
     * @brief 
     * 
     * @return size_t 
     */
    size_t how_many_threads_running () const {
        std::scoped_lock report_lock(this->task_mutex);
        return this->number_of_tasks_running_t;
    }

    //aditional functionallities...
    //comments...

private:
    std::unique_ptr<std::thread[]> threads = nullptr;
    mutable  std::mutex task_mutex = {};                //declaring to unlock state
    std::queue<std::function<void()>> que = {};         //tasks queue
    std::condition_variable task_available_cv = {};
    std::condition_variable tasks_complete_cv = {};
    bool thread_is_waiting_at_cv_b = false;
    bool worker_working_b = false;
    bool waiting_to_finish_all_tasks_b = false;
    size_t number_of_tasks_running_t = 0;

    unsigned int determine_max_hardware_threads (const unsigned int thread_count_ui) const {
        if (thread_count_ui > 0) return thread_count_ui;

        size_t return_value;
        try {
            return_value = std::thread::hardware_concurrency();
            if (return_value <= 0) throw std::runtime_error("Cannot determine the hardware concurrent thread capability...");
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            if (this->autoErrorCorrection_b) {
                return_value = 1;
                std::cerr << "auto correction is ON, created threads with count: " << return_value << std::endl;
                std::cerr << "auto correction can be turned off while initialising the thread_manager object..." << std::endl;
            } else {
                bool valid_input = false;
                do {
                    std::cerr << "Enter the maximum thread count (positive integer): ";
                    std::cin >> return_value;

                    if (std::cin.fail()) {
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cerr << "Invalid input. Please enter a positive integer." << std::endl;
                    } else if (return_value <= 0) {
                        std::cerr << "Invalid input. Please enter a positive integer." << std::endl;
                    } else {
                        valid_input = true;
                    }
                } while (!valid_input);
            }
        }
        std::cout << return_value << " ";
        return static_cast<unsigned int>(return_value);
    }

    void create_threads () {
        {
            std::scoped_lock create_lock(this->task_mutex);
            this->worker_working_b = true;
        }

        for (unsigned int i = 0; i < this->numberOfThreads_ui; i++) this->threads[i] = std::thread(&thread_manager_h::thread_manager::worker, this);
    }

    void worker () {
        while (true) {
            {
                std::unique_lock worker_lock(this->task_mutex);
                    this->thread_is_waiting_at_cv_b = true;

                    this->task_available_cv.wait(worker_lock, [this]{return !this->que.empty() || !this->worker_working_b;});

                    if (!this->worker_working_b) break;
                    std::function<void()> task = std::move(this->que.front());
                    this->que.pop();
                    this->number_of_tasks_running_t++;
                worker_lock.unlock();

                task();
                worker_lock.lock();
                this->number_of_tasks_running_t--;

                if (this->waiting_to_finish_all_tasks_b && (this->number_of_tasks_running_t == 0) && this->que.empty()) this->tasks_complete_cv.notify_all();
            }
        }
    }

    /***
     * @brief Creates number of loops per block. Used by loop parallelising methods.
     * @tparam startIndexType (const) Start Index Type (usually an int or size_t)
     * @tparam endIndexType (const) End Index Type (usually an int or size_t)
     * @param startIndex Start Index of the loop which is to divided into blocks
     * @param endIndex End Index of the loop which is to divided into blocks
     * @param num_blocks Number of blocks to divide the loop into. (usually declared with max thread count)
     * @return Returns a vector of size_t with num_blocks elements with number of loops per block.
     * @throws std::invalid_argument If the input ranges are invalid (e.g., startIndex is negative, endIndex is less than startIndex, num_blocks is less than 1), or if the types are not valid for the calculation.
    */
    template<typename startIndexType, typename endIndexType> std::vector<size_t> get_loops_per_block(const startIndexType startIndex, const endIndexType endIndex, size_t& num_blocks) {
        if (!std::is_same_v<startIndexType, endIndexType> || !std::is_integral_v<startIndexType> || std::is_floating_point_v<startIndexType> || std::is_same_v<startIndexType, std::string>) {
            throw std::invalid_argument("Invalid input ranges for get_loops_per_block...");
        }
        if (startIndex < 0 || endIndex < startIndex || num_blocks < 1) {
            throw std::invalid_argument("Invalid input ranges for get_loops_per_block");
        }

        std::vector<size_t> loops_per_block(num_blocks, 0);

        for (size_t i = 0; i < static_cast<size_t>(endIndex - startIndex); ++i) {
            const size_t currentBlock = i % num_blocks;
            loops_per_block[currentBlock]++;
        }
        for (auto it = loops_per_block.begin(); it != loops_per_block.end();) {
            if (*it == 0) it = loops_per_block.erase(it);
            else ++it;
        }
        return loops_per_block;
    }
};

#endif

/*
    -> task submission                                                      (pass)
    -> Thread Pool Size Management                                          (pass)
    -> Task Execution and Interdependence                                   (pass)
    -> Task Completion Handling                                             (pass)
    -> Error Handling                                                       (work needed)
    -> Idle Thread Management                                               (pass)
    -> Graceful Shutdown                                                    (work needed)
    -> specifying thread affinity, stack size, or other thread attributes   (not possible)
    -> Compatibility                                                        (pass)

    *************************************************************************************

    -> test and validate                                                     (pass)

    *************************************************************************************
*/
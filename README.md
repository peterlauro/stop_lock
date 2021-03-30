# stop_lock
cancellable unique_lock (c++20) - only an exercise

## Overview
The class stop_lock is an excercise on top of C\++20 feature: co-operative thread cancellation.
The class enables to cancel the running *lock* operation on the synchronization entity (mutex) when it's owned by the other thread.
However, the underlying shared mutex object is not unlocked and remains in the locked state.
From this point of view the class belongs to the category: **Maybe a good servant, but a really bad master**.
My advice is **Don't use the class, if you are not sure what you do and there is no other way how to cancell the thread**.
Usually there is always better and more secure way to stop the locking operation (send stop request to the current mutex owner and cancel its thread function).
The usage of *stop_lock* type may hide the real reason of issue or even create a different kind of complication.

There is even OS's kernel which supports a cancellable mutex
or a c++ wrapper on top of OS's API functions:
-  [RIOT](https://riot-os.org/api/group__core__sync__mutex.html)
-  [WIN: An interruptible mutex class](https://www.codeproject.com/Articles/7911/An-interruptible-mutex-class)
Note: The RIOT's mutex_cancel represents the internal kernel feature used by other kernel functions mainly.

As I already mentioned, the implementation is only an excercise with unwanted side effects.
If you need to synchronize the locking/unlocking of shared mutex object between more [std::jthread](https://en.cppreference.com/w/cpp/thread/jthread)s,
please
-  consider the C\++20 enhanced [std::condition_variable_any](https://en.cppreference.com/w/cpp/thread/condition_variable_any),
-  refactor your design,
-  be sure that shared mutex object is unlocked.

Note: in my private implementation of (C++23) std\::executor (static_thread_pool) based on the std\::jthread I have found no place
real for *stop_lock* type.

### Type *stop_lock*
The public interface of stop_lock mirros the interface of [std::unique_lock](https://en.cppreference.com/w/cpp/thread/unique_lock) type
enhanced to detect whether the stop operation is signalized/possible on the shared stop_token object.
```c++
template<typename Mutex>
class stop_lock
{
public:
    using mutex_type = Mutex;

    stop_lock() noexcept;
    stop_lock(Mutex& mtx, std::stop_token st);
    stop_lock(Mutex& mtx, std::stop_token st, std::adopt_lock_t lock_t);
    stop_lock(Mutex& mtx, std::stop_token st, std::defer_lock_t lock_t) noexcept;
    stop_lock(Mutex& mtx, std::stop_token st, std::try_to_lock_t lock_t);

    template<typename Rep, typename Period>
    stop_lock(Mutex& mtx,
      std::stop_token st,
      const std::chrono::duration<Rep, Period>& timeout_duration);

    template<typename Clock, class Duration>
    stop_lock(Mutex& mtx,
      std::stop_token st,
      const std::chrono::time_point<Clock, Duration>& timeout_time);

    stop_lock(const stop_lock&) = delete;
    stop_lock& operator=(const stop_lock&) = delete;

    stop_lock(stop_lock&& other) noexcept = default;
    stop_lock& operator=(stop_lock&& other) noexcept = default;

    void lock();

    [[nodiscard]]
    bool try_lock();

    template<typename Rep, typename Period>
    [[nodiscard]]
    bool try_lock_for(const std::chrono::duration<Rep, Period>& timeout_duration);

    template<typename Clock, typename Duration>
    [[nodiscard]]
    bool try_lock_until(const std::chrono::time_point<Clock, Duration>& timeout_time);

    void unlock();

    void swap(stop_lock& other) noexcept;

    mutex_type* release() noexcept;

    [[nodiscard]]
    mutex_type* mutex() const noexcept;

    [[nodiscard]]
    bool stop_requested() const noexcept;

    [[nodiscard]]
    bool stop_possible() const noexcept;

    [[nodiscard]]
    bool owns_lock() const noexcept;

    explicit operator bool() const noexcept;
  };

  template<typename Mutex>
  void swap(stop_lock<Mutex>& lhs, stop_lock<Mutex>& rhs) noexcept;
```


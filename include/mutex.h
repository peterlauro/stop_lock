#pragma once

#ifndef STDX_MUTEX_H
#define STDX_MUTEX_H

#include <mutex>
#include <stop_token>
#include <thread>

namespace stdx
{
  template<typename Mutex>
  class stop_lock
  {
  public:
    using mutex_type = Mutex;

    /**
     * \brief Constructs a stop_lock with no associated mutex and stop_token
     */
    stop_lock() noexcept = default;

    stop_lock(Mutex& mtx, std::stop_token st)
      : m_lock(mtx, std::defer_lock)
      , m_token(st)
    {
      lock();
    }

    stop_lock(Mutex& mtx, std::stop_token st, std::adopt_lock_t lock_t)
      : m_lock(mtx, lock_t)
      , m_token(st)
    {
    }

    stop_lock(Mutex& mtx, std::stop_token st, std::defer_lock_t lock_t) noexcept
      : m_lock(mtx, lock_t)
      , m_token(st)
    {
    }

    stop_lock(Mutex& mtx, std::stop_token st, std::try_to_lock_t lock_t)
      : m_lock(mtx, lock_t)
      , m_token(st)
    {
    }

    template<typename Rep, typename Period>
    stop_lock(Mutex& mtx,
      std::stop_token st,
      const std::chrono::duration<Rep, Period>& timeout_duration)
      : stop_lock(mtx, std::move(st), std::chrono::steady_clock::now() + timeout_duration)
    {
    }

    template<typename Clock, class Duration>
    stop_lock(Mutex& mtx,
      std::stop_token st,
      const std::chrono::time_point<Clock, Duration>& timeout_time)
      : m_lock(mtx, std::defer_lock)
      , m_token(st)
    {
      while (timeout_time >= std::chrono::steady_clock::now()
        && !m_token.stop_requested()
        && !m_lock.try_lock())
      {
        std::this_thread::yield();
      }
    }

    stop_lock(const stop_lock&) = delete;
    stop_lock& operator=(const stop_lock&) = delete;

    stop_lock(stop_lock&& other) noexcept = default;
    stop_lock& operator=(stop_lock&& other) noexcept = default;

    void lock()
    {
      while (!m_token.stop_requested()
        && !m_lock.try_lock())
      {
        std::this_thread::yield();
      }
    }

    [[nodiscard]]
    bool try_lock()
    {
      bool rv = false;
      if (!stop_requested())
      {
        rv = m_lock.try_lock();
      }
      return rv;
    }

    template<typename Rep, typename Period>
    [[nodiscard]]
    bool try_lock_for(const std::chrono::duration<Rep, Period>& timeout_duration)
    {
      return try_lock_until(std::chrono::steady_clock::now() + timeout_duration);
    }

    template<typename Clock, typename Duration>
    [[nodiscard]]
    bool try_lock_until(const std::chrono::time_point<Clock, Duration>& timeout_time)
    {
      while (timeout_time >= std::chrono::steady_clock::now()
        && !m_token.stop_requested()
        && !m_lock.try_lock())
      {
        std::this_thread::yield();
      }
      return owns_lock();
    }

    void unlock()
    {
      m_lock.unlock();
    }

    void swap(stop_lock& other) noexcept
    {
      std::swap(m_lock, other.m_lock);
      std::swap(m_token, other.m_token);
    }

    mutex_type* release() noexcept
    {
      return m_lock.release();
    }

    [[nodiscard]]
    mutex_type* mutex() const noexcept
    {
      return m_lock.mutex();
    }

    [[nodiscard]]
    bool stop_requested() const noexcept
    {
      return m_token.stop_requested();
    }

    [[nodiscard]]
    bool stop_possible() const noexcept
    {
      return m_token.stop_possible();
    }

    [[nodiscard]]
    bool owns_lock() const noexcept
    {
      return m_lock.owns_lock();
    }

    explicit operator bool() const noexcept
    {
      return owns_lock();
    }

  private:
    std::unique_lock<Mutex> m_lock;
    std::stop_token m_token;
  };

  template<typename Mutex>
  void swap(stop_lock<Mutex>& lhs, stop_lock<Mutex>& rhs) noexcept
  {
    lhs.swap(rhs);
  }
}

#endif

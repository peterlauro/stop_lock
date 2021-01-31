//---------------------------------------------------------------------------------------
//                         Copyright (c) Carl Zeiss Meditec AG
//                               - All Rights Reserved -
//
//                     THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
//                                  CARL ZEISS MEDITEC AG
//
//                       This copyright notice does not evidence any
//                    actual or intended publication of such source code.
//---------------------------------------------------------------------------------------

#include <mutex.h>

//  GTEST
#include <gtest/gtest.h>

#include <future>
#include <thread>
#include <vector>

namespace CZM::test
{
  using mutex_type = std::mutex;
  using lock_type = stdx::stop_lock<mutex_type>;
  using clock_type = std::chrono::steady_clock;

  TEST(StdX_Mutex_stop_lock, cons_default)
  {
    lock_type lock;

    EXPECT_FALSE(lock.owns_lock());
    EXPECT_FALSE(static_cast<bool>(lock));
    EXPECT_FALSE(lock.stop_possible());
    EXPECT_FALSE(lock.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, cons_with_invalid_stop_state)
  {
    mutex_type m;
    std::stop_token stoken;
    lock_type lock(m, stoken);

    EXPECT_TRUE(lock.owns_lock());
    EXPECT_TRUE(static_cast<bool>(lock));
    EXPECT_FALSE(lock.stop_possible());
    EXPECT_FALSE(lock.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, cons_with_valid_stop_state)
  {
    mutex_type m;
    std::stop_source ssource;
    lock_type lock(m, ssource.get_token());

    EXPECT_TRUE(lock.owns_lock());
    EXPECT_TRUE(static_cast<bool>(lock));
    EXPECT_TRUE(lock.stop_possible());
    EXPECT_FALSE(lock.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, cons_with_invalid_stop_state_and_defer_lock)
  {
    mutex_type m;
    std::stop_token stoken;
    lock_type lock(m, stoken, std::defer_lock);

    EXPECT_FALSE(lock.owns_lock());
    EXPECT_FALSE(static_cast<bool>(lock));
    EXPECT_FALSE(lock.stop_possible());
    EXPECT_FALSE(lock.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, cons_with_valid_stop_state_and_defer_lock)
  {
    mutex_type m;
    std::stop_source ssource;
    lock_type lock(m, ssource.get_token(), std::defer_lock);

    EXPECT_FALSE(lock.owns_lock());
    EXPECT_FALSE(static_cast<bool>(lock));
    EXPECT_TRUE(lock.stop_possible());
    EXPECT_FALSE(lock.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, cons_with_invalid_stop_state_and_try_to_lock)
  {
    mutex_type m;
    std::stop_token stoken;
    lock_type lock(m, stoken, std::try_to_lock);

    EXPECT_TRUE(lock.owns_lock());
    EXPECT_TRUE(static_cast<bool>(lock));
    EXPECT_FALSE(lock.stop_possible());
    EXPECT_FALSE(lock.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, cons_with_valid_stop_state_and_try_to_lock)
  {
    mutex_type m;
    std::stop_source ssource;
    lock_type lock(m, ssource.get_token(), std::try_to_lock);

    EXPECT_TRUE(lock.owns_lock());
    EXPECT_TRUE(static_cast<bool>(lock));
    EXPECT_TRUE(lock.stop_possible());
    EXPECT_FALSE(lock.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, cons_with_invalid_stop_state_and_adopt_lock)
  {
    mutex_type m;
    std::unique_lock l1(m);
    std::stop_token stoken;
    lock_type l2(*l1.release(), stoken, std::adopt_lock);

    EXPECT_TRUE(l2.owns_lock());
    EXPECT_TRUE(static_cast<bool>(l2));
    EXPECT_FALSE(l2.stop_possible());
    EXPECT_FALSE(l2.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, cons_with_valid_stop_state_and_adopt_lock)
  {
    mutex_type m;
    std::unique_lock l1(m);
    std::stop_source ssource;
    lock_type l2(*l1.release(), ssource.get_token(), std::adopt_lock);

    EXPECT_TRUE(l2.owns_lock());
    EXPECT_TRUE(static_cast<bool>(l2));
    EXPECT_TRUE(l2.stop_possible());
    EXPECT_FALSE(l2.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, cons_with_invalid_stop_state_and_time_point)
  {
    using namespace std::chrono_literals;
    mutex_type m;
    std::stop_token stoken;
    const auto tpoint = clock_type::now() + 500ms;
    lock_type lock(m, stoken, tpoint);

    EXPECT_TRUE(lock.owns_lock());
    EXPECT_TRUE(static_cast<bool>(lock));
    EXPECT_FALSE(lock.stop_possible());
    EXPECT_FALSE(lock.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, cons_with_valid_stop_state_and_time_point)
  {
    using namespace std::chrono_literals;
    mutex_type m;
    std::stop_source ssource;
    const auto tpoint = clock_type::now() + 500ms;
    lock_type lock(m, ssource.get_token(), tpoint);

    EXPECT_TRUE(lock.owns_lock());
    EXPECT_TRUE(static_cast<bool>(lock));
    EXPECT_TRUE(lock.stop_possible());
    EXPECT_FALSE(lock.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, cons_with_valid_stop_state_and_timeout_and_unlocked_mutex)
  {
    using namespace std::chrono_literals;
    bool flag{ false };
    mutex_type m;
    std::stop_source ssource;
    //let acquire the mutex for 250ms
    std::thread t([&m, &flag]()
      {
        std::lock_guard l(m);
        std::this_thread::sleep_for(250ms);
        flag = true;
      });
    std::this_thread::sleep_for(10ms);
    lock_type lock(m, ssource.get_token(), 500ms);
    t.join();

    EXPECT_TRUE(flag);
    EXPECT_TRUE(lock.owns_lock());
    EXPECT_TRUE(static_cast<bool>(lock));
    EXPECT_TRUE(lock.stop_possible());
    EXPECT_FALSE(lock.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, cons_with_valid_stop_state_and_timeout_and_locked_mutex)
  {
    using namespace std::chrono_literals;
    bool flag{ false };
    mutex_type m;
    std::stop_source ssource;
    //let acquire the mutex for 500ms
    std::thread t([&m, &flag]()
      {
        std::lock_guard l(m);
        std::this_thread::sleep_for(500ms);
        flag = true;
      });
    std::this_thread::sleep_for(10ms);
    lock_type lock(m, ssource.get_token(), 250ms);
    t.join();

    EXPECT_TRUE(flag);
    EXPECT_FALSE(lock.owns_lock());
    EXPECT_FALSE(static_cast<bool>(lock));
    EXPECT_TRUE(lock.stop_possible());
    EXPECT_FALSE(lock.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, lock_unlock_on_defer_lock)
  {
    mutex_type m;
    std::stop_source ssource;
    lock_type l(m, ssource.get_token(), std::defer_lock);

    l.lock();

    EXPECT_TRUE(l.owns_lock());
    EXPECT_TRUE(static_cast<bool>(l));
    EXPECT_TRUE(l.stop_possible());
    EXPECT_FALSE(l.stop_requested());

    l.unlock();

    EXPECT_FALSE(l.owns_lock());
    EXPECT_FALSE(static_cast<bool>(l));
    EXPECT_TRUE(l.stop_possible());
    EXPECT_FALSE(l.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, lock_unlock_on_unaquired)
  {
    lock_type l;

    try
    {
      l.lock();
      EXPECT_TRUE(false);
    }
    catch (const std::system_error& ex)
    {
      EXPECT_EQ(ex.code(), std::make_error_code(std::errc::operation_not_permitted));
    }
    catch (...)
    {
      EXPECT_TRUE(false);
    }
  }

  TEST(StdX_Mutex_stop_lock, lock_unlock_on_locked)
  {
    mutex_type m;
    std::stop_source ssource;
    lock_type l(m, ssource.get_token());

    try
    {
      l.lock();
      EXPECT_TRUE(false);
    }
    catch (const std::system_error& ex)
    {
      EXPECT_EQ(ex.code(), std::make_error_code(std::errc::resource_deadlock_would_occur));
    }
    catch (...)
    {
      EXPECT_TRUE(false);
    }
  }

  TEST(StdX_Mutex_stop_lock, try_lock_for_on_defer_lock)
  {
    using namespace std::chrono_literals;
    mutex_type m;
    std::stop_source ssource;
    lock_type l(m, ssource.get_token(), std::defer_lock);

    EXPECT_TRUE(l.try_lock_for(200ms));
    EXPECT_TRUE(l.owns_lock());
  }

  TEST(StdX_Mutex_stop_lock, try_lock_until_on_defer_lock)
  {
    using namespace std::chrono_literals;
    mutex_type m;
    std::stop_source ssource;
    lock_type l(m, ssource.get_token(), std::defer_lock);

    EXPECT_TRUE(l.try_lock_until(clock_type::now() + 200ms));
    EXPECT_TRUE(l.owns_lock());
  }

  TEST(StdX_Mutex_stop_lock, swap)
  {
    mutex_type m;
    std::stop_source ssource;
    lock_type l1(m, ssource.get_token());
    lock_type l2;

    EXPECT_TRUE(l1.owns_lock());
    EXPECT_TRUE(l1.stop_possible());
    EXPECT_FALSE(l1.stop_requested());
    EXPECT_FALSE(l2.owns_lock());
    EXPECT_FALSE(l2.stop_possible());
    EXPECT_FALSE(l2.stop_requested());

    ssource.request_stop();
    EXPECT_TRUE(l1.stop_requested());

    l1.swap(l2);

    EXPECT_TRUE(l2.owns_lock());
    EXPECT_TRUE(l2.stop_possible());
    EXPECT_TRUE(l2.stop_requested());
    EXPECT_FALSE(l1.owns_lock());
    EXPECT_FALSE(l1.stop_possible());
    EXPECT_FALSE(l1.stop_requested());
  }

  TEST(StdX_Mutex_stop_lock, cancel_in_jthread)
  {
    using namespace std::chrono_literals;
    mutex_type m;
    bool stopped = false;
    std::unique_lock lock(m);
    std::jthread t([&m, &stopped](std::stop_token stoken)
      {
        EXPECT_TRUE(stoken.stop_possible());
        lock_type l(m, stoken);
        EXPECT_FALSE(l.owns_lock());
        if (l.stop_requested())
        {
          stopped = true;
        }
        else
        {
          EXPECT_TRUE(false);
        }
      });
    std::this_thread::sleep_for(10ms);
    EXPECT_TRUE(t.request_stop());
    t.join();
    EXPECT_TRUE(stopped);
  }

  TEST(StdX_Mutex_stop_lock, cancel_in_jthread_with_stop_callback)
  {
    using namespace std::chrono_literals;
    mutex_type m;
    bool cbCalled = false;
    std::unique_lock lock(m);
    std::jthread t([&m, &cbCalled](std::stop_token stoken)
      {
        EXPECT_TRUE(stoken.stop_possible());
        std::stop_callback cb(stoken, [&cbCalled] { cbCalled = true; });
        lock_type l(m, stoken);
        EXPECT_FALSE(l.owns_lock());
      });
    std::this_thread::sleep_for(10ms);
    EXPECT_TRUE(t.request_stop());
    t.join();
    EXPECT_TRUE(cbCalled);
  }

  TEST(StdX_Mutex_stop_lock, incorrect_use_of_mutext)
  {
    const std::size_t num_threads = std::jthread::hardware_concurrency();
    mutex_type m;
    std::vector<std::promise<int>> promises(num_threads);
    std::vector<std::future<int>> futures;
    for (std::size_t i = 0U; i < num_threads; ++i)
    {
      futures.emplace_back(promises.at(i).get_future());
    }
    std::vector<std::jthread> threads;
    threads.reserve(num_threads);

    auto thread_func = [&m](std::stop_token stoken, std::promise<int> p)
    {
      EXPECT_TRUE(stoken.stop_possible());
      // the artificial case - don't do that in the real multithreading code.
      // let break the concept of mutex holding (hold the mutext only during "short required" time)
      // and hold the mutext during whole life of thread execution function
      lock_type l(m, stoken);
      if (l.stop_requested())
      {
        p.set_value(1);
      }
      else
      {
        while (true)
        {
          if (stoken.stop_requested())
          {
            p.set_value(0);
            break;
          }
        }
      }
    };

    for(std::size_t i = 0U; i < num_threads; ++i)
    {
      threads.emplace_back(thread_func, std::move(promises.at(i)));
    }

    for(auto& t: threads)
    {
      EXPECT_TRUE(t.request_stop());
    }

    for(auto& t : threads)
    {
      t.join();
    }

    std::size_t lock_cancelled{ 0U };
    for(auto& f : futures)
    {
      lock_cancelled += f.get();
    }

    EXPECT_GT(lock_cancelled, 0U);
    EXPECT_LT(lock_cancelled, num_threads);
  }

  TEST(StdX_Mutex_stop_lock, deadlock_cancellation)
  {
    using namespace std::chrono_literals;

    mutex_type m;
    auto f1 = [&m](std::stop_token stoken)
    {
      EXPECT_TRUE(stoken.stop_possible());
      lock_type l(m, stoken); //deadlock; mutex was already acquired
      EXPECT_TRUE(stoken.stop_possible());
      EXPECT_TRUE(stoken.stop_requested());
    };

    std::jthread t([&m, &f1](std::stop_token stoken)
    {
      EXPECT_TRUE(stoken.stop_possible());
      lock_type l(m, stoken); //aquire mutex
      f1(stoken);
      EXPECT_TRUE(stoken.stop_requested());
    });

    std::this_thread::sleep_for(200ms);

    EXPECT_TRUE(t.request_stop());

    t.join();
  }
}

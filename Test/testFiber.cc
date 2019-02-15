/// \file testFiber.cc Demonstration of boost::fiber
// Michael P. Mendenhall, 2019

#include <stdlib.h>
#include <stdio.h>
#include <string>
using std::string;
//#include <chrono> // boost includes get this
#include <cassert>

#include <boost/fiber/all.hpp>
using boost::fibers::fiber;

/// callable object
struct callable {
    void operator()() {
        while(j--) {
            printf("Callable called!\n");
            boost::this_fiber::yield();
        }
    }
    int j = 3;
};

/// function with arguments
void countdown(const string& s, unsigned int n) {
    while(n--) {
        printf("\t%s: %i\n", s.c_str(), n);
        boost::this_fiber::yield();
    }
}

int main(int, char**) {

    // optional: set scheduler. This is the default:
    boost::fibers::use_scheduling_algorithm< boost::fibers::algo::round_robin >();

    // get current context
    auto ctx = boost::fibers::context::active();
    assert(ctx);
    // get current scheduler
    auto sched = ctx->get_scheduler();
    assert(sched);

    // by default fiber is added to scheduler (may start running) on construction
    // fiber is move but not copy-assignable
    callable x;
    fiber f1(x); // gets a copy of x, unless std::ref(x) specified --- dangerous if x might get deleted.

    // constructor with function and arguments.
    // anonymous fiber must immediately be detached from local object that will be destructed;
    // keeps running in scheduler
    fiber(countdown, "hello", 6).detach();

    // constructor with launch policy, function, args
    // start running immediately (default is boost::fibers::launch::post)
    // might be first that actually runs in example
    fiber f3(boost::fibers::launch::dispatch, countdown, "there", 6);

    // optionally use boost::bind to bundle function, args... need to learn *why*
    fiber f4(countdown, "world", 7);
    f4.detach(); // detach from object 'f4'; keeps running on its own.

    boost::fibers::condition_variable cond;
    boost::fibers::mutex mtx;

    // with a lambda, capturing by reference
    string foo = "foo";
    auto ff5 = [&foo, &cond, &mtx](unsigned int j, const string& s) {
        while(j--) {
            printf("%i\t%s\n", j, foo.c_str());
            {
                // don't need to lock if fibers using 'foo' running in same thread
                std::unique_lock<boost::fibers::mutex> lk(mtx);
                foo += s;
            } // release lk
            cond.notify_one();
            boost::this_fiber::sleep_for(std::chrono::milliseconds(50*j));
            boost::this_fiber::yield();
        }
    };
    fiber f5(ff5, 4, ".");
    fiber f6(ff5, 7, "-");

    // use condition variable to wait for correct state
    auto waitfoo = [&foo, &cond, &mtx]() {
        {
            std::unique_lock<boost::fibers::mutex> lk(mtx);
            while(foo.size() < 9) cond.wait(lk);
        } // release lk

        printf("HEY HEY HEY\n");
    };
    fiber f7(waitfoo);

    // wait for fibers to finish
    // abort called if destructed without join() or detach()
    f1.join();
    f3.join();
    //f4.join(); // NOPE! it's detached

    // don't detach() fibers referencing objects that will be destroyed!
    // wait for them to complete before exiting scope.
    f5.join();
    f6.join();
    f7.join();

    return EXIT_SUCCESS;
}

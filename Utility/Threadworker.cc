/// \file Threadworker.cc

#include "Threadworker.hh"

void Threadworker::threadjob() {
    while(true) {
        unique_lock<mutex> lk(inputMut);  // acquire unique_lock on queue in this scope
        inputReady.wait(lk, [this]{return all_done;});  // unlock until notified
        if(all_done) break;
    }
}

void Threadworker::launch_mythread() {
    if(is_launched) throw std::logic_error("double launch attempted");
    is_launched = true;
    auto rc = pthread_create(&mythread, nullptr, run_Threadworker_thread, this);
    if(rc) throw rc;
}

void Threadworker::finish_mythread() {
    if(!is_launched) throw std::logic_error("attempt to finish un-begun work");
    all_done = true;
    {
        lock_guard<mutex> lk(inputMut);
        inputReady.notify_one(); // notification to catch all_done
    }
    int rc = pthread_join(mythread, NULL);
    is_launched = false;
    all_done = false;
    if(rc) throw rc;
}

void* Threadworker::run_Threadworker_thread(void* p) {
    static_cast<Threadworker*>(p)->threadjob();
    return nullptr;
}

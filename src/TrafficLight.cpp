#include <iostream>
#include <random>
#include <thread>
#include <mutex>
#include <future>
#include <queue>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> ulock(_mtx);
    _cond.wait(ulock, [this] { return !_queue.empty(); });

    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> ulock(_mtx);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    msg_queue = std::shared_ptr<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    // Add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while(true)
    {
        _currentPhase = msg_queue->receive();
        if (_currentPhase == TrafficLightPhase::green)
        {
            return; 
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    auto _preTime = std::chrono::high_resolution_clock::now();

    while(true)
    {
        auto _currTime = std::chrono::high_resolution_clock::now();
        auto _elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(_currTime - _preTime);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if((_elapsedTime >= std::chrono::seconds(4)) && (_elapsedTime <=std::chrono::seconds(6)))
        {
            if (_currentPhase == TrafficLightPhase::green)
                _currentPhase = TrafficLightPhase::red;
            else
                _currentPhase = TrafficLightPhase::green;


            auto msg = _currentPhase;
            msg_queue->send(std::move(_currentPhase));
            //auto isSend = std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, msg_queue, std::move(msg));
            //isSend.wait();

            _preTime = _currTime;

        }
        else
        {
            continue;
        }
    
    }
}


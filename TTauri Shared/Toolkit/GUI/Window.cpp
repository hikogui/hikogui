//
//  Window.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Window.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

using namespace std;

void Window::buildSwapChainAndPipeline(void)
{
    boost::upgrade_lock<boost::shared_mutex> lock(stateMutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

    if (state == WindowState::LINKED_TO_DEVICE) {
        // XXX setup swap chain.
        state = WindowState::READY_TO_DRAW;
    } else {
        BOOST_THROW_EXCEPTION(WindowStateError());
    }
}

void Window::teardownSwapChainAndPipeline(void)
{
    boost::upgrade_lock<boost::shared_mutex> lock(stateMutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

    if (state == WindowState::READY_TO_DRAW) {
        // XXX teardown swap chain.
        state = WindowState::LINKED_TO_DEVICE;
    } else {
        BOOST_THROW_EXCEPTION(WindowStateError());
    }
}

void Window::setDevice(Device *device) {
    if (device) {
        teardownSwapChainAndPipeline();

        {
            boost::upgrade_lock<boost::shared_mutex> lock(stateMutex);
            boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
            if (state == WindowState::LINKED_TO_DEVICE) {
                this->device = nullptr;
                state = WindowState::NO_DEVICE;
            } else {
                BOOST_THROW_EXCEPTION(WindowStateError());
            }
        }

    } else {
        {
            boost::upgrade_lock<boost::shared_mutex> lock(stateMutex);
            boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
            if (state == WindowState::NO_DEVICE) {
                this->device = device;
                state = WindowState::LINKED_TO_DEVICE;
            } else {
                BOOST_THROW_EXCEPTION(WindowStateError());
            }
        }

        buildSwapChainAndPipeline();
    }
}

void Window::frameUpdate(uint64_t nowTimestamp, uint64_t outputTimestamp)
{
    if (stateMutex.try_lock_shared()) {
        if (state == WindowState::READY_TO_DRAW) {

        }
        stateMutex.unlock_shared();
    }
}

}}}

// Copyright 2019 Pokitec
// All rights reserved.

#include "Application.hpp"
#include "logging.hpp"

namespace TTauri {

using namespace std;


void Application_base::initialize(std::shared_ptr<ApplicationDelegate> applicationDelegate) {
    initializeLogging();
    LOG_INFO("Starting application.");

    clockCalibrationThreadID = std::thread(Application::clockCalibrationThread);

    delegate = std::move(applicationDelegate);
}

Application_base::~Application_base()
{
    stopClockCalibration = true;
    clockCalibrationThreadID.join();
}

void Application_base::startingLoop()
{
    if (!loopStarted) {
        loopStarted = true;
        delegate->startingLoop();
    }
}

void Application_base::lastWindowClosed()
{
    delegate->lastWindowClosed();
}

void Application_base::clockCalibrationThread()
{
#ifdef _WIN32
    SetThreadDescription(GetCurrentThread(), L"ClockCalibration");
#endif

    LOG_INFO("Starting clock calibration thread.");

    size_t count = 0;
    while (!get_singleton<Application>().stopClockCalibration) {
        if ((count < 20) || (count % 10 == 0 && count < 80) || (count % 60 == 0)) {
            let clockDiff = Application::clock::checkCalibration();
            LOG_INFO("Clock calibration off by %i ns", clockDiff.count());

            Application::clock::calibrate();
        }

        // Don't wait too long, the application needs to join before exiting.
        std::this_thread::sleep_for(1s);
        count++;
    }
}

}

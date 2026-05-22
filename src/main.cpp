#include <atomic>
#include <chrono>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

#include "force_sensor/ForceSensorReader.h"

static std::atomic<bool> g_running{true};

namespace {

void handleSignal(int) {
  g_running = false;
}

}  // namespace

int main(int argc, char** argv) {
  std::signal(SIGINT, handleSignal);
  std::signal(SIGTERM, handleSignal);

  const std::string device = (argc >= 2 ? argv[1] : "/dev/ftSensorSerial");
  ForceSensorReader reader;
  if (!reader.start(device, 115200)) {
    std::cerr << "failed to start sensor reader for " << device << '\n';
    return 1;
  }

  std::cout << "reading force sensor from " << device << '\n';
  std::cout << "columns: Fx Fy Fz Mx My Mz" << '\n';

  while (g_running && reader.isRunning()) {
    const auto reading = reader.latestReading();
    if (reading) {
      std::cout << std::fixed << std::setprecision(4)
                << reading->F[0] << ' '
                << reading->F[1] << ' '
                << reading->F[2] << ' '
                << reading->M[0] << ' '
                << reading->M[1] << ' '
                << reading->M[2] << '\n';
    } else {
      std::cout << "waiting for sensor data..." << '\n';
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  reader.stop();
  return 0;
}

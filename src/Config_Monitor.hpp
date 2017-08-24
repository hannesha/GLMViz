#pragma once

#include <atomic>
#include <string>

void monitor(const std::string&, std::atomic<bool>&, std::atomic<bool>&);

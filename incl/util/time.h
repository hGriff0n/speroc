#pragma once

#include <chrono>

namespace spero::util {
	using TimePoint = std::chrono::system_clock::time_point;

	struct TimeData {
		TimePoint start, end;
		std::chrono::duration<double> time;
	};

	struct Timer {
		TimeData& ref;

		inline Timer(TimeData& ref) : ref{ ref } {
			ref.start = std::chrono::system_clock::now();
		}
		inline ~Timer() {
			ref.end = std::chrono::system_clock::now();
			ref.time = ref.end - ref.start;
		}
	};
}
#pragma once
#include "string"
#include "algorithm"
#include <windows.h>
#undef min
#undef max

// Returns given in string lowercase
inline std::string ToLower(std::string str) {
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
		return std::tolower(c);
		});
	return str;
}

// Tries to get a numeric value from a string and set it to a suitable range
// Returns false if attempt failed
template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline bool TrySetNumFromStr(T& variable, const std::string& value) {
	try {
		double doubleValue = std::stod(value);
		if constexpr (std::is_floating_point_v<T>) {
			// Use `lowest()` for floating-point types
			variable = std::clamp(doubleValue, (double)std::numeric_limits<T>::lowest(), (double)std::numeric_limits<T>::max());
		}
		else {
			// Use `min()` for integer types
			variable = std::clamp(doubleValue, (double)std::numeric_limits<T>::min(), (double)std::numeric_limits<T>::max());
		}
		return true;
	}
	catch (...) {
		return false;
	}
}

// Parses words from a given string according to given delimiter (. by default) and returns the words in a vector
inline std::vector<std::string> ParseWords(const std::string& str, char delimiter = '.') {
	std::stringstream ss(str);
	std::string word;
	std::vector<std::string> words;

	while (std::getline(ss, word, delimiter)) {
		words.push_back(ToLower(word));
	}
	return words;
}
#pragma once

#include <COLA.hh>

namespace cola {
	template <typename ConverterType>
	class CCLConverterFactory final : public VConverterFactory {
		public:
		std::unique_ptr<VFilter> Create(const std::unordered_map<std::string, std::string>& meta_data) {
			return ConverterType::ParseData(meta_data);
		}

		const std::string& GetFilterName() const override {
			return ConverterType::kName;
		}
	};

}


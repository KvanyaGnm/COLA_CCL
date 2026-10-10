#pragma once

#include <COLA.hh>
#include <libccl.h>



namespace cola {

	class MSTConverter final : public VConverter {
	public:
		static inline const std::string kName = "CCLMST";
		std::unique_ptr<EventData> operator()(std::unique_ptr<EventData>&& data) override;
		static std::unique_ptr<VConverter> ParseData(const std::unordered_map<std::string, std::string>& meta_data);
	private:
		CClusterizer cls_;
	};

	class SAConverter final : public VConverter {
	public:
		static inline const std::string kName = "CCLSA";
		std::unique_ptr<EventData> operator()(std::unique_ptr<EventData>&& data) override;
		static std::unique_ptr<VConverter> ParseData(const std::unordered_map<std::string, std::string>& meta_data);
	private:
		CClusterizer cls_;
	};

	class chainSAConverter final : public VConverter {
	public:
		static inline const std::string kName = "CCLchainSA";
		std::unique_ptr<EventData> operator()(std::unique_ptr<EventData>&& data) override;
		static std::unique_ptr<VConverter> ParseData(const std::unordered_map<std::string, std::string>& meta_data);
	private:
		CClusterizer cls_;
	};

	class CoalConverter final : public VConverter {
	public:
		static inline const std::string kName = "CCLCoalescence";
		std::unique_ptr<EventData> operator()(std::unique_ptr<EventData>&& data) override;
		static std::unique_ptr<VConverter> ParseData(const std::unordered_map<std::string, std::string>& meta_data);
	private:
		CClusterizer cls_;
		struct CoalLayer {
			double radius = 4.0;
			double momentum = 4.0;
			ParticleType type = ParticleType::Deuteron;
		};
	    float  prob_ = 1.0;
	    std::vector<CoalLayer> layers;
	};
}
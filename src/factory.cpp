#include "filter.hh"
#include "factory.hh"
#include <sstream>


namespace {

	double findDval (const char* name, const std::unordered_map<std::string, std::string>& params, double def = 0.0) {
		if (const auto it = params.find(name); it != params.end()){
			return std::stod(it->second);
		}
		return def;
	}

	bool findBval (const char* name, const std::unordered_map<std::string, std::string>& params, bool def = false) {
		if (const auto it = params.find(name); it != params.end()){
			return it->second == "true";
		}
		return def;
	}

	int findIval (const char* name, const std::unordered_map<std::string, std::string>& params, int def = 0) {
		if (const auto it = params.find(name); it != params.end()){
			return std::stoi(it->second);
		}
		return def;
	}

	SkyrmeFormula DefaultSkyrme() { 
		return [](const SkyrmeInput& in) {
	        double A = 0.0, B = 0.0, C = 0.0;
	        for (double v : in.pair_density) {
	            A += v;
	            B += std::pow(v, in.gamma);
	            C += v * std::log(1 + v);
	        }
	        return in.alpha0 * A + in.beta0 * B + 0.01 * C;
	    };
	}
	
	void SetEnergyParams(const std::unordered_map<std::string, std::string>& params) {
		CClusterizer::setEc2(findDval("Ec2", params, 0.00144));  // GeV * fm
		CClusterizer::setAl(findDval("Al", params, 3.0));  // fm^2
		CClusterizer::setSal(findDval("Sal", params, 5.0));  // fm^2
		CClusterizer::setAlphaBetaGamma(
			findDval("Alpha", params, -0.15), // GeV
			findDval("Beta", params, 0.05), // GeV
			findDval("Gamma", params, 1.5)
		);
		CClusterizer::setHyPot(findDval("HyPot", params, 1.0));
		CClusterizer::setYuk0(findDval("Yuk0", params, 0.0));  // GeV
		CClusterizer::setGamYuk(findDval("GamYuk", params, 1.4));  // fm

		CClusterizer::setComputeSkyrme(findBval("ComputeSkyrme", params, true));
		CClusterizer::setComputeCoul(findBval("ComputeCoul", params, false));
		CClusterizer::setSmearedCoulomb(findBval("SmearedCoulomb", params, false));
		CClusterizer::setComputeEasy(findBval("ComputeEasy", params, false));
		CClusterizer::setComputeYukawa(findBval("ComputeYukawa", params, false));

	}

	void SetSAParams(const std::unordered_map<std::string, std::string>& params, CClusterizer& cls) {
		cls.setSATmax(findDval("SATmax", params, 1.0));
		cls.setSATmin(findDval("SATmin", params, 1e-5));
		cls.setSACool(findDval("SACool", params, 0.95));
		cls.setSASteps(findDval("SASteps", params, 500));
		cls.setSAPnew(findDval("SAPnew", params, 0.25));
		cls.setSAPnewMin(findDval("SAPnewMin", params, 0.25));
		cls.setMHCriterion(findBval("MHCriterion", params, false));
		cls.setSAEbindBound(findDval("SAEbindBound", params, -0.004));
		cls.setSAStagMin(findIval("SAStagMin", params, 10));
		cls.setSAStagDenom(findIval("SAStagDenom", params, 10));
		cls.setSASteps2(findIval("SASteps2", params, -1));

	}

	void SetMSTParams(const std::unordered_map<std::string, std::string>& params, CClusterizer& cls) {
		cls.setRadius(findDval("radius", params));
		cls.setMomentum(findDval("momentum", params));
		cls.setCheckMomentum(findBval("checkMomentum", params));
    	cls.setComputeEbind(findBval("computEbind", params));
    	cls.setTrueTimeMST(findBval("trueTimeMST", params));
    	cls.setCheckPairMomentum(findBval("checkPairMomentum", params));
    	cls.setSkyrmeFormula(DefaultSkyrme());
	}

	std::vector<double> parseDArray(const std::unordered_map<std::string, std::string>& params, const char* name) {
		if (const auto it = params.find(name); it != params.end()){
			std::vector<double> result;
		    std::istringstream ss(it->second);
		    std::string item;
		    while (std::getline(ss, item, '/')) {
		        if (!item.empty()) {
		            result.push_back(std::stod(item));
		        }
		    }
		    return result;
		}
		return std::vector<double>(6, 4.0);
	    
	}

	ParticleType getType(const std::string& name) {
	    static const std::unordered_map<std::string, ParticleType> table = {
	        {"Unset", ParticleType::Unset},
	        {"Unknown",   ParticleType::Unknown},
	        {"Neutron",  ParticleType::Neutron},
	        {"Proton",      ParticleType::Proton},   
	        {"LambdaSigma",    ParticleType::LambdaSigma},
	        {"Deuteron",      ParticleType::Deuteron},
	        {"Triton",      ParticleType::Triton},
	        {"Helium3",      ParticleType::Helium3},
	        {"Alpha",      ParticleType::Alpha},
	        {"H3L",      ParticleType::H3L},
	        {"H4L",      ParticleType::H4L},
	        {"MSTNucleus",      ParticleType::MSTNucleus},
	    };
	    if (const auto it = table.find(name); it != table.end()){
			return it->second;
		}
		return ParticleType::Deuteron;
	}

	std::vector<ParticleType> ParseParticleTypes(const std::unordered_map<std::string, std::string>& params, const char* name) {
		if (const auto it = params.find(name); it != params.end()){
		    std::vector<ParticleType> res;
		    std::istringstream ss(it->second);
		    std::string token;
		    while (std::getline(ss, token, '/')) {
		        if (!token.empty())
		            res.push_back(getType(token));
		    }
		    return res;
		}

		return std::vector<ParticleType> {
			ParticleType::H4L,
			ParticleType::Alpha,
			ParticleType::H3L,
			ParticleType::Triton,
			ParticleType::Helium3,
			ParticleType::Deuteron
		};
	    
	}

} // anonymous namespace

namespace cola {
	std::unique_ptr<VConverter> MSTConverter::ParseData(const std::unordered_map<std::string, std::string>& meta_data) {

		auto conv = std::make_unique<MSTConverter>();
		SetEnergyParams(meta_data);
		SetMSTParams(meta_data, conv->cls_);
    	return conv;
	}

	std::unique_ptr<VConverter> SAConverter::ParseData(const std::unordered_map<std::string, std::string>& meta_data) {
		auto conv = std::make_unique<SAConverter>();
		SetEnergyParams(meta_data);
		SetMSTParams(meta_data, conv->cls_);
		SetSAParams(meta_data, conv->cls_);
		return conv;

	}

	std::unique_ptr<VConverter> chainSAConverter::ParseData(const std::unordered_map<std::string, std::string>& meta_data) {
		auto conv = std::make_unique<chainSAConverter>();
		SetEnergyParams(meta_data);
		SetMSTParams(meta_data, conv->cls_);
		SetSAParams(meta_data, conv->cls_);
		return conv;
	}

	std::unique_ptr<VConverter> CoalConverter::ParseData(const std::unordered_map<std::string, std::string>& meta_data) {
		auto conv = std::make_unique<CoalConverter>();
		SetEnergyParams(meta_data);
		SetMSTParams(meta_data, conv->cls_);
		conv->cls_.setClsUseEbind(findBval("ClsUseEbind", meta_data, false));
		conv->cls_.setClsUseIsoEbind(findBval("ClsUseIsoEbind", meta_data, false));

		conv->prob_ = static_cast<float>(findDval("probability", meta_data, 1.0));
		std::vector<double> radius = parseDArray(meta_data, "CoalRadius");
		std::vector<double> momentum = parseDArray(meta_data, "CoalMomentum");
		std::vector<ParticleType> types = ParseParticleTypes(meta_data, "ParticleType");

		for (size_t i = 0; i < types.size(); i++) {
			conv->layers.push_back({radius[i], momentum[i], types[i]});
		}
		return conv;

	}


}


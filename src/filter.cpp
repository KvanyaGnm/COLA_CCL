#include "filter.hh"


namespace {
	std::vector<CCParticle> ColaToCCL(const cola::EventParticles& Particles, std::vector<cola::ParticleClass>& classes_before){
		int uid = 0;
		const auto n = Particles.size();
   		std::vector<CCParticle> res;
    	res.reserve(n);
    	classes_before.clear();
    	classes_before.reserve(n);

		for (const auto& part : Particles) {
			CCParticle& cclParticle = res.emplace_back();
			cclParticle.setPosition(
				part.position.x,
				part.position.y,
				part.position.z,
				part.position.t
			);
			cclParticle.setMomentum(
				part.momentum.x,
				part.momentum.y,
				part.momentum.z,
				std::sqrt(std::max(0.0, part.momentum.Mag2()))
			);
			cclParticle.setPID(part.pdg_code);
			cclParticle.setUniqueID(uid++);
			classes_before.push_back(part.p_class);
		}
		return res;
	}

	cola::EventParticles CCLToCola(const std::vector<CCParticle>& Particles, const std::vector<cola::ParticleClass>& classes_after){
		cola::EventParticles res;
		res.reserve(Particles.size());
		cola::ParticleClass partClass;
		for (const auto& part : Particles) {
			const int cid = part.getClusterID();
			if (cid != -1) {
				partClass = cola::ParticleClass::kProduced;
			} else {
				const int uid = part.getUniqueID();
				if (uid >= 0 && static_cast<size_t>(classes_after.size()) > static_cast<size_t>(uid)) {
					partClass = classes_after[uid];
				} else {
					partClass = cola::ParticleClass::kProduced;
				}
			}

			res.push_back(cola::Particle{
			        .position = cola::LorentzVector{
			        	.e = part.getTime(),
			        	.x = part.getX(),
			        	.y = part.getY(),
			        	.z = part.getZ()
			        },
			        .momentum =
			            cola::LorentzVector{
			                .e = part.getE(),
			                .x = part.getPx(),
			                .y = part.getPy(),
			                .z = part.getPz(),
			            },
			        .pdg_code = static_cast<int>(part.getPID()),
			        .p_class = partClass
		    	}
	    	);
		}
		return res;
	}
}

namespace cola {

	std::unique_ptr<EventData> MSTConverter::operator()(std::unique_ptr<EventData>&& data) {
		std::vector<cola::ParticleClass> classes;
		std::vector<CCParticle> particles_after_mst = cls_.makeMST(ColaToCCL(data->particles, classes));
		data->particles = CCLToCola(particles_after_mst, classes);
		return std::move(data);
	}

	std::unique_ptr<EventData> SAConverter::operator()(std::unique_ptr<EventData>&& data) {
		std::vector<cola::ParticleClass> classes;
		std::vector<CCParticle> particles_after_mst = cls_.makeMST(ColaToCCL(data->particles, classes));
		data->particles = CCLToCola(cls_.makeSA(particles_after_mst), classes);
		return std::move(data);
	}

	std::unique_ptr<EventData> chainSAConverter::operator()(std::unique_ptr<EventData>&& data) {
		std::vector<cola::ParticleClass> classes;
		std::vector<CCParticle> particles_after_mst = cls_.makeMST(ColaToCCL(data->particles, classes));
		data->particles = CCLToCola(cls_.makeSAchain(particles_after_mst), classes);
		return std::move(data);
	}

	std::unique_ptr<EventData> CoalConverter::operator()(std::unique_ptr<EventData>&& data) {
		std::vector<cola::ParticleClass> classes;
		std::vector<CCParticle> particles_to_handle = ColaToCCL(data->particles, classes);
		for (const auto& layer : layers) {
			cls_.makeCoalescence(particles_to_handle, layer.type, layer.radius, layer.momentum, cls_.getCheckMomentum(), prob_);
		}
		data->particles = CCLToCola(particles_to_handle, classes);
		return std::move(data);
	}
}


#include "filter.hh"
#include "sMSTStorage.hh"
#include <unordered_set>

namespace {
	std::vector<CCParticle> ColaToCCL(const cola::EventParticles& Particles, std::vector<cola::ParticleClass>& classes_before, bool track = false){
		int uid = 0;
		const auto n = Particles.size();
   		std::vector<CCParticle> res;
    	res.reserve(n);
    	classes_before.clear();
    	classes_before.reserve(n);


		for (const auto& part : Particles) {
			CCParticle& cclParticle = res.emplace_back();

			cclParticle.setPosition(
				part.position.x, // fm
				part.position.y,
				part.position.z,
				track ? 0.0 : part.position.t
			);
			cclParticle.setMomentum(
				part.momentum.x / 1000, // from MeV to GeV
				part.momentum.y / 1000,
				part.momentum.z / 1000,
				std::sqrt(std::max(0.0, part.momentum.Mag2())) / 1000
			);

			if (track) cclParticle.setCollisionTime(part.position.t);
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
			                .e = part.getE() * 1000.0, // to GeV
			                .x = part.getPx() * 1000.0,
			                .y = part.getPy() * 1000.0,
			                .z = part.getPz() * 1000.0,
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

	std::unique_ptr<EventData> sMSTConverter::operator()(std::unique_ptr<EventData>&& data) {
		Stable& instance = Stable::get_stable();
		std::vector<CCParticle>& stable_clusters = instance.get_clusters();
		double& time = instance.get_time(); // needs to be thought trough
		std::vector<cola::ParticleClass> classes;
		std::vector<CCParticle> cola_parts = ColaToCCL(data->particles, classes, true);
		std::vector<CCParticle> particles_after_mst = cls_.makeMST(cola_parts);
		cls_.updateStableVector(particles_after_mst, static_cast<float>(time), stable_clusters);

		std::unordered_set<int> stable_uids;
		for (const auto& cluster : stable_clusters) {
			for (const auto& child : cluster.getChilds()) {
				stable_uids.insert(child.getUniqueID());
			}
		}

		std::vector<CCParticle> not_stable_parts;
		std::copy_if(cola_parts.begin(), cola_parts.end(), std::back_inserter(not_stable_parts), [&stable_uids](const CCParticle& b) {
        	int id = b.getUniqueID();
			return stable_uids.find(id) == stable_uids.end();
    	});

		std::vector<CCParticle> second_burn = cls_.makeMST(not_stable_parts);
		for (const auto& particle : second_burn) {
			if (static_cast<short>(particle.getType()) < 3) continue;
			double ebind = particle.getBindingEnergy();
			if (ebind < 0) {
				bool time_check = true;
				for (const auto& child : particle.getChilds()) {
					if (child.getCollisionTime() > time) {
                        time_check = false;
                        break;
					}
				}
                if (time_check) {
                    stable_clusters.push_back(particle);
				}
			}
		}

		if (last_in_chain) {
			data->particles = CCLToCola(stable_clusters, classes);
			instance.clearIfNewEvent();
		}
		return std::move(data);
	}	

	
} // cola namespace


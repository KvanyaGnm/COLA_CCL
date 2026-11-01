#include <sMSTStorage.hh>

Stable& Stable::get_stable() {
    thread_local Stable instance;
    return instance;
}

std::vector<CCParticle>& Stable::get_clusters() {
    return clusters;
}

double& Stable::get_time() {
    return time;
}

void Stable::clearIfNewEvent() {
    clusters.clear();
    time = 0.0;
}
    

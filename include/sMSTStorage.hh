#pragma once

#include<libccl.h>

class Stable {
    private:
    Stable() = default;
    std::vector<CCParticle> clusters;
    double time = 0.0;

    public:
    static Stable& get_stable();
    std::vector<CCParticle>& get_clusters();
    double& get_time();
    void clearIfNewEvent();
    
};
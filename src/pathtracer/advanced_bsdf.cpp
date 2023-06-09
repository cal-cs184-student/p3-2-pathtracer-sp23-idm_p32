#include "bsdf.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <utility>

#include "application/visual_debugger.h"

using std::max;
using std::min;
using std::swap;

namespace CGL {

// Mirror BSDF //

    Vector3D MirrorBSDF::f(const Vector3D wo, const Vector3D wi) {
        return Vector3D();
    }

    Vector3D MirrorBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {

        // TODO Project 3-2: Part 1
        // Implement MirrorBSDF
        *pdf = 1.0f;
        reflect(wo, wi);
        return reflectance / abs_cos_theta(*wi);
    }

    void MirrorBSDF::render_debugger_node()
    {
        if (ImGui::TreeNode(this, "Mirror BSDF"))
        {
            DragDouble3("Reflectance", &reflectance[0], 0.005);
            ImGui::TreePop();
        }
    }

// Microfacet BSDF //

    double MicrofacetBSDF::G(const Vector3D wo, const Vector3D wi) {
        return 1.0 / (1.0 + Lambda(wi) + Lambda(wo));
    }

    double MicrofacetBSDF::D(const Vector3D h) {
        // TODO Project 3-2: Part 2
        // Compute Beckmann normal distribution function (NDF) here.
        // You will need the roughness alpha.
        double tan_theta = sin_theta(h) / cos_theta(h);
        double up = exp(-1 * tan_theta * tan_theta / (alpha * alpha));
        double down = PI * alpha * alpha * pow(cos_theta(h), 4);
        return up / down;
    }

    Vector3D MicrofacetBSDF::F(const Vector3D wi) {
        // TODO Project 3-2: Part 2
        // Compute Fresnel term for reflection on dielectric-conductor interface.
        // You will need both eta and etaK, both of which are Vector3D.
        Vector3D temp = eta * eta + k * k;
        Vector3D Rs = (temp - 2 * eta * cos_theta(wi) + cos_theta(wi) * cos_theta(wi)) / (temp + 2 * eta * cos_theta(wi) + cos_theta(wi) * cos_theta(wi));
        Vector3D Rp = (temp * cos_theta(wi) * cos_theta(wi) - 2 * eta * cos_theta(wi) + 1.0) / (temp * cos_theta(wi) * cos_theta(wi) + 2 * eta * cos_theta(wi) + 1.0);
        return (Rs + Rp) / 2.0;
    }

    Vector3D MicrofacetBSDF::f(const Vector3D wo, const Vector3D wi) {
        // TODO Project 3-2: Part 2
        // Implement microfacet model here.
        Vector3D f;
        // half vector
        Vector3D h = (wo + wi).unit();
        // normal vector
        Vector3D n(0, 0, 1);
        if (dot(n, wo) < 0 || dot(n, wi) < 0) {
            return Vector3D();
        }
        f = F(wi) * G(wo, wi) * D(h) / (4 * dot(n, wo) * dot(n, wi));
        return f;
    }

    Vector3D MicrofacetBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {
        // TODO Project 3-2: Part 2
        // *Importance* sample Beckmann normal distribution function (NDF) here.
        // Note: You should fill in the sampled direction *wi and the corresponding *pdf,
        //       and return the sampled BRDF value.
        /*
        Vector2D temp = sampler.get_sample();
        double r1 = temp[0];
        double r2 = temp[1];
        double Phih = 2 * PI * r2;
        double thetah = atan(sqrt(-1 * alpha * alpha * log(1.0 - r1)));
        Vector3D h(sin(thetah) * sin(Phih), sin(thetah) * cos(Phih), cos(thetah));
        *wi = ((2 * h * dot(h, wo)) - wo).unit();
        if (wi->z <= 0) {
            *pdf = 0;
            return Vector3D();
        }
        double p_thetah = 2 * sin(thetah) / (alpha * alpha * pow(cos(thetah), 3)) * exp(-1 * tan(thetah) * tan(thetah) / (alpha * alpha));
        double p_phih = 1.0 / (2 * PI);
        double pwh = p_thetah * p_phih / sin(thetah);
        *pdf = pwh / (4 * dot(*wi, h));*/
        *wi = cosineHemisphereSampler.get_sample(pdf);
        return MicrofacetBSDF::f(wo, *wi);
    }

    void MicrofacetBSDF::render_debugger_node()
    {
        if (ImGui::TreeNode(this, "Micofacet BSDF"))
        {
            DragDouble3("eta", &eta[0], 0.005);
            DragDouble3("K", &k[0], 0.005);
            DragDouble("alpha", &alpha, 0.005);
            ImGui::TreePop();
        }
    }

// Refraction BSDF //

    Vector3D RefractionBSDF::f(const Vector3D wo, const Vector3D wi) {
        return Vector3D();
    }

    Vector3D RefractionBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {
        // TODO Project 3-2: Part 1
        // Implement RefractionBSDF
        if (!refract(wo, wi, ior)) {
            return Vector3D();
        }
        else {
            *pdf = 1.0f;
            float index = 0.0f;
            if (wo.z >= 0)
            {
                index = 1.0 / ior;
            }
            else {
                index = ior;
            }
            return (transmittance / abs_cos_theta(*wi)) / (index * index);
        }
    }

    void RefractionBSDF::render_debugger_node()
    {
        if (ImGui::TreeNode(this, "Refraction BSDF"))
        {
            DragDouble3("Transmittance", &transmittance[0], 0.005);
            DragDouble("ior", &ior, 0.005);
            ImGui::TreePop();
        }
    }

// Glass BSDF //

    Vector3D GlassBSDF::f(const Vector3D wo, const Vector3D wi) {
        return Vector3D();
    }

    Vector3D GlassBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {

        // TODO Project 3-2: Part 1
        // Compute Fresnel coefficient and either reflect or refract based on it.

        // compute Fresnel coefficient and use it as the probability of reflection
        // - Fundamentals of Computer Graphics page 305
        if (!refract(wo, wi, ior)) {
            *pdf = 1.0f;
            reflect(wo, wi);
            return reflectance / abs_cos_theta(*wi);
        }
        else {
            float R0 = ((1 - ior) / (1 + ior)) * ((1 - ior) / (1 + ior));
            float R = R0 + (1 - R0) * pow((1 - abs_cos_theta(wo)), 5);
            if (coin_flip(R)) {
                *pdf = R;
                reflect(wo, wi);
                return R * reflectance / abs_cos_theta(*wi);
            }
            else {
                *pdf = 1 - R;
                refract(wo, wi, ior);
                float index = 0.0f;
                if (wo.z >= 0){
                    index = 1.0 / ior;
                }else {
                    index = ior;
                }
                return (1 - R) * (transmittance / abs_cos_theta(*wi)) / (index * index);
            }
        }
    }

    void GlassBSDF::render_debugger_node()
    {
        if (ImGui::TreeNode(this, "Refraction BSDF"))
        {
            DragDouble3("Reflectance", &reflectance[0], 0.005);
            DragDouble3("Transmittance", &transmittance[0], 0.005);
            DragDouble("ior", &ior, 0.005);
            ImGui::TreePop();
        }
    }

    void BSDF::reflect(const Vector3D wo, Vector3D* wi) {

        // TODO Project 3-2: Part 1
        // Implement reflection of wo about normal (0,0,1) and store result in wi.
        Vector3D there = Vector3D(-wo.x, -wo.y, wo.z);
        *wi = there;

    }

    bool BSDF::refract(const Vector3D wo, Vector3D* wi, double ior) {

        // TODO Project 3-2: Part 1
        // Use Snell's Law to refract wo surface and store result ray in wi.
        // Return false if refraction does not occur due to total internal reflection
        // and true otherwise. When dot(wo,n) is positive, then wo corresponds to a
        // ray entering the surface through vacuum.
        float index = 0.0f;
        if (wo.z >= 0)
        {
            index = 1.0 / ior;
        }
        else {
            index = ior;
        }
        if ((1 - index * index * (1 - wo.z * wo.z)) < 0) {
            return false;
        }
        wi->x = -index * wo.x;
        wi->y = -index * wo.y;
        if (wo.z > 0) {
            wi->z = -sqrt((1 - index * index * (1 - wo.z * wo.z)));
        }
        else {
            wi->z = sqrt((1 - index * index * (1 - wo.z * wo.z)));
        }
        return true;

    }

} // namespace CGL

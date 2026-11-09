#pragma once

#include <filesystem>
#include <vector>

class EmissivityModel {
public:
    virtual ~EmissivityModel() = default;
    virtual double getEmissivity(double lambda_nm, double temp_K) const = 0;
};

class IdealBlackbodyModel final : public EmissivityModel {
public:
    double getEmissivity(double lambda_nm, double temp_K) const override;
};

class ConstantEmissivityModel final : public EmissivityModel {
public:
    explicit ConstantEmissivityModel(double emissivity);
    double getEmissivity(double lambda_nm, double temp_K) const override;

private:
    double emissivity_;
};

struct EmissivityPoint {
    double wavelength_nm;
    double emissivity;
};

class TabulatedEmissivityModel final : public EmissivityModel {
public:
    static TabulatedEmissivityModel loadFromCSV(
        const std::filesystem::path& filePath);

    double getEmissivity(double lambda_nm, double temp_K) const override;

private:
    explicit TabulatedEmissivityModel(std::vector<EmissivityPoint> points);
    std::vector<EmissivityPoint> points_;
};

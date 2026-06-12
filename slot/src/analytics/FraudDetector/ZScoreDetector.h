#pragma once

#include <cmath>
#include <vector>

/// Z-score anomaly detector for streaming numeric data.
///
/// Computes the Z-score of each new value against the mean and standard
/// deviation of the reference window.  Values whose |Z| exceeds the
/// threshold are flagged as anomalous.
///
/// ── Complexity ──────────────────────────────────────────────────────
///   train       O(n)  — compute mean and stdev once.
///   analyze     O(k)  — Z-score against stored stats.
///   Space       O(1)  — stores only mean/stdev/count.
class ZScoreDetector {
public:
    ZScoreDetector(double threshold = 3.0)
        : threshold_(threshold), mean_(0.0), stdev_(0.0), trained_(false) {}

    /// Train on a sample population to establish baseline mean/stdev.
    void train(const std::vector<double>& samples) {
        if (samples.empty()) return;
        double sum = 0.0;
        for (double v : samples) sum += v;
        mean_ = sum / samples.size();

        double sqSum = 0.0;
        for (double v : samples) {
            double d = v - mean_;
            sqSum += d * d;
        }
        stdev_ = std::sqrt(sqSum / samples.size());
        trained_ = true;
    }

    /// Compute Z-score of a single value against the trained baseline.
    double zScore(double value) const {
        if (!trained_ || stdev_ == 0.0) return 0.0;
        return (value - mean_) / stdev_;
    }

    /// Check if a value is anomalous (|Z| > threshold).
    bool isAnomalous(double value) const {
        return std::abs(zScore(value)) > threshold_;
    }

    /// Return anomaly score normalised to [0, 1].
    double anomalyScore(double value) const {
        double z = std::abs(zScore(value));
        if (z <= threshold_) return 0.0;
        return std::min(1.0, (z - threshold_) / 3.0);
    }

    double mean() const { return mean_; }
    double stdev() const { return stdev_; }
    bool isTrained() const { return trained_; }
    void setThreshold(double t) { threshold_ = t; }

private:
    double threshold_;
    double mean_;
    double stdev_;
    bool trained_;
};

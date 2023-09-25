// Some statistics functions
//
// Copyright HOLM, 2023

#pragma once

#include <numeric>
#include <string>
#include <limits>
#include <cmath>
#include <algorithm>
#include <vector>

// =================================================================================================
namespace chaos
{
    namespace math
    {
        namespace statistics
        {
            // =====================================================================================
            template <class T>
            static double max_element(T& c)
            {
                auto it = std::max_element(c.begin(), c.end());
                if(it != c.end())
                    return (*it);
                else
                    return 999999999.999999;
            }

            // =====================================================================================
            template <class T>
            static double min_element(T& c)
            {
                auto it = std::min_element(c.begin(), c.end());
                if(it != c.end())
                    return (*it);
                else
                    return -999999999.999999;
            }

            // =====================================================================================
            static double get_sign(double v)
            {
                return (v > 0) ? 1 : ((v < 0) ? -1 : 0);
            }

            // =====================================================================================
            static bool is_double_equal(double v1, double v2, double epsilon = std::numeric_limits<double>::epsilon())
            {
                // We are checking to 7 decimal places
                return (std::fabs(v1 - v2) <= epsilon);
            }

            // =====================================================================================
            template <class T>
            static double mean_v1(T& c)
            {
                double sum = 0;
                for(auto it=c.begin(); it!=c.end(); ++it)
                    sum += (*it);

                return (sum / c.size());
            }

            // =====================================================================================
            template <class T>
            static double mean_v2(T& c)
            {
                return (std::accumulate(c.begin(), c.end(), 0.0) / c.size());
            }

            // =====================================================================================
            template <class T>
            static double mean_v3(T& c, std::size_t n)
            {
                double sum = 0;
                for(std::size_t i=0; i<n; ++i)
                    sum += c[i];

                return (sum / n);
            }

            // =====================================================================================
            template <class T>
            static double mean(T& c)
            {
                return mean_v2(c);
            }

            // =====================================================================================
            template <class T>
            static double mean(T& c, std::size_t n)
            {
                return mean_v3(c, n);
            }

            // =====================================================================================
            template <class T>
            static double stdev_v1(T& c)
            {
                // Calculates stdev for a population
                double m = mean(c);
                double sq_sum = std::inner_product(c.begin(), c.end(), c.begin(), 0.0);
                return std::sqrt((sq_sum / c.size()) - (m * m));
            }

            // =====================================================================================
            template <class T>
            static double stdev_v2(T& c)
            {
                // Calculates stdev for a population
                double m = mean(c);
                T diff(c.size());
                std::transform(c.begin(), c.end(), diff.begin(), [m](double x) { return x-m; });
                double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
                return std::sqrt(sq_sum / c.size());
            }

            // =====================================================================================
            template <class T>
            static double stdev_v3(T& c)
            {
                // Calculates stdev for a sample
                double m = mean(c);
                T diff(c.size());
                std::transform(c.begin(), c.end(), diff.begin(), [m](double x) { return x-m; });
                double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
                return std::sqrt(sq_sum / (c.size()-1));
            }

            // =====================================================================================
            template <class T>
            static double stdev_p(T& c)
            {
                // Calculates stdev for a population
                return stdev_v1(c);
            }

            // =====================================================================================
            template <class T>
            static double stdev_s(T& c)
            {
                // Calculates stdev for a sample
                return stdev_v3(c);
            }

            // =====================================================================================
            template <class T>
            static double stdev_p(T& c, std::size_t n)
            {
                // Calculates stdev for a population
                double sum = 0, v = 0;
                double m = mean(c, n);
                for(std::size_t i=0; i<n; ++i)
                {
                    v = c[i] - m;
                    sum += v * v;
                }
                return std::sqrt(sum / n);
            }

            // =====================================================================================
            template <class T>
            static double stdev_s(T& c, std::size_t n)
            {
                // Calculates stdev for a sample
                double sum = 0, v = 0;
                double m = mean(c, n);
                for(std::size_t i=0; i<n; ++i)
                {
                    v = c[i] - m;
                    sum += v * v;
                }
                return std::sqrt(sum / (n-1));
            }

            // =====================================================================================
            template <class T>
            static double skewness_p(T& c, std::size_t n)
            {
                // Calculates skewness for a population
                double sum = 0, v = 0;
                double m = mean(c, n);
                double std = stdev_p(c, n);
                for(std::size_t i=0; i<n; ++i)
                {
                    v = c[i] - m;
                    sum += v * v * v;
                }
                return sum / (n * std * std * std);
            }

            // =====================================================================================
            template <class T>
            static double skewness(T& c)
            {
                return skewness_p(c, c.size());
            }

            // =====================================================================================
            template <class T>
            static double kurtosis_pearson(T& c, std::size_t n)
            {
                // Calculates kurtosis using Pearson's measure
                // https://github.com/spficklin/RMTGeneNet/blob/master/stats/kurtosis.cpp
                double q = 0, r = 0, v = 0;
                double m = mean(c, n);
                double std = stdev_s(c, n);
                for(std::size_t i=0; i<n; ++i)
                {
                    v = c[i] - m;
                    q += v * v;
                    r += v * v * v * v;
                }
                return (n * r) / (q * q);
            }

            // =====================================================================================
            template <class T>
            static double kurtosis(T& c)
            {
                return kurtosis_pearson(c, c.size()) - 3.0;
            }

            // =====================================================================================
            template <class T>
            static void moments(T& c, double& m, double& std, double& sk, double& ku)
            {
                // We want to calculate all of the moments in one call from a population POV
                double sum = 0, q = 0, r = 0, v = 0;
                std::size_t n = c.size();
                m = mean(c, n);
                for(std::size_t i=0; i<n; ++i)
                {
                    v = c[i] - m;
                    // q is actually variance
                    q += v * v;
                    sum += v * v * v;
                    r += v * v * v * v;
                }

                std = std::sqrt(q/n);
                sk = sum / (n * std * std * std);
                ku = ((n * r) / (q * q)) - 3;
            }

            // =====================================================================================
            // If we have a vector which is constantly updating then we want to optimize the mean
            // calculation. We can do this by keeping a rolling sum. This will speed everything up
            // and reflects how we actually use it in real life.
            class RollingWindow
            {
            public:
                RollingWindow(std::int32_t w = 500, bool avg_only = false) :
                    m_window(w),
                    m_avg_only(avg_only),
                    m_last_value(0),
                    m_mean(0),
                    m_stdev(0),
                    m_min_max_vol(0),
                    m_skew(0),
                    m_kurt(0),
                    m_min(999999999.999999),
                    m_max(-999999999.999999) {}

                double get_last_value() { return m_last_value; }
                bool is_buffer_full() { return (m_values.size() >= m_window ? true : false); }
                double get_percent_buffered() { return (static_cast<double>(m_values.size()) / static_cast<double>(m_window)); }
                double get_min() { return m_min; }
                double get_max() { return m_max; }

                bool get_average(double& v)
                {
                    if(is_buffer_full())
                    {
                        v = m_mean;
                        return true;
                    }
                    return false;
                }

                bool get_stdev(double& v)
                {
                    if(is_buffer_full())
                    {
                        v = m_stdev;
                        return true;
                    }
                    return false;
                }

                bool get_skew(double& v)
                {
                    if(is_buffer_full())
                    {
                        v = m_skew;
                        return true;
                    }
                    return false;
                }

                bool get_kurtosis(double& v)
                {
                    if(is_buffer_full())
                    {
                        v = m_kurt;
                        return true;
                    }
                    return false;
                }

                bool get_min_max_vol(double& v)
                {
                    if(is_buffer_full())
                    {
                        v = m_min_max_vol;
                        return true;
                    }
                    return false;
                }

                bool get_values(double& m, double& std, double& sk, double& ku)
                {
                    if(is_buffer_full())
                    {
                        m = m_mean;
                        std = m_stdev;
                        sk = m_skew;
                        ku = m_kurt;
                        return true;
                    }
                    return false;
                }

                void add(double v)
                {
                    m_last_value = v;
                    m_values.push_back(v);
                    if(m_values.size() > m_window)
                        m_values.erase(m_values.begin());

                    // Only start once the buffer is 75% full
                    if(m_values.size() > (m_window * .75))
                    {
                        // Do we only care about the mean
                        if(m_avg_only)
                            m_mean = mean(m_values);
                        else
                        {
                            // We need to calculate all of the moments
                            moments(m_values, m_mean, m_stdev, m_skew, m_kurt);
                            vol_min_max_check();
                        }
                    }
                }

            private:
                void vol_min_max_check()
                {
                    // Vol over time can become very small if the updates stop changing frequently
                    // Also depending on if the update is a price or change value we want to restrict
                    // vol getting to close to zero
                    if(m_values.size() > (m_window * .90))
                    {
                        if(m_stdev < m_min)
                            m_min = m_stdev;
                        if(m_max < m_stdev)
                            m_max = m_stdev;

                        m_min_max_vol = (m_min + m_max) / 2.0;
                    }
                }

                std::int32_t    m_window;
                bool            m_avg_only;
                double          m_last_value;
                double          m_mean;
                double          m_stdev;
                double          m_min_max_vol;
                double          m_skew;
                double          m_kurt;
                double          m_min;
                double          m_max;

                std::vector<double> m_values;
            };

        }
    }
}

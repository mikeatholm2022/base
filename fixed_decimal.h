// Fixed Decimal object
//
// Copyright HOLM, 2023

#pragma once

#include <sstream>
#include <iomanip>
#include <string>
#include <numeric>

#include <boost/lexical_cast.hpp>

// ===========================================================================================================
namespace chaos
{
    #define FIXED_DECIMAL_NAN           999999999
    #define FIXED_DECIMAL_DOUBLE_NAN    999999999.0

    class FixedDecimal
    {
    public:
        // NOTE WE NEED TO ADD CASES FOR NaN and Infinty
        // ====================================================================================================
        FixedDecimal()
        {
            m_value = m_dp = 0;
        }

        // ====================================================================================================
        FixedDecimal(std::int64_t value, std::int64_t dp)
        {
            m_value = value;
            m_dp = dp;
            normalize();
        }

        // ====================================================================================================
        FixedDecimal(double value)
        {
            from_double(value);
        }

        // ====================================================================================================
        FixedDecimal(const std::string& value)
        {
            from_string(value);
        }

        // ====================================================================================================
        bool is_valid()
        {
            if (m_value == FIXED_DECIMAL_NAN && m_dp == 0)
                return false;

            return true;
        }

        // ====================================================================================================
        bool is_positive()
        {
            if (m_value > 0)
                return true;

            return false;
        }

        // ====================================================================================================
        double as_double()
        {
            return (m_value / (double)(dec_pow(m_dp)));
        }

        // ====================================================================================================
        std::int64_t get_value() { return m_value; }
        std::int64_t get_decimal_places() { return m_dp; }

        // ====================================================================================================
        std::string as_string()
        {
          if (is_valid())
          {
            char buf[100];
            sprintf(buf, "%.*f", static_cast<std::int32_t>(m_dp), as_double());
            return buf;
          }
          else
            return "NaN";
        }

        // ====================================================================================================
        std::string as_serialized_string()
        {
            std::stringstream ss;
            ss << m_value << "-" << m_dp;
            return ss.str();
        }

        // ====================================================================================================
        void from_double(double value)
        {
            std::stringstream ss ;
            ss << std::setprecision(std::numeric_limits<double>::digits10) << value ;
            from_string( ss.str() );
        }

        // ====================================================================================================
        void from_serialized_string(const std::string& str)
        {
            // The default format is 999-1 --> 99.1

            // We need to search for the '-'
            m_value = m_dp = 0;
            std::size_t pos = str.find("-");
            std::string value = str.substr(0,pos);
            std::string dp = str.substr(pos+1,str.length()-pos);
            m_value = boost::lexical_cast<std::int64_t>(value);
            m_dp = boost::lexical_cast<std::int64_t>(dp);
        }

        // ====================================================================================================
        void from_string(const std::string& str)
        {
            if( !str.empty()  )
            {
                // The values we are to find
                std::int64_t value = 0;
                std::int64_t dp = 0;
                bool cont = true;
                std::int64_t dig = 0;
                std::int64_t decplace = 0;
                bool decpoint = false;
                bool neg = false;
                std::uint32_t i = 0;
                char c;
                std::int64_t len = (std::int64_t)str.length();

                if( str[0] == '-' )
                {
                    neg = true;
                    i = 1;
                }

                while( cont && i < len )
                {
                    c = str[i];
                    if( c >= '0' && c <= '9' )
                    {
                        // Get the digit value
                        std::int64_t digitValue = c - '0';

                        // Add it to the running total
                        value = (10 * value) + digitValue;
                        dig++;

                        if( decpoint )
                            dp++;
                    }
                    else if( c == ',' || c == '.' )
                    {
                        if ( !decpoint )
                            decpoint = true;
                        else
                            cont = false; // already found a decimal place character
                    }
                    else
                    {
                        // String cannot be parsed
                        cont = false;
                    }

                    i++;
                }

                if( dig )
                {
                    if ( neg )
                        m_value = -value;
                    else
                        m_value = value;

                    m_dp = dp;

                    normalize();
                }
                else
                {
                    // Zero digits found while parsing
                    m_value = 0;
                    m_dp = 0;
                }
            }
            else
            {
                m_value = 0;
                m_dp = 0;
            }
        }

        // ====================================================================================================
        bool operator==(const FixedDecimal& other) const
        {
            if( (this->m_value == other.m_value) && (this->m_dp == other.m_dp) )
                return true;
            else
                return false;
        }

        // ====================================================================================================
        bool operator!=(const FixedDecimal& other) const
        {
            if( (this->m_value != other.m_value) || (this->m_dp != other.m_dp) )
                return true;
            else
                return false;
        }

        std::int64_t value(){ return m_value; }
        std::int64_t decimal_places(){ return m_dp; }

    private:
        // ====================================================================================================
        std::int64_t dec_pow(std::int64_t dp)
        {
            std::int64_t result = 1;
            for( std::int64_t i=1; i<=dp; i++ )
                result *= 10;

            return result;
        }

        // ====================================================================================================
        void normalize()
        {
            while( m_value % 10 == 0 && m_dp > 0 )
            {
                m_value /= 10;
                m_dp--;
            }
        }

        std::int64_t	m_value;
        std::int64_t	m_dp;

    };
}

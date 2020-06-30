/**
* Copyright (c) 2020 Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include "typeChooser.hpp"


#ifndef UINT128_MAX
#define UINT128_MAX 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFULL
#endif // UINT128_MAX

namespace tinymind {

    template<typename T, unsigned NumberOfFractionalBits>
    class RoundUpPolicy
    {
    public:
        static T round(const T& value)
        {
            T result(value);

            //Round up
            result = result + ((result & (1 << (NumberOfFractionalBits - 1))) << 1);
            result >>= NumberOfFractionalBits;

            return result;
        }
    };

    template<typename T, unsigned NumberOfFractionalBits>
    class TruncatePolicy
    {
    public:
        static T round(const T& value)
        {
            T result(value);

            result >>= NumberOfFractionalBits;

            return result;
        }
    };

    template<typename T, unsigned NumberOfFixedBits, unsigned NumberOfFractionalBits, bool IsSigned>
    struct SignExtender
    {
        static const T SignBitMask = static_cast<T>(1 << (NumberOfFixedBits + NumberOfFractionalBits - 1));
        static const T SignExtensionBits = static_cast<T>(UINT128_MAX << (NumberOfFixedBits + NumberOfFractionalBits));

        static void signExtend(T& value)
        {
            if (value & SignBitMask)
            {
                value |= SignExtensionBits;
            }
        }        
    };

    template<typename T, unsigned NumberOfFixedBits, unsigned NumberOfFractionalBits>
    struct SignExtender<T, NumberOfFixedBits, NumberOfFractionalBits, false>
    {
        static void signExtend(T& value)
        {
        }
    };

    template<typename SourceType, typename ResultType>
    struct NoShiftPolicy
    {
        static ResultType shift(const SourceType& value)
        {
            return static_cast<ResultType>(value);
        }
    };

    template<typename SourceType, typename ResultType, unsigned SourceNumberOfFractionalBits, unsigned ResultNumberOfFractionalBits>
    struct ShiftRightPolicy
    {
        static ResultType shift(const SourceType& value)
        {
            return static_cast<ResultType>(value >> (SourceNumberOfFractionalBits - ResultNumberOfFractionalBits));
        }
    };

    template<typename SourceType, typename ResultType, unsigned SourceNumberOfFractionalBits, unsigned ResultNumberOfFractionalBits>
    struct ShiftLeftPolicy
    {
        static ResultType shift(const SourceType& value)
        {
            return (static_cast<ResultType>(value) << (ResultNumberOfFractionalBits - SourceNumberOfFractionalBits));
        }
    };

    template<typename SourceType, typename ResultType, unsigned SourceNumberOfFractionalBits, unsigned ResultNumberOfFractionalBits, bool IsSourceBigger>
    struct ShiftPolicySelectorShiftRequired
    {
    };

    template<typename SourceType, typename ResultType, unsigned SourceNumberOfFractionalBits, unsigned ResultNumberOfFractionalBits>
    struct ShiftPolicySelectorShiftRequired<SourceType, ResultType, SourceNumberOfFractionalBits, ResultNumberOfFractionalBits, true>
    {
        typedef ShiftRightPolicy<SourceType, ResultType, SourceNumberOfFractionalBits, ResultNumberOfFractionalBits> ShiftPolicyType;
    };

    template<typename SourceType, typename ResultType, unsigned SourceNumberOfFractionalBits, unsigned ResultNumberOfFractionalBits>
    struct ShiftPolicySelectorShiftRequired<SourceType, ResultType, SourceNumberOfFractionalBits, ResultNumberOfFractionalBits, false>
    {
        typedef ShiftLeftPolicy<SourceType, ResultType, SourceNumberOfFractionalBits, ResultNumberOfFractionalBits> ShiftPolicyType;
    };

    template<typename SourceType, typename ResultType, unsigned SourceNumberOfFractionalBits, unsigned ResultNumberOfFractionalBits, bool NumberOfBitsEqual>
    struct ShiftPolicySelectorCheckForEqual
    {
    };

    template<typename SourceType, typename ResultType, unsigned SourceNumberOfFractionalBits, unsigned ResultNumberOfFractionalBits>
    struct ShiftPolicySelectorCheckForEqual<SourceType, ResultType, SourceNumberOfFractionalBits, ResultNumberOfFractionalBits, true>
    {
        typedef NoShiftPolicy<SourceType, ResultType> ShiftPolicyType;
    };

    template<typename SourceType, typename ResultType, unsigned SourceNumberOfFractionalBits, unsigned ResultNumberOfFractionalBits>
    struct ShiftPolicySelectorCheckForEqual<SourceType, ResultType, SourceNumberOfFractionalBits, ResultNumberOfFractionalBits, false>
    {
        typedef typename ShiftPolicySelectorShiftRequired<SourceType, ResultType, SourceNumberOfFractionalBits, ResultNumberOfFractionalBits, (SourceNumberOfFractionalBits > ResultNumberOfFractionalBits)>::ShiftPolicyType ShiftPolicyType;
    };

    template<typename SourceType, typename ResultType, unsigned SourceNumberOfFractionalBits, unsigned ResultNumberOfFractionalBits>
    struct ShiftPolicySelector
    {
        typedef typename ShiftPolicySelectorCheckForEqual<SourceType, ResultType, SourceNumberOfFractionalBits, ResultNumberOfFractionalBits, (SourceNumberOfFractionalBits == ResultNumberOfFractionalBits)>::ShiftPolicyType ShiftPolicyType;
    };

    template<typename SourceType, typename ResultType, unsigned SourceNumberOfFractionalBits, unsigned ResultNumberOfFractionalBits>
    struct ShiftPolicy
    {
        typedef typename ShiftPolicySelector<SourceType, ResultType, SourceNumberOfFractionalBits, ResultNumberOfFractionalBits>::ShiftPolicyType ShiftPolicyType;

        static ResultType shift(const SourceType& value)
        {
            const ResultType result = ShiftPolicyType::shift(value);

            return result;
        }
    };

    template<unsigned NumBits, bool IsSigned>
    struct MultiplicationResultFullWidthFieldTypeChooser
    {
        static const unsigned FullWidthResult = FullWidthFieldTypeChooser<NumBits, IsSigned>::Result;
        typedef typename FullWidthFieldTypeChooser<(FullWidthResult << 1), IsSigned>::FullWidthValueType MultiplicationResultFullWidthFieldType;
    };

    template<unsigned NumBits, bool IsSigned>
    struct DivisionResultFullWidthFieldTypeChooser
    {
        static const unsigned FullWidthResult = FullWidthFieldTypeChooser<NumBits, IsSigned>::Result;
        typedef typename FullWidthFieldTypeChooser<(FullWidthResult << 1), IsSigned>::FullWidthValueType DivisionResultFullWidthFieldType;
    };

    template<unsigned NumFixedBits, unsigned NumFractionalBits, bool IsSigned>
    struct QTypeChooser
    {
        typedef typename FullWidthFieldTypeChooser<NumFixedBits + NumFractionalBits, IsSigned>::FixedPartFieldType                                         FixedPartFieldType;
        typedef typename FullWidthFieldTypeChooser<NumFixedBits + NumFractionalBits, IsSigned>::FractionalPartFieldType                                    FractionalPartFieldType;
        typedef typename FullWidthFieldTypeChooser<NumFixedBits + NumFractionalBits, IsSigned>::FullWidthFieldType                                         FullWidthFieldType;
        typedef typename FullWidthFieldTypeChooser<NumFixedBits + NumFractionalBits, IsSigned>::FullWidthValueType                                         FullWidthValueType;
        typedef typename MultiplicationResultFullWidthFieldTypeChooser<NumFixedBits + NumFractionalBits, IsSigned>::MultiplicationResultFullWidthFieldType MultiplicationResultFullWidthFieldType;
        typedef typename DivisionResultFullWidthFieldTypeChooser<NumFixedBits + NumFractionalBits, IsSigned>::DivisionResultFullWidthFieldType             DivisionResultFullWidthFieldType;
    };

    template<unsigned NumFixedBits, unsigned NumFractionalBits, bool QValueIsSigned, template<typename, unsigned> class QValueRoundingPolicy = TruncatePolicy>
    struct QValue
    {
        static const unsigned NumberOfFixedBits = NumFixedBits;
        static const unsigned NumberOfFractionalBits = NumFractionalBits;
        static const bool IsSigned = QValueIsSigned;

        typedef typename QTypeChooser<NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::FullWidthValueType                     FullWidthValueType;
        typedef typename QTypeChooser<NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::FixedPartFieldType                     FixedPartFieldType;
        typedef typename QTypeChooser<NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::FractionalPartFieldType                FractionalPartFieldType;
        typedef typename QTypeChooser<NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::FullWidthFieldType                     FullWidthFieldType;
        typedef typename QTypeChooser<NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::MultiplicationResultFullWidthFieldType MultiplicationResultFullWidthFieldType;
        typedef typename QTypeChooser<NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::DivisionResultFullWidthFieldType       DivisionResultFullWidthFieldType;
        typedef QValueRoundingPolicy<MultiplicationResultFullWidthFieldType, NumberOfFractionalBits>                               RoundingPolicy;

        QValue() : mValue(0)
        {
        }

        QValue(const QValue& other) : mValue(other.mValue)
        {
        }

        QValue(const FullWidthFieldType& value) : mValue(value)
        {
        }

        QValue(const FixedPartFieldType& fixedPart, const FractionalPartFieldType& fractionalPart)
        {
            FixedPartFieldType fixedResult(fixedPart);

            SignExtender<FixedPartFieldType, NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::signExtend(fixedResult);

            mFixedPart = fixedResult;
            mFractionalPart = fractionalPart;
        }

        QValue& operator=(const QValue& other)
        {
            mValue = other.mValue;

            return *this;
        }

        QValue& operator=(const FullWidthFieldType& value)
        {
            mValue = value;

            return *this;
        }

        QValue& operator+=(const QValue& other)
        {
            mValue += other.mValue;

            return *this;
        }

        QValue& operator+=(const FullWidthFieldType& value)
        {
            mValue += value;

            return *this;
        }

        QValue& operator++()
        {
            ++mFixedPart;

            return *this;
        }

        QValue operator++(int)
        {
            ++mFixedPart;

            return *this;
        }

        QValue& operator-=(const QValue& other)
        {
            mValue -= other.mValue;

            return *this;
        }

        QValue& operator-=(const FullWidthFieldType& value)
        {
            mValue -= value;

            return *this;
        }

        QValue& operator--()
        {
            --mFixedPart;

            return *this;
        }

        QValue operator--(int)
        {
            --mFixedPart;

            return *this;
        }

        QValue& operator*=(const QValue& other)
        {
            MultiplicationResultFullWidthFieldType left;
            MultiplicationResultFullWidthFieldType right;
            MultiplicationResultFullWidthFieldType result;

            left = static_cast<MultiplicationResultFullWidthFieldType>(mValue);
            right = static_cast<MultiplicationResultFullWidthFieldType>(other.mValue);

            SignExtender<MultiplicationResultFullWidthFieldType, NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::signExtend(left);
            SignExtender<MultiplicationResultFullWidthFieldType, NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::signExtend(right);

            result = left * right;

            result = RoundingPolicy::round(result);

            this->mValue = static_cast<FullWidthFieldType>(result);

            return *this;
        }

        QValue& operator*=(const int multiplicand)
        {
            const QValue other(static_cast<FixedPartFieldType>(multiplicand), 0);
            MultiplicationResultFullWidthFieldType left;
            MultiplicationResultFullWidthFieldType right;
            MultiplicationResultFullWidthFieldType result;

            left = static_cast<MultiplicationResultFullWidthFieldType>(mValue);
            right = static_cast<MultiplicationResultFullWidthFieldType>(other.mValue);

            SignExtender<MultiplicationResultFullWidthFieldType, NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::signExtend(left);
            SignExtender<MultiplicationResultFullWidthFieldType, NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::signExtend(right);

            result = left * right;

            result = RoundingPolicy::round(result);

            this->mValue = static_cast<FullWidthFieldType>(result);

            return *this;
        }

        QValue& operator/=(const QValue& other)
        {
            DivisionResultFullWidthFieldType left;
            DivisionResultFullWidthFieldType right;

            left = static_cast<DivisionResultFullWidthFieldType>(mValue);
            right = static_cast<DivisionResultFullWidthFieldType>(other.mValue);

            SignExtender<DivisionResultFullWidthFieldType, NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::signExtend(left);
            SignExtender<DivisionResultFullWidthFieldType, NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::signExtend(right);

            const FullWidthFieldType result = static_cast<FullWidthFieldType>((left << NumberOfFractionalBits) / right);

            this->mValue = result;

            return *this;
        }

        QValue& operator/=(const int divisor)
        {
            const QValue other(static_cast<FixedPartFieldType>(divisor), 0);
            DivisionResultFullWidthFieldType left;
            DivisionResultFullWidthFieldType right;

            left = static_cast<DivisionResultFullWidthFieldType>(mValue);
            right = static_cast<DivisionResultFullWidthFieldType>(other.mValue);

            SignExtender<DivisionResultFullWidthFieldType, NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::signExtend(left);
            SignExtender<DivisionResultFullWidthFieldType, NumberOfFixedBits, NumberOfFractionalBits, IsSigned>::signExtend(right);

            const FullWidthFieldType result = static_cast<FullWidthFieldType>((left << NumberOfFractionalBits) / right);

            this->mValue = result;

            return *this;
        }

        bool operator==(const QValue& other) const
        {
            return (mValue == other.mValue);
        }

        bool operator==(const FullWidthValueType& value) const
        {
            return (mValue.getValue() == value);
        }

        bool operator!=(const QValue& other) const
        {
            return (mValue != other.mValue);
        }

        bool operator!=(const FullWidthValueType& value) const
        {
            return (mValue.getValue() != value);
        }


        

#ifdef ENABLE_OSTREAMS
        friend std::ostream& operator<<(std::ostream& os, const QValue& value)
        {
            os << value.getValue();
            return os;
        }
#endif // ENABLE_OSTREAMS
        template<typename OtherQValueType>
        void convertFromOtherQValueType(const OtherQValueType& other)
        {
            typedef typename ShiftPolicy<
                                        typename OtherQValueType::FractionalPartFieldType,
                                        FractionalPartFieldType,
                                        OtherQValueType::NumberOfFractionalBits,
                                        NumberOfFractionalBits>::ShiftPolicyType ShiftPolicyType;
            mFixedPart = static_cast<FixedPartFieldType>(other.getFixedPart());
            mFractionalPart = ShiftPolicyType::shift(other.getFractionalPart());
        }

        FixedPartFieldType getFixedPart() const
        {
            return mFixedPart;
        }

        FractionalPartFieldType getFractionalPart() const
        {
            return mFractionalPart;
        }

        FullWidthValueType getValue() const
        {
            return static_cast<FullWidthValueType>(mValue);
        }

        void setValue(const FullWidthValueType& value)
        {
            mValue = value;
        }

        void setValue(const FixedPartFieldType& fixedPart, const FractionalPartFieldType& fractionalPart)
        {
            mFixedPart = fixedPart;
            mFractionalPart = fractionalPart;
        }

    private:
        union
        {
            struct
            {
                FractionalPartFieldType mFractionalPart : NumberOfFractionalBits;
                FixedPartFieldType mFixedPart : NumberOfFixedBits;
            };
            FullWidthFieldType mValue;
        };

        static_assert(((8 * sizeof(FullWidthFieldType)) >= (NumberOfFixedBits + NumberOfFractionalBits)), "Incorrect type choice for ValueType.");
    };

    template<typename QValueType>
    QValueType operator+(const QValueType& left, const QValueType& right)
    {
        QValueType result(left);

        result += right;

        return result;
    }

    template<typename QValueType>
    QValueType operator+(const QValueType& left, const int right)
    {
        QValueType result(left);
        QValueType other(right, 0);

        result += other;

        return result;
    }

    template<typename QValueType>
    QValueType operator-(const QValueType& left, const QValueType& right)
    {
        QValueType result(left);

        result -= right;

        return result;
    }

    template<typename QValueType>
    QValueType operator-(const QValueType& left, const int right)
    {
        QValueType result(left);
        QValueType other(right, 0);

        result -= other;

        return result;
    }

    template<typename QValueType>
    QValueType operator*(const QValueType& left, const QValueType& right)
    {
        QValueType result(left);

        result *= right;

        return result;
    }

    template<typename QValueType>
    QValueType operator*(const QValueType& left, const int right)
    {
        QValueType result(left);
        QValueType other(static_cast<typename QValueType::FixedPartFieldType>(right), 0);

        result *= other;

        return result;
    }

    template<typename QValueType>
    QValueType operator/(const QValueType& left, const QValueType& right)
    {
        QValueType result(left);

        result /= right;

        return result;
    }

    template<typename QValueType>
    QValueType operator/(const QValueType& left, const int right)
    {
        const QValueType other(static_cast<typename QValueType::FixedPartFieldType>(right), 0);
        QValueType result(left);

        result /= other;

        return result;
    }

    template<typename QValueType>
    bool operator>(const QValueType& left, const QValueType& right)
    {
        return (left.getValue() > right.getValue());
    }

    template<typename QValueType>
    bool operator>(const QValueType& left, const int right)
    {
        const QValueType other(static_cast<typename QValueType::FixedPartFieldType>(right), 0);

        return (left.getValue() > other.getValue());
    }

    template<typename QValueType>
    bool operator>=(const QValueType& left, const QValueType& right)
    {
        return (left.getValue() >= right.getValue());
    }

    template<typename QValueType>
    bool operator>=(const QValueType& left, const int right)
    {
        const QValueType other(static_cast<typename QValueType::FixedPartFieldType>(right), 0);

        return (left.getValue() >= other.getValue());
    }

    template<typename QValueType>
    bool operator<(const QValueType& left, const QValueType& right)
    {
        return (left.getValue() < right.getValue());
    }

    template<typename QValueType>
    bool operator<(const QValueType& left, const int right)
    {
        const QValueType other(static_cast<typename QValueType::FixedPartFieldType>(right), 0);

        return (left.getValue() < other.getValue());
    }

    template<typename QValueType>
    bool operator<=(const QValueType& left, const QValueType& right)
    {
        return (left.getValue() <= right.getValue());
    }

    template<typename QValueType>
    bool operator<=(const QValueType& left, const int right)
    {
        const QValueType other(static_cast<typename QValueType::FixedPartFieldType>(right), 0);

        return (left.getValue() <= other.getValue());
    }
}
std::ostream& operator << (std::ostream& dest, __int128_t value) 
{
    std::ostream::sentry s(dest);
    if (s) {
        __uint128_t tmp = value<0?-value:value;
        char buffer[128];
        char* d = std::end(buffer);
        do {
            -- d;
            *d = "0123456789"[tmp%10];
            tmp/=10;
        }while(tmp!=0);
        if(value<0) {
            --d;
            *d = '-';
        }
        int len = std::end(buffer)-d;
        if (dest.rdbuf()->sputn(d,len)!=len) {
            dest.setstate(std::ios_base::badbit);
        }
    }
    return dest;
}
#include <iostream>
#include <sstream>
#include <limits>
#include "DataStruct.h"

std::istream& fedorova::operator>>(std::istream& is, DelimiterIO&& dest)
{
    std::istream::sentry sentry(is);
    if (!sentry)
    {
        return is;
    }

    char c;
    is >> c;
    if (is && (c != dest.exp))
    {
        is.setstate(std::ios::failbit);
    }

    return is;
}

std::istream& fedorova::operator>>(std::istream& is, ULLLiteralIO&& dest)
{
    std::istream::sentry sentry(is);
    if (!sentry)
    {
        return is;
    }

    is >> dest.ref;
    if (is)
    {
        is >> DelimiterIO{ 'u' } >> DelimiterIO{ 'l' } >> DelimiterIO{ 'l' };
    }

    return is;
}

std::istream& fedorova::operator>>(std::istream& is, ULLBinaryIO&& dest)
{
    std::istream::sentry sentry(is);
    if (!sentry)
    {
        return is;
    }

    is >> DelimiterIO{ '0' } >> DelimiterIO{ 'b' };

    unsigned long long value = 0;
    bool has_digits = false;

    while (true)
    {
        char c = is.peek();
        if (c == '0' || c == '1')
        {
            is.get();
            value = (value << 1) | (c - '0');
            has_digits = true;
        }
        else
        {
            break;
        }
    }

    if (!has_digits)
    {
        is.setstate(std::ios::failbit);
    }
    else
    {
        dest.ref = value;
    }

    return is;
}

std::istream& fedorova::operator>>(std::istream& is, StringIO&& dest)
{
    std::istream::sentry sentry(is);
    if (!sentry)
    {
        return is;
    }

    return std::getline(is >> DelimiterIO{ '\"' }, dest.ref, '\"');
}

std::istream& fedorova::operator>>(std::istream& is, LabelIO&& dest)
{
    std::istream::sentry sentry(is);
    if (!sentry)
    {
        return is;
    }

    for (char exp_char : dest.exp)
    {
        char c;
        is >> c;
        if (c != exp_char)
        {
            is.setstate(std::ios::failbit);
            break;
        }
    }
    return is;
}

std::istream& fedorova::operator>>(std::istream& is, DataStruct& data)
{
    std::istream::sentry sentry(is);
    if (!sentry)
    {
        return is;
    }

    DataStruct in;

    {
        bool key1 = true, key2 = true, key3 = true;

        is >> DelimiterIO{ '(' };

        while ((key1 || key2 || key3) && is)
        {
            std::string fieldName;
            is >> fieldName;
            if (fieldName == ":key1" && key1)
            {
                is >> ULLLiteralIO{ in.key1 };
                key1 = false;
            }
            else if (fieldName == ":key2" && key2)
            {
                is >> ULLBinaryIO{ in.key2 };
                key2 = false;
            }
            else if (fieldName == ":key3" && key3)
            {
                is >> StringIO{ in.key3 };
                key3 = false;
            }
            else
            {
                is.setstate(std::ios::failbit);
                break;
            }
        }
    }

    if (is)
    {
        data = in;
    }

    is >> DelimiterIO{ ':' } >> DelimiterIO{ ')' };
    return is;
}

std::ostream& fedorova::operator<<(std::ostream& os, const DataStruct& data)
{
    std::ostream::sentry sentry(os);
    if (!sentry)
    {
        return os;
    }

    IoGuard fmtguard(os);

    os << "(:key1 " << data.key1 << "ull"
        << ":key2 0b";

    if (data.key2 == 0)
    {
        os << "0";
    }
    else
    {
        unsigned long long mask = 1ULL << (sizeof(data.key2) * 8 - 1);
        while ((mask & data.key2) == 0 && mask != 0)
        {
            mask >>= 1;
        }

        while (mask != 0)
        {
            os << ((data.key2 & mask) ? "1" : "0");
            mask >>= 1;
        }
    }

    os << ":key3 \"" << data.key3 << "\":)";

    return os;
}

fedorova::IoGuard::IoGuard(std::basic_ios< char >& s) :
    s_(s),
    width_(s.width()),
    fill_(s.fill()),
    precision_(s.precision()),
    fmt_(s.flags())
{}

fedorova::IoGuard::~IoGuard()
{
    s_.width(width_);
    s_.fill(fill_);
    s_.precision(precision_);
    s_.flags(fmt_);
}

bool fedorova::compareDataStruct(const DataStruct& a, const DataStruct& b)
{
    if (a.key1 != b.key1)
    {
        return a.key1 < b.key1;
    }

    if (a.key2 != b.key2)
    {
        return a.key2 < b.key2;
    }

    return a.key3.length() < b.key3.length();
}

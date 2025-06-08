#include "DataStruct.h"

#include <sstream>
#include <bitset>
#include <string>

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

std::istream& fedorova::operator>>(std::istream& is, ULLBinaryIO&& dest) {
    std::istream::sentry sentry(is);
    if (!sentry)
    {
        return is;
    }

    is >> DelimiterIO{ '0' };
    char b = '0';
    is >> b;
    if (b != 'b' && b != 'B') {
        is.setstate(std::ios::failbit);
        return is;
    }

    std::string bits = "";
    char c = '0';
    size_t leading_zeros = 0;
    bool counting_zeros = true;

    while (is.get(c)) {
        if (c == '0' || c == '1') {
            bits += c;
            if (counting_zeros && c == '0') {
                leading_zeros++;
            }
            else {
                counting_zeros = false;
            }
        }
        else {
            is.unget();
            break;
        }
    }

    if (bits.empty()) {
        is.setstate(std::ios::failbit);
        return is;
    }

    try {
        dest.ref = std::stoull(bits, nullptr, 2);
        dest.leading_zeros = leading_zeros;
    }
    catch (...) {
        is.setstate(std::ios::failbit);
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

    char key;
    for (size_t i = 0; i < dest.exp.size(); i++)
    {
        is >> key;
        if (!is && key != dest.exp[i])
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
                is >> ULLLiteralIO{ data.key1 };
                in.key1 = data.key1;
                key1 = false;
            }
            else if (fieldName == ":key2" && key2)
            {
                size_t leading_zeros = 0;
                is >> ULLBinaryIO{ data.key2.value, leading_zeros };
                data.key2.leading_zeros = leading_zeros;
                in.key2 = data.key2;
                key2 = false;
            }
            else if (fieldName == ":key3" && key3)
            {
                is >> StringIO{ data.key3 };
                in.key3 = data.key3;
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

    os << "(:"
        << "key1 " << data.key1 << "ull" << ":"
        << "key2 " << "0b" << std::string(data.key2.leading_zeros, '0')
        << std::bitset<64>(data.key2.value).to_string().substr(64 - (64 - data.key2.leading_zeros)) << ":"
        << "key3 " << "\"" << data.key3 << "\""
        << ":)";

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

    if (a.key2.value != b.key2.value)
    {
        return a.key2.value < b.key2.value;
    }

    return a.key3.length() < b.key3.length();
}

#include "pch.h"

#include "csmnet/util/XmlDocument.h"
#include "csmnet/common/Error.h"

using namespace std;

namespace csmnet::util
{
    expected<void, error_code> XmlDocument::ParseFromFile(filesystem::path xmlFilePath) noexcept
    {
        if (filesystem::exists(xmlFilePath) == false)
        {
            return unexpected(make_error_code(errc::no_such_file_or_directory));
        }

        ifstream xmlFile{ xmlFilePath };
        if (!xmlFile)
        {
            return unexpected(make_error_code(errc::permission_denied));
        }

        string content{ istreambuf_iterator<char>(xmlFile), istreambuf_iterator<char>() };
        xmlFile.close();

        return ParseFromString(std::move(content));
    }

    expected<void, error_code> XmlDocument::ParseFromString(std::string xmlContent) noexcept
    {
        if (xmlContent.empty() == false && xmlContent.back() != '\0')
        {
            // rapidxml이 문자열을 수정하므로 널 종료 문자 추가
            xmlContent.push_back('\0');
        }

        _xmlContent = std::move(xmlContent);
        try
        {
            _data.parse<0>(_xmlContent.data());
        }
        catch (const rapidxml::parse_error&)
        {
            return unexpected(std::make_error_code(std::errc::invalid_argument));
        }

        return {};
    }
}
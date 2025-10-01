// [rapidxml](https://rapidxml.sourceforge.net/manual.html) 라이브러리를 사용하여 XML 문서를 파싱하고 직렬화하는 XML 파서 유틸리티
// XmlDocument, XmlNode, XmlAttribute 클래스를 제공하며, UTF-8 인코딩만 지원한다.

#pragma once

#include "csmnet/detail/Config.h"

#include <string>
#include <string_view>
#include <filesystem>
#include <charconv>
#include <concepts>

#include <rapidxml/rapidxml.hpp>

namespace csmnet::util
{
    using xml_attr_t = rapidxml::xml_attribute<>;
    using xml_node_t = rapidxml::xml_node<>;
    using xml_doc_t = rapidxml::xml_document<>;

    // XML 속성을 나타낸다.
    // 항상 사용하기 전 IsValid()를 호출해 유효성을 검사해야 한다.
    class XmlAttribute final
    {    
    public:
        // XmlAttribute는 `data`를 소멸할 때까지 이용한다.
        // `data`는 null일 수 있으며, 이 경우 빈 속성임을 나타낸다.
        explicit XmlAttribute(xml_attr_t* data = nullptr)
            : _data(data) {
        }

        XmlAttribute(const XmlAttribute&) = default;
        XmlAttribute& operator=(const XmlAttribute&) = default;

        bool IsValid() const noexcept { return _data != nullptr; }

        // 반환 값은 빈 문자열("")일 수 있다.
        std::string_view GetName() const noexcept
        {
            if (IsValid() == false)
            {
                return "";
            }

            return std::string_view{ _data->name(), _data->name_size() };
        }

        // 반환 값은 빈 문자열("")일 수 있다.
        std::string_view GetValue() const noexcept
        {
            if (IsValid() == false)
            {
                return "";
            }

            return std::string_view{ _data->value(), _data->value_size() };
        }
        
        template <typename T>
            requires std::integral<T> || std::floating_point<T>
        std::optional<T> GetValue() const noexcept
        {
            if (IsValid() == false)
            {
                return std::nullopt;
            }

            if (_data->value() == nullptr || _data->value_size() == 0) 
            {
                return std::nullopt;
            }

            T result{};
            const char* first = _data->value();
            const char* last = first + _data->value_size();
            
            auto [ptr, ec] = std::from_chars(first, last, result);
            
            if (ec == std::errc{}) 
            {
                return result;
            }
            
            return std::nullopt;
        }
    private:
        xml_attr_t* _data;
    };

    // Xml 노드를 나타낸다.
    // 항상 사용하기 전 IsValid()를 호출해 유효성을 검사해야 한다.
    class XmlNode final
    {
    public:
        // XmlNode는 `data`를 소멸할 때까지 이용한다.
        // `data`는 null일 수 있으며, 이 경우 빈 노드임을 나타낸다.  
        explicit XmlNode(xml_node_t* data = nullptr)
            : _data(data)
        {
        }

        XmlNode(const XmlNode&) = default;
        XmlNode& operator=(const XmlNode&) = default;

        bool IsValid() const noexcept { return _data != nullptr; }

        // 반환 값은 빈 문자열("")일 수 있다.
        std::string_view GetName() const noexcept
        {
            if (IsValid() == false)
            {
                return "";
            }

            return std::string_view{ _data->name(), _data->name_size() };
        }
        
        // 반환 값은 빈 문자열("")일 수 있다.
        std::string_view GetValue() const noexcept
        {
            if (IsValid() == false)
            {
                return "";
            }

            return std::string_view{ _data->value(), _data->value_size() };
        }

        template<typename T>
            requires std::integral<T> || std::floating_point<T>
        std::optional<T> GetValue() const noexcept
        {
            if (IsValid() == false)
            {
                return std::nullopt;
            }

            if (_data->value() == nullptr || _data->value_size() == 0)
            {
                return std::nullopt;
            }

            T result{};
            const char* first = _data->value();
            const char* last = first + _data->value_size();

            auto [ptr, ec] = std::from_chars(first, last, result);

            if (ec == std::errc{})
            {
                return result;
            }

            return std::nullopt;
        }

        XmlAttribute GetAttribute(std::string_view name) const noexcept
        {
            xml_attr_t* attr = _data->first_attribute(name.data());
            return XmlAttribute(attr);
        }

        XmlNode GetSiblingNode(std::string_view name) const noexcept
        {
            xml_node_t* sibling = _data->next_sibling(name.data());
            return XmlNode(sibling);
        }

        XmlNode GetChild(std::string_view name) const noexcept
        {
            xml_node_t* child = _data->first_node(name.data());
            return XmlNode(child);
        }
    private:
        xml_node_t* _data;
    };

    // Xml문서를 나타낸다.
    // 사용 전 Parse()를 호출해야 한다.
    // 사용 예시:
    //  XmlDodument doc;
    //  doc.ParseFromFile(filesystem::path{ "config.xml" });
    //  XmlNode rootNode = doc.GetNode("root");
    class XmlDocument final
    {
    public:
        XmlDocument() noexcept = default;

        XmlDocument(const XmlDocument&) = delete;
        XmlDocument& operator = (const XmlDocument&) = delete;
        
        // XML 파일을 읽고 파싱한다.
        // 잘못된 경로라면 errc::no_such_file_or_directory 오류가 발생한다.
        // 파일을 읽을 수 없다면 errc::permission_denied 오류가 발생한다.
        // 파싱이 실패한 경우 errc::invalid_argument 오류가 발생한다.
        expected<void, error_code> ParseFromFile(std::filesystem::path xmlFilePath) noexcept;

        // `xmlContent`를 파싱한다.
        // 파싱이 실패한 경우 errc::invalid_argument 오류가 발생한다.
        expected<void, error_code> ParseFromString(std::string xmlContent) noexcept;

        XmlNode GetNode(std::string_view name) const noexcept
        {
            xml_node_t* node = _data.first_node(name.data());
            return XmlNode(node);
        }
    private:
        xml_doc_t _data;
        std::string _xmlContent;
    };
}
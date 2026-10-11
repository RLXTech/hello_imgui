#include "vd_path/vd_path.h"
#include "hello_imgui/internal/functional_utils.h"

#include <string>
#include <fstream>

namespace HelloImGui
{
    namespace FunctionalUtils
    {
        inline std::vector<std::string> split(const std::string& s, const std::string& token)
        {
            std::string str =s;
            std::vector<std::string>result;
            while(str.size())
            {
                size_t index = str.find(token);
                if(index != std::string::npos)
                {
                    result.push_back(str.substr(0,index));
                    str = str.substr(index + token.size());
                    if(str.size()==0)result.push_back(str);
                }
                else
                {
                    result.push_back(str);
                    str = "";
                }
            }
            return result;
        }

        std::vector<std::string> split_lines(const std::string& s)
        {
            return split(s, "\n");
        }

        std::string read_text_file_or_empty(const std::string& path)
        {
            constexpr auto read_size = std::size_t(4096);
            auto admitted = vdas::path::parseUtf8(path, "hello-text-read");
            if (!admitted) throw std::runtime_error("Invalid UTF-8 text file path");
            auto opened = vdas::path::openRead(admitted.value());
            if (!opened)
            {
                if (opened.error().code == vdas::path::PathErrorCode::NotFound) return "";
                throw std::runtime_error("Hello text read failed: " + opened.error().nativeError.message());
            }
            auto stream = std::move(opened).value();
            stream.exceptions(std::ios_base::badbit);

            auto out = std::string();
            auto buf = std::string(read_size, '\0');
            while (stream.read(& buf[0], read_size))
                out.append(buf, 0, stream.gcount());
            out.append(buf, 0, stream.gcount());
            return out;
        }

        void write_text_file(const std::string& path, const std::string& content)
        {
            auto admitted = vdas::path::parseUtf8(path, "hello-text-write");
            if (!admitted) throw std::runtime_error("Invalid UTF-8 text file path");
            auto opened = vdas::path::openWrite(admitted.value());
            if (!opened) throw std::runtime_error("Hello text write failed: " + opened.error().nativeError.message());
            auto stream = std::move(opened).value();
            stream << content;
            stream.flush();
            if (!stream) throw std::runtime_error("Hello text write/flush failed");
        }

        std::string lower_string(const std::string& s)
        {
            std::string r;
            for (auto c: s)
                c += std::tolower(c);
            return r;
        }


    }
}
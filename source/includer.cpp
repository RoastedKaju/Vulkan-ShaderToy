#include "includer.hpp"

shaderc_include_result *Includer::GetInclude(const char *requested_source, shaderc_include_type type, const char *requesting_source, size_t)
{
    std::filesystem::path includePath;

    if (type == shaderc_include_type_relative)
    {
        includePath = std::filesystem::path(requesting_source).parent_path() / requested_source;
    }
    else
    {
        includePath = requested_source;
    }

    includePath = std::filesystem::weakly_canonical(includePath);

    auto *result = new shaderc_include_result();

    std::ifstream file(includePath, std::ios::binary);
    if (!file)
    {
        std::string error = "Failed to open include file: " + includePath.string();

        char *errorMsg = new char[error.size() + 1];
        memcpy(errorMsg, error.c_str(), error.size() + 1);

        result->source_name = "";
        result->source_name_length = 0;
        result->content = errorMsg;
        result->content_length = error.size();
        result->user_data = errorMsg;

        return result;
    }

    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    char *content = new char[source.size() + 1];
    memcpy(content, source.c_str(), source.size() + 1);

    std::string filename = includePath.string();
    char *name = new char[filename.size() + 1];
    memcpy(name, filename.c_str(), filename.size() + 1);

    result->source_name = name;
    result->source_name_length = filename.size();
    result->content = content;
    result->content_length = source.size();
    result->user_data = new std::pair<char *, char *>(name, content);

    return result;
}

void Includer::ReleaseInclude(shaderc_include_result *data)
{
    if (data->source_name_length == 0)
    {
        delete[] data->content;
    }
    else
    {
        auto *pair = static_cast<std::pair<char *, char *> *>(data->user_data);
        delete[] pair->first;
        delete[] pair->second;
        delete pair;
    }

    delete data;
}

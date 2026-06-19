#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <shaderc/shaderc.hpp>

class Includer : public shaderc::CompileOptions::IncluderInterface
{
public:
    shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t /*include depth*/) override;
    void ReleaseInclude(shaderc_include_result* data) override;
};
#ifndef FLIPS_WRAPPER_FLIPS_WRAPPER_H
#define FLIPS_WRAPPER_FLIPS_WRAPPER_H

#include <filesystem>
#include <utility>
#include <optional>

#include <boost/process.hpp>

#include "fmt/format.h"

namespace fs = std::filesystem;
namespace bp = boost::process;

namespace flips_wrapper {

    enum class PatchType {
        IPS,
        BPS,
        BPS_DELTA,
        BPS_LINEAR,
        BPS_DELTA_MORE_MEMORY
    };

    struct Result{
    public:
        const std::vector<std::string> out{};
        const std::vector<std::string> err{};

        const bool succeeded{};
        const std::optional<int> error_code{};

        Result(int exit_code, std::vector<std::string> out, std::vector<std::string> err)
                : out(std::move(out)), err(std::move(err)), succeeded(exit_code == 0),
                  error_code(exit_code == 0 ? std::nullopt : std::make_optional(exit_code)) {}
    };

    class FLIPSWrapperException : public std::runtime_error {
    private:
        const Result result;

    public:
        FLIPSWrapperException(const std::string& message, Result result)
                : std::runtime_error(message), result(std::move(result)) {}

        [[nodiscard]] const Result& getResult() const {
            return result;
        }
    };

    class FLIPSWrapper {
    protected:
        const fs::path flips_path;

        static std::string convertPatchTypeToString(PatchType patch_type);

        template<typename ...Args>
        Result call(Args... args) {
            bp::ipstream out;
            bp::ipstream err;

            auto child{ bp::child(args..., bp::std_out > out, bp::std_err > err) };

            std::vector<std::string> out_vec;
            std::vector<std::string> err_vec;

            std::string line;
            while (child.running() && std::getline(out, line) && !line.empty()) {
                out_vec.push_back(line);
            }

            std::string err_line;
            while (child.running() && std::getline(err, err_line) && !err_line.empty()) {
                err_vec.push_back(err_line);
            }

            child.wait();

            return { child.exit_code(), out_vec, err_vec };
        }

    public:
        explicit FLIPSWrapper(fs::path flips_path) : flips_path(std::move(flips_path)) {}

        // TODO support -m/--manifest and -mfilename/--manifest=filename
        Result createPatch(const fs::path& clean_rom_path, const fs::path& modified_rom_path,
                           const std::optional<fs::path>& patch_patch = std::nullopt, bool exact = false,
                           const std::optional<PatchType>& patch_type = std::nullopt);

        Result applyPatch(const fs::path& clean_rom_path, const fs::path& patch_path,
                        const std::optional<fs::path>& output_rom_path = std::nullopt,
                        bool exact = false, bool ignore_checksum = false);
    };
}

#endif //FLIPS_WRAPPER_FLIPS_WRAPPER_H

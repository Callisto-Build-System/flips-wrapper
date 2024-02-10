#include "../include/flips_wrapper.h"

namespace flips_wrapper {

    std::string FLIPSWrapper::convertPatchTypeToString(PatchType patch_type) {
        switch (patch_type) {
            case PatchType::BPS:
                return "--bps";

            case PatchType::BPS_DELTA:
                return "--bps-delta";

            case PatchType::BPS_LINEAR:
                return "--bps-linear";

            case PatchType::BPS_DELTA_MORE_MEMORY:
                return "--bps-delta-moremem";

            case PatchType::IPS:
                return "--ips";
        }

        throw std::runtime_error("Unknown patch type passed");
    }

    Result FLIPSWrapper::createPatch(const fs::path &clean_rom_path, const fs::path &modified_rom_path,
                                     const std::optional<fs::path>& patch_path, bool exact,
                                     const std::optional<PatchType> &patch_type) {

        const auto res = [&] {
            if (patch_type) {
                const auto patch_option{convertPatchTypeToString(*patch_type) };
                if (exact) {
                    if (patch_path) {
                        return call(flips_path.string(), "--create", "--exact", patch_option,
                                    clean_rom_path.string(), modified_rom_path.string(), patch_path->string());
                    } else {
                        return call(flips_path.string(), "--create", "--exact", patch_option,
                                    clean_rom_path.string(), modified_rom_path.string());
                    }
                } else {
                    if (patch_path) {
                        return call(flips_path.string(), "--create", patch_option,
                                    clean_rom_path.string(), modified_rom_path.string(), patch_path->string());
                    } else {
                        return call(flips_path.string(), "--create", patch_option,
                                    clean_rom_path.string(), modified_rom_path.string());
                    }
                }
            } else {
                if (exact) {
                    if (patch_path) {
                        return call(flips_path.string(), "--create", "--exact", clean_rom_path.string(), modified_rom_path.string(),
                                    patch_path->string());
                    } else {
                        return call(flips_path.string(), "--create", "--exact", clean_rom_path.string(),
                                    modified_rom_path.string());
                    }
                } else {
                    if (patch_path) {
                        return call(flips_path.string(), "--create", clean_rom_path.string(), modified_rom_path.string(),
                                    patch_path->string());
                    } else {
                        return call(flips_path.string(), "--create", clean_rom_path.string(), modified_rom_path.string());
                    }
                }
            }
        }();

        if (!res.succeeded) {
            throw FLIPSWrapperException(fmt::format(
                "Failed to create BPS patch with arguments:\n"
                "Base ROM: {}\n"
                "Modified ROM: {}\n"
                "Output BPS patch: {}\n"
                "Exact: {}\n"
                "Patch Type: {}\n",
                clean_rom_path.string(), modified_rom_path.string(),
                patch_path ? patch_path->string() : "nullopt", exact,
                patch_type ? convertPatchTypeToString(*patch_type) : "nullopt"
            ), res);
        }

        return res;
    }

    Result FLIPSWrapper::applyPatch(const fs::path &clean_rom_path, const fs::path &patch_path,
                                    const std::optional<fs::path> &output_rom_path, bool exact, bool ignore_checksum) {
        const auto res = [&] {
            if (output_rom_path) {
                if (exact) {
                    if (ignore_checksum) {
                        return call(flips_path.string(), "--apply", "--exact", "--ignore-checksum", patch_path.string(),
                                    clean_rom_path.string(), output_rom_path->string());
                    } else {
                        return call(flips_path.string(), "--apply", "--exact", patch_path.string(),
                                    clean_rom_path.string(), output_rom_path->string());
                    }
                } else {
                    if (ignore_checksum) {
                        return call(flips_path.string(), "--apply", "--ignore-checksum", patch_path.string(),
                                    clean_rom_path.string(), output_rom_path->string());
                    } else {
                        return call(flips_path.string(), "--apply", patch_path.string(),
                                    clean_rom_path.string(), output_rom_path->string());
                    }
                }
            } else {
                if (exact) {
                    if (ignore_checksum) {
                        return call(flips_path.string(), "--apply", "--exact", "--ignore-checksum", patch_path.string(),
                                    clean_rom_path.string());
                    } else {
                        return call(flips_path.string(), "--apply", "--exact", patch_path.string(),
                                    clean_rom_path.string());
                    }
                } else {
                    if (ignore_checksum) {
                        return call(flips_path.string(), "--apply", "--ignore-checksum", patch_path.string(),
                                    clean_rom_path.string());
                    } else {
                        return call(flips_path.string(), "--apply", patch_path.string(),
                                    clean_rom_path.string());
                    }
                }
            }
        }();

        if (!res.succeeded) {
            throw FLIPSWrapperException(fmt::format(
                    "Failed to apply BPS patch with arguments:\n"
                    "Base ROM: {}\n"
                    "Output ROM: {}\n"
                    "BPS patch: {}\n"
                    "Exact: {}\n"
                    "Ignore Checksum: {}\n",
                    clean_rom_path.string(), output_rom_path ? output_rom_path->string() : "nullopt",
                    patch_path.string(), exact, ignore_checksum
            ), res);
        }

        return res;
    }
}

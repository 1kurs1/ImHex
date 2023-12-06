#include <hex/api/layout_manager.hpp>

#include <hex/helpers/fs.hpp>
#include <hex/helpers/logger.hpp>
#include <wolv/utils/string.hpp>

#include <imgui.h>
#include <hex/api/content_registry.hpp>

namespace hex {

    namespace {

        std::optional<std::fs::path> s_layoutPathToLoad;
        std::optional<std::string> s_layoutStringToLoad;
        std::vector<LayoutManager::Layout> s_layouts;

        bool s_layoutLocked = false;

    }


    void LayoutManager::load(const std::fs::path &path) {
        s_layoutPathToLoad = path;
    }

    void LayoutManager::loadString(const std::string &content) {
        s_layoutStringToLoad = content;
    }

    void LayoutManager::save(const std::string &name) {
        auto fileName = name;
        fileName = wolv::util::replaceStrings(fileName, " ", "_");
        std::transform(fileName.begin(), fileName.end(), fileName.begin(), tolower);
        fileName += ".hexlyt";

        std::fs::path layoutPath;
        for (const auto &path : hex::fs::getDefaultPaths(fs::ImHexPath::Layouts)) {
            if (!hex::fs::isPathWritable(layoutPath))
                continue;

            layoutPath = path / fileName;
        }

        if (layoutPath.empty()) {
            log::error("Failed to save layout '{}'. No writable path found", name);
            return;
        }

        const auto pathString = wolv::util::toUTF8String(layoutPath);
        ImGui::SaveIniSettingsToDisk(pathString.c_str());
        log::info("Layout '{}' saved to {}", name, pathString);

        LayoutManager::reload();
    }

    std::vector<LayoutManager::Layout> LayoutManager::getLayouts() {
        return s_layouts;
    }

    void LayoutManager::process() {
        if (s_layoutPathToLoad.has_value()) {
            const auto pathString = wolv::util::toUTF8String(*s_layoutPathToLoad);
            ImGui::LoadIniSettingsFromDisk(pathString.c_str());
            s_layoutPathToLoad = std::nullopt;
            log::info("Loaded layout from {}", pathString);
        }

        if (s_layoutStringToLoad.has_value()) {
            ImGui::LoadIniSettingsFromMemory(s_layoutStringToLoad->c_str());
            s_layoutStringToLoad = std::nullopt;
            log::info("Loaded layout from string");
        }
    }

    void LayoutManager::reload() {
        s_layouts.clear();

        for (const auto &directory : hex::fs::getDefaultPaths(fs::ImHexPath::Layouts)) {
            for (const auto &entry : std::fs::directory_iterator(directory)) {
                const auto &path = entry.path();

                if (path.extension() != ".hexlyt")
                    continue;

                auto name = path.stem().string();
                name = wolv::util::replaceStrings(name, "_", " ");
                for (size_t i = 0; i < name.size(); i++) {
                    if (i == 0 || name[i - 1] == '_')
                        name[i] = char(std::toupper(name[i]));
                }

                s_layouts.push_back({
                    name,
                    path
                });
            }
        }
    }

    void LayoutManager::reset() {
        s_layoutPathToLoad.reset();
        s_layoutStringToLoad.reset();
        s_layouts.clear();
    }

    bool LayoutManager::isLayoutLocked() {
        return s_layoutLocked;
    }

    void LayoutManager::lockLayout(bool locked) {
        log::info("Layout {}", locked ? "locked" : "unlocked");
        s_layoutLocked = locked;
    }

}

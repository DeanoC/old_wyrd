#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include "optional_wrapper.h"
#include "json.h"

namespace propertyeditor {
    class PropertyEditorMetaData
    {
    public:
        enum class Type
        {
            Natural,
            Integer,
            Float,
            String,
            Colour,
        };
        template<typename T>
        static bool isSameAsJsonType(nlohmann::json::value_t type)
        {
            switch(type)
            {

                case nlohmann::detail::value_t::null:
                    return typeid(T) == typeid(nullptr_t);
                case nlohmann::detail::value_t::object:
                    return false;
                case nlohmann::detail::value_t::array:
                    return false;
                case nlohmann::detail::value_t::string:
                    return typeid(T) == typeid(std::string);
                case nlohmann::detail::value_t::boolean:
                    return typeid(T) == typeid(bool);
                case nlohmann::detail::value_t::number_integer:
                    return typeid(T) == typeid(int);
                case nlohmann::detail::value_t::number_unsigned:
                    return typeid(T) == typeid(unsigned int);
                case nlohmann::detail::value_t::number_float:
                    return typeid(T) == typeid(float);
                case nlohmann::detail::value_t::discarded:
                    return false;
            }
        }

        PropertyEditorMetaData(nlohmann::json const& metadata_) : metadata(metadata_) {}


        template<typename T>
        std::optional<T> hasAttribute(std::string const& key, char const*const attrib, char const*const altern = nullptr) const
        {
            if (metadata.is_object()) {
                auto meta = metadata.find(key);
                if (meta != metadata.end()) {
                    nlohmann::json info = meta.value();
                    if(attrib) {
                        auto at = info.find(attrib);
                        if (at != info.end() && isSameAsJsonType<T>(at->type())) {
                            return at->get<T>();
                        }
                    }
                    if(altern) {
                        auto aat = info.find(altern);
                        if (aat != info.end() && isSameAsJsonType<T>(aat->type())) {
                            return aat->get<T>();
                        }
                    }
                }
            }
            return {};
        }

        Type getType(std::string const& key) const
        {
            if (metadata.is_object()) {
                auto meta = metadata.find(key);
                if (meta != metadata.end()) {
                    nlohmann::json info = meta.value();
                    auto type = info.find("type");
                    if(type != info.end() && type->is_string() ) {
                        if (*type == "color" || *type == "colour")
                        {
                            return Type::Colour;
                        }
                        if(*type == "float")
                        {
                            return Type::Float;
                        }
                        if(*type == "integer")
                        {
                            return Type::Integer;
                        }
                    }
                }
            }
            return Type::Natural;
        }
        std::string getLabel(std::string const& key)
        {
            nlohmann::json metaField;

            auto meta = metadata.find(key);
            if(meta != metadata.end())
            {
                metaField = meta.value();
            }

            if(metaField.is_object())
            {
                auto const& label = metaField.find("label");
                if(label != metaField.end())
                {
                    return label.value();
                }
            }
            return key;
        }
        template<typename Type, int COUNT>
        void coerceIn(nlohmann::json const& a, Type* out)
        {
            if (a.is_array()) {
                auto &array = a;
                if (array.size() == 0) {
                    for (int i = 0; i < COUNT; ++i) {
                        out[i] = 0;
                    }
                    return;
                }

                for (int i = 0; i < COUNT; ++i) {
                    nlohmann::json const& value = a[i];

                    if (value.is_number_float()){
                        if(typeid(Type) == typeid(float)){
                            out[i] = array[i].get<float>();
                        } else
                        {
                            out[i] = static_cast<float>(array[i]);
                        }
                    } else if (value.is_number_integer()){
                        if(typeid(Type) == typeid(int)){
                            out[i] = array[i].get<int>();
                        } else
                        {
                            out[i] = static_cast<int>(array[i]);
                        }
                    }
                }
            }
        }
    private:
        nlohmann::json metadata;
    };

    void PropertyEditJsonObject(nlohmann::json &j, const nlohmann::json &metaObj) {
        using namespace nlohmann;

        PropertyEditorMetaData metaData(metaObj);
        using pemd = PropertyEditorMetaData;

        for (auto it : j.items())
        {
            if(it.key() == "__metadata__")
                continue;

            if(it.value().is_object())
            {
                ImGui::AlignTextToFramePadding();
                bool node_open = ImGui::TreeNode(it.key().c_str(), "%s", it.key().c_str());
                ImGui::NextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::NextColumn();
                if(node_open) {
                    PropertyEditJsonObject(it.value(), metaObj);
                    ImGui::TreePop();
                }
            } else
            {
                ImGui::AlignTextToFramePadding();
                ImGui::Bullet();

                std::string labelText = metaData.getLabel(it.key());
                ImGui::Selectable(labelText.c_str());
                ImGui::NextColumn();
                labelText = "##" + labelText;
                char const*const label = labelText.c_str();

                auto const tooltip = metaData.hasAttribute<std::string>(it.key(), "tooltip");
                auto const mut = metaData.hasAttribute<bool>(it.key(), "mut", "mutate");

                if(tooltip.has_value() && ImGui::IsItemHovered())
                {
                    std::string const& tts = *tooltip;
                    ImGui::SetTooltip("%s", tts.c_str());
                }
                bool const readOnly = mut.has_value() ? !mut.value() : true;
                if (readOnly)
                {
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.6f);
                }

                ImGui::PushItemWidth(-1);

                if (it.value().is_string()) {
                    char buf[1024];
                    std::string value = it.value().get<std::string>();
                    size_t value_size = std::min(value.size(), size_t(1024));
                    memcpy(buf, value.c_str(), value_size);
                    buf[value_size] = 0;
                    ImGuiInputTextFlags flags = 0;
                    flags |= readOnly ? ImGuiInputTextFlags_ReadOnly : 0;
                    ImGui::InputText(label, buf, 1024,flags);
                    if (!readOnly) { it.value() = buf; }
                } else if (it.value().is_boolean()) {
                    bool value = it.value().get<bool>();
                    ImGui::Checkbox("", &value);
                    if (!readOnly) { it.value() = value; }
                } else if (it.value().is_number_float()) {
                    float min = -10.0f;
                    float max = 10.0f;
                    if (metaObj.is_object()) {
                        auto meta = metaObj.find(it.key());
                        if (meta != metaObj.end()) {
                            nlohmann::json info = meta.value();
                            min = info["min"].get<float>();
                            max = info["max"].get<float>();
                        }
                    }
                    float value = it.value().get<float>();
                    ImGui::SliderFloat(label, &value, min, max);
                    if (!readOnly) { it.value() = value; }
                } else if (it.value().is_number_integer()) {
                    int min = 0;
                    int max = 255;
                    int inc = 1;
                    std::optional<int> mino = metaData.hasAttribute<int>(it.key(), "min");
                    std::optional<int> maxo = metaData.hasAttribute<int>(it.key(), "max");
                    std::optional<int> inco = metaData.hasAttribute<int>(it.key(), "inc");
                    if(mino.has_value()) { min = *mino; }
                    if(maxo.has_value()) { max = *maxo; }
                    if(inco.has_value()) { inc = *inco; }

                    int value = it.value().get<int>();
                    ImGui::SliderInt(label, &value, min, max);
                    if (!readOnly) { it.value() = value; }
                } else if (it.value().is_array()) {
                    auto& array = it.value();
                    if(array.size() == 0) continue;

                    bool smallVecOpt = false;
                    if(array.size() >=2 && array.size() <= 4)
                    {
                        float fv[4];
                        int iv[4];
                        bool isAllFloat = true;
                        bool isAllInt = true;
                        int i = 0;
                        for( auto& val : array) {
                            if (val.is_number_float()) {
                                isAllInt = false;
                                fv[i] = val;
                            } else if(val.is_number_integer())
                            {
                                isAllFloat = true;
                                iv[i] = val;
                            } else
                            {
                                isAllFloat = false;
                                isAllInt = false;
                            }
                            ++i;
                        }
                        pemd::Type type = metaData.getType(it.key());

                        if(isAllFloat) {
                            switch(array.size()) {
                                case 2: {
                                    if(type != pemd::Type::Integer) {
                                        ImGui::InputFloat2(label, fv);
                                        array[0] = fv[0]; array[1] = fv[1];
                                    } else
                                    {
                                        metaData.coerceIn<int,2>(array,iv);
                                        ImGui::InputInt2(label, iv);
                                        array[0] = static_cast<float>(iv[0]);
                                        array[1] = static_cast<float>(iv[1]);
                                    }
                                } break;
                                case 3:{
                                    if(type != pemd::Type::Colour) {
                                        if(type != pemd::Type::Integer) {
                                            ImGui::InputFloat3(label, fv);
                                            array[0] = fv[0];array[1] = fv[1];array[2] = fv[2];
                                        } else
                                        {
                                            iv[0] = static_cast<int>(fv[0]);
                                            iv[1] = static_cast<int>(fv[1]);
                                            iv[2] = static_cast<int>(fv[2]);
                                            ImGui::InputInt3(label, iv);
                                            array[0] = static_cast<float>(iv[0]);
                                            array[1] = static_cast<float>(iv[1]);
                                            array[2] = static_cast<float>(iv[2]);
                                        }
                                    } else
                                    {
                                        ImGui::ColorEdit3(label, fv);
                                        array[0] = fv[0]; array[1] = fv[1]; array[2] = fv[2];
                                    }
                                } break;
                                case 4:{
                                    if(type != pemd::Type::Colour) {
                                        if(type != pemd::Type::Integer) {
                                            ImGui::InputFloat4(label, fv);
                                            array[0] = fv[0]; array[1] = fv[1]; array[2] = fv[2]; array[3] = fv[3];
                                        } else
                                        {
                                            iv[0] = static_cast<int>(fv[0]);
                                            iv[1] = static_cast<int>(fv[1]);
                                            iv[2] = static_cast<int>(fv[2]);
                                            iv[3] = static_cast<int>(fv[3]);
                                            ImGui::InputInt4(label, iv);
                                            array[0] = static_cast<float>(iv[0]);
                                            array[1] = static_cast<float>(iv[1]);
                                            array[2] = static_cast<float>(iv[2]);
                                            array[3] = static_cast<float>(iv[3]);
                                        }
                                    } else
                                    {
                                        ImGui::ColorEdit4(label, fv);
                                        array[0] = fv[0]; array[1] = fv[1]; array[2] = fv[2]; array[3] = fv[3];
                                    }
                                } break;
                            }
                            smallVecOpt = true;
                        }
                        if(isAllInt){

                            switch(array.size()) {
                                case 2: {
                                    ImGui::InputInt2(label, iv);
                                    array[0] = iv[0]; array[1] = iv[1];
                                } break;
                                case 3:{
                                    ImGui::InputInt3(label, iv);
                                    array[0] = iv[0]; array[1] = iv[1]; array[2] = iv[2];
                                } break;
                                case 4:{
                                    ImGui::InputInt4(label, iv);
                                    array[0] = iv[0]; array[1] = iv[1]; array[2] = iv[2]; array[3] = iv[3];
                                } break;
                            }
                            smallVecOpt = true;
                        }
                    }
                    if(smallVecOpt == false) {
                        int count = 0;
                        for( auto& val : array)
                        {
                            std::string lab = labelText + std::to_string(count);
                            if(val.is_number_float())
                            {
                                float value = val.get<float>();
                                ImGui::InputFloat(lab.c_str(), &value);
                                if (!readOnly) { val = value; }
                            } else if(val.is_number_integer())
                            {
                                int value = val.get<int>();
                                ImGui::InputInt(lab.c_str(), &value);
                                if (!readOnly) { val = value; }
                            } else if(val.is_string()) {
                                char buf[1024];
                                std::string value = val.get<std::string>();
                                size_t value_size = std::min(value.size(), size_t(1024));
                                memcpy(buf, value.c_str(), value_size);
                                buf[value_size] = 0;
                                ImGuiInputTextFlags flags = 0;
                                flags |= readOnly ? ImGuiInputTextFlags_ReadOnly : 0;
                                ImGui::InputText(lab.c_str(), buf, 1024, flags);
                                if (!readOnly) { val = buf; }
                            }
                            ++count;
                        }
                    }
                }
                if(tooltip.has_value() && ImGui::IsItemHovered())
                {
                    std::string const& tts = *tooltip;
                    ImGui::SetTooltip("%s", tts.c_str());
                }

                ImGui::PopItemWidth();
                if (readOnly)
                {
                    ImGui::PopStyleVar();
                    ImGui::PopItemFlag();
                }
                ImGui::NextColumn();
            }
        }
    }

void propertyEditor(nlohmann::json& j)
{
    using namespace nlohmann;

    ImGui::Begin("Property Editor");

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, 0x80808080);
    ImGui::Columns(2);
    ImGui::SetColumnWidth(-1, ImGui::GetFont()->FallbackAdvanceX * 20.0f);
    ImGui::Separator();

    json metaObj;
    json::iterator metaIt = j.find("__metadata__");
    if (metaIt != j.end() && metaIt.value().is_object())
    {
        metaObj = metaIt.value();
    }

    PropertyEditJsonObject(j, metaObj);

    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::End();
}

} // end namespace propertyeditor


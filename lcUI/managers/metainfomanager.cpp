#include "metainfomanager.h"

using namespace lc;
using namespace ui;

MetaInfoManager::MetaInfoManager() :
    _linePattern(nullptr),
    _color(nullptr),
    _lineWidth(nullptr) {

}

meta::MetaInfo_CSPtr MetaInfoManager::metaInfo() {
    meta::MetaInfo_SPtr metaInfo = nullptr;

    metaInfo = addMetaType(metaInfo, _linePattern);
    metaInfo = addMetaType(metaInfo, _lineWidth);
    metaInfo = addMetaType(metaInfo, _color);

    return metaInfo;
}

meta::MetaInfo_SPtr MetaInfoManager::addMetaType(meta::MetaInfo_SPtr metaInfo, meta::EntityMetaType_CSPtr metaType) {
    if(metaType != nullptr) {
        if(metaInfo == nullptr) {
            metaInfo = meta::MetaInfo::create();
        }

        metaInfo->add(std::move(metaType));
    }

    return metaInfo;
}

meta::DxfLinePattern_CSPtr MetaInfoManager::linePattern() const {
    return _linePattern;
}

meta::MetaColor_CSPtr MetaInfoManager::color() const {
    return _color;
}

meta::MetaLineWidth_CSPtr MetaInfoManager::lineWidth() const {
    return _lineWidth;
}

void MetaInfoManager::setLinePattern(const meta::DxfLinePattern_CSPtr& linePattern) {
    _linePattern = linePattern;
}

void MetaInfoManager::setColor(const meta::MetaColor_CSPtr& color) {
    _color = color;
}

void MetaInfoManager::setLineWidth(const meta::MetaLineWidth_CSPtr& lineWidth) {
    _lineWidth = lineWidth;
}

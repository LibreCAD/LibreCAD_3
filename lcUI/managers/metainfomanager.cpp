#include "metainfomanager.h"

using namespace lc;
using namespace ui;

MetaInfoManager::MetaInfoManager() :
    _linePattern(nullptr),
    _color(nullptr),
    _lineWidth(nullptr) {

}

MetaInfo_CSPtr MetaInfoManager::metaInfo() {
    MetaInfo_SPtr metaInfo = nullptr;

    metaInfo = addMetaType(metaInfo, _linePattern);
    metaInfo = addMetaType(metaInfo, _lineWidth);
    metaInfo = addMetaType(metaInfo, _color);

    return metaInfo;
}

MetaInfo_SPtr MetaInfoManager::addMetaType(MetaInfo_SPtr metaInfo, EntityMetaType_CSPtr metaType) {
    if(metaType != nullptr) {
        if(metaInfo == nullptr) {
            metaInfo = MetaInfo::create();
        }

        metaInfo->add(metaType);
    }

    return metaInfo;
}

DxfLinePattern_CSPtr MetaInfoManager::linePattern() const {
    return _linePattern;
}

MetaColor_CSPtr MetaInfoManager::color() const {
    return _color;
}

MetaLineWidth_CSPtr MetaInfoManager::lineWidth() const {
    return _lineWidth;
}

void MetaInfoManager::setLinePattern(const DxfLinePattern_CSPtr& linePattern) {
    _linePattern = linePattern;
}

void MetaInfoManager::setColor(const MetaColor_CSPtr& color) {
    _color = color;
}

void MetaInfoManager::setLineWidth(const MetaLineWidth_CSPtr& lineWidth) {
    _lineWidth = lineWidth;
}

#include "uisettings.h"

#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>

#include <memory>
#include <fstream>
#include <iostream>
#include <sstream>

namespace
{
const char* defaultUiSettings = R"(
{
    "toolbar": {
        "tabs": [
            {
                "label": "Quick Access",
                "groups": [
                    {
                        "label": "Creation",
                        "width": 3,
                        "buttons": [ "LineOperations", "PointOperations", "EllipseOperations", "SplineOperations", "LWPolylineOperations", "CircleOperations", "ArcOperations", "TextOperations" ]
                    },
                    {
                        "label": "Modify",
                        "width": 3,
                        "buttons": [ "MoveOperation", "RemoveOperation", "CopyOperation", "ScaleOperation", "SplitOperation", "RotateOperation", "TrimOperation", "AreaOperation"]
                    },
                    {
                        "label": "Dimensions",
                        "width": 3,
                        "buttons": [ "DimAngularOperations", "DimAlignedOperations", "DimLinearOperations", "DimRadialOperations", "DimDiametricOperations" ]
                    },
                    {
                        "label": "Snap Options",
                        "width": 2,
                        "buttons": [ "SnapGrid", "SnapIntersection", "SnapMiddle", "SnapEntity" ]
                    }
                ]
            }
        ]
    },
    "dockPositions": [
        {
            "widget": "CliCommand",
            "position": 8,
            "proportion": 65
        },
        {
            "widget": "Layers",
            "position": 2
        },
        {
            "widget": "PropertyEditor",
            "position": 8,
            "proportion": 35
        },
        {
            "widget": "Toolbar",
            "position": 4
        }
    ]
}
)";

const char* settingSchema = R"(
{
   "$schema": "http://json-schema.org/schema#",
   "type": "object",
   "required": [
     "toolbar"
   ],
   "title": "Toolbar Data Schema",
   "properties": {
     "toolbar": {
       "type": "object",
       "properties": {
         "tabs": {
           "type": "array",
           "items": {
             "$ref": "#/definitions/tab"
           }
         }
       }
     }
   },
   "definitions": {
     "tab": {
       "type": "object",
       "required": [
         "label"
       ],
       "properties": {
         "label": {
           "type": "string"
         },
         "groups": {
           "type": "array",
           "items": {
             "$ref": "#/definitions/group"
           }
         }
       }
     },
     "group": {
       "type": "object",
       "required": [
         "label"
       ],
       "properties": {
         "label": {
           "type": "string"
         },
         "width": {
           "type": "number",
           "minimum": 0,
           "maximum": 10
         },
         "buttons": {
           "type": "array",
           "items": {
             "type": "string"
           }
         }
       }
     }
   }
}
)";

// An empty *object* document: getSettingsDocument() never hands back a kNullType
// document, because rapidjson asserts on IsObject() in HasMember()/AddMember()/
// operator[](const Ch*), which would abort the application instead of degrading.
rapidjson::Document emptySettingsDocument() {
    rapidjson::Document document;
    document.SetObject();
    return document;
}
}

using namespace lc::ui;

void UiSettings::writeSettings(widgets::CustomizeToolbar* customizeToolbar) {
    rapidjson::Document settingsDocument = getSettingsDocument(settingsFileName);
    if (settingsDocument.HasMember("toolbar")) {
        settingsDocument.RemoveMember("toolbar");
    }

    customizeToolbar->generateData(settingsDocument);
    writeSettingsFile(settingsDocument);
}

void UiSettings::readSettings(widgets::CustomizeToolbar* customizeToolbar, bool defaultSettings) {
    std::string fileName = defaultSettings ? defaultSettingsFileName : settingsFileName;
    rapidjson::Document settingsDocument = getSettingsDocument(fileName);

    customizeToolbar->readData(settingsDocument);
}

bool UiSettings::validateSettingsDocument(rapidjson::Document& inputDocument) {
    std::unique_ptr<std::istream> schemaFile=std::make_unique<std::ifstream>(_filePaths["settings_load"] + schemaFileName);
    if (schemaFile->fail())
        schemaFile = std::make_unique<std::istringstream>(settingSchema);

    if (schemaFile->fail()) {
        std::cout << "Schema file not found" << std::endl;
        return true;
    }

    rapidjson::IStreamWrapper isw(*schemaFile);
    rapidjson::Document schemaDocument;

    if (schemaDocument.ParseStream(isw).HasParseError()) {
        std::cout << "Schema file is invalid, not json format." << std::endl;
        return true;
    }

    rapidjson::SchemaDocument schema(schemaDocument);
    rapidjson::SchemaValidator validator(schema);

    if (!inputDocument.Accept(validator)) {
        rapidjson::StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        printf("Invalid schema: %s\n", sb.GetString());
        printf("Invalid keyword: %s\n", validator.GetInvalidSchemaKeyword());
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        printf("Invalid document: %s\n", sb.GetString());

        return false;
    }

    return true;
}

void UiSettings::writeDockSettings(const std::map<std::string, int>& positions, std::map<std::string, int>& proportions) {
    rapidjson::Document settingsDocument = getSettingsDocument(settingsFileName);
    rapidjson::Value dockPositions(rapidjson::kArrayType);

    for (auto iter = positions.begin(); iter != positions.end(); ++iter) {
        rapidjson::Value posValue(rapidjson::kObjectType);
        rapidjson::Value widgetname;
        widgetname.SetString(iter->first.c_str(), iter->first.size(), settingsDocument.GetAllocator());
        posValue.AddMember("widget", widgetname, settingsDocument.GetAllocator());
        posValue.AddMember("position", rapidjson::Value(iter->second), settingsDocument.GetAllocator());

        if (proportions.find(iter->first) != proportions.end()) {
            posValue.AddMember("proportion", rapidjson::Value(proportions[iter->first]), settingsDocument.GetAllocator());
        }

        dockPositions.PushBack(posValue, settingsDocument.GetAllocator());
    }

    if (settingsDocument.HasMember("dockPositions")) {
        settingsDocument.RemoveMember("dockPositions");
    }

    settingsDocument.AddMember("dockPositions", dockPositions, settingsDocument.GetAllocator());
    writeSettingsFile(settingsDocument);
}

std::map<std::string, int> UiSettings::readDockSettings(std::map<std::string, int>& proportions) {
    rapidjson::Document settingsDocument = getSettingsDocument(settingsFileName);
    std::map<std::string, int> positions;

    if (settingsDocument.HasMember("dockPositions") && settingsDocument["dockPositions"].IsArray()) {
        const rapidjson::Value& dockPositions = settingsDocument["dockPositions"];

        for (rapidjson::Value::ConstValueIterator iter = dockPositions.Begin(); iter != dockPositions.End(); ++iter) {
            if (!iter->IsObject()) {
                continue;
            }

            if (iter->HasMember("widget") && (*iter)["widget"].IsString() &&
                iter->HasMember("position") && (*iter)["position"].IsInt()) {
                std::string widgetName = (*iter)["widget"].GetString();
                int pos = (*iter)["position"].GetInt();
                positions[widgetName] = pos;

                if (iter->HasMember("proportion") && (*iter)["proportion"].IsInt()) {
                    int prop = (*iter)["proportion"].GetInt();
                    proportions[widgetName] = prop;
                }
            }
        }
    }

    return positions;
}

rapidjson::Document UiSettings::getSettingsDocument(std::string fileName) {
    std::unique_ptr<std::istream> settingsFile = std::make_unique<std::ifstream>(_filePaths["settings_load"] + fileName);

    if (settingsFile->fail() && fileName == settingsFileName) {
        std::cout <<__func__<<"("<<settingsFileName<<"): No settings file found, loading default settings." << std::endl;
        fileName = defaultSettingsFileName;
        std::cout << "Loading default settings: "<<_filePaths["settings_load"] + fileName<<std::endl;
        static_cast<std::ifstream*>(settingsFile.get())->open(_filePaths["settings_load"] + fileName);
    }

    // The default settings file can be missing too (a relocated executable, an
    // incomplete install). Fall back on the settings compiled into the binary
    // rather than handing the caller an unusable document.
    if (settingsFile->fail()) {
        std::cout <<__func__<<"("<<fileName<<"): Settings file not found, using built-in default settings." << std::endl;
        settingsFile = std::make_unique<std::istringstream>(defaultUiSettings);

        if (settingsFile->fail()) {
            return emptySettingsDocument();
        }
    }

    rapidjson::IStreamWrapper isw(*settingsFile);
    rapidjson::Document settingsDocument;

    if (settingsDocument.ParseStream(isw).HasParseError()) {
        std::cout << "Error with settings document, not in json format" << std::endl;
        return emptySettingsDocument();
    }

    if (!validateSettingsDocument(settingsDocument)) {
        return emptySettingsDocument();
    }

    // validateSettingsDocument() is permissive when the schema itself cannot be
    // loaded, so the top level type still has to be checked here.
    if (!settingsDocument.IsObject()) {
        std::cout << "Settings document is not a json object" << std::endl;
        return emptySettingsDocument();
    }

    return settingsDocument;
}

void UiSettings::writeSettingsFile(rapidjson::Document& document) {
    std::ofstream outFile(_filePaths["settings_load"] + settingsFileName);

    if (outFile.fail()) {
        std::cout << "File could not be opened" << std::endl;
        return;
    }

    rapidjson::OStreamWrapper osw(outFile);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);

    document.Accept(writer);
    outFile.close();
}

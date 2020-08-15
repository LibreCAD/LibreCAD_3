#pragma once

// Defination for hatch pattern
// May be this must be in diffrent namespace
namespace lc {
namespace objects {
struct Pattern {
    lc::geo::Area boundingBox;
    std::string name;
    std::vector<lc::entity::CADEntity_CSPtr> entities;
};
}
}

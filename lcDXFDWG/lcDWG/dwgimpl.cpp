#include "dwgimpl.h"
#include <stdio.h>
#include <cmath>

DWGimpl::DWGimpl(lc::StorageManager_SPtr s, lc::Document* d) {
    _storageManager = s;
    _document = d;
}

lc::Document* DWGimpl::document() {
    return _document;
}

lc::StorageManager_SPtr DWGimpl::storageManager() {
    return _storageManager;
}

int
DWGimpl::readFile(char* filename) {
    int error;
    error = dwg_read_file(filename, &data);

    if (!error) {
        ReadDWG(&data);
    }

    dwg_free(&data);
    /* This value is the return value for `main',
    so clamp it to either 0 or 1.  */
    return error ? 1 : 0;
}

void DWGimpl::addLine(dwg_object* obj) {
    int error, index;
    Dwg_Entity_LINE* line;
    dwg_point_3d start, end;
    index = dwg_obj_object_get_index(obj, &error);
    line = obj->tio.entity->tio.LINE;
    dwg_ent_line_get_start_point(line, &start, &error);
    dwg_ent_line_get_end_point(line, &end, &error);

    // Create a cross at position 0,0
    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(
                                                   start.x,
                                                   start.y),
                                               lc::geo::Coordinate(
                                                   end.x,
                                                   end.y),
                                               layer)).push();
    builder->execute();
}

void DWGimpl::addCircle(dwg_object* obj) {
    Dwg_Entity_CIRCLE* circle;
    int error, index;
    float radius;
    dwg_point_3d center;
    index = dwg_obj_object_get_index(obj, &error);
    circle = obj->tio.entity->tio.CIRCLE;
    radius = dwg_ent_circle_get_radius(circle, &error);
    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(
                                                     center.x, center.y),
                                                 radius,
                                                 layer)).push();
    builder->execute();
}

void
DWGimpl::addArc(dwg_object* obj) {
    Dwg_Entity_ARC* arc;
    int error, index;
    float radius, start_angle, end_angle;
    dwg_point_3d center;
    index = dwg_obj_object_get_index(obj, &error);
    arc = obj->tio.entity->tio.ARC;
    radius = dwg_ent_arc_get_radius(arc, &error);
    start_angle = dwg_ent_arc_get_start_angle(arc, &error);
    end_angle = dwg_ent_arc_get_end_angle(arc, &error);
    dwg_ent_arc_get_center(arc, &center, &error);
    double x_start = center.x + radius * cos(start_angle);
    double y_start = center.y + radius * sin(start_angle);
    double x_end = center.x + radius * cos(end_angle);
    double y_end = center.y + radius * sin(end_angle);
    //Assuming clockwise arcs.
    int large_arc = (end_angle - start_angle < 3.1415) ? 0 : 1;

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(
                                                  center.x, center.y),
                                              radius, start_angle, end_angle,
                                              layer)).push();
    builder->execute();

}
void
DWGimpl::addText(dwg_object* obj) {
    int error, index;
    dwg_point_2d ins_pt;
    Dwg_Entity_TEXT* text;
    char* text_value;
    index = dwg_obj_object_get_index(obj, &error);
    text = obj->tio.entity->tio.TEXT;
    //    text = dwg_object_to_TEXT(obj);
    text_value = dwg_ent_text_get_text(text, &error);
    dwg_ent_text_get_insertion_point(text, &ins_pt, &error);

    /*TODO: Juca, fix it properly: */
    if (text_value[0] == '&') {
        return;
    }
}

void
DWGimpl::OutputObject(dwg_object* obj) {
    if (!obj) {
        fprintf(stderr, "object is NULL\n");
        return;
    }

    if (dwg_get_type(obj) == DWG_TYPE_LINE) {
        addLine(obj);
    }

    //    if (dwg_get_type(obj) == DWG_TYPE_CIRCLE) {
    //        output_CIRCLE(obj);
    //    }

    //    if (dwg_get_type(obj) == DWG_TYPE_TEXT) {
    //        output_TEXT(obj);
    //    }

    //    if (dwg_get_type(obj) == DWG_TYPE_ARC) {
    //        output_ARC(obj);
    //    }
}

void DWGimpl::output_BLOCK_HEADER(dwg_object_ref* ref) {
    dwg_object* obj, *variable_obj;
    dwg_obj_block_header* hdr;
    int error;
    unsigned long abs_ref;
    obj = dwg_obj_reference_get_object(ref, &error);
    abs_ref = dwg_obj_ref_get_abs_ref(ref, &error);

    if (!ref) {
        fprintf(stderr, "Found null object reference. Could not output an SVG symbol for this BLOCK_HEADER\n");
        return;
    }

    if (!obj) {
        fprintf(stderr, "Found null ref->obj\n");
        return;
    }

    /* TODO: Review.  (This check avoids a segfault, but it is
    still unclear whether or not the condition is valid.)  */
    if (!dwg_object_to_object(obj, &error)) {
        fprintf(stderr, "Found null ref->obj->tio.object\n");
        return;
    }

    //    hdr = dwg_object_to_BLOCK_HEADER(obj);
    hdr = obj->tio.object->tio.BLOCK_HEADER;
    variable_obj = get_first_owned_object(obj, hdr);

    while (variable_obj) {
        OutputObject(variable_obj);
        variable_obj = get_next_owned_object(obj, variable_obj, hdr);
    }
}

void
DWGimpl::ReadDWG(dwg_data* dwg) {
    unsigned int i, num_hdr_objs;
    int error;
    dwg_object* obj;
    dwg_obj_block_header* hdr;
    dwg_obj_block_control* ctrl;
    dwg_object_ref** hdr_refs;

    hdr = dwg_get_block_header(dwg, &error);
    ctrl = dwg_block_header_get_block_control(hdr, &error);

    hdr_refs = dwg_obj_block_control_get_block_headers(ctrl, &error);

    num_hdr_objs = dwg_obj_block_control_get_num_entries(ctrl, &error);

    for (i = 0; i < num_hdr_objs; i++) {
        output_BLOCK_HEADER(hdr_refs[i]);
    }

    output_BLOCK_HEADER(dwg_obj_block_control_get_model_space(ctrl, &error));
    output_BLOCK_HEADER(dwg_obj_block_control_get_paper_space(ctrl, &error));

}

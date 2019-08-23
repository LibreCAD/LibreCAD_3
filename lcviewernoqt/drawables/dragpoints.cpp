#include "dragpoints.h"

using namespace lc;
using namespace lc::viewer;
using namespace lc::viewer::drawable;

void DragPoints::setPoints(event::DragPointsEvent const &points) {
	_points = points.dragPoints();
	_size = points.size();
}

void DragPoints::onDraw(event::DrawEvent const &event) const {
	double x = _size;
	double y = _size;

	double zeroCornerX = 0.;
	double zeroCornerY = 0.;

	event.painter().device_to_user(&zeroCornerX, &zeroCornerY);
	event.painter().device_to_user(&x, &y);

	double size = (x - zeroCornerX);

	event.painter().source_rgb(255, 255, 255);

	for(const auto& point : _points) {
		event.painter().rectangle(point.x() - size / 2, point.y() - size / 2, size, size);
	}
	event.painter().stroke();
}


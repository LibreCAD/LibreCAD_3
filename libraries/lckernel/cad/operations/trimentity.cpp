#include "trimentity.h"

#include "cad/functions/trim.h"


using namespace lc;

TrimEntity::TrimEntity(AbstractDocument* document) : Operation(document), Undoable("Trim Entity") {
}



void TrimEntity::undo() const {

}

void TrimEntity::redo() const {

}


void TrimEntity::processInternal() const {



}

void TrimEntity::trimCoordinate(const geo::Coordinate& coord) {
    _trimCoordinate = coord;
}

void TrimEntity::trimTwoEntities(const std::tr1::shared_ptr<const lc::CADEntity>& trimmedEntity, const std::tr1::shared_ptr<const lc::CADEntity>& limitEntity) {

    // QList<std::tr1::shared_ptr<const lc::CADEntity> > result = trimmedEntity->trim(limitEntity, this->_trimCoordinate);



    //    QList<geo::Coordinate> geo::GeoIntersect::Intersect()
}

void TrimEntity::addLimitingEntity(std::tr1::shared_ptr<const lc::CADEntity> cadEntity) {
    if (_limitingEntities.indexOf(cadEntity) == -1) {
        this->_trimmingEntities.append(cadEntity);
    }
}

void TrimEntity::removeLimitingEntity(std::tr1::shared_ptr<const lc::CADEntity> cadEntity) {
    this->_limitingEntities.removeOne(cadEntity);
}

void TrimEntity::addTrimmedEntity(std::tr1::shared_ptr<const lc::CADEntity> cadEntity) {
    // We cannot trim to ourselve
    if (_limitingEntities.indexOf(cadEntity) == -1) {
        this->_trimmingEntities.append(cadEntity);
    }
}



/** Intersection aproximation
template<class A, class B>
class Function
{
public:
  typedef A Domain;
  typedef B CoDomain;

  virtual ~Function() { }
  virtual B Index(A a) const=0;
};

template<class A, class B, class C>
class FunctionCompose : public Function<A,C>
{
public:
  FunctionCompose(Function<A,B> &f1, Function<B,C> &f2) : f1(f1), f2(f2) { }
  C Index(A a) const { return f2.Index(f1.Index(a)); }
private:
  Function<A,B> &f1;
  Function<B,C> &f2;
};

template<class T>
class SolvableCompose : public Function<float, float>
{
public:
  SolvableCompose(Function<float, T> &curve,
                  Function<T, float> &value)
    : compose(curve, value) { }
  float Index(float x) const { return compose.Index(x); }
private:
  FunctionCompose<float, T, float> compose;
};

class IntersectionPrimitive : public Function<float, Point2d>, public Function<Point2d, float>
{
public:
  Point2d Index(float val) const { return Curve(val); }
  float Index(Point2d p) const { return Distance(p); }

  virtual Point2d Curve(float val) const=0;
  virtual float Distance(Point2d p) const=0;
  float StartRange() const =0;
  float EndRange() const=0;
};

float Intersection1(IntersectionPrimitive &p1, IntersectionPrimitive &p2)
{
  SolvableCompose<Point2d> compose(p1,p2);
  float intersectionpoint = Solve(compose, p1.StartRange(), p1.EndRange());
  return intersectionpoint;
}
float Intersection2(IntersectionPrimitive &p1, IntersectionPrimitive &p2)
{
  SolvableCompose<Point2d> compose(p2,p1);
  float intersectionpoint = Solve(compose, p2.StartRange(), p2.EndRange());
  return intersectionpoint;
}
float Solve(const Function<float, float> &func, float t_0, float t_1);
float Solve(const Function<float, float> &func, float t_0, float t_1)
{
  float t = t_0;
  for(;t<t_1;)
    {
      float Ht = func.Index(t);
      if (Ht < 0.1) return t;
      t += Ht / 20.0;
    }
  return t_0;
}*/

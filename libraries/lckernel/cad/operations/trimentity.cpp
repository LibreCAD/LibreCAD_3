#include "trimentity.h"

#include "cad/functions/trim.h"


using namespace lc;

TrimEntity::TrimEntity(AbstractDocument* document,  shared_ptr<lc::Trim> trim) : Operation(document), _trim(trim), Undoable("Trim Entity") {
    _entityLayer = document->findEntityLayerByID(trim->trimmedShape()->id());
}

void TrimEntity::processInternal() const {
    QList<shared_ptr<const lc::CADEntity> > newEntities = _trim->result();

    if (newEntities.size() > 0) {
        for (int i = 0; i < newEntities.size(); ++i) {
            document()->addEntity(_entityLayer, newEntities.at(i));
        }

        document()->removeEntity(_trim->trimmedShape()->id());
    }
}

void TrimEntity::undo() const {
    QList<shared_ptr<const lc::CADEntity> > newEntities = _trim->result();

    if (newEntities.size() > 0) {
        for (int i = 0; i < newEntities.size(); ++i) {
            document()->removeEntity(newEntities.at(i)->id());
        }

        document()->addEntity(_entityLayer, _trim->trimmedShape());
    }
}

void TrimEntity::redo() const {
    processInternal();
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

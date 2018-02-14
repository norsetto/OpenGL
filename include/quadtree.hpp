#pragma once

#include <vector>
#include <iostream>

namespace quadtree {
  
  struct Point {
    float x = 0.0f;
    float y = 0.0f;

    Point(float x = 0.0f, float y = 0.0f) : x(x), y(y) {};
  };

  struct AABB {

    Point centre;
    Point halfSize;

    AABB(Point centre = {}, Point halfSize = {}) : centre(centre), halfSize(halfSize) {};

    bool contains(const Point& a) const {
      if (a.x <= centre.x + halfSize.x && a.x >= centre.x - halfSize.x)	{
	if (a.y <= centre.y + halfSize.y && a.y >= centre.y - halfSize.y) {
	  return true;
	}
      }
      return false;
    }

    bool intersects(const AABB& other) const {
      //this right > that left                                          this left < that right
      if (centre.x + halfSize.x >= other.centre.x - other.halfSize.x || centre.x - halfSize.x <= other.centre.x + other.halfSize.x) {
	// This bottom > that top
	if (centre.y + halfSize.y >= other.centre.y - other.halfSize.y || centre.y - halfSize.y <= other.centre.y + other.halfSize.y) {
	  return true;
	}
      }
      return false;
    }
  };

  template <typename T>
  struct Data {
    Point pos;
    T* value;

    Data(Point pos, T* data) : pos(pos), value(data) {};
  };

  template <class T>
  class Quadtree {

  public:
    typedef std::vector< Data<T> > DVec;
    Quadtree<T>();
    Quadtree<T>(const AABB &boundary);

    ~Quadtree();

    //Set the appropriate boundary after creation
    void set_boundary(const AABB& boundary);

    //The following method inserts a point into the appropriate quad of a quadtree, splitting if necessary
    bool insert(const Data<T>& d);

    //Create four children that fully divide this quad into four quads of equal area
    void subdivide();

    //The following method finds all points contained within a range
    void queryRange(const AABB& range, std::vector<T*>& pInRange) const;

  private:
    //4 children
    Quadtree* nw;
    Quadtree* ne;
    Quadtree* sw;
    Quadtree* se;

    AABB boundary;

    DVec objects;

    static constexpr int capacity = 16;
  };

  template <class T>
  Quadtree<T>::Quadtree() {
    nw = nullptr;
    ne = nullptr;
    sw = nullptr;
    se = nullptr;
    {}
  }

  template <class T>
  Quadtree<T>::Quadtree(const AABB& boundary) {
    objects = DVec();
    nw = nullptr;
    ne = nullptr;
    sw = nullptr;
    se = nullptr;
    this->boundary = boundary;
  }

  template <class T>
  Quadtree<T>::~Quadtree() {
    delete nw;
    delete sw;
    delete ne;
    delete se;
  }

  template <class T>
  void Quadtree<T>::set_boundary(const AABB& boundary) {
    this->boundary = boundary;
  }

  template <class T>
  void Quadtree<T>::subdivide() {
    Point qSize = Point(boundary.halfSize.x / 2.0f, boundary.halfSize.y / 2.0f);
    Point qCentre = Point(boundary.centre.x - qSize.x, boundary.centre.y - qSize.y);
    nw = new Quadtree(AABB(qCentre, qSize));

    qCentre = Point(boundary.centre.x + qSize.x, boundary.centre.y - qSize.y);
    ne = new Quadtree(AABB(qCentre, qSize));

    qCentre = Point(boundary.centre.x - qSize.x, boundary.centre.y + qSize.y);
    sw = new Quadtree(AABB(qCentre, qSize));

    qCentre = Point(boundary.centre.x + qSize.x, boundary.centre.y + qSize.y);
    se = new Quadtree(AABB(qCentre, qSize));
  }

  template <class T>
  bool Quadtree<T>::insert(const Data<T>& d) {
    if (!boundary.contains(d.pos)) {
      return false;
    }

    if (objects.size() < capacity) {
      objects.push_back(d);
      return true;
    }

    if (nw == nullptr)	{
      subdivide();
    }

    if (nw->insert(d)) {
      return true;
    }
    if (ne->insert(d)) {
      return true;
    }
    if (sw->insert(d)) {
      return true;
    }
    if (se->insert(d)) {
      return true;
    }

    return false;
  }

  template <class T>
  void Quadtree<T>::queryRange(const AABB& range, std::vector<T*>& pInRange) const {
    if (!boundary.intersects(range)) {
      return;
    }

    for (auto&& object : objects) {
      if (range.contains(object.pos)) {
	pInRange.push_back(object.value);
      }
    }

    if (nw == nullptr) {
      return;
    }

    nw->queryRange(range, pInRange);
    ne->queryRange(range, pInRange);
    sw->queryRange(range, pInRange);
    se->queryRange(range, pInRange);

    return;
  }
}

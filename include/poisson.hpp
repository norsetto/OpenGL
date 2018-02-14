#pragma once

#include <vector>
#include <cstdlib>
#include <math.h>

//See http://devmag.org.za/2009/05/03/poisson-disk-sampling/
//Original code by Sergey Kosarevsky, http://www.linderdaum.com, http://blog.linderdaum.com

class Poisson {
public:

  template<typename T>
  struct Point {
    Point(): x(T(0)), y(T(0)) {}
    Point(T const& X, T const& Y): x(X), y(Y) {}
  
    T x;
    T y;
  };

private:
  struct Grid {
    Grid(int W, int H, float CellSize): w( W ), h( H ), cellSize(CellSize) {
      grid.resize(h);

      for (std::vector<std::vector<Point<float>>>::iterator row = grid.begin();
	   row != grid.end(); row++) { row->resize(w); }
    }
  
    void insert(const Point<float>& P) {
      Point<int> G = Point<int>((int)(P.x / cellSize), (int)(P.y / cellSize));
      grid[G.x][G.y] = P;
    }
  
    bool isInNeighbourhood(Point<float>& P, float minDist, float cellSize) {
      Point<int> G = Point<int>((int)(P.x / cellSize), (int)(P.y / cellSize));

      const int D = 2;

      for (int i = G.x - D; i < G.x + D; i++)
	{
	  for (int j = G.y - D; j < G.y + D; j++)
	    {
	      if ( i >= 0 && i < w && j >= 0 && j < h )
		{
		  Point<float> p = grid[i][j];

		  if ((P.x - p.x) * (P.x - p.x) + (P.y - p.y) * (P.y - p.y) < minDist * minDist) { return true; }
		}
	    }
	}
      return false;
    }

  private:
    int w;
    int h;
    float cellSize;

    std::vector<std::vector<Point<float>>> grid;
  };

  Point<float> popRandom(std::vector<Point<float>>& Points) {
    int Idx = rand() * (Points.size() - 1) / RAND_MAX;
    Point<float> P = Points[Idx];
    Points.erase(Points.begin() + Idx);
    
    return P;
  }

  Point<float> generateRandomPoint(const Point<float>& P, float minDist) {
    float R1 = (float)rand() / (float)RAND_MAX;
    float R2 = (float)rand() / (float)RAND_MAX;

    float Radius = minDist * ( R1 + 1.0f );
    float Angle = 2.0f * 3.141592653589f * R2;

    float X = P.x + Radius * cosf(Angle);
    float Y = P.y + Radius * sinf(Angle);

    return Point<float>(X, Y);
  }

public:
  void generate(size_t numPoints, int trialPoints = 30);
  std::vector<Point<float>> points;
};

void Poisson::generate(size_t numPoints, int trialPoints) {
  points.clear();
  std::vector<Poisson::Point<float>> ProcessList;

  // create the grid
  const float MinDist = sqrtf( float(numPoints) ) / float(numPoints);
  float CellSize = MinDist / sqrt( 2.0f );

  int GridW = ( int )ceil( 1.0f / CellSize );
  int GridH = ( int )ceil( 1.0f / CellSize );

  Poisson::Grid Grid(GridW, GridH, CellSize);

  Poisson::Point<float> FirstPoint((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);	     

  // update containers
  ProcessList.push_back(FirstPoint);
  points.push_back(FirstPoint);
  Grid.insert(FirstPoint);

  // generate new points for each point in the queue
  while (!ProcessList.empty() && points.size() < numPoints)
    {
      Poisson::Point<float> Point = popRandom(ProcessList);
	  
      for (int i = 0; i < trialPoints; i++)
	{
	  Poisson::Point<float> NewPoint = generateRandomPoint(Point, MinDist);
			
	  if (NewPoint.x >= 0.0f && NewPoint.y >= 0.0f && NewPoint.x <= 1.0f && NewPoint.y <= 1.0f)
	    if (!Grid.isInNeighbourhood(NewPoint, MinDist, CellSize))
	      {
		ProcessList.push_back(NewPoint);
		points.push_back(NewPoint);
		Grid.insert(NewPoint);
	      }
	}
    }
}

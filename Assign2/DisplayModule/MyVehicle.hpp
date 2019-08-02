#ifndef __MYVEHICLE_HPP__
#define __MYVEHICLE_HPP__


#include "Vehicle.hpp"

class MyVehicle : public Vehicle
{
private:
	int *NumPoints;
	double *X;
	double *Y;
public:
	MyVehicle();
	MyVehicle(int * NumPoints, double * X, double * Y);
	virtual void draw();
	void drawLaser();
};

#endif
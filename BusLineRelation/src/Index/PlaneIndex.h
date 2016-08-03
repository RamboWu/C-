#pragma once

#ifndef _PLANE_INDEX_
#define _PLANE_INDEX_
#include <map>
#include <set>
#include <list>
#include <stdint.h>
#include <algorithm>
using namespace std;

const int di[5][2] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } };
const int di_8[9][2] = { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 }, { -1, 0 }, { -1, 1 }, { 0, 1 } };

class GPSPoint {
public:
	double lon, lat;

	GPSPoint() {

	}

	GPSPoint(double lon, double lat) {
		this->lon = lon;
		this->lat = lat;
	}

	GPSPoint(GPSPoint &point) {
		this->lon = point.lon;
		this->lat = point.lat;
	}

	GPSPoint(const GPSPoint &point) {
		this->lon = point.lon;
		this->lat = point.lat;
	}

	~GPSPoint() {

	}

	double Dot(GPSPoint &p) {
		return this->lon*p.lon + this->lat*p.lat;
	}

	double Cross(GPSPoint &p) {
		return this->lon*p.lat - this->lat*p.lon;
	}

	double EuclideanDistance(GPSPoint &p) {
		return sqrt((this->lon - p.lon)*(this->lon - p.lon) + (this->lat - p.lat)*(this->lat - p.lat));
	}

	double EuclideanDistance2(GPSPoint &p) {
		return (this->lon - p.lon)*(this->lon - p.lon) + (this->lat - p.lat)*(this->lat - p.lat);
	}

	double GPSDistance(GPSPoint &p) {
		const double PI = 3.1415926;
		const double r = 6378137;
		double lat1 = this->lat / 180.0 *PI;
		double lon1 = this->lon / 180.0 *PI;
		double lat2 = p.lat / 180.0 *PI;
		double lon2 = p.lon / 180.0 *PI;
		double d1 = abs(lat1 - lat2);
		double d2 = abs(lon1 - lon2);
		double t = pow(sin(d1 / 2), 2) + cos(lat1)
			* cos(lat2) * pow(sin(d2 / 2), 2);
		double dis = r * 2 * asin(sqrt(t));
		return dis;
	}
};

class Point {
public:
	int64_t x, y;

	Point() {

	}

	Point(GPSPoint &point) {
		this->x = point.lon * 1000000;
		this->y = point.lat * 1000000;
	}

	Point(double lon, double lat) {
		this->x = lon * 1000000;
		this->y = lat * 1000000;
	}

	Point(int64_t x, int64_t y) {
		this->x = x;
		this->y = y;
	}

	Point(Point& p) {
		this->x = p.x;
		this->y = p.y;
	}

	Point(const Point& p) {
		this->x = p.x;
		this->y = p.y;
	}

	~Point() {

	}

	int64_t Dot(Point& p) {
		return this->x*p.x + this->y*p.y;
	}

	int64_t Cross(Point& p) {
		return this->x*p.y - this->y*p.x;
	}

	double EuclideanDistance(Point &p) {
		return sqrt((this->x - p.x)*(this->x - p.x) + (this->y - p.y)*(this->y - p.y));
	}

	double EuclideanDistance2(Point &p) {
		return (this->x - p.x)*(this->x - p.x) + (this->y - p.y)*(this->y - p.y);
	}
};

class PointCompare {
public:
	bool operator() (const Point &a, const Point &b) {
		if (a.x < b.x) return true;
		if (a.x > b.x) return false;
		if (a.y < b.y) return true;
		return false;
	}
};

class GPSPointCompare {
public:
	bool operator() (const GPSPoint &a, const GPSPoint &b) {
		if (a.lon < b.lon) return true;
		if (a.lon > b.lon) return false;
		if (a.lat < b.lat) return true;
		return false;
	}
};

class Segment {
public:
	GPSPoint start, end;

	Segment() {

	}

	Segment(GPSPoint &start, GPSPoint &end) {
		this->start = start;
		this->end = end;
	}

	Segment(Segment &seg) {
		this->start = GPSPoint(seg.start);
		this->end = GPSPoint(seg.end);
	}

	Segment(const Segment &seg) {
		this->start = GPSPoint(seg.start);
		this->end = GPSPoint(seg.end);
	}

	double PointShadow(GPSPoint& point) {
		GPSPoint point_a = GPSPoint(point.lon - this->start.lon, point.lat - this->start.lat);
		GPSPoint point_b = GPSPoint(this->end.lon - this->start.lon, this->end.lat - this->start.lat);
		return point_a.Dot(point_b) / this->start.EuclideanDistance(this->end);
	}

	double PointShadow2(GPSPoint& point) {
		GPSPoint point_a = GPSPoint(point.lon - this->start.lon, point.lat - this->start.lat);
		GPSPoint point_b = GPSPoint(this->end.lon - this->start.lon, this->end.lat - this->start.lat);
		return point_a.Dot(point_b)*point_a.Dot(point_b) / this->start.EuclideanDistance2(this->end);
	}

	inline double PointShadowWithMetre(GPSPoint& point,double gpsAP) {
		GPSPoint vectorAB = GPSPoint(end.lon - start.lon, end.lat - start.lat);
		GPSPoint vectorAP = GPSPoint(point.lon - start.lon, point.lat - start.lat);
		double cosAlpha = 0;
		double disPA = point.EuclideanDistance(start);
		double disAB = start.EuclideanDistance(end);
		if (disPA*disAB != 0) {
			cosAlpha = vectorAP.Dot(vectorAB) / (disPA*disAB);
		}
		return gpsAP*cosAlpha;
	}

	inline double NearestDistanceWithMetre(GPSPoint& point, double gpsAP) {
		double disPA = point.EuclideanDistance2(start);
		double disPB = point.EuclideanDistance2(end);
		if (disPA < 1e-10) return gpsAP;
		if (disPB < 1e-10) return point.GPSDistance(end);
		double disAB = start.EuclideanDistance2(end);
		if (disAB < 1e-10) return gpsAP;
		if (disPA >= disAB + disPB) return point.GPSDistance(end);
		if (disPB >= disAB + disPA) return gpsAP;

		GPSPoint vectorAB = GPSPoint(end.lon - start.lon, end.lat - start.lat);
		GPSPoint vectorAP = GPSPoint(point.lon - start.lon, point.lat - start.lat);
		if (abs(vectorAP.Cross(vectorAB)) < 1e-10) return 0;
		double sinAlpha = 0;
		if (disPA*disAB != 0) {
			sinAlpha = vectorAP.Cross(vectorAB) / (sqrt(disPA)*sqrt(disAB));
		}

		return abs(gpsAP*sinAlpha);
	}

	double NearestDistance(GPSPoint & point) {
		double dis_p_a = point.EuclideanDistance(this->start);
		double dis_p_b = point.EuclideanDistance(this->end);
		if (dis_p_a < 0.000001) return dis_p_a;
		if (dis_p_b < 0.000001) return dis_p_b;
		double dis_a_b = this->start.EuclideanDistance(this->end);
		if (dis_a_b < 0.000001) return dis_p_a;
		if (pow(dis_p_a, 2) >= pow(dis_p_b, 2) + pow(dis_a_b, 2)) return dis_p_b;
		if (pow(dis_p_b, 2) >= pow(dis_p_a, 2) + pow(dis_a_b, 2)) return dis_p_a;
		double l = (dis_a_b + dis_p_a + dis_p_b) / 2.0;
		if ((l * (l - dis_a_b) * (l - dis_p_a) * (l - dis_p_b)) <= 0) return 0;
		Point p_s = Point(point.lon - this->start.lon, point.lat - this->start.lat);
		Point e_s = Point(this->end.lon - this->start.lon, this->end.lat - this->start.lat);
		if (abs(p_s.Cross(e_s)) < 0.00001) return 0;
		double s = sqrt(l * (l - dis_a_b) * (l - dis_p_a) * (l - dis_p_b));
		return 2 * s / dis_a_b;
	}

	bool Between(GPSPoint &p) {
		GPSPoint p_a = GPSPoint(p.lon - this->start.lon, p.lat - this->start.lat);
		GPSPoint a_b = GPSPoint(this->start.lon - this->end.lon, this->start.lat - this->end.lat);
		GPSPoint p_b = GPSPoint(p.lon - this->end.lon, p.lat - this->end.lat);
		GPSPoint b_a = GPSPoint(this->end.lon - this->start.lon, this->end.lat - this->start.lat);

		return p_a.Dot(b_a) >= 0 && p_b.Dot(a_b) >= 0;
	}

	bool InRegion(Segment& seg) {
		if (Between(seg.start) || Between(seg.end)) return true;
		return false;
	}

	double Distance(Segment& seg) {
		if (InRegion(seg)) {
			return min(NearestDistance(seg.start), NearestDistance(seg.end));
		}
		else if (seg.InRegion(*this)) {
			return min(seg.NearestDistance(start), seg.NearestDistance(end));
		}
		else {
			return min(NearestDistance(seg.start), NearestDistance(seg.end));
		}
	}
};

class SegmentInt {
public:
	Point start, end;
	SegmentInt() {

	}

	SegmentInt(int64_t s_x, int64_t s_y, int64_t e_x, int64_t e_y) {
		this->start.x = s_x;
		this->start.y = s_y;
		this->end.x = e_x;
		this->end.y = e_y;
	}

	SegmentInt(GPSPoint &start, GPSPoint &end) {
		this->start = Point(start);
		this->end = Point(end);
	}

	SegmentInt(Point &start, Point &end) {
		this->start = start;
		this->end = end;
	}

	SegmentInt(SegmentInt &seg) {
		this->start = Point(seg.start);
		this->end = Point(seg.end);
	}

	SegmentInt(const SegmentInt &seg) {
		this->start = Point(seg.start);
		this->end = Point(seg.end);
	}

	double PointShadow(Point& point) {
		Point point_a = Point(point.x - this->start.x, point.y - this->start.y);
		Point point_b = Point(this->end.x - this->start.x, this->end.y - this->start.y);
		return point_a.Dot(point_b) / this->start.EuclideanDistance(this->end);
	}

	double PointShadow2(Point& point) {
		Point point_a = Point(point.x - this->start.x, point.y - this->start.y);
		Point point_b = Point(point.x - this->end.x, point.y - this->end.y);
		return point_a.Dot(point_b)*point_a.Dot(point_b) / this->start.EuclideanDistance2(this->end);
	}

	double NearestDistance(Point & point) {
		double dis_p_a = point.EuclideanDistance(this->start);
		double dis_p_b = point.EuclideanDistance(this->end);
		if (dis_p_a < 0.000001) return dis_p_a;
		if (dis_p_b < 0.000001) return dis_p_b;
		double dis_a_b = this->start.EuclideanDistance(this->end);
		if (dis_a_b < 0.000001) return dis_p_a;
		if (pow(dis_p_a, 2) >= pow(dis_p_b, 2) + pow(dis_a_b, 2)) return dis_p_b;
		if (pow(dis_p_b, 2) >= pow(dis_p_a, 2) + pow(dis_a_b, 2)) return dis_p_a;
		double l = (dis_a_b + dis_p_a + dis_p_b) / 2.0;
		if ((l * (l - dis_a_b) * (l - dis_p_a) * (l - dis_p_b)) <= 0) return 0;
		Point p_s = Point(point.x - this->start.x, point.y - this->start.y);
		Point e_s = Point(this->end.x - this->start.x, this->end.y - this->start.y);
		if (abs(p_s.Cross(e_s)) < 0.00001)	return 0;
		return abs(p_s.Cross(e_s)) / dis_a_b;
	}

	bool Between(Point &p) {
		Point p_a = Point(p.x - this->start.x, p.y - this->start.y);
		Point a_b = Point(this->start.x - this->end.x, this->start.y - this->end.y);
		Point p_b = Point(p.x - this->end.x, p.y - this->end.y);
		Point b_a = Point(this->end.x - this->start.x, this->end.y - this->start.y);

		return p_a.Dot(b_a) >= 0 && p_b.Dot(a_b) >= 0;
	}

	bool InRegion(SegmentInt& seg) {
		if (Between(seg.start) || Between(seg.end)) return true;
		return false;
	}

	double Distance(SegmentInt& seg) {
		if (InRegion(seg)) {
			return min(NearestDistance(seg.start), NearestDistance(seg.end));
		}
		else if (seg.InRegion(*this)) {
			return min(seg.NearestDistance(start), seg.NearestDistance(end));
		}
		else {
			return min(NearestDistance(seg.start), NearestDistance(seg.end));
		}
	}

	bool Cross(SegmentInt &seg2) {
		Point a = Point(seg2.start.x - this->start.x, seg2.start.y - this->start.y);
		Point b = Point(this->end.x - this->start.x, this->end.y - this->start.y);
		int64_t d1 = a.Cross(b);

		a = Point(seg2.end.x - this->start.x, seg2.end.y - this->start.y);
		int64_t d2 = a.Cross(b);

		a = Point(this->start.x - seg2.start.x, this->start.y - seg2.start.y);
		b = Point(seg2.end.x - seg2.start.x, seg2.end.y - seg2.start.y);
		int64_t d3 = a.Cross(b);

		a = Point(this->end.x - seg2.start.x, this->end.y - seg2.start.y);
		int64_t d4 = a.Cross(b);
		if (d1*d2 < 0 && d3*d4 < 0) {
			return true;
		}
		else if (d1 == 0) {
			if (this->Between(seg2.start)) {
				return true;
			}
			else {
				return false;
			}
		}
		else if (d2 == 0) {
			if (this->Between(seg2.end)) {
				return true;
			}
			else {
				return false;
			}
		}
		else if (d3 == 0) {
			if (seg2.Between(this->start)) {
				return true;
			}
			else {
				return false;
			}
		}
		else if (d4 == 0) {
			if (seg2.Between(this->end)) {
				return true;
			}
			else {
				return false;
			}
		}
		return false;
	}

};

class PlaneBorder {
public:
	int64_t left, right;
};

class PlaneBorderCompareOpt {
public:
	bool operator() (const PlaneBorder &a, const PlaneBorder &b) {
		if (a.right < b.left) return true;
		return false;
	}
};

class DeepCompareData {
public:
	int64_t x;
	int64_t y;
	int deep;
	DeepCompareData(int64_t x, int64_t y, int deep) {
		this->x = x;
		this->y = y;
		this->deep = deep;
	}
};

class DeepCompareOpt {
public:
	bool operator() (const DeepCompareData &a, const DeepCompareData &b) {
		if (a.x < b.x) return true;
		if (a.x > b.x) return false;
		if (a.y < b.y) return true;
		if (a.y > b.y) return false;
		if (a.deep < b.deep) return true;
		return false;
	}
};

template<typename T>
class PlaneColumn {
public:
	map<PlaneBorder, pair<GPSPoint*, T*>*, PlaneBorderCompareOpt> index;
	~PlaneColumn() {
		for (auto iter = index.begin(); iter != index.end(); iter++) {
			delete iter->second->first;
			delete iter->second->second;
			delete iter->second;
		}
		index.clear();
	}
};


template<typename T>
class PlaneIndex {
	int64_t unit_num = 0;
	const int64_t magnify = 1000000;
	int64_t edge_length = 500;
	int64_t max_deep = 2;
	int64_t min_lat, min_lon, max_lat, max_lon;
	map<PlaneBorder, PlaneColumn<T>*, PlaneBorderCompareOpt> index;

	int64_t MagnifyToInt(double);

	void DFSSegment(set<DeepCompareData, DeepCompareOpt>&, SegmentInt&, int64_t, int64_t, int64_t, int64_t, int, int, T*);
	void DFSPointDeep(set<DeepCompareData, DeepCompareOpt>&, int64_t, int64_t, int, T*);

	void DFSGetSegment(set<DeepCompareData, DeepCompareOpt>&, list<pair<GPSPoint*, T*>*> &, SegmentInt&, int64_t, int64_t, int64_t, int64_t, int, int, int);
	void DFSGetPointDeep(set<DeepCompareData, DeepCompareOpt>&, list<pair<GPSPoint*, T*>*> &, int64_t, int64_t, int, int);
	void DFSGetPointDirDeep(set<DeepCompareData, DeepCompareOpt>&, list<pair<GPSPoint*, T*>*> &, int64_t, int64_t, int, int, int);

	void SetPoint(int64_t, int64_t, T*);

	bool Contain(int64_t, int64_t);

	bool InRegion(int64_t, int64_t);

	void SetSegment(int64_t, int64_t, int64_t, int64_t, T*);

	pair<GPSPoint*, T*>* GetPoint(int64_t, int64_t);
	list<pair<GPSPoint*, T*>*>* GetPoint(int64_t, int64_t, int);

	pair<GPSPoint*, T*>* CreatePoint(int64_t, int64_t);

	list<pair<GPSPoint*, T*>*>* GetSegment(int64_t, int64_t, int64_t, int64_t, int);

	int64_t GetLonLeftBorder(int64_t);
	int64_t GetLatLeftBorder(int64_t);

public:
	PlaneIndex(double, double, double, double, int64_t = 500, int64_t = 2);
	~PlaneIndex();

	int GetIndexError();
	int64_t GetMagnify();

	double GetLonLeftBorder(double);
	double GetLatLeftBorder(double);

	int64_t GetEdgeLength();
	int64_t GetUnitNumber();

	bool InRegion(double, double);

	bool Contain(double, double);
	bool Contain(GPSPoint&);

	Point GPSToPoint(GPSPoint&);

	void SetPoint(double, double, T*);
	void SetPoint(GPSPoint&, T*);
	void SetPoint(Point &, T*);

	void SetSegment(double, double, double, double, T*);
	void SetSegment(GPSPoint&, GPSPoint&, T*);
	void SetSegment(Segment&, T*);

	pair<GPSPoint*, T*>* CreatePoint(GPSPoint&);
	pair<GPSPoint*, T*>* CreatePoint(double, double);
	pair<GPSPoint*, T*>* CreatePoint(Point &);

	pair<GPSPoint*, T*>* GetPoint(GPSPoint&);
	pair<GPSPoint*, T*>* GetPoint(double, double);
	pair<GPSPoint*, T*>* GetPoint(Point&);

	list<pair<GPSPoint*, T*>*>* GetPoint(GPSPoint &, int);
	list<pair<GPSPoint*, T*>*>* GetPoint(double, double, int);
	list<pair<GPSPoint*, T*>*>* GetPoint(Point &, int);

	list<pair<GPSPoint*, T*>*>* GetSegment(double, double, double, double);
	list<pair<GPSPoint*, T*>*>* GetSegment(GPSPoint&, GPSPoint&);
	list<pair<GPSPoint*, T*>*>* GetSegment(Segment &);

	list<pair<GPSPoint*, T*>*>* GetSegment(double, double, double, double, int);
	list<pair<GPSPoint*, T*>*>* GetSegment(GPSPoint&, GPSPoint&, int);
	list<pair<GPSPoint*, T*>*>* GetSegment(Segment &, int);
};


template<typename T>
PlaneIndex<T>::PlaneIndex(double min_lon, double min_lat, double max_lon, double max_lat, int64_t edge_length = 500, int64_t deep = 2) {
	this->max_lat = (max_lat+1)*magnify + edge_length + 1;
	this->max_lon = (max_lon+1)*magnify + edge_length + 1;
	this->min_lat = (min_lat-1)*magnify - edge_length;
	this->min_lon = (min_lon-1)*magnify - edge_length;
	this->edge_length = edge_length;
	this->max_deep = deep;
}

template<typename T>
PlaneIndex<T>::~PlaneIndex() {
	for (auto iter = index.begin(); iter != index.end(); iter++) {
		delete iter->second;
	}
	index.clear();
}

template<typename T>
int PlaneIndex<T>::GetIndexError() {
	return edge_length / 10 * max_deep;
}

template<typename T>
int64_t PlaneIndex<T>::GetMagnify() {
	return magnify;
}

template<typename T>
int64_t PlaneIndex<T>::GetEdgeLength() {
	return edge_length;
}

template<typename T>
int64_t PlaneIndex<T>::MagnifyToInt(double var) {
	return var*magnify;
}

template<typename T>
Point PlaneIndex<T>::GPSToPoint(GPSPoint &gps) {
	Point point;
	point.x = MagnifyToInt(gps.lon);
	point.y = MagnifyToInt(gps.lat);
	return point;
}

template<typename T>
bool PlaneIndex<T>::Contain(GPSPoint& point) {
	return Contain(MagnifyToInt(point.lon), MagnifyToInt(point.lat));
}

template<typename T>
bool PlaneIndex<T>::Contain(double lon, double lat) {
	return Contain(MagnifyToInt(lon), MagnifyToInt(lat));
}

template<typename T>
bool PlaneIndex<T>::Contain(int64_t x, int64_t y) {
	PlaneBorder left_x;
	left_x.left = GetLonLeftBorder(x);
	left_x.right = left_x.left + edge_length - 1;
	if (this->index.find(left_x) == this->index.end()) {
		return false;
	}
	else {
		PlaneColumn<T>* plane_col = this->index[left_x];
		PlaneBorder left_y;
		left_y.left = GetLatLeftBorder(y);
		left_y.right = left_y.left + edge_length - 1;
		if (plane_col->index.find(left_y) == plane_col->index.end()) {
			return false;
		}
		else {
			return true;
		}
	}
}

template<typename T>
bool PlaneIndex<T>::InRegion(int64_t x, int64_t y) {
	if (x >= min_lon && x < max_lon && y >= min_lat && y < max_lat) return true;
	return false;
}

template<typename T>
bool PlaneIndex<T>::InRegion(double lon, double lat) {
	return InRegion(MagnifyToInt(lon), MagnifyToInt(lat));
}

template<typename T>
int64_t PlaneIndex<T>::GetLonLeftBorder(int64_t lon) {
	return (lon - min_lon) / edge_length*edge_length + min_lon;
}

template<typename T>
int64_t PlaneIndex<T>::GetLatLeftBorder(int64_t lat) {
	return (lat - min_lat) / edge_length*edge_length + min_lat;
}

template<typename T>
double PlaneIndex<T>::GetLonLeftBorder(double lon) {
	return (double)GetLonLeftBorder(MagnifyToInt(lon)) / magnify;
}

template<typename T>
double PlaneIndex<T>::GetLatLeftBorder(double lat) {
	return (double)GetLatLeftBorder(MagnifyToInt(lat)) / magnify;
}

template<typename T>
int64_t PlaneIndex<T>::GetUnitNumber() {
	return this->unit_num;
}

template<typename T>
pair<GPSPoint*, T*>* PlaneIndex<T>::CreatePoint(double lon, double lat) {
	return CreatePoint(MagnifyToInt(lon), MagnifyToInt(lat));
}

template<typename T>
pair<GPSPoint*, T*>* PlaneIndex<T>::CreatePoint(Point& point) {
	return CreatePoint(point.x, point.y);
}

template<typename T>
pair<GPSPoint*, T*>* PlaneIndex<T>::CreatePoint(GPSPoint& point) {
	return CreatePoint(MagnifyToInt(point.lon), MagnifyToInt(point.lat));
}

template<typename T>
pair<GPSPoint*, T*>* PlaneIndex<T>::CreatePoint(int64_t x, int64_t y) {
	if (!InRegion(x, y)) {
		return NULL;
	}

	PlaneBorder left_x;
	left_x.left = GetLonLeftBorder(x);
	left_x.right = left_x.left + edge_length - 1;
	if (this->index.find(left_x) == this->index.end()) {
		this->index[left_x] = new PlaneColumn < T > ;
	}
	PlaneColumn<T>* plane_col = this->index[left_x];
	PlaneBorder left_y;
	left_y.left = GetLatLeftBorder(y);
	left_y.right = left_y.left + edge_length - 1;


	pair<GPSPoint*, T*>* p = new pair < GPSPoint*, T* > ;
	T* t = new T;
	GPSPoint* point = new GPSPoint(((double)left_x.left) / magnify, ((double)left_y.left) / magnify);
	p->first = point;
	p->second = t;

	if (plane_col->index.find(left_y) == plane_col->index.end()) {
		this->unit_num++;
		plane_col->index[left_y] = p;
	}
	else {
		delete plane_col->index[left_y]->first;
		delete plane_col->index[left_y]->second;
		delete plane_col->index[left_y];
		plane_col->index[left_y] = p;
	}
	return p;
}

template<typename T>
void PlaneIndex<T>::SetPoint(int64_t x, int64_t y, T* unit) {
	if (!InRegion(x, y)) {
		return;
	}

	PlaneBorder left_x;
	left_x.left = GetLonLeftBorder(x);
	left_x.right = left_x.left + edge_length - 1;
	if (this->index.find(left_x) == this->index.end()) {
		this->index[left_x] = new PlaneColumn < T > ;
	}
	PlaneColumn<T>* plane_col = this->index[left_x];
	PlaneBorder left_y;
	left_y.left = GetLatLeftBorder(y);
	left_y.right = left_y.left + edge_length - 1;
	if (plane_col->index.find(left_y) == plane_col->index.end()) {
		this->unit_num++;
		GPSPoint* p = new GPSPoint(((double)left_x.left) / magnify, ((double)left_y.left) / magnify);
		plane_col->index[left_y] = new pair < GPSPoint*, T* > ;
		plane_col->index[left_y]->first = p;
		plane_col->index[left_y]->second = unit;
	}
	else {
		delete plane_col->index[left_y]->first;
		delete plane_col->index[left_y]->second;
		delete plane_col->index[left_y];
		GPSPoint* p = new GPSPoint(((double)left_x.left) / magnify, ((double)left_y.left) / magnify);
		plane_col->index[left_y] = new pair < GPSPoint*, T* > ;
		plane_col->index[left_y]->first = p;
		plane_col->index[left_y]->second = unit;
	}
}

template<typename T>
void PlaneIndex<T>::SetPoint(double lon, double lat, T *unit) {
	SetPoint(MagnifyToInt(lon), MagnifyToInt(lat), unit);
}

template<typename T>
void PlaneIndex<T>::SetPoint(GPSPoint& point, T *unit) {
	SetPoint(MagnifyToInt(point.lon), MagnifyToInt(point.lat), unit);
}

template<typename T>
void PlaneIndex<T>::SetPoint(Point& point, T *unit) {
	SetPoint(point.x, point.y, unit);
}

template<typename T>
void PlaneIndex<T>::DFSPointDeep(set<DeepCompareData, DeepCompareOpt> &plane_unit_set, int64_t x, int64_t y, int deep, T* unit) {
	for (int i = deep; i <= this->max_deep; i++) {
		DeepCompareData deep_metadata = DeepCompareData(x, y, i);
		if (plane_unit_set.find(deep_metadata) != plane_unit_set.end()) {
			return;
		}
	}
	DeepCompareData deep_meta_data = DeepCompareData(x, y, deep);
	plane_unit_set.insert(deep_meta_data);

	SetPoint(x, y, unit);

	if (deep == 1) return;
	for (int i = 0; i < 8; i++) {
		int64_t u = x + di_8[i][0] * edge_length;
		int64_t v = y + di_8[i][1] * edge_length;
		DFSPointDeep(plane_unit_set, u, v, deep - 1, unit);
	}
}

template<typename T>
void PlaneIndex<T>::DFSSegment(set<DeepCompareData, DeepCompareOpt> &plane_unit_set, SegmentInt& seg2, int64_t s_x, int64_t s_y, int64_t e_x, int64_t e_y, int s_d, int d_l, T* unit) {
	DFSPointDeep(plane_unit_set, s_x, s_y, this->max_deep, unit);

	if (s_x == e_x && s_y == e_y) {
		return;
	}
	else {
		for (int i = 0; i < d_l; i++) {
			int64_t u_x = s_x + di[s_d + i][0] * edge_length;
			int64_t u_y = s_y + di[s_d + i][1] * edge_length;
			SegmentInt seg1;
			switch (s_d + i) {
				case 0:
					seg1.start.x = u_x;
					seg1.start.y = u_y;
					seg1.end.x = u_x + edge_length;
					seg1.end.y = u_y;
					break;
				case 1:
					seg1.start.x = u_x;
					seg1.start.y = u_y;
					seg1.end.x = u_x;
					seg1.end.y = u_y + edge_length;
					break;
				case 2:
					seg1.start.x = s_x;
					seg1.start.y = s_y;
					seg1.end.x = s_x + edge_length;
					seg1.end.y = s_y;
					break;
				case 3:
					seg1.start.x = s_x;
					seg1.start.y = s_y;
					seg1.end.x = s_x;
					seg1.end.y = s_y + edge_length;
					break;
				case 4:
					seg1.start.x = u_x;
					seg1.start.y = u_y;
					seg1.end.x = u_x + edge_length;
					seg1.end.y = u_y;
					break;
				default:
					break;
			}
			if (seg1.Cross(seg2)) {
				DFSSegment(plane_unit_set, seg2, u_x, u_y, e_x, e_y, s_d, d_l, unit);
				return;
			}
		}
	}
}

template<typename T>
void PlaneIndex<T>::SetSegment(int64_t s_x, int64_t s_y, int64_t e_x, int64_t e_y, T* unit) {
	SegmentInt temp_segment = SegmentInt(s_x, s_y, e_x, e_y);
	set<DeepCompareData, DeepCompareOpt> plane_unit_set;
	s_x = GetLonLeftBorder(s_x);
	s_y = GetLatLeftBorder(s_y);
	e_x = GetLonLeftBorder(e_x);
	e_y = GetLatLeftBorder(e_y);
	if (s_x == e_x && s_y == e_y) {
		DFSSegment(plane_unit_set, temp_segment, s_x, s_y, e_x, e_y, 0, 0, unit);
	}
	else if (s_x == e_x && s_y < e_y) {
		DFSSegment(plane_unit_set, temp_segment, s_x, s_y, e_x, e_y, 0, 1, unit);
	}
	else if (s_x < e_x && s_y < e_y) {
		DFSSegment(plane_unit_set, temp_segment, s_x, s_y, e_x, e_y, 0, 2, unit);
	}
	else if (s_x < e_x && s_y == e_y) {
		DFSSegment(plane_unit_set, temp_segment, s_x, s_y, e_x, e_y, 1, 1, unit);
	}
	else if (s_x< e_x && s_y>e_y) {
		DFSSegment(plane_unit_set, temp_segment, s_x, s_y, e_x, e_y, 1, 2, unit);
	}
	else if (s_x == e_x && s_y > e_y) {
		DFSSegment(plane_unit_set, temp_segment, s_x, s_y, e_x, e_y, 2, 1, unit);
	}
	else if (s_x > e_x && s_y > e_y) {
		DFSSegment(plane_unit_set, temp_segment, s_x, s_y, e_x, e_y, 2, 2, unit);
	}
	else if (s_x > e_x && s_y == e_y) {
		DFSSegment(plane_unit_set, temp_segment, s_x, s_y, e_x, e_y, 3, 1, unit);
	}
	else if (s_x > e_x && s_y < e_y) {
		DFSSegment(plane_unit_set, temp_segment, s_x, s_y, e_x, e_y, 3, 2, unit);
	}
	plane_unit_set.clear();
}

template<typename T>
void PlaneIndex<T>::SetSegment(double s_x, double s_y, double e_x, double e_y, T* unit) {
	SetSegment(MagnifyToInt(s_x), MagnifyToInt(s_y), MagnifyToInt(e_x), MagnifyToInt(e_y), unit);
}

template<typename T>
void PlaneIndex<T>::SetSegment(GPSPoint& start, GPSPoint& end, T* unit) {
	SetSegment(MagnifyToInt(start.lon), MagnifyToInt(start.lat), MagnifyToInt(end.lon), MagnifyToInt(end.lat), unit);
}

template<typename T>
void PlaneIndex<T>::SetSegment(Segment& segment, T* unit) {
	SetSegment(MagnifyToInt(segment.start.lon), MagnifyToInt(segment.start.lat), MagnifyToInt(segment.end.lon), MagnifyToInt(segment.end.lat), unit);
}

template<typename T>
pair<GPSPoint*, T*>* PlaneIndex<T>::GetPoint(Point & point) {
	return GetPoint(point.x, point.y);
}

template<typename T>
pair<GPSPoint*, T*>* PlaneIndex<T>::GetPoint(double lon, double lat) {
	return GetPoint(MagnifyToInt(lon), MagnifyToInt(lat));
}

template<typename T>
pair<GPSPoint*, T*>* PlaneIndex<T>::GetPoint(GPSPoint& point) {
	return GetPoint(MagnifyToInt(point.lon), MagnifyToInt(point.lat));
}

template<typename T>
pair<GPSPoint*, T*>* PlaneIndex<T>::GetPoint(int64_t x, int64_t y) {
	if (!InRegion(x, y)) {
		return NULL;
	}
	PlaneBorder left_x;
	left_x.left = GetLonLeftBorder(x);
	left_x.right = left_x.left + edge_length - 1;
	if (this->index.find(left_x) == this->index.end()) {
		return NULL;
	}
	PlaneColumn<T>* plane_col = this->index[left_x];
	PlaneBorder left_y;
	left_y.left = GetLatLeftBorder(y);
	left_y.right = left_y.left + edge_length - 1;
	if (plane_col->index.find(left_y) == plane_col->index.end()) {
		return NULL;
	}

	return plane_col->index[left_y];

}

template<typename T>
void PlaneIndex<T>::DFSGetPointDirDeep(set<DeepCompareData, DeepCompareOpt> &plane_unit_set, list<pair<GPSPoint*, T*>*> &result_list, int64_t x, int64_t y, int deep, int max_deep, int dir) {

	for (int i = deep; i <= max_deep; i++) {
		DeepCompareData deep_metadata = DeepCompareData(x, y, i);
		if (plane_unit_set.find(deep_metadata) != plane_unit_set.end()) {
			return;
		}
	}
	DeepCompareData deep_meta_data = DeepCompareData(x, y, deep);
	plane_unit_set.insert(deep_meta_data);
	DeepCompareData temp_deep_data = DeepCompareData(x, y, -1);

	if (plane_unit_set.find(temp_deep_data) == plane_unit_set.end()) {
		pair<GPSPoint*, T*>* temp = GetPoint(x, y);
		if (temp == NULL) {
			temp = CreatePoint(x, y);
		}

		result_list.push_back(temp);
		plane_unit_set.insert(temp_deep_data);
	}

	if (deep == 1) return;
	for (int i = 0; i < 3; i++) {
		int64_t u = x + di_8[dir + i][0] * edge_length;
		int64_t v = y + di_8[dir + i][1] * edge_length;
		DFSGetPointDirDeep(plane_unit_set, result_list, u, v, deep - 1, max_deep, dir);
	}
}

template<typename T>
list<pair<GPSPoint*, T*>*>* PlaneIndex<T>::GetPoint(int64_t x, int64_t y, int deep) {
	if (!InRegion(x, y)) {
		return NULL;
	}
	list<pair<GPSPoint*, T*>*>* result_list = new list<pair<GPSPoint*, T*>*>();
	set<DeepCompareData, DeepCompareOpt> plane_unit_set;

	DeepCompareData deep_meta_data = DeepCompareData(x, y, deep);
	plane_unit_set.insert(deep_meta_data);
	DeepCompareData temp_deep_data = DeepCompareData(x, y, -1);
	pair<GPSPoint*, T*>* temp = GetPoint(x, y);
	if (temp == NULL) {
		temp = CreatePoint(x, y);
	}

	result_list->push_back(temp);
	plane_unit_set.insert(temp_deep_data);

	if (deep > 1) {
		for (int i = 0; i < 8; i++) {
			int64_t u = x + di_8[i][0] * edge_length;
			int64_t v = y + di_8[i][1] * edge_length;
			DFSGetPointDirDeep(plane_unit_set, *result_list, u, v, deep - 1, deep, i);
		}
	}

	plane_unit_set.clear();

	return result_list;
}

template<typename T>
list<pair<GPSPoint*, T*>*>* PlaneIndex<T>::GetPoint(double lon, double lat, int deep) {
	return GetPoint(MagnifyToInt(lon), MagnifyToInt(lat), deep);
}

template<typename T>
list<pair<GPSPoint*, T*>*>* PlaneIndex<T>::GetPoint(Point & point, int deep) {
	return GetPoint(point.x, point.y, deep);
}

template<typename T>
list<pair<GPSPoint*, T*>*>* PlaneIndex<T>::GetPoint(GPSPoint& point, int deep) {
	return GetPoint(MagnifyToInt(point.lon), MagnifyToInt(point.lat), deep);
}


template<typename T>
void PlaneIndex<T>::DFSGetPointDeep(set<DeepCompareData, DeepCompareOpt> &plane_unit_set, list<pair<GPSPoint*, T*>*> &result_list, int64_t x, int64_t y, int deep, int max_deep) {

	for (int i = deep; i <= max_deep; i++) {
		DeepCompareData deep_metadata = DeepCompareData(x, y, i);
		if (plane_unit_set.find(deep_metadata) != plane_unit_set.end()) {
			return;
		}
	}
	DeepCompareData deep_meta_data = DeepCompareData(x, y, deep);
	plane_unit_set.insert(deep_meta_data);
	DeepCompareData temp_deep_data = DeepCompareData(x, y, -1);

	if (plane_unit_set.find(temp_deep_data) == plane_unit_set.end()) {
		pair<GPSPoint*, T*>* temp = GetPoint(x, y);
		if (temp == NULL) {
			temp = CreatePoint(x, y);
		}

		if (temp != NULL) {
			result_list.push_back(temp);
			plane_unit_set.insert(temp_deep_data);
		}
	}

	if (deep == 1) return;
	for (int i = 0; i < 8; i++) {
		int64_t u = x + di_8[i][0] * edge_length;
		int64_t v = y + di_8[i][1] * edge_length;
		DFSGetPointDeep(plane_unit_set, result_list, u, v, deep - 1, max_deep);
	}
}

template<typename T>
void PlaneIndex<T>::DFSGetSegment(set<DeepCompareData, DeepCompareOpt> &plane_unit_set, list<pair<GPSPoint*, T*>*>& result_list, SegmentInt& seg2, int64_t s_x, int64_t s_y, int64_t e_x, int64_t e_y, int s_d, int d_l, int deep) {
	DFSGetPointDeep(plane_unit_set, result_list, s_x, s_y, deep, deep);

	if (s_x == e_x && s_y == e_y) {
		return;
	}
	else {
		for (int i = 0; i < d_l; i++) {
			int64_t u_x = s_x + di[s_d + i][0] * edge_length;
			int64_t u_y = s_y + di[s_d + i][1] * edge_length;
			SegmentInt seg1;
			switch (s_d + i) {
				case 0:
					seg1.start.x = u_x;
					seg1.start.y = u_y;
					seg1.end.x = u_x + edge_length;
					seg1.end.y = u_y;
					break;
				case 1:
					seg1.start.x = u_x;
					seg1.start.y = u_y;
					seg1.end.x = u_x;
					seg1.end.y = u_y + edge_length;
					break;
				case 2:
					seg1.start.x = s_x;
					seg1.start.y = s_y;
					seg1.end.x = s_x + edge_length;
					seg1.end.y = s_y;
					break;
				case 3:
					seg1.start.x = s_x;
					seg1.start.y = s_y;
					seg1.end.x = s_x;
					seg1.end.y = s_y + edge_length;
					break;
				case 4:
					seg1.start.x = u_x;
					seg1.start.y = u_y;
					seg1.end.x = u_x + edge_length;
					seg1.end.y = u_y;
					break;
				default:
					break;
			}
			if (seg1.Cross(seg2)) {
				DFSGetSegment(plane_unit_set, result_list, seg2, u_x, u_y, e_x, e_y, s_d, d_l, deep);
				return;
			}
		}
	}
}

template<typename T>
list<pair<GPSPoint*, T*>*>* PlaneIndex<T>::GetSegment(int64_t s_x, int64_t s_y, int64_t e_x, int64_t e_y, int deep) {
	list<pair<GPSPoint*, T*>*>* result_list = new list<pair<GPSPoint*, T*>*>();
	SegmentInt temp_segment = SegmentInt(s_x, s_y, e_x, e_y);
	set<DeepCompareData, DeepCompareOpt> plane_unit_set;
	s_x = GetLonLeftBorder(s_x);
	s_y = GetLatLeftBorder(s_y);
	e_x = GetLonLeftBorder(e_x);
	e_y = GetLatLeftBorder(e_y);
	if (s_x == e_x && s_y == e_y) {
		DFSGetSegment(plane_unit_set, *result_list, temp_segment, s_x, s_y, e_x, e_y, 0, 0, deep);
	}
	else if (s_x == e_x && s_y < e_y) {
		DFSGetSegment(plane_unit_set, *result_list, temp_segment, s_x, s_y, e_x, e_y, 0, 1, deep);
	}
	else if (s_x < e_x && s_y < e_y) {
		DFSGetSegment(plane_unit_set, *result_list, temp_segment, s_x, s_y, e_x, e_y, 0, 2, deep);
	}
	else if (s_x < e_x && s_y == e_y) {
		DFSGetSegment(plane_unit_set, *result_list, temp_segment, s_x, s_y, e_x, e_y, 1, 1, deep);
	}
	else if (s_x< e_x && s_y>e_y) {
		DFSGetSegment(plane_unit_set, *result_list, temp_segment, s_x, s_y, e_x, e_y, 1, 2, deep);
	}
	else if (s_x == e_x && s_y > e_y) {
		DFSGetSegment(plane_unit_set, *result_list, temp_segment, s_x, s_y, e_x, e_y, 2, 1, deep);
	}
	else if (s_x > e_x && s_y > e_y) {
		DFSGetSegment(plane_unit_set, *result_list, temp_segment, s_x, s_y, e_x, e_y, 2, 2, deep);
	}
	else if (s_x > e_x && s_y == e_y) {
		DFSGetSegment(plane_unit_set, *result_list, temp_segment, s_x, s_y, e_x, e_y, 3, 1, deep);
	}
	else if (s_x > e_x && s_y < e_y) {
		DFSGetSegment(plane_unit_set, *result_list, temp_segment, s_x, s_y, e_x, e_y, 3, 2, deep);
	}
	plane_unit_set.clear();
	return result_list;
}

template<typename T>
list<pair<GPSPoint*, T*>*>* PlaneIndex<T>::GetSegment(double s_x, double s_y, double e_x, double e_y) {
	return GetSegment(MagnifyToInt(s_x), MagnifyToInt(s_y), MagnifyToInt(e_x), MagnifyToInt(e_y), this->max_deep);
}

template<typename T>
list<pair<GPSPoint*, T*>*>* PlaneIndex<T>::GetSegment(GPSPoint& start, GPSPoint& end) {
	return GetSegment(MagnifyToInt(start.lon), MagnifyToInt(start.lat), MagnifyToInt(end.lon), MagnifyToInt(end.lat), this->max_deep);
}

template<typename T>
list<pair<GPSPoint*, T*>*>* PlaneIndex<T>::GetSegment(Segment& segment) {
	return GetSegment(MagnifyToInt(segment.start.lon), MagnifyToInt(segment.start.lat), MagnifyToInt(segment.end.lon), MagnifyToInt(segment.end.lat), this->max_deep);
}

template<typename T>
list<pair<GPSPoint*, T*>*>* PlaneIndex<T>::GetSegment(double s_x, double s_y, double e_x, double e_y, int deep) {
	return GetSegment(MagnifyToInt(s_x), MagnifyToInt(s_y), MagnifyToInt(e_x), MagnifyToInt(e_y), deep);
}

template<typename T>
list<pair<GPSPoint*, T*>*>* PlaneIndex<T>::GetSegment(GPSPoint& start, GPSPoint& end, int deep) {
	return GetSegment(MagnifyToInt(start.lon), MagnifyToInt(start.lat), MagnifyToInt(end.lon), MagnifyToInt(end.lat), deep);
}

template<typename T>
list<pair<GPSPoint*, T*>*>* PlaneIndex<T>::GetSegment(Segment& segment, int deep) {
	return GetSegment(MagnifyToInt(segment.start.lon), MagnifyToInt(segment.start.lat), MagnifyToInt(segment.end.lon), MagnifyToInt(segment.end.lat), deep);
}

#endif
#include "core/gesture/GeometricRecognizerTypes.h"
#include <fstream>
using namespace std;
NS_JOFW_BEGIN

Point2D::Point2D()
{
	this->x = 0;
	this->y = 0;
}

Point2D::Point2D(double x, double y)
{
	this->x = x;
	this->y = y;
}


Rectangle::Rectangle(double x, double y, double width, double height)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
}


RecognitionResult::RecognitionResult(std::string name, double score)
{
	this->name = name;
	this->score = score;
}


GestureTemplate::GestureTemplate(std::string name, Path2D points)
{
	this->name = name;
	this->points = points;
}


bool PathWriter::writeToFile(Path2D path, const std::string fileName /*= "savedPath.txt"*/, const std::string gestureName /*= "DefaultName"*/)
{
	fstream file(fileName.c_str(), ios::out);

	file << "Path2D getGesture" << gestureName << "()" << endl;
	file << "{" << endl;
	file << "\t" << "Path2D path;" << endl;

	Path2D::const_iterator i;
	for (i = path.begin(); i != path.end(); i++)
	{
		Point2D point = *i;
		file << "\t" << "path.push_back(Point2D(" << point.x << ","
			<< point.y << "));" << endl;
	}

	file << endl;
	file << "\t" << "return path;" << endl;
	file << "}" << endl;

	file.close();

	return true;
}

NS_JOFW_END
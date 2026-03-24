#include "splitter.h"

// Useful aliases
class model;
using model_ptr = std::shared_ptr<model>;
template <typename... Args>
model_ptr create_model(Args... args)
{
	return std::make_shared<model>(args...);
}
using two_models_ptr = std::pair<std::shared_ptr<model>, std::shared_ptr<model>>;

// Check if number values are equal
template <typename T>
bool is_equal(T a, T b, T epsilon = 1e-9)
{
	return std::abs(a - b) < epsilon;
}

// Main class of model read/write/splitting functionality
class model
{
private:
	// Inner implementation of basic objects and methods
	struct innerStructure
	{
	public:
		// Used for usage in structures
		using coordinate = double;
		// Used for lenght calculations
		using lenght = double;
		// Used for mathemathic function parameters
		using mathValueType = double;
		// Used for representing 3d point in space
		struct Point3d
		{
			coordinate x;
			coordinate y;
			coordinate z;
			Point3d() : x(0.0), y(0.0), z(0.0) {}
			Point3d(coordinate x, coordinate y, coordinate z) : x(x), y(y), z(z) {}
			Point3d operator*(const mathValueType val) const
			{
				Point3d point{ x * val, y * val, z * val };
				return point;
			}
			Point3d operator/(const mathValueType val) const
			{
				Point3d point{ x / val, y / val, z / val };
				return point;
			}
			Point3d operator+(const Point3d& other) const
			{
				Point3d point{x + other.x, y + other.y, z + other.z};
				return point;
			}
			Point3d operator-(const Point3d& other) const
			{
				Point3d point{ x - other.x, y - other.y, z - other.z };
				return point;
			}
			// Vector multiplication
			Point3d cross(Point3d b)
			{
				Point3d c;
				c.x = this->y * b.z - this->z * b.y;
				c.y = this->z * b.x - this->x * b.z;
				c.z = this->x * b.y - this->y * b.x;
				return c;
			}
			lenght size()
			{
				return std::sqrt(x * x + y * y + z * z);
			}
			Point3d norm()
			{
				return (*this / size());
			}
		};
		// Used for represent 3d plane
		struct Plane3d
		{
			mathValueType a;
			mathValueType b;
			mathValueType c;
			mathValueType d;
			Plane3d() : a(0), b(0), c(0), d(0) {}
			Plane3d(Point3d P1, Point3d P2, Point3d P3) 
			{
				Point3d V1 = P2 - P1;
				Point3d V2 = P3 - P1;

				Point3d N = V1.cross(V2);

				a = N.x;
				b = N.y;
				c = N.z;
				d = -(a * P1.x + b * P1.y + c * P1.z);
			}
		};
		// Used for vertex enumeration
		using vertexId = size_t;
		// Used for normals enumeration
		using normalId = size_t;
		static constexpr const size_t TriangleVertexCount = 3;
		// Used for stroring and saving face
		// it stores id's of vertexes and normals (which starts with 1)
		struct Triangle
		{
			vertexId vert[TriangleVertexCount];
			normalId normal[TriangleVertexCount];
		};
		// Vertex and normals storage
		// vertex id is equal to (index + 1) cause they start from 1 not 0
		std::vector<Point3d> vertexes;
		std::vector<Point3d> normals;

		std::vector<Triangle> triangles;

	public:
		// Roughly calculates is points the same
		// useful for vertex packing
		static bool is_points_same(Point3d a, Point3d b)
		{
			return is_equal(a.x, b.x) && is_equal(a.y, b.y) && is_equal(a.z, b.z);		
		}
		
		// TODO: replace double with aliased value
		// Value of the plane at point
		static mathValueType planeValue(Plane3d plane, Point3d p) {
			return plane.a * p.x + plane.b * p.y + plane.c * p.z + plane.d;
		}
		// Intersection of line with plane
		static Point3d intersect(Point3d p1, Point3d p2, Plane3d plane) {
			mathValueType v1 = planeValue(plane, p1);
			mathValueType v2 = planeValue(plane, p2);

			mathValueType t = v1 / (v1 - v2);

			Point3d res;
			res.x = p1.x + t * (p2.x - p1.x);
			res.y = p1.y + t * (p2.y - p1.y);
			res.z = p1.z + t * (p2.z - p1.z);

			return res;
		}
		// Next 3 functions return id of stored object
		// Adds vertex to DB but only it's value is unique
		// TODO: rewrite more optimal
		vertexId addVertexIfNotExist(Point3d value) // minifies count of records by reusing old indices
		{
			for (vertexId i = 0; i < vertexes.size(); i++)
			{
				Point3d curr = vertexes[i];
				if (is_points_same(value, curr))
				{
					return i + 1; // vertexes starts from 1
				}
			}
			vertexes.push_back(value);
			return vertexes.size() + 1 - 1; // vertexes starts from 1
		}
		// Add vertex fast no matter what
		vertexId addVertexForce(Point3d value) // used at parsing
		{
			vertexes.push_back(value);
			return vertexes.size() + 1 - 1; // vertexes starts from 1
		}
		// Add normal fast no matter what
		normalId addNormalForce(Point3d value)
		{
			normals.push_back(value);
			return vertexes.size() + 1 - 1;// normals starts from 1
		}
		// Adds face with id of vertex/normals
		void addTriangle(Triangle value)
		{
			triangles.push_back(value);
		}
		// Self explanatory
		Point3d returnVertexById(vertexId id)
		{
			if (id > 0 && ((id - 1) < vertexes.size()))
			{
				return vertexes[id - 1]; // vertexes starts from 1
			}
			else
			{
				throw std::runtime_error("Incorrect vertex id");
			}
		}
		Point3d returnNormalById(vertexId id)
		{
			if (id > 0 && ((id - 1) < normals.size()))
			{
				return normals[id - 1]; // vertexes starts from 1
			}
			else
			{
				throw std::runtime_error("Incorrect vertex id");
			}
		}
	};
	// Inner model guts, works as model database
	innerStructure structure;
public:
	// Split current model by plane
	// returns 2 smart pointers with new models
	two_models_ptr split(double input_x1, double input_y1, double input_z1,
		double input_x2, double input_y2, double input_z2,
		double input_x3, double input_y3, double input_z3)
	{
		model_ptr model_cut_1 = create_model();
		model_ptr model_cut_2 = create_model();
		two_models_ptr result;
		result.first = model_cut_1;
		result.second = model_cut_2;

		for (size_t i = 0; i < structure.triangles.size(); i++)
		{
			//std::cout << "Triangle " << i << " of " << structure.triangles.size() << std::endl;
			const innerStructure::Triangle currTriangle = structure.triangles[i];

			innerStructure::Point3d p1;
			innerStructure::Point3d p2;
			innerStructure::Point3d p3;
			innerStructure::Point3d n1;
			innerStructure::Point3d n2;
			innerStructure::Point3d n3;

			try
			{
				p1 = structure.returnVertexById(currTriangle.vert[0]);
				p2 = structure.returnVertexById(currTriangle.vert[1]);
				p3 = structure.returnVertexById(currTriangle.vert[2]);
				n1 = structure.returnNormalById(currTriangle.normal[0]);
				n2 = structure.returnNormalById(currTriangle.normal[1]);
				n3 = structure.returnNormalById(currTriangle.normal[2]);
			}
			catch (const std::runtime_error& err)
			{
				std::cerr << "Internal error: " << err.what();
				continue;
			}

			// Cutting plane coords
			innerStructure::Point3d P1{ input_x1, input_y1, input_z1}, P2{ input_x2, input_y2, input_z2 }, P3{ input_x3, input_y3, input_z3 };
			innerStructure::Plane3d Plane(P1, P2, P3);			

			innerStructure::lenght S1 = innerStructure::planeValue(Plane, p1);
			innerStructure::lenght S2 = innerStructure::planeValue(Plane, p2);
			innerStructure::lenght S3 = innerStructure::planeValue(Plane, p3);

			// Calculated normal of triangle - to restore order of vertexes later
			innerStructure::Point3d AB(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
			innerStructure::Point3d AC(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);
			innerStructure::Point3d calculatedN = AB.cross(AC);
			calculatedN = calculatedN.norm();

			// Add triangle to database of selected model
			// restore original normal of triangle
			auto addTriangleWithRestore = [](
				model_ptr currModel,
				innerStructure::Point3d p1, innerStructure::Point3d p2, innerStructure::Point3d p3,
				innerStructure::Point3d n1, innerStructure::Point3d n2, innerStructure::Point3d n3,
				innerStructure::Point3d calculatedN ) 
				{
					// Check order
					innerStructure::Point3d AB(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
					innerStructure::Point3d AC(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);
					innerStructure::Point3d norm = AB.cross(AC);
					norm = norm.norm();
					
					//Restore correct order of vertexes
					if (!innerStructure::is_points_same(norm, calculatedN))
					{
						std::swap(p1, p3);
					}
															

					innerStructure::vertexId p1_new = currModel->structure.addVertexForce(p1);
					innerStructure::vertexId p2_new = currModel->structure.addVertexForce(p2);
					innerStructure::vertexId p3_new = currModel->structure.addVertexForce(p3);

					innerStructure::normalId n1_new = currModel->structure.addNormalForce(n1);
					innerStructure::normalId n2_new = currModel->structure.addNormalForce(n2);
					innerStructure::normalId n3_new = currModel->structure.addNormalForce(n3);

					currModel->structure.addTriangle(innerStructure::Triangle{ p1_new, p2_new, p3_new, n1_new, n2_new, n3_new });
				};

			// Add triangle to database
			auto addTrangle = [](
				model_ptr currModel,
				innerStructure::Point3d p1, innerStructure::Point3d p2, innerStructure::Point3d p3,
				innerStructure::Point3d n1, innerStructure::Point3d n2, innerStructure::Point3d n3)
				{
					innerStructure::vertexId p1_new = currModel->structure.addVertexForce(p1);
					innerStructure::vertexId p2_new = currModel->structure.addVertexForce(p2);
					innerStructure::vertexId p3_new = currModel->structure.addVertexForce(p3);

					innerStructure::normalId n1_new = currModel->structure.addNormalForce(n1);
					innerStructure::normalId n2_new = currModel->structure.addNormalForce(n2);
					innerStructure::normalId n3_new = currModel->structure.addNormalForce(n3);

					currModel->structure.addTriangle(innerStructure::Triangle{ p1_new, p2_new, p3_new, n1_new, n2_new, n3_new });
				};

			// Check if all 3 point exist entirely from one or other side
			if ((std::signbit(S1) == std::signbit(S2)) && (std::signbit(S2) == std::signbit(S3)) 
				&& (!is_equal(S1, 0.0)) && (!is_equal(S2, 0.0)) && (!is_equal(S3, 0.0)))
				
			{
				model_ptr currModel;
				if (S1 > 0)
				{
					currModel = model_cut_1;
					
				}
				else
				{
					currModel = model_cut_2;
				}
				addTrangle(currModel, p1, p2, p3, n1, n2, n3);
			}
			else
			{
				auto calcIntersect = [](const innerStructure::Point3d T1,
					const innerStructure::Point3d T2,
					const innerStructure::Point3d T3,
					const innerStructure::Plane3d plane)
					-> std::vector<innerStructure::Point3d>
					{
						std::vector<innerStructure::Point3d> result;
						std::vector<innerStructure::Point3d> pts = { T1, T2, T3 };

						auto add_unique = [&](const innerStructure::Point3d& p) {
							for (const auto& r : result) {
								if (innerStructure::is_points_same(r, p)) return;
							}
							result.push_back(p);
							};

						for (size_t i = 0; i < 3; i++) {
							auto p1 = pts[i];
							auto p2 = pts[(i + 1) % 3];

							auto v1 = innerStructure::planeValue(plane, p1);
							auto v2 = innerStructure::planeValue(plane, p2);

							if (is_equal(v1, 0.0)) add_unique(p1);
							if (is_equal(v2, 0.0)) add_unique(p2);

							if (v1 * v2 < 0) {
								add_unique(innerStructure::intersect(p1, p2, plane));
							}
						}

						return result;
					};
				std::vector<innerStructure::Point3d> cutPoints = calcIntersect(p1, p2, p3, Plane);

				const double eps = 1e-6;

				auto classify = [eps](double s) {
					if (s > eps) return 1;    // сверху
					if (s < -eps) return -1;  // снизу
					return 0;                 // на плоскости
					};

				//TODO: refactor
				if (cutPoints.size() == 3)
				{
					addTriangleWithRestore(model_cut_1, p1, p2, p3, n1, n2, n3, calculatedN);
					addTriangleWithRestore(model_cut_2, p1, p2, p3, n1, n2, n3, calculatedN);
				}
				else if (cutPoints.size() == 1) // If no intersection
				{		
					//addTriangleWithRestore(model_cut_1, p1, p2, p3, n1, n2, n3);
					//addTriangleWithRestore(model_cut_2, p1, p2, p3, n1, n2, n3);
					int C1 = classify(S1);
					int C2 = classify(S2);
					int C3 = classify(S3);
					if (C1 == 1 && C2 == 1)
					{
						addTriangleWithRestore(model_cut_1, p1, p2, p3, n1, n2, n3, calculatedN);
					}
					if (C1 == 1 && C3 == 1)
					{
						addTriangleWithRestore(model_cut_1, p1, p2, p3, n1, n2, n3, calculatedN);
					}
					if (C2 == 1 && C3 == 1)
					{
						addTriangleWithRestore(model_cut_1, p1, p2, p3, n1, n2, n3, calculatedN);
					}
					if (C1 == -1 && C2 == -1)
					{
						addTriangleWithRestore(model_cut_2, p1, p2, p3, n1, n2, n3, calculatedN);
					}
					if (C1 == -1 && C3 == -1)
					{
						addTriangleWithRestore(model_cut_2, p1, p2, p3, n1, n2, n3, calculatedN);
					}
					if (C2 == -1 && C3 == -1)
					{
						addTriangleWithRestore(model_cut_2, p1, p2, p3, n1, n2, n3, calculatedN);
					}
					continue;
				}

				innerStructure::Point3d I1 = cutPoints[0];
				innerStructure::Point3d I2 = cutPoints[1];

				std::vector<innerStructure::Point3d> pts = { p1, p2, p3 };

				// Split vertexes by the plane
				std::vector<innerStructure::Point3d> pos, neg;

				for (auto& p : pts) {
					if (innerStructure::planeValue(Plane, p) >= 0)
						pos.push_back(p);
					else
						neg.push_back(p);
				}

				innerStructure::Point3d I_AB, I_BC, I_CA;
				bool hasAB = false, hasBC = false, hasCA = false;

				// Fix for wrong vertex problem
				// when triangle was created by wrong coordinates
				auto checkEdge = [&](innerStructure::Point3d p1, innerStructure::Point3d p2, innerStructure::Point3d& out, bool& flag) {
					innerStructure::mathValueType v1 = innerStructure::planeValue(Plane, p1);
					innerStructure::mathValueType v2 = innerStructure::planeValue(Plane, p2);

					if (v1 * v2 < 0) {
						out = innerStructure::intersect(p1, p2, Plane);
						flag = true;
					}
					};

				// case when there is 1 vertex from one side and 2 from other
				if (pos.size() == 1 && neg.size() == 2) {
					innerStructure::Point3d A = pos[0];
					innerStructure::Point3d B = neg[0];
					innerStructure::Point3d C = neg[1];

					checkEdge(A, B, I_AB, hasAB);
					checkEdge(B, C, I_BC, hasBC);
					checkEdge(C, A, I_CA, hasCA);

					if (hasAB && hasCA) 
					{
						I1 = I_AB;
						I2 = I_CA;
					}
					else if (hasAB && hasBC) 
					{
						I1 = I_AB;
						I2 = I_BC;
					}
					else 
					{
						I1 = I_BC;
						I2 = I_CA;
					}

					// Small triangle
					addTriangleWithRestore(model_cut_1, A, I1, I2, n1, n2, n3, calculatedN);

					addTriangleWithRestore(model_cut_2, B, C, I1, n1, n2, n3, calculatedN);
					addTriangleWithRestore(model_cut_2, C, I1, I2, n1, n2, n3, calculatedN);
				}
				else if (pos.size() == 2 && neg.size() == 1) {
					
					innerStructure::Point3d A = neg[0];
					innerStructure::Point3d B = pos[0];
					innerStructure::Point3d C = pos[1];

					checkEdge(A, B, I_AB, hasAB);
					checkEdge(B, C, I_BC, hasBC);
					checkEdge(C, A, I_CA, hasCA);

					if (hasAB && hasCA) 
					{
						I1 = I_AB;
						I2 = I_CA;
					}
					else if (hasAB && hasBC) 
					{
						I1 = I_AB;
						I2 = I_BC;
					}
					else 
					{
						I1 = I_BC;
						I2 = I_CA;
					}

					addTriangleWithRestore(model_cut_2, A, I1, I2, n1, n2, n3, calculatedN);

					addTriangleWithRestore(model_cut_1, B, C, I1, n1, n2, n3, calculatedN);
					addTriangleWithRestore(model_cut_1, I1, I2, C, n1, n2, n3, calculatedN);
				}
				else if (pos.size() == 3 && neg.size() == 0)
				{
					innerStructure::Point3d A = pos[0];
					innerStructure::Point3d B = pos[1];
					innerStructure::Point3d C = pos[2];
					addTriangleWithRestore(model_cut_1, A, B, C, n1, n2, n3, calculatedN);
					//addTriangleWithRestore(model_cut_2, A, B, C, n1, n2, n3);
				}
				else if (pos.size() == 0 && neg.size() == 3)
				{
					innerStructure::Point3d A = neg[0];
					innerStructure::Point3d B = neg[1];
					innerStructure::Point3d C = neg[2];
					//addTriangleWithRestore(model_cut_1, A, B, C, n1, n2, n3);
					addTriangleWithRestore(model_cut_2, A, B, C, n1, n2, n3, calculatedN);
				}
			}
		}

		return result;
	}
	// Parses file in .obj format
	// Ignores lines with incorrect format
	void readModel(const std::string& filename)
	{
		// Buffer for each line
		std::string line;
		// File to read
		std::ifstream in(filename);

		if (in.is_open()) 
		{
			// Iterates with reading each line to 'line'
			// Also counting line number
			for (long lineNumber = 1; std::getline(in, line); lineNumber++)
			{
				if (line.empty()) // Ignore empty line
				{
					continue;
				}
				// Object to parse eacj string
				std::stringstream iss(line);
				// Prefix with wich ste string starts
				std::string prefix;

				if (!(iss >> prefix)) // If error while reading part of line
				{
					std::cerr << "Error while parsing prefix at line " << lineNumber << std::endl;
					continue;
				}									
				if (prefix[0] == '#' || prefix == "#") // Ignore comment
				{
					continue;
				}
				else if (prefix == "v") // Parse vertex
				{
					innerStructure::coordinate x, y, z;
					if (iss >> x >> y >> z) // Read 3 numbers and checks read success
					{
						structure.addVertexForce(innerStructure::Point3d{ x,y,z });
					}
					else
					{
						std::cerr << "Error while parsing vertex at line " << lineNumber << std::endl;
						continue;
					}
				}
				else if (prefix == "vn") // Parse normal
				{					
					innerStructure::coordinate x, y, z;
					if (iss >> x >> y >> z)
					{
						structure.addNormalForce(innerStructure::Point3d{ x,y,z });
					}
					else
					{
						std::cerr << "Error while parsing normals at line " << lineNumber << std::endl;
						continue;
					}
				}
				else if (prefix == "f") // Parse triangle
				{					
					// Parse indexes/normals listed in triangle record
					// It reads first float point number
					// then checks for '//' value
					// then reads the second
					// the rest of string is discard
					auto helperParseOctet = [&lineNumber](const std::string & part, innerStructure::vertexId& vertex, innerStructure::normalId& normal) -> bool
					{
						std::stringstream ss(part);
						if (!(ss >> vertex))
							return false;
						char _skip1, _skip2;
						ss >> _skip1 >> _skip2;
						if ((_skip1 != _skip2) && (_skip1 != L'/'))
							return false;
						if (!(ss >> normal))
							return false;
						std::string _check;
						if (ss >> _check)
						{
							std::cerr << "Warning additional info \"" << _check << "\" at face definition ignored line: " << lineNumber << std::endl;
						}
						return true;
					};
					// String to store packs of parameters at face record
					std::string part;
					innerStructure::vertexId vertex_1;
					innerStructure::normalId normal_1;

					innerStructure::vertexId vertex_2;
					innerStructure::normalId normal_2;

					innerStructure::vertexId vertex_3;
					innerStructure::normalId normal_3;
					if (iss >> part)
					{
						if (!helperParseOctet(part, vertex_1, normal_1)) // read 1st parameter pack at face record (then read the rest)
						{
							std::cerr << "Error while parsing triangle, 1st param, inner parse line " << lineNumber << std::endl;
							continue;
						}
					}
					else
					{
						std::cerr << "Error while parsing triangle, 1st param line " << lineNumber << std::endl;
						continue;
					}
					if (iss >> part)
					{
						if (!helperParseOctet(part, vertex_2, normal_2))
						{
							std::cerr << "Error while parsing triangle, 2ndt param, inner parse line " << lineNumber << std::endl;
							continue;
						}
					}
					else
					{
						std::cerr << "Error while parsing triangle, 2nd param line " << lineNumber << std::endl;
						continue;
					}
					if (iss >> part)
					{
						if (!helperParseOctet(part, vertex_3, normal_3))
						{
							std::cerr << "Error while parsing triangle, 3кrd param, inner parse line " << lineNumber << std::endl;
							continue;
						}
					}
					else
					{
						std::cerr << "Error while parsing triangle, 3rd param line " << lineNumber << std::endl;
						continue;
					}

					try 
					{
						structure.addTriangle(innerStructure::Triangle{ vertex_1, vertex_2, vertex_3, normal_1, normal_2, normal_3 });
					}
					catch (...)
					{
						std::cerr << "Error while adding triangle" << std::endl;
						in.close();
						throw;
					}
				}
				else
				{
					std::cerr << "Error while parsing, not known prefix \"" << prefix<< "\" at line " << lineNumber << std::endl;
					continue;
				}
					
			}
		}
		else 
		{
			std::cerr << "Can't open input file" << std::endl;
		}

		in.close();
	}
	// Saves current model to file in .obj format
	void saveModel(const std::string& filename)
	{
		std::ofstream out(filename);

		if (!out.is_open())
		{
			std::cerr << "Can't write to file." << std::endl;
			out.close();
			return;
		}

		// Output commentary
		out << "# Created by test program" << std::endl;

		// Output vertexes
		for (size_t i = 0; i < structure.vertexes.size(); i++)
		{
			out << "v" 
				<< " " << structure.vertexes[i].x
				<< " " << structure.vertexes[i].y
				<< " " << structure.vertexes[i].z << std::endl;
		}
		// Output normals
		for (size_t i = 0; i < structure.normals.size(); i++)
		{
			out << "vn"
				<< " " << structure.normals[i].x
				<< " " << structure.normals[i].y
				<< " " << structure.normals[i].z << std::endl;
		}
		// Output triangles
		for (size_t i = 0; i < structure.triangles.size(); i++)
		{
			out << "f"
				<< " " << structure.triangles[i].vert[0] << "//" << structure.triangles[i].normal[0]
				<< " " << structure.triangles[i].vert[1] << "//" << structure.triangles[i].normal[1]
				<< " " << structure.triangles[i].vert[2] << "//" << structure.triangles[i].normal[2] 
				<< std::endl;
		}
		out.close();
	}
};

// Function for adding postfix at the end of filename
std::string add2ToFilename(const std::string& path, const std::string& postfix) {
	size_t slashPos = path.find_last_of("/\\");   // Where filename starts
	size_t dotPos = path.find_last_of('.');       // where is file extension

	// If not path with slashes
	if (slashPos == std::string::npos) {
		slashPos = 0;
	}

	// if extension not found
	if (dotPos == std::string::npos || dotPos < slashPos) {
		return path + postfix;
	}

	return path.substr(0, dotPos) + postfix + path.substr(dotPos);
}

int main(int argc, char* argv[])
{	
	std::string configPath = "config.txt";
	configPath = "D:\\VMShare\\test-MagmaComputer\\splitter\\tests\\files\\config.txt";
	if (argc == 2)
	{
		configPath = argv[1];
	}
	std::string originalFile = "cube.obj";

	double x1, y1, z1;
	double x2, y2, z2;
	double x3, y3, z3;

	{
		std::ifstream in(configPath);
		if (in.is_open())
		{
			in >> originalFile;
			in >> x1 >> y1 >> z1;
			in >> x2 >> y2 >> z2;
			in >> x3 >> y3 >> z3;
			if (in.fail() || in.bad())
			{
				std::cout << "Error while parsing config" << std::endl;
				return 1;
			}
		}
		else
		{
			std::cout << "Can't open config file." << std::endl;
			return 1;
		}
		in.close();
	}

	std::string saveFileLeft = add2ToFilename(originalFile, "_left");
	std::string saveFileRight = add2ToFilename(originalFile, "_right");
		
	model_ptr m = create_model();
	auto startRead = std::chrono::steady_clock::now();
	try
	{
		m->readModel(originalFile);
	}
	catch (...)
	{
		std::cerr << "Can't read model" << std::endl;
		return 1;
	}
	
	auto endRead = std::chrono::steady_clock::now();

	auto durationRead = std::chrono::duration_cast<std::chrono::milliseconds>(endRead - startRead);

	//m->saveModel(outTestFile);
	auto startSplit = std::chrono::steady_clock::now();
	two_models_ptr models;
	try
	{
		models = m->split(
			x1, y1, z1,	// Vector 1
			x2, y2, z2,		// Vector 2
			x3, y3, z3);		// Vector 3
	}
	catch (...)
	{
		std::cerr << "Can't split model" << std::endl;
		return 1;
	}
	
	auto endSplit = std::chrono::steady_clock::now();
	auto durationSplit = std::chrono::duration_cast<std::chrono::milliseconds>(endSplit - startSplit);

	try
	{
		models.first->saveModel(saveFileLeft);
		models.second->saveModel(saveFileRight);
	}
	catch (...)
	{
		std::cerr << "Can't write models" << std::endl;
		return 1;
	}

	std::cout << "Read time: " << durationRead.count() << std::endl;
	std::cout << "Split time: " << durationSplit.count() << std::endl;

	return 0;
}

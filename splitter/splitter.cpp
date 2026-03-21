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

// Main class of model read/write splitting functionality
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
		// Used for representing 3d point in space
		struct Point3d
		{
			coordinate x;
			coordinate y;
			coordinate z;
		};
		// Used for vertex enumeration
		using vertexId = size_t;
		// Used for normals enumeration
		using normalId = size_t;
		static constexpr const size_t TriangleVertexCount = 3;
		// Used for stroring and saving face
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

		// Adds vertex to DB but only it's value is unique
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
	two_models_ptr split() 
	{
		model_ptr model_cut_1 = create_model();
		model_ptr model_cut_2 = create_model();
		two_models_ptr result;
		result.first = model_cut_1;
		result.second = model_cut_2;

		for (size_t i = 0; i < structure.triangles.size(); i++)
		{
			std::wcout << "Triangle " << i << " of " << structure.triangles.size() << std::endl;
			const innerStructure::Triangle currTriangle = structure.triangles[i];

			// TODO: add try catch
			innerStructure::Point3d p1 = structure.returnVertexById(currTriangle.vert[0]);
			innerStructure::Point3d p2 = structure.returnVertexById(currTriangle.vert[1]);
			innerStructure::Point3d p3 = structure.returnVertexById(currTriangle.vert[2]);
			innerStructure::Point3d n1 = structure.returnNormalById(currTriangle.normal[0]);
			innerStructure::Point3d n2 = structure.returnNormalById(currTriangle.normal[1]);
			innerStructure::Point3d n3 = structure.returnNormalById(currTriangle.normal[2]);

			double a = -5;
			double b = 3;
			double c = 2;
			double d = 1;

			auto getDistance = [a,b,c,d](innerStructure::Point3d p) -> innerStructure::coordinate
				{
					return a * p.x + b * p.y + c * p.z + d;
				};

			innerStructure::lenght S1 = getDistance(p1);
			innerStructure::lenght S2 = getDistance(p2);
			innerStructure::lenght S3 = getDistance(p3);

			if ((std::signbit(S1) == std::signbit(S2)) && (std::signbit(S2) == std::signbit(S3)))
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
				innerStructure::vertexId p1_new = currModel->structure.addVertexForce(p1);
				innerStructure::vertexId p2_new = currModel->structure.addVertexForce(p2);
				innerStructure::vertexId p3_new = currModel->structure.addVertexForce(p3);

				innerStructure::normalId n1_new = currModel->structure.addNormalForce(n1);
				innerStructure::normalId n2_new = currModel->structure.addNormalForce(n2);
				innerStructure::normalId n3_new = currModel->structure.addNormalForce(n3);

				currModel->structure.addTriangle(innerStructure::Triangle{ p1_new, p2_new, p3_new, n1_new, n2_new, n3_new });
			}			
		}

		return result;
	}
	// Parses file in .obj format
	// Ignores lines with incorrect format
	void readModel(const std::wstring& filename)
	{
		// Buffer for each line
		std::wstring line;
		// File to read
		std::wifstream in(filename);

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
				std::wistringstream iss(line);
				// Prefix with wich ste string starts
				std::wstring prefix;

				if (!(iss >> prefix)) // If error while reading part of line
				{
					std::wcerr << L"Error while parsing prefix at line " << lineNumber << std::endl;
					continue;
				}
					
				if (prefix == L"#") // Ignore comment
				{
					continue;
				}
				else if (prefix == L"v") // Parse vertex
				{
					innerStructure::coordinate x, y, z;
					if (iss >> x >> y >> z) // Read 3 numbers and checks read success
					{
						structure.addVertexForce(innerStructure::Point3d{ x,y,z });
					}
					else
					{
						std::wcerr << L"Error while parsing vertex at line " << lineNumber << std::endl;
						continue;
					}
				}
				else if (prefix == L"vn") // Parse normal
				{					
					innerStructure::coordinate x, y, z;
					if (iss >> x >> y >> z)
					{
						structure.addNormalForce(innerStructure::Point3d{ x,y,z });
					}
					else
					{
						std::wcerr << L"Error while parsing normals at line " << lineNumber << std::endl;
						continue;
					}
				}
				else if (prefix == L"f") // Parse triangle
				{					
					// Parse indexes/normals listed in triangle record
					// It reads first float point number
					// then checks for '//' value
					// then reads the second
					// the rest of string is discard
					auto helperParseOctet = [](const std::wstring & part, innerStructure::vertexId& vertex, innerStructure::normalId& normal) -> bool
					{
						std::wstringstream ss(part);
						if (!(ss >> vertex))
							return false;
						wchar_t _skip1, _skip2;
						ss >> _skip1 >> _skip2;
						if ((_skip1 != _skip2) && (_skip1 != L'/'))
							return false;
						if (!(ss >> normal))
							return false;
						return true;
					};
					// String to store packs of parameters at face record
					std::wstring part;
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
							std::wcerr << L"Error while parsing triangle, 1st param, inner parse line " << lineNumber << std::endl;
							continue;
						}
					}
					else
					{
						std::wcerr << L"Error while parsing triangle, 1st param line " << lineNumber << std::endl;
						continue;
					}
					if (iss >> part)
					{
						if (!helperParseOctet(part, vertex_2, normal_2))
						{
							std::wcerr << L"Error while parsing triangle, 2ndt param, inner parse line " << lineNumber << std::endl;
							continue;
						}
					}
					else
					{
						std::wcerr << L"Error while parsing triangle, 2nd param line " << lineNumber << std::endl;
						continue;
					}
					if (iss >> part)
					{
						if (!helperParseOctet(part, vertex_3, normal_3))
						{
							std::wcerr << L"Error while parsing triangle, 3кrd param, inner parse line " << lineNumber << std::endl;
							continue;
						}
					}
					else
					{
						std::wcerr << L"Error while parsing triangle, 3rd param line " << lineNumber << std::endl;
						continue;
					}

					try 
					{
						structure.addTriangle(innerStructure::Triangle{ vertex_1, vertex_2, vertex_3, normal_1, normal_2, normal_3 });
					}
					catch (...)
					{
						std::wcerr << L"Error while adding triangle" << std::endl;
						in.close();
						throw;
					}
				}
				else
				{
					std::wcerr << L"Error while parsing, not known prefix \"" << prefix<< "\" at line " << lineNumber << std::endl;
					continue;
				}
					
			}
		}
		else 
		{
			std::wcerr << L"Can't open input file" << std::endl;
		}

		in.close();
	}
	// Saves current model to file in .obj format
	void saveModel(const std::wstring& filename)
	{
		std::wofstream out(filename);

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
	}
};
int main()
{
	std::wstring originalFile = L"D:\\VMShare\\test-MagmaComputer\\splitter\\tests\\files\\cube.obj"; //L"/mnt/VMShare/test-MagmaComputer/splitter/tests/files/bulb.obj"
	std::wstring outTestFile = L"D:\\VMShare\\test-MagmaComputer\\splitter\\tests\\files\\cube_saved.obj";
	std::wstring saveFileLeft = L"D:\\VMShare\\test-MagmaComputer\\splitter\\tests\\files\\cube_left.obj"; // L"/mnt/VMShare/test-MagmaComputer/splitter/tests/files/bulb_left.obj"
	std::wstring saveFileRight = L"D:\\VMShare\\test-MagmaComputer\\splitter\\tests\\files\\cube_right.obj";
	
	model_ptr m = create_model();
	m->readModel(originalFile);
	//m->saveModel(outTestFile);
	two_models_ptr models = m->split();

	models.first->saveModel(saveFileLeft);
	models.second->saveModel(saveFileRight);

	return 0;
}

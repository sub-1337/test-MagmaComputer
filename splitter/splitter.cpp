#include "splitter.h"

class model;
using model_ptr = std::shared_ptr<model>;
template <typename... Args>
model_ptr create_model(Args... args)
{
	return std::make_shared<model>(args...);
}
using two_models_ptr = std::pair<std::shared_ptr<model>, std::shared_ptr<model>>;

template <typename T>
bool is_equal(T a, T b, T epsilon = 1e-9)
{
	return std::abs(a - b) < epsilon;
}

class model
{
private:
	struct innerStructure
	{
	public:
		struct Point3d
		{
			double x;
			double y;
			double z;
		};
		using vertexId = size_t;
		static constexpr const size_t TriangleVertexCount = 3;
		struct Triangle
		{
			vertexId vert[TriangleVertexCount];
		};
	private:
		std::vector<Point3d> vertexes;
		std::vector<Triangle> triangles;
		//std::vector<Point3d> normals;

	public:
		static bool is_points_same(Point3d a, Point3d b)
		{
			return is_equal(a.x, b.x) && is_equal(a.y, b.y) && is_equal(a.z, b.z);		
		}

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
		vertexId addVertexForce(Point3d value) // used at parsing
		{
			vertexes.push_back(value);
			return vertexes.size() + 1 - 1; // vertexes starts from 1
		}
		void addTriangle(Triangle value)
		{
			triangles.push_back(value);
		}
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
	};
	innerStructure structure;
public:
	two_models_ptr split() 
	{
		model_ptr model_cut_1 = create_model();
		model_ptr model_cut_2 = create_model();
		two_models_ptr result;
		result.first = model_cut_1;
		result.second = model_cut_2;
		return result;
	}
	
	void readModel(const std::wstring& filename)
	{
		std::wstring line;
		std::wifstream in(filename);

		if (in.is_open()) 
		{
			for (long lineNumber = 1; std::getline(in, line); lineNumber++)
			{
				if (line.empty()) // Ignore empty line
				{
					continue;
				}
				std::wistringstream iss(line);
				std::wstring prefix;
				if (!(iss >> prefix))
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
					double x, y, z;
					if (iss >> x >> y >> z)
					{
						structure.addVertexForce(innerStructure::Point3d{ x,y,z });
					}
					else
					{
						std::wcerr << L"Error while parsing numbers at line " << lineNumber << std::endl;
						continue;
					}
				}
				else if (prefix == L"vn")
				{
					std::wcerr << L"Error while parsing prefix, \"vn\" normals not supported at line " << lineNumber << std::endl;
				}
				else if (prefix == L"f")
				{					
					// Parse slashes at triangle definition
					// reads first long number from string (before slashes)
					auto helperParseOctet = [](const std::wstring & part, innerStructure::vertexId & vertex) -> bool 
					{
						std::wcout << part << std::endl;
						innerStructure::vertexId value = 1;
						std::wstring numbersString;
						innerStructure::vertexId result;
						for (size_t i = 0; i < part.size(); i++)
						{
							wchar_t curr = part[i];
							if (iswdigit(curr))
							{
								numbersString += curr;
							}
							else
							{
								break;
							}								
						}
						try
						{
							result = std::stol(numbersString);
							vertex = result;
							return true;
						}
						catch (...)
						{
							return false;
						}
					};
					std::wstring part;
					innerStructure::vertexId vertex_1;
					innerStructure::vertexId vertex_2;
					innerStructure::vertexId vertex_3;
					if (iss >> part)
					{
						if (!helperParseOctet(part, vertex_1))
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
						if (!helperParseOctet(part, vertex_2))
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
						if (!helperParseOctet(part, vertex_3))
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
	void saveModel(const std::wstring& filename)
	{

	}
};
int main()
{
	std::wstring originalFile = L"D:\\VMShare\\test-MagmaComputer\\splitter\\tests\\files\\surface.obj"; //L"/mnt/VMShare/test-MagmaComputer/splitter/tests/files/bulb.obj"
	std::wstring saveFileLeft = L"D:\\VMShare\\test - MagmaComputer\\splitter\\tests\\files\\surface_left.obj"; // L"/mnt/VMShare/test-MagmaComputer/splitter/tests/files/bulb_left.obj"
	std::wstring saveFileRight = L"D:\\VMShare\\test - MagmaComputer\\splitter\\tests\\files\\surface_right.obj";
	
	model_ptr m = create_model();
	m->readModel(originalFile);
	two_models_ptr models = m->split();

	models.first->saveModel(saveFileLeft);
	models.second->saveModel(saveFileRight);

	return 0;
}

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
		struct Point3d
		{
			double x;
			double y;
			double z;
		};
		using vertexId = size_t;
		std::vector<Point3d> vertexes;
		std::vector<Point3d> normals;
		
		static bool is_points_same(Point3d a, Point3d b)
		{
			return is_equal(a.x, b.x) && is_equal(a.y, b.y) && is_equal(a.z, b.z);		
		}

		vertexId addVertexIfNotExist(Point3d value)
		{
			for (vertexId i = 0; i < vertexes.size(); i++)
			{
				Point3d curr = vertexes[i];
				if (is_points_same(value, curr))
				{
					return i;
				}
			}
			vertexes.push_back(value);
			return vertexes.size() - 1;
		}
		vertexId addVertexForce(Point3d value)
		{
			vertexes.push_back(value);
			return vertexes.size() - 1;
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
		std::string line;
		std::ifstream in(filename);

		if (in.is_open()) 
		{
			for (long lineNumber = 0; std::getline(in, line); lineNumber++)
			{
				if (line.size() > 0)
				{
					if (line[0] == '#') // If we met comment
					{
						continue;
					}
					if (line[0] == 'v' && (line.size() >=2 && line[1] != 'n')) // If we met vertex clause not normal
					{
						try
						{
							double x = std::stod(line);
							double y = std::stod(line);
							double z = std::stod(line);
						}
						catch (const std::exception& e)
						{
							std::cerr << "Read failed on line " << lineNumber << ": " << e.what() << std::endl;
						}
						//structure.addVertexForce(innerStructure::Point3d{0,10,0});
					}
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

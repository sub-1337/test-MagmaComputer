#include "splitter.h"

class model;
using model_ptr = std::shared_ptr<model>;
template <typename... Args>
model_ptr create_model(Args... args)
{
	return std::make_shared<model>(args...);
}
using two_models_ptr = std::pair<std::shared_ptr<model>, std::shared_ptr<model>>;

class model
{
public:
	two_models_ptr split() 
	{
		model_ptr model_cutted_1 = create_model();
		model_ptr model_cutted_2 = create_model();
		two_models_ptr result;
		result.first = model_cutted_1;
		result.second = model_cutted_2;
		return result;
	}
	void readModel(const std::wstring& filename)
	{

	}
	void saveModel(const std::wstring& filename)
	{

	}
};
int main()
{
	std::wstring originalFile = L"D:\\VMShare\\test - MagmaComputer\\splitter\\tests\\files\\bulb.obj"; //L"/mnt/VMShare/test-MagmaComputer/splitter/tests/files/bulb.obj"
	std::wstring saveFileLeft = L"D:\\VMShare\\test - MagmaComputer\\splitter\\tests\\files\\bulb_left.obj"; // L"/mnt/VMShare/test-MagmaComputer/splitter/tests/files/bulb_left.obj"
	std::wstring saveFileRight = L"D:\\VMShare\\test - MagmaComputer\\splitter\\tests\\files\\bulb_right.obj";
	
	model_ptr m = create_model();
	m->readModel(originalFile);
	two_models_ptr models = m->split();

	models.first->saveModel(saveFileLeft);
	models.second->saveModel(saveFileRight);

	return 0;
}

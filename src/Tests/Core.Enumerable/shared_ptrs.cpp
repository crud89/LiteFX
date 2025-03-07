#include "common.h"

class IFactory {
public:
	virtual ~IFactory() noexcept = default;
	virtual Enumerable<std::shared_ptr<const Base>> bases() const = 0;
};

class Factory final : public IFactory {
private:
	std::vector<std::shared_ptr<Bar>> m_bars;

public:
	explicit Factory(int n)
	{
		for (int i{ 0 }; i < n; ++i)
			m_bars.emplace_back(std::make_shared<Bar>(i));
	}

	~Factory() noexcept override = default;

	Enumerable<const std::shared_ptr<const Bar>> bars() const {
		return m_bars;
	}

	Enumerable<std::shared_ptr<const Base>> bases() const override {
		return m_bars;
	}
};

int main(int /*argc*/, char* /*argv*/[])
{
	Factory factory(4);

	for (int i{ 0 }; auto bar : factory.bars())
		if (bar->index() != i++)
			return -1;

	for (int i{ 0 }; auto base : factory.bases())
		if (base->index() != i++)
			return -2;

	for (int i{ 0 }; auto bar : factory.bases() | std::views::transform([](auto base) { return std::dynamic_pointer_cast<const Bar>(base); }))
		if (bar->index() != i++)
			return -3;

	return 0;
}
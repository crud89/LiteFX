#include <litefx/app.hpp>

using namespace LiteFX;

const IBackend* AppBuilder::findBackend(const BackendType& type) const noexcept 
{ 
	return this->instance()->operator[](type);
}

void AppBuilder::use(UniquePtr<IBackend>&& backend)
{
	this->instance()->use(std::move(backend));
}

UniquePtr<App> AppBuilder::go()
{
	this->instance()->run();
	return builder_type::go();
}
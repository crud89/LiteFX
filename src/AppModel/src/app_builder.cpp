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
	Logger::get(this->instance()->getName()).info("Starting app (Version {1}) on platform {0}...", this->instance()->getPlatform(), this->instance()->getVersion());
	Logger::get(this->instance()->getName()).debug("Using engine: {0:e}.", this->instance()->getVersion());

	this->instance()->run();
	return builder_type::go();
}